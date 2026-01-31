#include "test.h"

#include "CppUTest/TestHarness.h"
#include "logger.h"
#include "nbio_stream_logger.h"

TEST_GROUP (NetTest) {
  void setup() {}
  void teardown() {}
};

static constexpr u16 g_test_port     = 8080;
static bool          g_server_ready  = false;
static bool          g_cond          = false;
static Logger*       g_active_logger = nullptr;

void LogMessage(const char* msg, const u64 log_res) {
  g_active_logger->LogMessage(msg, log_res);
}

void* ServerThread(void* user_arg) {
  // This will be a thread for actually sending data
  // Create a stream socket
  s32 stream_sock = sceNetSocket("TestStreamServer", ORBIS_NET_AF_INET, ORBIS_NET_SOCK_STREAM, 0);
  LogMessage("Server: socket(AF_INET, SOCK_STREAM, 0) returns 0x%08x\n", stream_sock);

  // Use setsockopt to ensure it uses specified addr and port
  s32 opt    = 1;
  s32 result = sceNetSetsockopt(stream_sock, ORBIS_NET_SOL_SOCKET, ORBIS_NET_SO_REUSEADDR, &opt, sizeof(opt));
  LogMessage("Server: setsockopt(SOL_SOCKET, SO_REUSEADDR) returns 0x%08x\n", result);
  opt    = 1;
  result = sceNetSetsockopt(stream_sock, ORBIS_NET_SOL_SOCKET, ORBIS_NET_SO_REUSEPORT, &opt, sizeof(opt));
  LogMessage("Server: setsockopt(SOL_SOCKET, SO_REUSEPORT) returns 0x%08x\n", result);

  // Bind this socket to a specific address and port
  OrbisNetSockaddrIn in_addr {};
  in_addr.sin_family      = ORBIS_NET_AF_INET;
  in_addr.sin_addr.s_addr = ORBIS_NET_INADDR_ANY;
  in_addr.sin_port        = sceNetHtons(g_test_port);
  result                  = sceNetBind(stream_sock, (OrbisNetSockaddr*)&in_addr, sizeof(in_addr));
  LogMessage("Server: bind returns 0x%08x\n", result);

  // Mark the socket as passive
  result = sceNetListen(stream_sock, 3);
  LogMessage("Server: listen returns 0x%08x\n", result);

  // Mark server as ready, so client knows to try connecting.
  g_server_ready = true;

  // Accept a connection with a socket, producing a new socket id
  u32 addr_len       = sizeof(in_addr);
  s32 connected_sock = sceNetAccept(stream_sock, (OrbisNetSockaddr*)&in_addr, &addr_len);
  LogMessage("Server: accept returns 0x%08x\n", connected_sock);

  // Send a message to the client thread
  const char* message = "This is a test message coming from the server";
  result              = sceNetSend(connected_sock, message, strlen(message), 0);
  LogMessage("Server: send returns 0x%08x\n", result);

  // Receive a message from the client thread
  char buffer[1024];
  memset(buffer, 0, sizeof(buffer));
  result = sceNetRecv(connected_sock, buffer, sizeof(buffer), 0);
  LogMessage("Server: recv returns 0x%08x\n", result);

  // Create an epoll, these are primarily used to wait for a file descriptor
  s32 epoll = sceNetEpollCreate("ServerEpoll", 0);
  LogMessage("Server: epoll_create returns 0x%08x\n", epoll);

  // These can take any "s32", and an "event" describing what to wait on.
  OrbisNetEpollEvent epoll_event {};
  // ORBIS_NET_EPOLLIN event means the socket is ready for reads
  epoll_event.events = ORBIS_NET_EPOLLIN;
  // OrbisNetEpollData is data that gets returns when wait completes
  OrbisNetEpollData epoll_data {};
  epoll_data.fd    = connected_sock;
  epoll_event.data = epoll_data;
  result           = sceNetEpollControl(epoll, ORBIS_NET_EPOLL_CTL_ADD, connected_sock, &epoll_event);
  LogMessage("Server: epoll_ctl returns 0x%08x\n", result);

  // Perform an epoll_wait with no timeout.
  OrbisNetEpollEvent epoll_ev_out {};
  result = sceNetEpollWait(epoll, &epoll_ev_out, 1, 0);
  // epoll_wait returns number of ready FDs that were polled.
  // Until client thread sends a message, this should be 0.
  LogMessage("Server: epoll_wait returns 0x%08x\n", result);

  // Tell client to send data over the socket
  g_cond = true;

  // Perform a blocking epoll_wait to wait for data
  // -1 timeout indicates no timeout.
  result = sceNetEpollWait(epoll, &epoll_ev_out, 1, -1);
  LogMessage("Server: epoll_wait returns 0x%08x\n", result);

  // Now it should be safe to read from the socket.
  memset(buffer, 0, sizeof(buffer));
  result = sceNetRecv(connected_sock, buffer, sizeof(buffer), 0);
  LogMessage("Server: recv returns 0x%08x\n", result);

  // Close the epoll
  result = sceNetEpollDestroy(epoll);
  LogMessage("Server: epoll_destroy returns 0x%08x\n", result);

  return nullptr;
}

void* ClientThread(void* user_arg) {
  while (!g_server_ready) {
    // Wait for server to prepare it's socket.
    sceKernelUsleep(10000);
  }

  // This will be a thread for testing communication via sockets
  // Create a stream socket
  s32 stream_sock = sceNetSocket("TestStreamSocket", ORBIS_NET_AF_INET, ORBIS_NET_SOCK_STREAM, 0);
  LogMessage("Client: socket(AF_INET, SOCK_STREAM, 0) returns 0x%08x\n", stream_sock);

  // Connect the socket to the server thread's socket
  OrbisNetSockaddrIn in_addr {};
  in_addr.sin_family = ORBIS_NET_AF_INET;
  in_addr.sin_port   = sceNetHtons(g_test_port);
  s32 result         = sceNetInetPton(ORBIS_NET_AF_INET, "127.0.0.1", &in_addr.sin_addr);
  LogMessage("Client: inet_pton(ORBIS_NET_AF_INET) returns 0x%08x\n", result);
  result = sceNetConnect(stream_sock, (OrbisNetSockaddr*)&in_addr, sizeof(in_addr));
  LogMessage("Client: connect returns 0x%08x\n", result);

  // Read a message through the connected socket
  char buffer[1024];
  memset(buffer, 0, sizeof(buffer));
  result = sceNetRecv(stream_sock, buffer, sizeof(buffer), 0);
  LogMessage("Client: recv returns 0x%08x\n", result);

  // Send a message to the server through this socket
  const char* message = "This is a test message coming from the client";
  result              = sceNetSend(stream_sock, message, strlen(message), 0);
  LogMessage("Client: send returns 0x%08x\n", result);

  // Wait for g_cond to be true, this will serve as a signal to test epoll behavior
  while (!g_cond) {
    sceKernelUsleep(10000);
  }
  g_cond = false;

  // Server wants to test epoll behavior, send data over the socket
  result = sceNetSend(stream_sock, message, strlen(message), 0);
  LogMessage("Client: send returns 0x%08x\n", result);

  return nullptr;
}

TEST(NetTest, Test) {
  // Init libSceNet
  s32 result = sceNetInit();
  printf("sceNetInit returns 0x%08x\n", result);

  // Create a server and client thread.
  // These will serve as a way of testing behavior all in one application, hopefully.
  pthread_attr_t thread_attr {};
  result = pthread_attr_init(&thread_attr);
  printf("pthread_attr_init returns 0x%08x\n", result);

  g_active_logger = new NBIOStreamLogger(true);

  pthread_t server_tid {};
  result = pthread_create(&server_tid, &thread_attr, ServerThread, nullptr);
  printf("Server thread created, pthread_create returns 0x%08x\n", result);
  pthread_t client_tid {};
  result = pthread_create(&client_tid, &thread_attr, ClientThread, nullptr);
  printf("Client thread created, pthread_create returns 0x%08x\n", result);

  // Wait for all threads to exit
  result = pthread_join(server_tid, nullptr);
  result = pthread_join(client_tid, nullptr);

  delete (g_active_logger);

  // Terminate libSceNet
  result = sceNetTerm();
  printf("sceNetTerm returns 0x%08x\n", result);
}
