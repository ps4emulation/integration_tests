#pragma once

#include "CppUTest/TestHarness.h"

#define UNSIGNED_INT_EQUALS(expected, actual) UNSIGNED_LONGS_EQUAL_LOCATION((u32)expected, (u32)actual, NULLPTR, __FILE__, __LINE__)

using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

struct OrbisNetInAddr {
  u32 s_addr;
};

struct OrbisNetSockaddrIn {
  u8             sin_len;
  u8             sin_family;
  u16            sin_port;
  OrbisNetInAddr sin_addr;
  u16            sin_vport;
  char           sin_zero[6];
};

struct OrbisNetIovec {
  void* iov_base;
  u64   iov_len;
};

struct OrbisNetMsghdr {
  void*          msg_name;
  u32            msg_namelen;
  OrbisNetIovec* msg_iov;
  s32            msg_iovlen;
  void*          msg_control;
  u32            msg_controllen;
  s32            msg_flags;
};

struct OrbisNetSockaddr {
  u8   sa_len;
  u8   sa_family;
  char sa_data[14];
};

union OrbisNetEpollData {
  void* ptr;
  u32   u32;
  s32   fd;
  u64   u64;
};

struct OrbisNetEpollEvent {
  u32               events;
  u32               reserved;
  u64               ident;
  OrbisNetEpollData data;
};

// Socket types
#define ORBIS_NET_SOCK_STREAM 1

// Socket options
#define ORBIS_NET_SO_REUSEADDR 0x4
#define ORBIS_NET_SO_REUSEPORT 0x200
#define ORBIS_NET_SO_NBIO      0x1200

// Socket option levels
#define ORBIS_NET_SOL_SOCKET 0xffff

// Socket address families
#define ORBIS_NET_AF_INET 2

// Default input addresses
#define ORBIS_NET_INADDR_ANY ((u32)0x00000000)

// Epoll event flags
#define ORBIS_NET_EPOLLIN  0x00000001
#define ORBIS_NET_EPOLLOUT 0x00000002

// Epoll control operations
#define ORBIS_NET_EPOLL_CTL_ADD 1
#define ORBIS_NET_EPOLL_CTL_MOD 2
#define ORBIS_NET_EPOLL_CTL_DEL 3

// Error codes
enum OrbisNetError : s32 {
  ORBIS_NET_ERROR_EPERM                    = s32(0x80410101),
  ORBIS_NET_ERROR_ENOENT                   = s32(0x80410102),
  ORBIS_NET_ERROR_ESRCH                    = s32(0x80410103),
  ORBIS_NET_ERROR_EINTR                    = s32(0x80410104),
  ORBIS_NET_ERROR_EIO                      = s32(0x80410105),
  ORBIS_NET_ERROR_ENXIO                    = s32(0x80410106),
  ORBIS_NET_ERROR_E2BIG                    = s32(0x80410107),
  ORBIS_NET_ERROR_ENOEXEC                  = s32(0x80410108),
  ORBIS_NET_ERROR_EBADF                    = s32(0x80410109),
  ORBIS_NET_ERROR_ECHILD                   = s32(0x8041010A),
  ORBIS_NET_ERROR_EDEADLK                  = s32(0x8041010B),
  ORBIS_NET_ERROR_ENOMEM                   = s32(0x8041010C),
  ORBIS_NET_ERROR_EACCES                   = s32(0x8041010D),
  ORBIS_NET_ERROR_EFAULT                   = s32(0x8041010E),
  ORBIS_NET_ERROR_ENOTBLK                  = s32(0x8041010F),
  ORBIS_NET_ERROR_EBUSY                    = s32(0x80410110),
  ORBIS_NET_ERROR_EEXIST                   = s32(0x80410111),
  ORBIS_NET_ERROR_EXDEV                    = s32(0x80410112),
  ORBIS_NET_ERROR_ENODEV                   = s32(0x80410113),
  ORBIS_NET_ERROR_ENOTDIR                  = s32(0x80410114),
  ORBIS_NET_ERROR_EISDIR                   = s32(0x80410115),
  ORBIS_NET_ERROR_EINVAL                   = s32(0x80410116),
  ORBIS_NET_ERROR_ENFILE                   = s32(0x80410117),
  ORBIS_NET_ERROR_EMFILE                   = s32(0x80410118),
  ORBIS_NET_ERROR_ENOTTY                   = s32(0x80410119),
  ORBIS_NET_ERROR_ETXTBSY                  = s32(0x8041011A),
  ORBIS_NET_ERROR_EFBIG                    = s32(0x8041011B),
  ORBIS_NET_ERROR_ENOSPC                   = s32(0x8041011C),
  ORBIS_NET_ERROR_ESPIPE                   = s32(0x8041011D),
  ORBIS_NET_ERROR_EROFS                    = s32(0x8041011E),
  ORBIS_NET_ERROR_EMLINK                   = s32(0x8041011F),
  ORBIS_NET_ERROR_EPIPE                    = s32(0x80410120),
  ORBIS_NET_ERROR_EDOM                     = s32(0x80410121),
  ORBIS_NET_ERROR_ERANGE                   = s32(0x80410122),
  ORBIS_NET_ERROR_EAGAIN                   = s32(0x80410123),
  ORBIS_NET_ERROR_EWOULDBLOCK              = s32(0x80410123),
  ORBIS_NET_ERROR_EINPROGRESS              = s32(0x80410124),
  ORBIS_NET_ERROR_EALREADY                 = s32(0x80410125),
  ORBIS_NET_ERROR_ENOTSOCK                 = s32(0x80410126),
  ORBIS_NET_ERROR_EDESTADDRREQ             = s32(0x80410127),
  ORBIS_NET_ERROR_EMSGSIZE                 = s32(0x80410128),
  ORBIS_NET_ERROR_EPROTOTYPE               = s32(0x80410129),
  ORBIS_NET_ERROR_ENOPROTOOPT              = s32(0x8041012A),
  ORBIS_NET_ERROR_EPROTONOSUPPORT          = s32(0x8041012B),
  ORBIS_NET_ERROR_ESOCKTNOSUPPORT          = s32(0x8041012C),
  ORBIS_NET_ERROR_EOPNOTSUPP               = s32(0x8041012D),
  ORBIS_NET_ERROR_ENOTSUP                  = s32(0x8041012D),
  ORBIS_NET_ERROR_EPFNOSUPPORT             = s32(0x8041012E),
  ORBIS_NET_ERROR_EAFNOSUPPORT             = s32(0x8041012F),
  ORBIS_NET_ERROR_EADDRINUSE               = s32(0x80410130),
  ORBIS_NET_ERROR_EADDRNOTAVAIL            = s32(0x80410131),
  ORBIS_NET_ERROR_ENETDOWN                 = s32(0x80410132),
  ORBIS_NET_ERROR_ENETUNREACH              = s32(0x80410133),
  ORBIS_NET_ERROR_ENETRESET                = s32(0x80410134),
  ORBIS_NET_ERROR_ECONNABORTED             = s32(0x80410135),
  ORBIS_NET_ERROR_ECONNRESET               = s32(0x80410136),
  ORBIS_NET_ERROR_ENOBUFS                  = s32(0x80410137),
  ORBIS_NET_ERROR_EISCONN                  = s32(0x80410138),
  ORBIS_NET_ERROR_ENOTCONN                 = s32(0x80410139),
  ORBIS_NET_ERROR_ESHUTDOWN                = s32(0x8041013A),
  ORBIS_NET_ERROR_ETOOMANYREFS             = s32(0x8041013B),
  ORBIS_NET_ERROR_ETIMEDOUT                = s32(0x8041013C),
  ORBIS_NET_ERROR_ECONNREFUSED             = s32(0x8041013D),
  ORBIS_NET_ERROR_ELOOP                    = s32(0x8041013E),
  ORBIS_NET_ERROR_ENAMETOOLONG             = s32(0x8041013F),
  ORBIS_NET_ERROR_EHOSTDOWN                = s32(0x80410140),
  ORBIS_NET_ERROR_EHOSTUNREACH             = s32(0x80410141),
  ORBIS_NET_ERROR_ENOTEMPTY                = s32(0x80410142),
  ORBIS_NET_ERROR_EPROCLIM                 = s32(0x80410143),
  ORBIS_NET_ERROR_EUSERS                   = s32(0x80410144),
  ORBIS_NET_ERROR_EDQUOT                   = s32(0x80410145),
  ORBIS_NET_ERROR_ESTALE                   = s32(0x80410146),
  ORBIS_NET_ERROR_EREMOTE                  = s32(0x80410147),
  ORBIS_NET_ERROR_EBADRPC                  = s32(0x80410148),
  ORBIS_NET_ERROR_ERPCMISMATCH             = s32(0x80410149),
  ORBIS_NET_ERROR_EPROGUNAVAIL             = s32(0x8041014A),
  ORBIS_NET_ERROR_EPROGMISMATCH            = s32(0x8041014B),
  ORBIS_NET_ERROR_EPROCUNAVAIL             = s32(0x8041014C),
  ORBIS_NET_ERROR_ENOLCK                   = s32(0x8041014D),
  ORBIS_NET_ERROR_ENOSYS                   = s32(0x8041014E),
  ORBIS_NET_ERROR_EFTYPE                   = s32(0x8041014F),
  ORBIS_NET_ERROR_EAUTH                    = s32(0x80410150),
  ORBIS_NET_ERROR_ENEEDAUTH                = s32(0x80410151),
  ORBIS_NET_ERROR_EIDRM                    = s32(0x80410152),
  ORBIS_NET_ERROR_ENOMS                    = s32(0x80410153),
  ORBIS_NET_ERROR_EOVERFLOW                = s32(0x80410154),
  ORBIS_NET_ERROR_ECANCELED                = s32(0x80410155),
  ORBIS_NET_ERROR_EPROTO                   = s32(0x8041015C),
  ORBIS_NET_ERROR_EADHOC                   = s32(0x804101A0),
  ORBIS_NET_ERROR_EINACTIVEDISABLED        = s32(0x804101A3),
  ORBIS_NET_ERROR_ENODATA                  = s32(0x804101A4),
  ORBIS_NET_ERROR_EDESC                    = s32(0x804101A5),
  ORBIS_NET_ERROR_EDESCTIMEDOUT            = s32(0x804101A6),
  ORBIS_NET_ERROR_ENETINTR                 = s32(0x804101A7),
  ORBIS_NET_ERROR_ENOTINIT                 = s32(0x804101C8),
  ORBIS_NET_ERROR_ENOLIBMEM                = s32(0x804101C9),
  ORBIS_NET_ERROR_ECALLBACK                = s32(0x804101CB),
  ORBIS_NET_ERROR_EINTERNAL                = s32(0x804101CC),
  ORBIS_NET_ERROR_ERETURN                  = s32(0x804101CD),
  ORBIS_NET_ERROR_ENOALLOCMEM              = s32(0x804101CE),
  ORBIS_NET_ERROR_RESOLVER_EINTERNAL       = s32(0x804101DC),
  ORBIS_NET_ERROR_RESOLVER_EBUSY           = s32(0x804101DD),
  ORBIS_NET_ERROR_RESOLVER_ENOSPACE        = s32(0x804101DE),
  ORBIS_NET_ERROR_RESOLVER_EPACKET         = s32(0x804101DF),
  ORBIS_NET_ERROR_RESOLVER_ENODNS          = s32(0x804101E1),
  ORBIS_NET_ERROR_RESOLVER_ETIMEDOUT       = s32(0x804101E2),
  ORBIS_NET_ERROR_RESOLVER_ENOSUPPORT      = s32(0x804101E3),
  ORBIS_NET_ERROR_RESOLVER_EFORMAT         = s32(0x804101E4),
  ORBIS_NET_ERROR_RESOLVER_ESERVERFAILURE  = s32(0x804101E5),
  ORBIS_NET_ERROR_RESOLVER_ENOHOST         = s32(0x804101E6),
  ORBIS_NET_ERROR_RESOLVER_ENOTIMPLEMENTED = s32(0x804101E7),
  ORBIS_NET_ERROR_RESOLVER_ESERVERREFUSED  = s32(0x804101E8),
  ORBIS_NET_ERROR_RESOLVER_ENORECORD       = s32(0x804101E9),
  ORBIS_NET_ERROR_RESOLVER_EALIGNMENT      = s32(0x804101EA),
  ORBIS_NET_ERROR_RESOLVER_ENOTFOUND       = s32(0x804101EB),
  ORBIS_NET_ERROR_RESOLVER_ENOTINIT        = s32(0x804101EC),
};

extern "C" {
s32 sceNetInit();
s32 sceNetTerm();

s32 sceNetSocket(const char* name, s32 domain, s32 type, s32 protocol);
s32 sceNetAccept(s32 s, OrbisNetSockaddr* addr, u32* addrlen);
s32 sceNetBind(s32 s, const OrbisNetSockaddr* addr, u32 addrlen);
s32 sceNetConnect(s32 s, const OrbisNetSockaddr* name, u32 namelen);
s32 sceNetGetpeername(s32 s, OrbisNetSockaddr* name, u32* namelen);
s32 sceNetGetsockname(s32 s, OrbisNetSockaddr* name, u32* namelen);
s32 sceNetGetsockopt(s32 s, s32 level, s32 optname, void* optval, u32* optlen);
s32 sceNetListen(s32 s, s32 backlog);
s32 sceNetRecv(s32 s, void* buf, u64 len, s32 flags);
s32 sceNetRecvfrom(s32 s, void* buf, u64 len, s32 flags, OrbisNetSockaddr* from, u32* fromlen);
s32 sceNetRecvmsg(s32 s, OrbisNetMsghdr* msg, s32 flags);
s32 sceNetSend(s32 s, const void* msg, u64 len, s32 flags);
s32 sceNetSendto(s32 s, const void* msg, u64 len, s32 flags, const OrbisNetSockaddr* to, u32 tolen);
s32 sceNetSendmsg(s32 s, const OrbisNetMsghdr* msg, s32 flags);
s32 sceNetSetsockopt(s32 s, s32 level, s32 optname, const void* optval, u32 optlen);
s32 sceNetShutdown(s32 s, s32 how);
s32 sceNetSocketClose(s32 s);
s32 sceNetSocketAbort(s32 s, s32 flags);

s32 sceNetEpollCreate(const char* name, s32 flags);
s32 sceNetEpollControl(s32 eid, s32 op, s32 id, OrbisNetEpollEvent* event);
s32 sceNetEpollWait(s32 eid, OrbisNetEpollEvent* events, s32 maxevents, s32 timeout);
s32 sceNetEpollDestroy(s32 eid);
s32 sceNetEpollAbort(s32 eid, s32 flags);

u16 sceNetHtons(u16 host16);

s32 sceNetInetPton(s32 af, const char* src, void* dst);

s32 sceKernelUsleep(u32 micros);
}