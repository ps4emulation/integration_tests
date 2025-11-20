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
#define ORBIS_KERNEL_ERROR_UNKNOWN         (0x80020000)
#define ORBIS_KERNEL_ERROR_EPERM           (0x80020001)
#define ORBIS_KERNEL_ERROR_ENOENT          (0x80020002)
#define ORBIS_KERNEL_ERROR_ESRCH           (0x80020003)
#define ORBIS_KERNEL_ERROR_EINTR           (0x80020004)
#define ORBIS_KERNEL_ERROR_EIO             (0x80020005)
#define ORBIS_KERNEL_ERROR_ENXIO           (0x80020006)
#define ORBIS_KERNEL_ERROR_E2BIG           (0x80020007)
#define ORBIS_KERNEL_ERROR_ENOEXEC         (0x80020008)
#define ORBIS_KERNEL_ERROR_EBADF           (0x80020009)
#define ORBIS_KERNEL_ERROR_ECHILD          (0x8002000A)
#define ORBIS_KERNEL_ERROR_EDEADLK         (0x8002000B)
#define ORBIS_KERNEL_ERROR_ENOMEM          (0x8002000C)
#define ORBIS_KERNEL_ERROR_EACCES          (0x8002000D)
#define ORBIS_KERNEL_ERROR_EFAULT          (0x8002000E)
#define ORBIS_KERNEL_ERROR_ENOTBLK         (0x8002000F)
#define ORBIS_KERNEL_ERROR_EBUSY           (0x80020010)
#define ORBIS_KERNEL_ERROR_EEXIST          (0x80020011)
#define ORBIS_KERNEL_ERROR_EXDEV           (0x80020012)
#define ORBIS_KERNEL_ERROR_ENODEV          (0x80020013)
#define ORBIS_KERNEL_ERROR_ENOTDIR         (0x80020014)
#define ORBIS_KERNEL_ERROR_EISDIR          (0x80020015)
#define ORBIS_KERNEL_ERROR_EINVAL          (0x80020016)
#define ORBIS_KERNEL_ERROR_ENFILE          (0x80020017)
#define ORBIS_KERNEL_ERROR_EMFILE          (0x80020018)
#define ORBIS_KERNEL_ERROR_ENOTTY          (0x80020019)
#define ORBIS_KERNEL_ERROR_ETXTBSY         (0x8002001A)
#define ORBIS_KERNEL_ERROR_EFBIG           (0x8002001B)
#define ORBIS_KERNEL_ERROR_ENOSPC          (0x8002001C)
#define ORBIS_KERNEL_ERROR_ESPIPE          (0x8002001D)
#define ORBIS_KERNEL_ERROR_EROFS           (0x8002001E)
#define ORBIS_KERNEL_ERROR_EMLINK          (0x8002001F)
#define ORBIS_KERNEL_ERROR_EPIPE           (0x80020020)
#define ORBIS_KERNEL_ERROR_EDOM            (0x80020021)
#define ORBIS_KERNEL_ERROR_ERANGE          (0x80020022)
#define ORBIS_KERNEL_ERROR_EAGAIN          (0x80020023)
#define ORBIS_KERNEL_ERROR_EWOULDBLOCK     (0x80020023)
#define ORBIS_KERNEL_ERROR_EINPROGRESS     (0x80020024)
#define ORBIS_KERNEL_ERROR_EALREADY        (0x80020025)
#define ORBIS_KERNEL_ERROR_ENOTSOCK        (0x80020026)
#define ORBIS_KERNEL_ERROR_EDESTADDRREQ    (0x80020027)
#define ORBIS_KERNEL_ERROR_EMSGSIZE        (0x80020028)
#define ORBIS_KERNEL_ERROR_EPROTOTYPE      (0x80020029)
#define ORBIS_KERNEL_ERROR_ENOPROTOOPT     (0x8002002A)
#define ORBIS_KERNEL_ERROR_EPROTONOSUPPORT (0x8002002B)
#define ORBIS_KERNEL_ERROR_ESOCKTNOSUPPORT (0x8002002C)
#define ORBIS_KERNEL_ERROR_ENOTSUP         (0x8002002D)
#define ORBIS_KERNEL_ERROR_EOPNOTSUPP      (0x8002002D)
#define ORBIS_KERNEL_ERROR_EPFNOSUPPORT    (0x8002002E)
#define ORBIS_KERNEL_ERROR_EAFNOSUPPORT    (0x8002002F)
#define ORBIS_KERNEL_ERROR_EADDRINUSE      (0x80020030)
#define ORBIS_KERNEL_ERROR_EADDRNOTAVAIL   (0x80020031)
#define ORBIS_KERNEL_ERROR_ENETDOWN        (0x80020032)
#define ORBIS_KERNEL_ERROR_ENETUNREACH     (0x80020033)
#define ORBIS_KERNEL_ERROR_ENETRESET       (0x80020034)
#define ORBIS_KERNEL_ERROR_ECONNABORTED    (0x80020035)
#define ORBIS_KERNEL_ERROR_ECONNRESET      (0x80020036)
#define ORBIS_KERNEL_ERROR_ENOBUFS         (0x80020037)
#define ORBIS_KERNEL_ERROR_EISCONN         (0x80020038)
#define ORBIS_KERNEL_ERROR_ENOTCONN        (0x80020039)
#define ORBIS_KERNEL_ERROR_ESHUTDOWN       (0x8002003A)
#define ORBIS_KERNEL_ERROR_ETOOMANYREFS    (0x8002003B)
#define ORBIS_KERNEL_ERROR_ETIMEDOUT       (0x8002003C)
#define ORBIS_KERNEL_ERROR_ECONNREFUSED    (0x8002003D)
#define ORBIS_KERNEL_ERROR_ELOOP           (0x8002003E)
#define ORBIS_KERNEL_ERROR_ENAMETOOLONG    (0x8002003F)
#define ORBIS_KERNEL_ERROR_EHOSTDOWN       (0x80020040)
#define ORBIS_KERNEL_ERROR_EHOSTUNREACH    (0x80020041)
#define ORBIS_KERNEL_ERROR_ENOTEMPTY       (0x80020042)
#define ORBIS_KERNEL_ERROR_EPROCLIM        (0x80020043)
#define ORBIS_KERNEL_ERROR_EUSERS          (0x80020044)
#define ORBIS_KERNEL_ERROR_EDQUOT          (0x80020045)
#define ORBIS_KERNEL_ERROR_ESTALE          (0x80020046)
#define ORBIS_KERNEL_ERROR_EREMOTE         (0x80020047)
#define ORBIS_KERNEL_ERROR_EBADRPC         (0x80020048)
#define ORBIS_KERNEL_ERROR_ERPCMISMATCH    (0x80020049)
#define ORBIS_KERNEL_ERROR_EPROGUNAVAIL    (0x8002004A)
#define ORBIS_KERNEL_ERROR_EPROGMISMATCH   (0x8002004B)
#define ORBIS_KERNEL_ERROR_EPROCUNAVAIL    (0x8002004C)
#define ORBIS_KERNEL_ERROR_ENOLCK          (0x8002004D)
#define ORBIS_KERNEL_ERROR_ENOSYS          (0x8002004E)
#define ORBIS_KERNEL_ERROR_EFTYPE          (0x8002004F)
#define ORBIS_KERNEL_ERROR_EAUTH           (0x80020050)
#define ORBIS_KERNEL_ERROR_ENEEDAUTH       (0x80020051)
#define ORBIS_KERNEL_ERROR_EIDRM           (0x80020052)
#define ORBIS_KERNEL_ERROR_ENOMSG          (0x80020053)
#define ORBIS_KERNEL_ERROR_EOVERFLOW       (0x80020054)
#define ORBIS_KERNEL_ERROR_ECANCELED       (0x80020055)
#define ORBIS_KERNEL_ERROR_EILSEQ          (0x80020056)
#define ORBIS_KERNEL_ERROR_ENOATTR         (0x80020057)
#define ORBIS_KERNEL_ERROR_EDOOFUS         (0x80020058)
#define ORBIS_KERNEL_ERROR_EBADMSG         (0x80020059)
#define ORBIS_KERNEL_ERROR_EMULTIHOP       (0x8002005A)
#define ORBIS_KERNEL_ERROR_ENOLINK         (0x8002005B)
#define ORBIS_KERNEL_ERROR_EPROTO          (0x8002005C)
#define ORBIS_KERNEL_ERROR_ENOTCAPABLE     (0x8002005D)
#define ORBIS_KERNEL_ERROR_ECAPMODE        (0x8002005E)
#define ORBIS_KERNEL_ERROR_ENOBLK          (0x8002005F)
#define ORBIS_KERNEL_ERROR_EICV            (0x80020060)
#define ORBIS_KERNEL_ERROR_ENOPLAYGOENT    (0x80020061)

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