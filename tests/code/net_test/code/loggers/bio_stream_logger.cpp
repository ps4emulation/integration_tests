#include "bio_stream_logger.h"

static constexpr u16 log_port     = 8181;
static bool          logger_ready = false;
static std::mutex    logger_mutex {};
static pthread_t     logger_server_tid {};

struct ClientArgs {
  const char* fmt;
  const u64   log_res;
};

void BIOStreamLogger::LogMessage(const char* fmt, const u64 log_res) {
  // Creates a thread that performs the logging
  std::scoped_lock<std::mutex> lk {logger_mutex};

  pthread_t      client_tid {};
  pthread_attr_t thread_attr {};
  s32            result = pthread_attr_init(&thread_attr);
  if (result < 0) {
    printf("BIOStreamLogger unable to send message, pthread_attr_init failed with 0x%08x\n", result);
    return;
  }

  // For the user args, submit both arguments through a struct
  ClientArgs user_args {fmt, log_res};
  result = pthread_create(&client_tid, &thread_attr, LoggingClientThread, &user_args);
  if (result < 0) {
    printf("BIOStreamLogger unable to send message, pthread_create failed with 0x%08x\n", result);
    return;
  }

  char thread_name[128];
  memset(thread_name, 0, sizeof(thread_name));
  sprintf(thread_name, "BIOStreamLoggerClientThread0x%08lx", client_tid);
  pthread_set_name_np(client_tid, thread_name);

  // Wait for logger thread to terminate
  result = pthread_join(client_tid, nullptr);
  if (result < 0) {
    printf("LogMessage thread failed to terminate\n");
  }
}

void* BIOStreamLogger::LoggingServerThread(void* user_arg) {
  // To continue in the lovely steps of this test suite, this logger server will be built on socket communication.
  s32 stream_sock = sceNetSocket("BIOStreamLoggerServerSocket", ORBIS_NET_AF_INET, ORBIS_NET_SOCK_STREAM, 0);
  if (stream_sock <= 0) {
    printf("BIOStreamLogger unable to initialize, socket creation failed with 0x%08x\n", stream_sock);
    return nullptr;
  }

  // Use setsockopt to ensure it uses specified addr and port
  s32 opt    = 1;
  s32 result = sceNetSetsockopt(stream_sock, ORBIS_NET_SOL_SOCKET, ORBIS_NET_SO_REUSEADDR, &opt, sizeof(opt));
  if (result < 0) {
    printf("BIOStreamLogger unable to initialize, setsockopt failed with 0x%08x\n", result);
    sceNetSocketClose(stream_sock);
    return nullptr;
  }
  opt    = 1;
  result = sceNetSetsockopt(stream_sock, ORBIS_NET_SOL_SOCKET, ORBIS_NET_SO_REUSEPORT, &opt, sizeof(opt));
  if (result < 0) {
    printf("BIOStreamLogger unable to initialize, setsockopt failed with 0x%08x\n", result);
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
    printf("BIOStreamLogger unable to initialize, bind failed with 0x%08x\n", result);
    sceNetSocketClose(stream_sock);
    return nullptr;
  }

  // Mark socket as passive. There will only ever be one client at a time.
  result = sceNetListen(stream_sock, 1);
  if (result < 0) {
    printf("BIOStreamLogger unable to initialize, listen failed with 0x%08x\n", result);
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
    } else if (log_sock <= 0) {
      // Log errors
      printf("BIOStreamLogger: Unexpected error 0x%08x while waiting for client\n", log_sock);
      continue;
    }

    // Read in the message from the socket
    // Since this logger uses blocking socets, all sends should be complete data, so this will retrieve full packets.
    char log_buf[0x1000];
    memset(log_buf, 0, sizeof(log_buf));
    result = sceNetRecv(log_sock, log_buf, sizeof(log_buf), 0);
    if (result <= 0) {
      // Log errors
      printf("BIOStreamLogger: Unexpected error 0x%08x while reading from socket\n", result);
      sceNetSocketClose(log_sock);
      continue;
    }
    printf("%s", log_buf);

    // Close the connection socket
    result = sceNetSocketClose(log_sock);
    if (result < 0) {
      printf("BIOStreamLogger: Failed to close connection, error 0x%08x\n", result);
    }
  }

  // Close the server socket
  result = sceNetSocketClose(stream_sock);
  if (result < 0) {
    printf("BIOStreamLogger: Failed to close server socket, error 0x%08x\n", result);
  }
  return nullptr;
}

void* BIOStreamLogger::LoggingClientThread(void* user_arg) {
  // user_arg here is the ClientArgs struct, unpack args before continuing
  ClientArgs* argp    = (ClientArgs*)user_arg;
  const char* fmt     = argp->fmt;
  const u64   log_res = argp->log_res;

  // Create a socket for the connection
  s32 stream_sock = sceNetSocket("BIOStreamLoggerClientSocket", ORBIS_NET_AF_INET, ORBIS_NET_SOCK_STREAM, 0);
  if (stream_sock <= 0) {
    printf("BIOStreamLogger: Failed to create client socket, error 0x%08x\n", stream_sock);
    return nullptr;
  }

  // Get the loopback address for the connection
  OrbisNetSockaddrIn in_addr {};
  in_addr.sin_family = ORBIS_NET_AF_INET;
  in_addr.sin_port   = sceNetHtons(log_port);
  s32 result         = sceNetInetPton(ORBIS_NET_AF_INET, "127.0.0.1", &in_addr.sin_addr);
  if (result < 1) {
    printf("BIOStreamLogger: Failed to retrieve loopback address, error 0x%08x\n", result);
    sceNetSocketClose(stream_sock);
    return nullptr;
  }

  // Connect to the server
  result = sceNetConnect(stream_sock, (OrbisNetSockaddr*)&in_addr, sizeof(in_addr));
  if (!logger_ready) {
    // Logging was disabled, abort here.
    sceNetSocketClose(stream_sock);
    return nullptr;
  } else if (result < 0) {
    // Non-blocking socket connect should block until success, unless a server error occurs
    printf("BIOStreamLogger: Unexpected error while trying to connect to server, error 0x%08x\n", result);
    sceNetSocketClose(stream_sock);
    return nullptr;
  }

  char send_buf[0x1000];
  memset(send_buf, 0, sizeof(send_buf));
  sprintf(send_buf, fmt, log_res);
  result = sceNetSend(stream_sock, send_buf, strlen(send_buf), 0);
  if (result <= 0) {
    printf("BIOStreamLogger: Unexpected error while sending data to logger server, error 0x%08x\n", result);
  }

  result = sceNetSocketClose(stream_sock);
  if (result < 0) {
    printf("BIOStreamLogger: Failed to close logger socket, error 0x%08x\n", result);
  }
  return nullptr;
}

BIOStreamLogger::BIOStreamLogger() {
  std::scoped_lock lk {logger_mutex};
  // Initialize global "logger ready" value to false
  logger_ready = false;

  // Create non-blocking stream socket logger thread.
  pthread_attr_t thread_attr {};
  s32            result = pthread_attr_init(&thread_attr);
  if (result < 0) {
    printf("BIOStreamLogger unable to initialize\n");
    return;
  }
  result = pthread_create(&logger_server_tid, &thread_attr, LoggingServerThread, nullptr);
  if (result < 0) {
    printf("BIOStreamLogger unable to initialize\n");
    return;
  }

  pthread_set_name_np(logger_server_tid, "BIOStreamLoggerServerThread");

  // Block until logger thread finishes initializing
  while (!logger_ready) {
    sceKernelUsleep(10000);
  }
}

BIOStreamLogger::~BIOStreamLogger() {
  // Terminate logging thread.
  // Since this server uses blocking socket IO, we need to mark the server as disabled, then connect with the server to free it.
  {
    std::scoped_lock lk {logger_mutex};
    logger_ready = false;
  }

  // This message shouldn't make it through to stdout, as checks on server and client side will terminate both before any logging.
  LogMessage("Closing BIOLoggerServer%lx\n", 0);

  s32 result = pthread_join(logger_server_tid, nullptr);
  if (result < 0) {
    printf("BIOStreamLogger unable to terminate\n");
  }
}