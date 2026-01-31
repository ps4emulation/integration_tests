#include "nbio_stream_logger.h"

#include "test.h"

static constexpr u16          log_port     = 8181;
static constexpr s32          max_clients  = 10;
static bool                   logger_ready = false;
static std::mutex             logger_mutex {};
static pthread_t              logger_server_tid {};
static std::vector<pthread_t> client_tids {};
static bool                   async_logging = false;

struct ClientArgs {
  const char* fmt;
  const u64   log_res;
};

void NBIOStreamLogger::LogMessage(const char* fmt, const u64 log_res) {
  // Creates a thread that performs the logging
  std::scoped_lock<std::mutex> lk {logger_mutex};

  pthread_t      client_tid {};
  pthread_attr_t thread_attr {};
  s32            result = pthread_attr_init(&thread_attr);
  if (result < 0) {
    printf("NBIOStreamLogger unable to send message, pthread_attr_init failed with 0x%08x\n", result);
    return;
  }

  // For async logging, we need to limit possible client threads.
  // To do this, just exit all threads any time the client_tids vector gets too large.
  if (async_logging && client_tids.size() == max_clients) {
    for (pthread_t client_tid: client_tids) {
      result = pthread_join(client_tid, nullptr);
      if (result < 0) {
        printf("NBIOStreamLogger unable to terminate\n");
      }
    }

    // Clear old client tids
    client_tids.clear();
  }

  // For the user args, submit both arguments through a struct
  ClientArgs user_args {fmt, log_res};
  result = pthread_create(&client_tid, &thread_attr, LoggingClientThread, &user_args);
  if (result < 0) {
    printf("NBIOStreamLogger unable to send message, pthread_create failed with 0x%08x\n", result);
    return;
  }

  char thread_name[128];
  memset(thread_name, 0, sizeof(thread_name));
  sprintf(thread_name, "LoggerThread0x%08lx", client_tid);
  pthread_set_name_np(client_tid, thread_name);

  if (async_logging) {
    // Add new client thread id to vector, to be exited on-demand or in destructor
    client_tids.emplace_back(client_tid);
  } else {
    // Wait for logger thread to terminate
    result = pthread_join(client_tid, nullptr);
    if (result < 0) {
      printf("LogMessage thread failed to terminate\n");
    }
  }
}

void* NBIOStreamLogger::LoggingServerThread(void* user_arg) {
  // To continue in the lovely steps of this test suite, this logger server will be built on socket communication.
  s32 stream_sock = sceNetSocket("LoggerServer", ORBIS_NET_AF_INET, ORBIS_NET_SOCK_STREAM, 0);
  if (stream_sock <= 0) {
    printf("NBIOStreamLogger unable to initialize, socket creation failed with 0x%08x\n", stream_sock);
    return nullptr;
  }

  // Use setsockopt to ensure it uses specified addr and port
  s32 opt    = 1;
  s32 result = sceNetSetsockopt(stream_sock, ORBIS_NET_SOL_SOCKET, ORBIS_NET_SO_REUSEADDR, &opt, sizeof(opt));
  if (result < 0) {
    printf("NBIOStreamLogger unable to initialize, setsockopt failed with 0x%08x\n", result);
    sceNetSocketClose(stream_sock);
    return nullptr;
  }
  opt    = 1;
  result = sceNetSetsockopt(stream_sock, ORBIS_NET_SOL_SOCKET, ORBIS_NET_SO_REUSEPORT, &opt, sizeof(opt));
  if (result < 0) {
    printf("NBIOStreamLogger unable to initialize, setsockopt failed with 0x%08x\n", result);
    sceNetSocketClose(stream_sock);
    return nullptr;
  }

  // Bind this socket to a specific address and port
  OrbisNetSockaddrIn in_addr {};
  in_addr.sin_family      = ORBIS_NET_AF_INET;
  in_addr.sin_addr.s_addr = ORBIS_NET_INADDR_ANY;
  in_addr.sin_port        = sceNetHtons(log_port);
  result                  = sceNetBind(stream_sock, (OrbisNetSockaddr*)&in_addr, sizeof(in_addr));
  if (result < 0) {
    printf("NBIOStreamLogger unable to initialize, bind failed with 0x%08x\n", result);
    sceNetSocketClose(stream_sock);
    return nullptr;
  }

  // Set to non-blocking
  opt    = 1;
  result = sceNetSetsockopt(stream_sock, ORBIS_NET_SOL_SOCKET, ORBIS_NET_SO_NBIO, &opt, sizeof(opt));
  if (result < 0) {
    printf("NBIOStreamLogger unable to initialize, setsockopt failed with 0x%08x\n", result);
    sceNetSocketClose(stream_sock);
    return nullptr;
  }

  // Mark socket as passive
  result = sceNetListen(stream_sock, max_clients);
  if (result < 0) {
    printf("NBIOStreamLogger unable to initialize, listen failed with 0x%08x\n", result);
    sceNetSocketClose(stream_sock);
    return nullptr;
  }

  // Socket is ready, mark server as initialized.
  logger_ready = true;

  // For the remainder of this test, this thread will run a loop.
  while (logger_ready) {
    // Use accept to wait for, then connect with a client
    u32 addr_len = sizeof(in_addr);
    s32 log_sock = sceNetAccept(stream_sock, (OrbisNetSockaddr*)&in_addr, &addr_len);
    if (!logger_ready) {
      // Logger terminated
      if (log_sock > 0) {
        // Accept succeeded after logging terminated, abort anyway.
        sceNetSocketClose(log_sock);
      }
      break;
    } else if (log_sock == ORBIS_NET_ERROR_EAGAIN) {
      // Not ready for client, wait then loop.
      sceKernelUsleep(10000);
      continue;
    } else if (log_sock <= 0) {
      // Log errors
      printf("NBIOStreamLogger: Unexpected error 0x%08x while waiting for client\n", log_sock);
      continue;
    }

    // Create an epoll
    s32 log_epoll = sceNetEpollCreate("LoggerMessageEpoll", 0);
    if (log_epoll <= 0) {
      // Log errors
      printf("NBIOStreamLogger: Unexpected error 0x%08x while creating epoll\n", log_epoll);
      sceNetSocketClose(log_sock);
      continue;
    }

    // Add the new socket to the epoll
    OrbisNetEpollEvent epoll_event {};
    epoll_event.events = ORBIS_NET_EPOLLIN;
    OrbisNetEpollData epoll_data {};
    epoll_data.fd    = log_sock;
    epoll_event.data = epoll_data;
    result           = sceNetEpollControl(log_epoll, ORBIS_NET_EPOLL_CTL_ADD, log_sock, &epoll_event);
    if (result < 0) {
      // Log errors
      printf("NBIOStreamLogger: Unexpected error 0x%08x while preparing epoll\n", result);
      sceNetEpollDestroy(log_epoll);
      sceNetSocketClose(log_sock);
      continue;
    }

    // Perform an epoll_wait with indefinite timeout
    OrbisNetEpollEvent epoll_ev_out {};
    result = sceNetEpollWait(log_epoll, &epoll_ev_out, 1, -1);
    if (result < 0) {
      // Log errors
      printf("NBIOStreamLogger: Unexpected error 0x%08x while waiting for socket\n", result);
      sceNetEpollDestroy(log_epoll);
      sceNetSocketClose(log_sock);
      continue;
    } else if (result == 0) {
      // Log errors
      printf("NBIOStreamLogger: Blocking epoll wait returned no ready fds?\n");
      sceNetEpollDestroy(log_epoll);
      sceNetSocketClose(log_sock);
      continue;
    }

    // Read in the message from the socket
    char log_buf[0x1000];
    do {
      memset(log_buf, 0, sizeof(log_buf));
      result = sceNetRecv(log_sock, log_buf, sizeof(log_buf), 0);
      if (result == ORBIS_NET_ERROR_EWOULDBLOCK || result == 0) {
        // No more data to read
        break;
      } else if (result < 0) {
        // Log errors
        printf("NBIOStreamLogger: Unexpected error 0x%08x while reading from socket\n", result);
        sceNetEpollDestroy(log_epoll);
        sceNetSocketClose(log_sock);
        continue;
      }
      printf("%s", log_buf);
    } while (result > 0);

    // Destroy the epoll and close the connection socket
    result = sceNetEpollDestroy(log_epoll);
    if (result < 0) {
      printf("NBIOStreamLogger: Failed to destroy epoll, error 0x%08x\n", result);
    }
    result = sceNetSocketClose(log_sock);
    if (result < 0) {
      printf("NBIOStreamLogger: Failed to close connection, error 0x%08x\n", result);
    }
  }
  result = sceNetSocketClose(stream_sock);
  if (result < 0) {
    printf("NBIOStreamLogger: Failed to close server socket, error 0x%08x\n", result);
  }
  return nullptr;
}

void* NBIOStreamLogger::LoggingClientThread(void* user_arg) {
  // user_arg here is the ClientArgs struct, unpack args before continuing
  ClientArgs* argp    = (ClientArgs*)user_arg;
  const char* fmt     = argp->fmt;
  const u64   log_res = argp->log_res;

  // Because I want to make everything extraordinarily difficult, this creates a socket, connects to logging socket, sends message, closes socket.
  s32 stream_sock = sceNetSocket("LoggerClientSocket", ORBIS_NET_AF_INET, ORBIS_NET_SOCK_STREAM, 0);
  if (stream_sock <= 0) {
    printf("NBIOStreamLogger: Failed to create client socket, error 0x%08x\n", stream_sock);
    return nullptr;
  }

  OrbisNetSockaddrIn in_addr {};
  in_addr.sin_family = ORBIS_NET_AF_INET;
  in_addr.sin_port   = sceNetHtons(log_port);
  s32 result         = sceNetInetPton(ORBIS_NET_AF_INET, "127.0.0.1", &in_addr.sin_addr);
  if (result < 1) {
    printf("NBIOStreamLogger: Failed to retrieve loopback address, error 0x%08x\n", result);
    sceNetSocketClose(stream_sock);
    return nullptr;
  }

  // Set to non-blocking
  s32 opt = 1;
  result  = sceNetSetsockopt(stream_sock, ORBIS_NET_SOL_SOCKET, ORBIS_NET_SO_NBIO, &opt, sizeof(opt));
  if (result < 0) {
    printf("NBIOStreamLogger: Failed to mark client socket as non-blocking, error 0x%08x\n", result);
    sceNetSocketClose(stream_sock);
    return nullptr;
  }

  result = sceNetConnect(stream_sock, (OrbisNetSockaddr*)&in_addr, sizeof(in_addr));
  while (result != 0) {
    if (!logger_ready) {
      // Logger was aborted while trying to log message
      sceNetSocketClose(stream_sock);
      return nullptr;
    }
    if (result == ORBIS_NET_ERROR_EINPROGRESS) {
      // The connection could not be completed. Wait, then retry the connection.
      sceKernelUsleep(100000);
      result = sceNetConnect(stream_sock, (OrbisNetSockaddr*)&in_addr, sizeof(in_addr));
      continue;
    } else if (result == ORBIS_NET_ERROR_EISCONN) {
      // Connection finished, break out of loop.
      break;
    } else {
      printf("NBIOStreamLogger: Unexpected error while trying to connect to server, error 0x%08x\n", result);
      sceNetSocketClose(stream_sock);
      return nullptr;
    }
  }

  // Create an epoll, use it to wait until socket is ready for writing.
  s32 log_epoll = sceNetEpollCreate("LoggerMessageEpoll", 0);
  if (log_epoll <= 0) {
    printf("NBIOStreamLogger: Unexpected error while creating epoll, error 0x%08x\n", log_epoll);
    sceNetSocketClose(stream_sock);
    return nullptr;
  }

  // Add the new socket to the epoll
  OrbisNetEpollEvent epoll_event {};
  epoll_event.events = ORBIS_NET_EPOLLOUT;
  OrbisNetEpollData epoll_data {};
  epoll_data.fd    = stream_sock;
  epoll_event.data = epoll_data;
  result           = sceNetEpollControl(log_epoll, ORBIS_NET_EPOLL_CTL_ADD, stream_sock, &epoll_event);
  if (result < 0) {
    printf("NBIOStreamLogger: Unexpected error while preparing epoll, error 0x%08x\n", result);
    sceNetEpollDestroy(log_epoll);
    sceNetSocketClose(stream_sock);
    return nullptr;
  }

  // Perform an epoll_wait with indefinite timeout
  OrbisNetEpollEvent epoll_ev_out {};
  result = sceNetEpollWait(log_epoll, &epoll_ev_out, 1, -1);
  if (result < 0) {
    printf("NBIOStreamLogger: Unexpected error while waiting for socket, error 0x%08x\n", result);
    sceNetEpollDestroy(log_epoll);
    sceNetSocketClose(stream_sock);
    return nullptr;
  } else if (result == 0) {
    printf("NBIOStreamLogger: Blocking epoll wait returned no ready fds?\n");
    sceNetEpollDestroy(log_epoll);
    sceNetSocketClose(stream_sock);
    return nullptr;
  }

  char send_buf[0x1000];
  memset(send_buf, 0, sizeof(send_buf));
  sprintf(send_buf, fmt, log_res);
  result = sceNetSend(stream_sock, send_buf, strlen(send_buf), 0);
  if (result < 0) {
    printf("NBIOStreamLogger: Unexpected error while sending data to logger server, error 0x%08x\n", result);
  } else if (result == 0) {
    printf("NBIOStreamLogger: Failed to send data to logging server?\n");
  }

  result = sceNetEpollDestroy(log_epoll);
  if (result < 0) {
    printf("NBIOStreamLogger: Failed to destroy logger epoll, error 0x%08x\n", result);
  }

  result = sceNetSocketClose(stream_sock);
  if (result < 0) {
    printf("NBIOStreamLogger: Failed to close logger socket, error 0x%08x\n", result);
  }
  return nullptr;
}

NBIOStreamLogger::NBIOStreamLogger(bool async) {
  std::scoped_lock lk {logger_mutex};
  // Initialize global "logger ready" value to false
  logger_ready  = false;
  async_logging = async;

  // Create non-blocking stream socket logger thread.
  pthread_attr_t thread_attr {};
  s32            result = pthread_attr_init(&thread_attr);
  if (result < 0) {
    printf("NBIOStreamLogger unable to initialize\n");
    return;
  }
  result = pthread_create(&logger_server_tid, &thread_attr, LoggingServerThread, nullptr);
  if (result < 0) {
    printf("NBIOStreamLogger unable to initialize\n");
    return;
  }

  pthread_set_name_np(logger_server_tid, "LoggerServerThread");

  // Block until logger thread finishes initializing
  while (!logger_ready) {
    sceKernelUsleep(10000);
  }

  printf("NBIOStreamLogger initialized successfully\n");
}

NBIOStreamLogger::~NBIOStreamLogger() {
  std::scoped_lock lk {logger_mutex};
  // Now go through and join with all child threads
  for (pthread_t client_tid: client_tids) {
    s32 result = pthread_join(client_tid, nullptr);
    if (result < 0) {
      printf("NBIOStreamLogger unable to terminate\n");
    }
  }

  // Terminate logging thread.
  // Since this class uses non-blocking sockets, we can just mark logger_ready as false, then wait for the thread to exit.
  logger_ready = false;
  s32 result   = pthread_join(logger_server_tid, nullptr);
  if (result < 0) {
    printf("NBIOStreamLogger unable to terminate\n");
  }

  // Clear client_tids vector
  client_tids.clear();
  // Manually destruct vector, as memory leaks otherwise.
  client_tids.~vector();

  printf("NBIOStreamLogger terminated successfully\n");
}