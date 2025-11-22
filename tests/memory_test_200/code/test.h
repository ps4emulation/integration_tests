#include "CppUTest/TestHarness.h"

#define UNSIGNED_INT_EQUALS(expected, actual) _unsigned_int_equals(expected, actual);

void _unsigned_int_equals(uint32_t expected, uint32_t actual) {
  UNSIGNED_LONGS_EQUAL(expected, actual);
}

// Function definitions (with modified types to improve testability)
extern "C" {
// Direct memory functions
uint64_t sceKernelGetDirectMemorySize();
int32_t  sceKernelEnableDmemAliasing();
int32_t  sceKernelAvailableDirectMemorySize(int64_t start, int64_t end, uint64_t alignment, int64_t* phys_addr, uint64_t* size);
int32_t  sceKernelAllocateDirectMemory(int64_t start, int64_t end, uint64_t size, uint64_t alignment, int32_t type, int64_t* phys_addr);
int32_t  sceKernelAllocateMainDirectMemory(uint64_t size, uint64_t alignment, int32_t type, int64_t* phys_addr);
int32_t  sceKernelInternalMapDirectMemory(int32_t pool_id, uint64_t* addr, uint64_t size, int32_t prot, int32_t flags, int64_t phys_addr, uint64_t alignment);
int32_t  sceKernelInternalMapNamedDirectMemory(int32_t pool_id, uint64_t* addr, uint64_t size, int32_t prot, int32_t flags, int64_t phys_addr,
                                               uint64_t alignment, const char* name);
int32_t  sceKernelMapDirectMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags, int64_t phys_addr, uint64_t alignment);
int32_t  sceKernelMapNamedDirectMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags, int64_t phys_addr, uint64_t alignment, const char* name);
int32_t  sceKernelMapDirectMemory2(uint64_t* addr, uint64_t size, int32_t type, int32_t prot, int32_t flags, int64_t phys_addr, uint64_t alignment);
int32_t  sceKernelGetDirectMemoryType(int64_t phys_addr, int32_t* type, int64_t* start, int64_t* end);
int32_t  sceKernelCheckedReleaseDirectMemory(int64_t phys_addr, uint64_t size);
int32_t  sceKernelReleaseDirectMemory(int64_t phys_addr, uint64_t size);

// Flexible memory functions
int32_t sceKernelAvailableFlexibleMemorySize(uint64_t* size);
int32_t sceKernelMapFlexibleMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags);
int32_t sceKernelMapNamedFlexibleMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags, const char* name);
int32_t sceKernelMapNamedSystemFlexibleMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags, const char* name);
int32_t sceKernelReleaseFlexibleMemory(uint64_t* addr, uint64_t size);

// Reserve memory function
int32_t sceKernelReserveVirtualRange(uint64_t* addr, uint64_t size, int32_t flags, uint64_t alignment);

// Generic memory functions
int32_t getpagesize();
int32_t sceKernelMmap(uint64_t addr, uint64_t size, int32_t prot, int32_t flags, int32_t fd, int64_t offset, uint64_t* out_addr);
int32_t sceKernelMunmap(uint64_t addr, uint64_t size);
int32_t sceKernelMprotect(uint64_t addr, uint64_t size, int32_t prot);
int32_t sceKernelMtypeprotect(uint64_t addr, uint64_t size, int32_t mtype, int32_t prot);
int32_t sceKernelQueryMemoryProtection(uint64_t addr, uint64_t* start, uint64_t* end, int32_t* prot);
int32_t sceKernelVirtualQuery(uint64_t addr, int32_t flags, void* info, uint64_t info_size);
int32_t sceKernelSetVirtualRangeName(uint64_t addr, uint64_t size, const char* name);
int32_t sceKernelMlock(uint64_t addr, uint64_t size);

// Memory pool functions
int32_t sceKernelMemoryPoolExpand(int64_t start, int64_t end, uint64_t len, uint64_t alignment, int64_t* phys_addr);
int32_t sceKernelMemoryPoolReserve(uint64_t addr_in, uint64_t len, uint64_t alignment, int32_t flags, uint64_t* addr_out);
int32_t sceKernelMemoryPoolCommit(uint64_t addr, uint64_t len, int32_t type, int32_t prot, int32_t flags);
int32_t sceKernelMemoryPoolDecommit(uint64_t addr, uint64_t len, int32_t flags);

// Filesystem functions
int32_t sceKernelOpen(const char* path, int32_t flags, uint16_t mode);
int64_t sceKernelRead(int32_t fd, void* buf, uint64_t size);
int64_t sceKernelWrite(int32_t fd, const void* buf, uint64_t size);
int32_t sceKernelFtruncate(int32_t fd, int64_t size);
int64_t sceKernelLseek(int32_t fd, int64_t offset, int32_t whence);
int32_t sceKernelClose(int32_t fd);

// System functions
const char* sceKernelGetFsSandboxRandomWord();
}

// Some error codes
enum OrbisError : int32_t {
  ORBIS_KERNEL_ERROR_UNKNOWN         = int(0x80020000),
  ORBIS_KERNEL_ERROR_EPERM           = int(0x80020001),
  ORBIS_KERNEL_ERROR_ENOENT          = int(0x80020002),
  ORBIS_KERNEL_ERROR_ESRCH           = int(0x80020003),
  ORBIS_KERNEL_ERROR_EINTR           = int(0x80020004),
  ORBIS_KERNEL_ERROR_EIO             = int(0x80020005),
  ORBIS_KERNEL_ERROR_ENXIO           = int(0x80020006),
  ORBIS_KERNEL_ERROR_E2BIG           = int(0x80020007),
  ORBIS_KERNEL_ERROR_ENOEXEC         = int(0x80020008),
  ORBIS_KERNEL_ERROR_EBADF           = int(0x80020009),
  ORBIS_KERNEL_ERROR_ECHILD          = int(0x8002000A),
  ORBIS_KERNEL_ERROR_EDEADLK         = int(0x8002000B),
  ORBIS_KERNEL_ERROR_ENOMEM          = int(0x8002000C),
  ORBIS_KERNEL_ERROR_EACCES          = int(0x8002000D),
  ORBIS_KERNEL_ERROR_EFAULT          = int(0x8002000E),
  ORBIS_KERNEL_ERROR_ENOTBLK         = int(0x8002000F),
  ORBIS_KERNEL_ERROR_EBUSY           = int(0x80020010),
  ORBIS_KERNEL_ERROR_EEXIST          = int(0x80020011),
  ORBIS_KERNEL_ERROR_EXDEV           = int(0x80020012),
  ORBIS_KERNEL_ERROR_ENODEV          = int(0x80020013),
  ORBIS_KERNEL_ERROR_ENOTDIR         = int(0x80020014),
  ORBIS_KERNEL_ERROR_EISDIR          = int(0x80020015),
  ORBIS_KERNEL_ERROR_EINVAL          = int(0x80020016),
  ORBIS_KERNEL_ERROR_ENFILE          = int(0x80020017),
  ORBIS_KERNEL_ERROR_EMFILE          = int(0x80020018),
  ORBIS_KERNEL_ERROR_ENOTTY          = int(0x80020019),
  ORBIS_KERNEL_ERROR_ETXTBSY         = int(0x8002001A),
  ORBIS_KERNEL_ERROR_EFBIG           = int(0x8002001B),
  ORBIS_KERNEL_ERROR_ENOSPC          = int(0x8002001C),
  ORBIS_KERNEL_ERROR_ESPIPE          = int(0x8002001D),
  ORBIS_KERNEL_ERROR_EROFS           = int(0x8002001E),
  ORBIS_KERNEL_ERROR_EMLINK          = int(0x8002001F),
  ORBIS_KERNEL_ERROR_EPIPE           = int(0x80020020),
  ORBIS_KERNEL_ERROR_EDOM            = int(0x80020021),
  ORBIS_KERNEL_ERROR_ERANGE          = int(0x80020022),
  ORBIS_KERNEL_ERROR_EAGAIN          = int(0x80020023),
  ORBIS_KERNEL_ERROR_EWOULDBLOCK     = int(0x80020023),
  ORBIS_KERNEL_ERROR_EINPROGRESS     = int(0x80020024),
  ORBIS_KERNEL_ERROR_EALREADY        = int(0x80020025),
  ORBIS_KERNEL_ERROR_ENOTSOCK        = int(0x80020026),
  ORBIS_KERNEL_ERROR_EDESTADDRREQ    = int(0x80020027),
  ORBIS_KERNEL_ERROR_EMSGSIZE        = int(0x80020028),
  ORBIS_KERNEL_ERROR_EPROTOTYPE      = int(0x80020029),
  ORBIS_KERNEL_ERROR_ENOPROTOOPT     = int(0x8002002A),
  ORBIS_KERNEL_ERROR_EPROTONOSUPPORT = int(0x8002002B),
  ORBIS_KERNEL_ERROR_ESOCKTNOSUPPORT = int(0x8002002C),
  ORBIS_KERNEL_ERROR_ENOTSUP         = int(0x8002002D),
  ORBIS_KERNEL_ERROR_EOPNOTSUPP      = int(0x8002002D),
  ORBIS_KERNEL_ERROR_EPFNOSUPPORT    = int(0x8002002E),
  ORBIS_KERNEL_ERROR_EAFNOSUPPORT    = int(0x8002002F),
  ORBIS_KERNEL_ERROR_EADDRINUSE      = int(0x80020030),
  ORBIS_KERNEL_ERROR_EADDRNOTAVAIL   = int(0x80020031),
  ORBIS_KERNEL_ERROR_ENETDOWN        = int(0x80020032),
  ORBIS_KERNEL_ERROR_ENETUNREACH     = int(0x80020033),
  ORBIS_KERNEL_ERROR_ENETRESET       = int(0x80020034),
  ORBIS_KERNEL_ERROR_ECONNABORTED    = int(0x80020035),
  ORBIS_KERNEL_ERROR_ECONNRESET      = int(0x80020036),
  ORBIS_KERNEL_ERROR_ENOBUFS         = int(0x80020037),
  ORBIS_KERNEL_ERROR_EISCONN         = int(0x80020038),
  ORBIS_KERNEL_ERROR_ENOTCONN        = int(0x80020039),
  ORBIS_KERNEL_ERROR_ESHUTDOWN       = int(0x8002003A),
  ORBIS_KERNEL_ERROR_ETOOMANYREFS    = int(0x8002003B),
  ORBIS_KERNEL_ERROR_ETIMEDOUT       = int(0x8002003C),
  ORBIS_KERNEL_ERROR_ECONNREFUSED    = int(0x8002003D),
  ORBIS_KERNEL_ERROR_ELOOP           = int(0x8002003E),
  ORBIS_KERNEL_ERROR_ENAMETOOLONG    = int(0x8002003F),
  ORBIS_KERNEL_ERROR_EHOSTDOWN       = int(0x80020040),
  ORBIS_KERNEL_ERROR_EHOSTUNREACH    = int(0x80020041),
  ORBIS_KERNEL_ERROR_ENOTEMPTY       = int(0x80020042),
  ORBIS_KERNEL_ERROR_EPROCLIM        = int(0x80020043),
  ORBIS_KERNEL_ERROR_EUSERS          = int(0x80020044),
  ORBIS_KERNEL_ERROR_EDQUOT          = int(0x80020045),
  ORBIS_KERNEL_ERROR_ESTALE          = int(0x80020046),
  ORBIS_KERNEL_ERROR_EREMOTE         = int(0x80020047),
  ORBIS_KERNEL_ERROR_EBADRPC         = int(0x80020048),
  ORBIS_KERNEL_ERROR_ERPCMISMATCH    = int(0x80020049),
  ORBIS_KERNEL_ERROR_EPROGUNAVAIL    = int(0x8002004A),
  ORBIS_KERNEL_ERROR_EPROGMISMATCH   = int(0x8002004B),
  ORBIS_KERNEL_ERROR_EPROCUNAVAIL    = int(0x8002004C),
  ORBIS_KERNEL_ERROR_ENOLCK          = int(0x8002004D),
  ORBIS_KERNEL_ERROR_ENOSYS          = int(0x8002004E),
  ORBIS_KERNEL_ERROR_EFTYPE          = int(0x8002004F),
  ORBIS_KERNEL_ERROR_EAUTH           = int(0x80020050),
  ORBIS_KERNEL_ERROR_ENEEDAUTH       = int(0x80020051),
  ORBIS_KERNEL_ERROR_EIDRM           = int(0x80020052),
  ORBIS_KERNEL_ERROR_ENOMSG          = int(0x80020053),
  ORBIS_KERNEL_ERROR_EOVERFLOW       = int(0x80020054),
  ORBIS_KERNEL_ERROR_ECANCELED       = int(0x80020055),
  ORBIS_KERNEL_ERROR_EILSEQ          = int(0x80020056),
  ORBIS_KERNEL_ERROR_ENOATTR         = int(0x80020057),
  ORBIS_KERNEL_ERROR_EDOOFUS         = int(0x80020058),
  ORBIS_KERNEL_ERROR_EBADMSG         = int(0x80020059),
  ORBIS_KERNEL_ERROR_EMULTIHOP       = int(0x8002005A),
  ORBIS_KERNEL_ERROR_ENOLINK         = int(0x8002005B),
  ORBIS_KERNEL_ERROR_EPROTO          = int(0x8002005C),
  ORBIS_KERNEL_ERROR_ENOTCAPABLE     = int(0x8002005D),
  ORBIS_KERNEL_ERROR_ECAPMODE        = int(0x8002005E),
  ORBIS_KERNEL_ERROR_ENOBLK          = int(0x8002005F),
  ORBIS_KERNEL_ERROR_EICV            = int(0x80020060),
  ORBIS_KERNEL_ERROR_ENOPLAYGOENT    = int(0x80020061)
};

#define mem_scan() _mem_scan(__FILE__, __LINE__)

void _mem_scan(const char* file, int line) {
  // Helper method from red_prig for printing out memory map information.
  const char* _F = "_F";
  const char* _D = "_D";
  const char* _S = "_S";
  const char* _P = "_P";
  const char* _C = "_C";
  const char* _R = "_R";
  const char* _W = "_W";
  const char* _X = "_X";

  struct OrbisKernelVirtualQueryInfo {
    unsigned long long start_addr;
    unsigned long long end_addr;
    unsigned long long offset;
    int32_t            prot;
    int32_t            mtype;
    uint8_t            isFlexibleMemory : 1;
    uint8_t            isDirectMemory   : 1;
    uint8_t            isStack          : 1;
    uint8_t            isPooledMemory   : 1;
    uint8_t            isCommitted      : 1;
    char               name[32];
  };

  printf("mem_scan[%s:%d]\n", file, line);

  uint64_t addr = {};
  int      ret  = {};
  while (true) {
    OrbisKernelVirtualQueryInfo info = {};
    ret                              = sceKernelVirtualQuery(addr, 1, &info, sizeof(info));
    if (ret != 0) break;
    addr = static_cast<uint64_t>(info.end_addr);
    printf("0x%012llX" // start_addr
           "-"
           "0x%012llX" // end_addr
           "|"
           "0x%016llX" // offset
           "|"
           "%c%c%c%c%c%c" // RWXCRW
           "|"
           "0x%01X" // mtype
           "|"
           "%c%c%c%c%c" // FDSPC
           "|"
           "%s" // name
           "\n",
           info.start_addr, info.end_addr, info.offset, _R[(info.prot >> 0) & 1], _W[(info.prot >> 1) & 1], _X[(info.prot >> 2) & 1], _C[(info.prot >> 3) & 1],
           _R[(info.prot >> 4) & 1], _W[(info.prot >> 5) & 1], info.mtype, _F[info.isFlexibleMemory], _D[info.isDirectMemory], _S[info.isStack],
           _P[info.isPooledMemory], _C[info.isCommitted], info.name);
  }
  printf("\n");
}