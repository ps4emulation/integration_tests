#include "CppUTest/TestHarness.h"

#include <list>
#include <stdio.h>
#include <string>
#include <unistd.h>

extern "C" {
// Direct memory functions
uint64_t sceKernelGetDirectMemorySize();
int32_t  sceKernelEnableDmemAliasing();
int32_t  sceKernelAvailableDirectMemorySize(int64_t start, int64_t end, uint64_t alignment, int64_t* phys_addr, uint64_t* size);
int32_t  sceKernelAllocateDirectMemory(int64_t start, int64_t end, uint64_t size, uint64_t alignment, int32_t type, int64_t* phys_addr);
int32_t  sceKernelAllocateMainDirectMemory(uint64_t size, uint64_t alignment, int32_t type, int64_t* phys_addr);
int32_t  sceKernelMapDirectMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags, int64_t phys_addr, uint64_t alignment);
int32_t  sceKernelMapDirectMemory2(uint64_t* addr, uint64_t size, int32_t type, int32_t prot, int32_t flags, int64_t phys_addr, uint64_t alignment);
int32_t  sceKernelGetDirectMemoryType(int64_t phys_addr, int32_t* type, int64_t* start, int64_t* end);
int32_t  sceKernelCheckedReleaseDirectMemory(int64_t phys_addr, uint64_t size);
int32_t  sceKernelReleaseDirectMemory(int64_t phys_addr, uint64_t size);

// Flexible memory functions
int32_t sceKernelAvailableFlexibleMemorySize(uint64_t* size);
int32_t sceKernelMapFlexibleMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags);
int32_t sceKernelReleaseFlexibleMemory(uint64_t* addr, uint64_t size);

// Reserve memory function
int32_t sceKernelReserveVirtualRange(uint64_t* addr, uint64_t size, int32_t flags, uint64_t alignment);

// Generic memory functions
int32_t sceKernelMmap(uint64_t addr, uint64_t size, int32_t prot, int32_t flags, int32_t fd, int64_t offset, uint64_t* out_addr);
int32_t sceKernelMunmap(uint64_t addr, uint64_t size);
int32_t sceKernelMprotect(uint64_t addr, uint64_t size, int32_t prot);
int32_t sceKernelMtypeprotect(uint64_t addr, uint64_t size, int32_t mtype, int32_t prot);
int32_t sceKernelQueryMemoryProtection(uint64_t addr, uint64_t* start, uint64_t* end, int32_t* prot);
int32_t sceKernelVirtualQuery(uint64_t addr, int32_t flags, void* info, uint64_t info_size);

// Memory pool functions
int32_t sceKernelMemoryPoolExpand(int64_t start, int64_t end, uint64_t len, uint64_t alignment, int64_t* phys_addr);
int32_t sceKernelMemoryPoolReserve(uint64_t addr_in, uint64_t len, uint64_t alignment, int32_t flags, uint64_t addr_out);
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

TEST_GROUP(MemoryTests) {void setup() {

} void teardown() {

}};

// These tests are at the top of the file so they run last.
// This is to prevent issues related to sceKernelEnableDmemAliasing, which afaik cannot be undone without direct syscall usage.
TEST(MemoryTests, MapMemoryTest) {
  // Most memory functions in libkernel rely on sceKernelMmap,
  // leading to overlap in some edge cases.
  // Start with testing libkernel's error returns for sceKernelMapFlexibleMemory
  // If size is less than page size, or size is not page aligned, return EINVAL
  uint64_t addr   = 0;
  int32_t  result = sceKernelMapFlexibleMemory(&addr, 0x5000, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapFlexibleMemory(&addr, 0x2000, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Protection should be a bitwise-or'ed combination of flags. It can be zero,
  // or include Read (1) | Write (2) | Execute (4) | GpuRead (0x10) | GpuWrite (0x20).
  // If a value is inputted for protection that doesn't match those flags, it returns EINVAL.
  result = sceKernelMapFlexibleMemory(&addr, 0x4000, 8, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapFlexibleMemory(&addr, 0x4000, 64, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // The only permitted flags are: Fixed (0x10) | NoOverwrite (0x80) | NoCoalesce (0x400000)
  // If flags is non-zero, and includes bits not contained in that list, then it returns EINVAL.
  result = sceKernelMapFlexibleMemory(&addr, 0x4000, 0, 1);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapFlexibleMemory(&addr, 0x4000, 0, 0x100);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapFlexibleMemory(&addr, 0x4000, 0, 0x2000);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Other sceKernelMapFlexibleMemory Notes:
   * If address is not specified, it defaults to 0x200000000
   * Not possible to test here, but there's a condition it follows that instead defaults to 0x880000000
   * On SDK version < 1.70, flag Fixed with unspecified address would remove the Fixed flag.
   */

  // Now for edge cases in sceKernelMapDirectMemory. This function allows flags:
  // Fixed (0x10) | NoOverwrite (0x80) | DmemCompat (0x400) | Sanitizer (0x200000) | NoCoalesce (0x400000)
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 1, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0x100, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0x2000, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Notes:
   * Flag DmemCompat is exclusive to libkernel code, mmap flag 0x400 is Stack.
   * For games compiled with sdk version < 2.50, DmemCompat does nothing.
   * For newer titles, it will redirect sceKernelMapDirectMemory to sceKernelMapDirectMemory2.
   * We can check for edge cases specific to the latter case when testing sys_mmap_dmem later.
   */

  // Protection should be a bitwise-or'ed combination of flags. It can be zero,
  // or include Read (1) | Write (2) | Execute (4) | GpuRead (0x10) | GpuWrite (0x20).
  // If a value is inputted for protection that doesn't match those flags, it returns EINVAL.
  // Note that the execute flag causes mmap to fail, that will come up during later edge case tests.
  result = sceKernelMapDirectMemory(&addr, 0x4000, 8, 0, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapDirectMemory(&addr, 0x4000, 64, 0, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Note:
   * The execute protection causes sceKernelMapDirectMemory calls to fail.
   * This will likely come up in sys_mmap tests.
   */

  // Input address, length, phys_addr, and alignment all must be page-aligned.
  addr   = (uint64_t)0x200002000;
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  addr   = 0x200000000;
  result = sceKernelMapDirectMemory(&addr, 0x2000, 0, 0, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0, 0x2000, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0, 0, 0x2000);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Additionally, alignment must be a power of two.
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0, 0, 0xc000);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Finally, alignment must be less than 0x100000000
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0, 0, 0x100000000);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Other sceKernelMapDirectMemory Notes:
   * If address is not specified, it defaults to 0x200000000
   * Not possible to test here, but there's a condition it follows that instead defaults to 0x880000000
   * On SDK version < 1.70, flag Fixed with unspecified address would remove the Fixed flag.
   */

  // Now for sceKernelReserveVirtualRange edge cases.
  // Alignment must be power of two
  addr   = 0x200000000;
  result = sceKernelReserveVirtualRange(&addr, 0x4000, 0, 0xc000);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Flags can only be Fixed, NoOverwrite, and NoCoalesce
  result = sceKernelReserveVirtualRange(&addr, 0x4000, 0x400, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelReserveVirtualRange(&addr, 0x4000, 0x1000, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Address input must be aligned.
  addr   = 0x200002000;
  result = sceKernelReserveVirtualRange(&addr, 0x4000, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // length input must be aligned.
  addr   = 0x200000000;
  result = sceKernelReserveVirtualRange(&addr, 0x2000, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Alignment input must be aligned.
  result = sceKernelReserveVirtualRange(&addr, 0x4000, 0, 0x2000);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Alignment input must be less than 0x100000000
  result = sceKernelReserveVirtualRange(&addr, 0x4000, 0, 0x100000000);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Other notes for sceKernelReserveVirtualRange:
   * If Fixed flag is specified and address is null, sdk ver 1.70 and below removes flag fixed.
   * Games compiled for newer firmwares will instead return EINVAL.
   */

  // Now for sys_mmap edge cases.
  // To test each of these, I'll use sceKernelMmap (which is just a wrapper for the syscall that converts errors to ORBIS errors)
  // If sys_mmap is called with the Sanitizer flag, then non-devkit consoles return EINVAL.
  addr              = 0x200000000;
  uint64_t addr_out = 0;

  // For this test call, use flags Sanitizer (0x200000) | System (0x2000) | Anon (0x1000) | NoOverwrite (0x80) | Fixed (0x10)
  // This combination of flags is what sceKernelMapSanitizerShadowMemory uses.
  addr   = 0x300000000;
  result = sceKernelMmap(addr, 0x4000, 0, 0x203090, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If length == 0, return EINVAL
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0, 0, 0x1000, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If flags include Void (0x100) or Anon (0x1000), and phys_addr is non-zero or fd is not -1, return EINVAL.
  // Note: Void flag means this is reserving memory, sceKernelReserveVirtualRange uses it internally.
  result = sceKernelMmap(addr, 0x4000, 0, 0x1000, -1, 0x4000, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMmap(addr, 0x4000, 0, 0x1000, 0, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMmap(addr, 0x4000, 0, 0x100, -1, 0x4000, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMmap(addr, 0x4000, 0, 0x100, 0, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Successful call with anon flag for comparison.
  result = sceKernelMmap(addr, 0x4000, 0, 0x1000, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  // Successful call with void flag for comparison.
  result = sceKernelMmap(addr, 0x4000, 0, 0x100, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  // If flags include Stack (0x400) and prot does not include CpuReadWrite or fd is not -1, return EINVAL.
  addr   = 0xfc00000000;
  result = sceKernelMmap(addr, 0x4000, 0, 0x400, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, 0, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Seems like stack flag acts as if fixed | no overwrite are both present?
  addr   = 0xfb00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xfc00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0x200400000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0x200004000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  /**
   * Other notes here:
   * Stack and Void flags append Anon flag internally.
   * Stack, Anon, and Void flags skip fget_mmap call (which gets the file object for the fd input)
   * If Fixed is not specified and address is 0, then it's set to 0x200000000
   * Length parameter is aligned up to the nearest page.
   */

  // If flag Fixed (0x10) is specified, address must have the same offset as phys_addr.
  // This effectively prevents misaligned mappings, unless you manually opened a dmem file yourself.
  addr   = 0x200002000;
  result = sceKernelMmap(addr, 0x4000, 0, 0x1010, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Notes:
   * This is where sys_mmap calls fget_mmap to get a file for mmap.
   * I still need to decompile everything here, especially the switch case for file type.
   * Insert error cases here if there are any errors that I can expose through homebrew.
   */

  // If an unopened file descriptor is specified, then returns EBADF.
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x4000, 1, 0, 100, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EBADF, result);

  int32_t fd = sceKernelOpen("/app0/assets/misc/test_file.txt", 0, 0666);
  CHECK(fd > 0);

  // This sceKernelMmap should try mapping the file to memory with read-write permissions.
  // Due to the flags, writing to this memory will likely fail (but that's to test when done with edge cases)
  int64_t phys_addr = 0;
  result            = sceKernelAllocateMainDirectMemory(0x4000, 0, 0, &phys_addr);
  CHECK_EQUAL(0, result);

  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  result = sceKernelMmap(addr, 0x4000, 7, 0, fd, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  // Offset is where in the file we want to map.
  // The file is ~500KB large, so any phys_addr produced by the above sceKernelAllocateMainDirectMemory call should work.
  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, phys_addr, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x4000);
  CHECK_EQUAL(0, result);

  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);

  // Try file mmap with read-write file
  fd = sceKernelOpen("/download0/test_file.txt", 0x602, 0666);
  CHECK(fd > 0);

  // Write empty data to the file
  char buf[0x10000];
  memset(buf, 0, sizeof(buf));
  result = sceKernelWrite(fd, buf, 0x8000);
  CHECK_EQUAL(0x8000, result);

  // Read only file mmap for read-write file
  result = sceKernelMmap(addr, 0x4000, 1, 0, fd, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  // Read-write file mmap on read-write file
  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  // Read-write-execute file mmap on read-write file
  result = sceKernelMmap(addr, 0x4000, 7, 0, fd, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  // file mmap with offset == file size.
  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0x8000, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // file mmap with offset + size greater than file size
  result = sceKernelMmap(addr, 0x8000, 3, 0, fd, 0x4000, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // file mmap with offset 0 + size greater than file size
  result = sceKernelMmap(addr, 0x10000, 3, 0, fd, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);

  fd = sceKernelOpen("/download0/test_file.txt", 0x0, 0666);
  CHECK(fd > 0);

  // Read-only file mmap on read-only file
  result = sceKernelMmap(addr, 0x4000, 1, 0, fd, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  // Read-write file mmap on read-only file
  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  // Read-write-execute file mmap on read-only file
  result = sceKernelMmap(addr, 0x4000, 7, 0, fd, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  // file mmap with valid offset.
  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0x4000, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  // file mmap with offset == file size.
  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0x8000, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // file mmap with offset + size greater than file size
  result = sceKernelMmap(addr, 0x8000, 3, 0, fd, 0x4000, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // file mmap with offset 0 + size greater than file size
  result = sceKernelMmap(addr, 0x10000, 3, 0, fd, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);

  // Try file mmap with read-write file
  fd = sceKernelOpen("/download0/test_file.txt", 0x602, 0666);
  CHECK(fd > 0);

  // Write empty data to the file
  result = sceKernelWrite(fd, buf, 0xf000);
  CHECK_EQUAL(0xf000, result);

  // Checks for file size in both size and offset parameters are aligned up.
  // Here, we shouldn't see errors unless I try size + offset > 0x10000
  // file mmap with offset == file size.
  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0x10000, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // file mmap with offset + size greater than file size
  result = sceKernelMmap(addr, 0xc000, 3, 0, fd, 0x8000, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // file mmap with offset 0 + size greater than file size
  result = sceKernelMmap(addr, 0x14000, 3, 0, fd, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // file mmap with size + offset greater than file size, but less than page-aligned file size.
  result = sceKernelMmap(addr, 0x8000, 3, 0, fd, 0x8000, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x8000);
  CHECK_EQUAL(0, result);

  // file mmap with size greater than file size, but less than page-aligned file size.
  result = sceKernelMmap(addr, 0x10000, 3, 0, fd, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x10000);
  CHECK_EQUAL(0, result);

  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);

  // file mmap on directories fails with EINVAL
  fd = sceKernelOpen("/download0", 0, 0666);
  CHECK(fd > 0);

  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);

  /**
   * Notes:
   * File mmaps to GPU cause the PS4 to crash, this should get investigated.
   * Somewhere along the line, mapping with flag stack fails. Probably some stack-specific flag check later on?
   *
   * Based on decompilation, Sanitizer flag with a valid address (below a hardcoded 0x800000000000) restricts prot here.
   * Specifically, if address input > 0xfc00000000, prot is restricted to GpuReadWrite.
   * If address input is still zero here (Fixed flag with null address input?), then address defaults to 0xfc00000000.
   * sys_mmap calls vm_mmap2 with offset aligned down to the nearest page.
   */

  /**
   * Notes for vm_mmap2:
   * Starts with a check for size == 0, returning EINVAL. This can't be tested because sys_mmap had an earlier return for this case.
   * Then checks if our mapping will fit in the virtual memory limit? (if not return ENOMEM)
   * Then checks if phys_addr is aligned, returning EINVAL if it isn't. sys_mmap aligned phys_addr for us though.
   * If flags Fixed is specified, and address input is misaligned, return EINVAL. Again, sys_mmap checked this for us.
   * A bunch of checks specific to file mmaps, most of which are un-important for homebrew testing (mostly internal stuff like devices)
   */

  /**
   * Notes for sceKernelMapDirectMemory2:
   * Alignment must be a power of 2, page aligned, and less than 0x100000000
   * If alignment is less than page size, it's increased to the page size.
   * Remaining behavior seems to rely on checks in sys_mmap_dmem.
   */
  addr = 0x200000000;
  // Not power of 2
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0, 0, 0xc000);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  // Not page aligned
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0, 0, 0x100);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  // Too large.
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0, 0, 0x100000000);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Notes for sys_mmap_dmem:
   * Starts with parameter checks.
   * Address, phys_addr, and length must be page aligned. Length = 0 is not allowed.
   * Flags must only contain Fixed, NoOverwrite, Sanitizer, NoCoalesce, and the alignment bits.
   * Prot contains only CpuRead, CpuWrite, GpuRead, or GpuWrite.
   * mtype must be less than 10
   * If any of these error checks fail, the function returns EINVAL.
   *
   * sys_mmap_dmem also follows the same address input logic as sys_mmap.
   * If address is null and map Fixed is specified, returns EINVAL.
   *
   * sys_mmap_dmem has an extra check for physical addresses overlapping.
   * This check is removed if sceKernelEnableDmemAliasing is called.
   */

  // To run sys_mmap_dmem without worries from phys addresses, allocate some dmem
  phys_addr = 0;
  result    = sceKernelAllocateMainDirectMemory(0x4000, 0, 0, &phys_addr);
  CHECK_EQUAL(0, result);

  // Misaligned addr
  addr   = 0x200002000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Misaligned phys_addr
  addr   = 0x200000000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0, phys_addr - 0x2000, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Misaligned length
  result = sceKernelMapDirectMemory2(&addr, 0x2000, 0, 1, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Zero length
  result = sceKernelMapDirectMemory2(&addr, 0, 0, 1, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Invalid flags
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 1, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Invalid prot
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 0x7, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 0x8, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Invalid mtype
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 11, 1, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // -1 mtype skips assigning type.
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -1, 1, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x4000);
  CHECK_EQUAL(0, result);

  // Lower negatives are invalid.
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -2, 1, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Map Sanitizer
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0x200000, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // null address with map fixed
  addr   = 0;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0x10, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x4000);
  CHECK_EQUAL(0, result);

  // Overlapping phys addr (relying on libSceGnmDriver mapping)
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EBUSY, result);

  // Try enabling dmem aliasing
  result = sceKernelEnableDmemAliasing();
  CHECK_EQUAL(0, result);

  // Overlapping phys addr works now.
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0, 0, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x4000);
  CHECK_EQUAL(0, result);

  // Might continue down this rabbit hole at some point, but I don't see too much of a point?
  // I've gleamed the error cases they start with, everything else will just come from testing, as always.
}

TEST(MemoryTests, AvailableDirectMemoryTest) {
  // Not exactly a way for this one to fail. Just need it to test other functions.
  uint64_t dmem_size = sceKernelGetDirectMemorySize();
  CHECK(dmem_size != 0);

  // Test sceKernelAvailableDirectMemorySize.
  // Typical use case:
  int64_t  phys_addr = 0;
  uint64_t size      = 0;
  int32_t  result    = sceKernelAvailableDirectMemorySize(0, dmem_size, 0, &phys_addr, &size);
  // Check for success
  CHECK_EQUAL(0, result);
  // Check for known quirks on success.
  // Specifically, the dmem map already has a mapping (which comes from libSceGnmDriver).
  // Many games are built with this assumption in mind, though most don't need it emulated.
  // This mapping will be from physical addresses 0 to 0x10000 under these circumstances.
  CHECK_EQUAL(0x10000, phys_addr);
  CHECK_EQUAL(dmem_size - 0x10000, size);

  // Now test for potential edge cases.
  // Based on decompilation, this function should accept both phys_addr and size of nullptr.
  result = sceKernelAvailableDirectMemorySize(0, dmem_size, 0, nullptr, nullptr);
  CHECK_EQUAL(0, result);

  // If there is no size to output, this function returns ENOMEM
  result = sceKernelAvailableDirectMemorySize(0, 0, 0, nullptr, nullptr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // The alignment check is very lenient, they run (align - 1) & align to check.
  // While this alignment value is far lower than the usually acceptable amount, this still works.
  result = sceKernelAvailableDirectMemorySize(0, dmem_size, 0x100, nullptr, nullptr);
  CHECK_EQUAL(0, result);

  // If you use a value that triggers the check, it returns EINVAL
  result = sceKernelAvailableDirectMemorySize(0, dmem_size, 0x11111, nullptr, nullptr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // There is no specific error for start > end, it still returns ENOMEM.
  result = sceKernelAvailableDirectMemorySize(dmem_size, 0, 0, nullptr, nullptr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // There's also no specific error for the range being outside the dmem map, it returns ENOMEM.
  result = sceKernelAvailableDirectMemorySize(dmem_size, dmem_size * 2, 0, nullptr, nullptr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  result = sceKernelAvailableDirectMemorySize(dmem_size * 5, dmem_size * 6, 0, nullptr, nullptr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // Start and end phys addresses are limited internally, with no error return for them.
  // Make sure size is appropriately restricted.
  result = sceKernelAvailableDirectMemorySize(0, dmem_size * 5, 0, nullptr, &size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(dmem_size - 0x10000, size);

  // phys_addr and size are restricted based on the start and end.
  // So in a case like this, where start is after the start of the free dmem area, size and phys_addr are restricted.
  result = sceKernelAvailableDirectMemorySize(dmem_size / 2, dmem_size, 0, &phys_addr, &size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(dmem_size / 2, phys_addr);
  CHECK_EQUAL(dmem_size / 2, size);

  // phys_addr and size are restricted based on the start and end.
  // So in a case like this, where end is before the end of the free dmem area, size and phys_addr are restricted.
  result = sceKernelAvailableDirectMemorySize(0, dmem_size / 2, 0, &phys_addr, &size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0x10000, phys_addr);
  CHECK_EQUAL((dmem_size / 2) - 0x10000, size);

  // start is rounded up to alignment.
  result = sceKernelAvailableDirectMemorySize(0x1c001, dmem_size, 0, &phys_addr, &size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0x20000, phys_addr);
  CHECK_EQUAL(dmem_size - 0x20000, size);

  // end ignores alignment
  result = sceKernelAvailableDirectMemorySize(0x10000, dmem_size - 0x3fff, 0, &phys_addr, &size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0x10000, phys_addr);
  CHECK_EQUAL(dmem_size - 0x13fff, size);

  // Alignments below the system requirements are rounded up to the system-wide page size of 0x4000.
  // This can be tested with the search start, as that is rounded up to alignment.
  result = sceKernelAvailableDirectMemorySize(0x1c001, dmem_size, 0x100, &phys_addr, &size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0x20000, phys_addr);
  CHECK_EQUAL(dmem_size - 0x20000, size);

  // phys_addr is aligned up to alignment, size is aligned down to compensate.
  result = sceKernelAvailableDirectMemorySize(0, dmem_size, 0x20000, &phys_addr, &size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0x20000, phys_addr);
  CHECK_EQUAL(dmem_size - 0x20000, size);

  // returned statistics are always from the largest area of free space.
  // Do an allocation in the middle of the dmem map to demonstrate this.
  int64_t phys_addr_out = 0;
  result                = sceKernelAllocateDirectMemory(dmem_size / 2, dmem_size, 0x100000, 0, 0, &phys_addr_out);
  CHECK_EQUAL(0, result);
  // Nothing else in that area of the dmem map, so this should be the returned phys address.
  CHECK_EQUAL(dmem_size / 2, phys_addr_out);

  result = sceKernelAvailableDirectMemorySize(0, dmem_size, 0, &phys_addr, &size);
  CHECK_EQUAL(0, result);
  // The larger area should be from 0x10000 to dmem_size / 2 here.
  CHECK_EQUAL(0x10000, phys_addr);
  CHECK_EQUAL((dmem_size / 2) - 0x10000, size);

  // Release the direct memory used for testing this, we want a clean dmem_map.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr_out, 0x100000);
  CHECK_EQUAL(0, result);

  // Free mappings should coalesce, make sure sceKernelAvailableDirectMemorySize still returns the expected values.
  result = sceKernelAvailableDirectMemorySize(0, dmem_size, 0, &phys_addr, &size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0x10000, phys_addr);
  CHECK_EQUAL(dmem_size - 0x10000, size);
}

TEST(MemoryTests, AllocateDirectMemoryTest) {
  // Test sceKernelAllocateDirectMemory
  // Start with the typical working case.
  uint64_t dmem_size = sceKernelGetDirectMemorySize();
  int64_t  phys_addr = 0;
  int32_t  result    = sceKernelAllocateDirectMemory(0, dmem_size, 0x10000, 0, 0, &phys_addr);
  CHECK_EQUAL(0, result);
  // Due to libSceGnmDriver, the first phys_addr we expect to see from sceKernelAllocateDirectMemory is 0x10000.
  CHECK_EQUAL(0x10000, phys_addr);

  // Keep dmem_map clean.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // If either start or end is less than zero, returns EINVAL
  phys_addr = 0;
  result    = sceKernelAllocateDirectMemory(-1, dmem_size, 0x10000, 0, 0, &phys_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelAllocateDirectMemory(0, -1, 0x10000, 0, 0, &phys_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If length is zero, returns EINVAL
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0, 0, 0, &phys_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If there is more than one active bit in align, returns EINVAL
  // 0xc000 is page-aligned, but should fail this.
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0x10000, 0xc000, 0, &phys_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If alignment or length is not page aligned, returns EINVAL
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0x1000, 0x10000, 0, &phys_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0x10000, 0x1000, 0, &phys_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If type > 10, return EINVAL
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0x10000, 0, 11, &phys_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If type < 0, return EINVAL
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0x10000, 0, -1, &phys_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If limited end <= start, return EAGAIN
  result = sceKernelAllocateDirectMemory(0x100000, 0x10000, 0x10000, 0, 0, &phys_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EAGAIN, result);

  // If limited end < length, return EAGAIN
  result = sceKernelAllocateDirectMemory(0, 0x100000, 0x200000, 0, 0, &phys_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EAGAIN, result);

  // If limited end - length < start, return EAGAIN
  result = sceKernelAllocateDirectMemory(0x10000, 0x40000, 0x40000, 0, 0, &phys_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EAGAIN, result);

  // There is no check for if phys_addr is null in firmware. This call WILL crash the application.
  // result = sceKernelAllocateDirectMemory(0, dmem_size, 0x20000, 0, 0, nullptr);

  // Now on to edge cases that don't cause immediate returns.
  // This call fails with EAGAIN, since there's no direct memory in that area (libSceGnmDriver took it)
  result = sceKernelAllocateDirectMemory(0, 0x10000, 0x10000, 0, 0, &phys_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EAGAIN, result);

  // phys_addr will be aligned to alignment.
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0x20000, 0x20000, 0, &phys_addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0x20000, phys_addr);

  // Keep dmem_map clean.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x20000);
  CHECK_EQUAL(0, result);

  // size is not affected by alignment
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0x10000, 0x20000, 0, &phys_addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0x20000, phys_addr);

  // Run sceKernelGetDirectMemoryType to confirm mapping dimensions.
  int32_t out_type;
  int64_t out_start;
  int64_t out_end;
  result = sceKernelGetDirectMemoryType(phys_addr, &out_type, &out_start, &out_end);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(phys_addr, out_start);
  CHECK_EQUAL(phys_addr + 0x10000, out_end);

  // Keep dmem_map clean.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // If start is later than the start of the free pages, phys_addr = start
  result = sceKernelAllocateDirectMemory(dmem_size / 2, dmem_size, 0x10000, 0, 0, &phys_addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(dmem_size / 2, phys_addr);

  // Keep dmem_map clean.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // If align would force phys_addr up to a mapped page, then that area is unsuitable.
  result = sceKernelAllocateDirectMemory(0x20000, dmem_size, 0x4000, 0, 0, &phys_addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0x20000, phys_addr);
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0x20000, 0x20000, 0, &phys_addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0x40000, phys_addr);

  // Keep dmem_map clean.
  result = sceKernelCheckedReleaseDirectMemory(0x20000, 0x4000);
  CHECK_EQUAL(0, result);
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x20000);
  CHECK_EQUAL(0, result);

  // Firmware doesn't align search start, which allows this call to create a misaligned dmem page.
  // A notable side effect of this behavior is crashing the entire console.
  // result = sceKernelAllocateDirectMemory(0x10001, dmem_size, 0x4000, 0, 0, &phys_addr);
}

TEST(MemoryTests, ReleaseDirectMemoryTest) {
  // Both checked and unchecked return an error if addr or length aren't aligned to page size.
  int64_t phys_addr = 0x10000;
  int32_t result    = sceKernelReleaseDirectMemory(phys_addr, 0x3000);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelReleaseDirectMemory(phys_addr + 0x3000, 0x10000);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Releasing free dmem causes error ENOENT on checked release
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOENT, result);

  // Releasing free dmem "succeeds" on unchecked release
  result = sceKernelReleaseDirectMemory(phys_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // Allocate a little dmem to test with
  result = sceKernelAllocateMainDirectMemory(0x10000, 0x20000, 0, &phys_addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0x20000, phys_addr);

  // Now we've got 0 - 0x10000 mapped by libSceGnmDriver, and 0x20000 - 0x30000 allocated.
  // Checked fails if anything in the range is free.
  result = sceKernelCheckedReleaseDirectMemory(0x10000, 0x30000);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOENT, result);
  result = sceKernelCheckedReleaseDirectMemory(0x20000, 0x20000);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOENT, result);

  // Because of how checked fails, the actual mapping is safe.
  int32_t out_type;
  int64_t out_start;
  int64_t out_end;
  result = sceKernelGetDirectMemoryType(phys_addr, &out_type, &out_start, &out_end);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(phys_addr, out_start);
  CHECK_EQUAL(phys_addr + 0x10000, out_end);

  // Unchecked will deallocate the mapping perfectly fine.
  result = sceKernelReleaseDirectMemory(0x10000, 0x30000);
  CHECK_EQUAL(0, result);

  // As a result of the unchecked release, this sceKernelGetDirectMemoryType call fails.
  result = sceKernelGetDirectMemoryType(phys_addr, &out_type, &out_start, &out_end);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOENT, result);

  // Release should unmap any mappings that reference the physical block.
  // Allocate a little dmem to test with
  result = sceKernelAllocateMainDirectMemory(0x10000, 0x20000, 0, &phys_addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0x20000, phys_addr);

  uint64_t addr = 0;
  result        = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);

  result = sceKernelReleaseDirectMemory(0x10000, 0x30000);
  CHECK_EQUAL(0, result);

  uint64_t start_addr;
  uint64_t end_addr;
  int32_t  out_prot;
  result = sceKernelQueryMemoryProtection(addr, &start_addr, &end_addr, &out_prot);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);
}

// Test for various edge cases related to file mmaps
TEST(MemoryTests, FileMappingTest) {
  // Start with behaviors of read-only mmaps
  // Open test file
  int32_t fd = sceKernelOpen("/app0/assets/misc/test_file.txt", 0, 0666);
  CHECK(fd > 0);

  // Run a sceKernelRead to read from the file directly.
  char    read_buf[0x40000];
  int64_t bytes_read = sceKernelRead(fd, read_buf, sizeof(read_buf));
  CHECK_EQUAL(sizeof(read_buf), bytes_read);

  // mmap file to memory
  // Using flags 0, prot read-write.
  uint64_t addr        = 0;
  uint64_t output_addr = 0;
  uint64_t offset      = 0;
  int32_t  result      = sceKernelMmap(addr, 0x10000, 3, 0, fd, offset, &output_addr);
  CHECK_EQUAL(0, result);

  // With this mmap, we should be able to read from the file.
  // Run a memcpy to copy from this memory to a buffer.
  char mem_buf[0x10000];
  memcpy(mem_buf, reinterpret_cast<void*>(output_addr), sizeof(mem_buf));

  // Compare memory buffer to file contents.
  result = memcmp(mem_buf, &read_buf[offset], sizeof(mem_buf));
  CHECK_EQUAL(0, result);

  // Write zeros to the memory area, then read back the new data.
  memset(reinterpret_cast<void*>(output_addr), 0, sizeof(mem_buf));
  memcpy(mem_buf, reinterpret_cast<void*>(output_addr), sizeof(mem_buf));

  // Re-read file contents
  bytes_read = sceKernelLseek(fd, 0, 0);
  CHECK_EQUAL(0, bytes_read);
  bytes_read = sceKernelRead(fd, read_buf, sizeof(read_buf));
  CHECK_EQUAL(sizeof(read_buf), bytes_read);

  // Compare memory buffer to file contents.
  result = memcmp(mem_buf, &read_buf[offset], sizeof(mem_buf));
  // File contents should not be modified by the memory write.
  CHECK(result != 0);

  // Unmap test memory
  result = sceKernelMunmap(output_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // mmap with offset
  offset = 0x4000;
  result = sceKernelMmap(addr, 0x10000, 3, 0, fd, offset, &output_addr);
  CHECK_EQUAL(0, result);

  // Run a memcpy to copy from this memory to a buffer.
  memcpy(mem_buf, reinterpret_cast<void*>(output_addr), sizeof(mem_buf));

  // Compare memory buffer to file contents.
  result = memcmp(mem_buf, &read_buf[offset], sizeof(mem_buf));
  CHECK_EQUAL(0, result);

  // Unmap test memory
  result = sceKernelMunmap(output_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // Check for alignment behavior.
  offset = 0x6000;
  result = sceKernelMmap(addr, 0xe000, 3, 0, fd, offset, &output_addr);
  CHECK_EQUAL(0, result);

  // Size should round up to 0x10000, offset should round down to 0x4000.
  uint64_t start_addr;
  uint64_t end_addr;
  int32_t  prot;
  result = sceKernelQueryMemoryProtection(output_addr, &start_addr, &end_addr, &prot);
  CHECK_EQUAL(0, result);
  // mmap's address output is increased by page offset.
  // This is not reflected in the actual mapping.
  CHECK_EQUAL(output_addr - 0x2000, start_addr);
  CHECK_EQUAL(output_addr + 0xe000, end_addr);
  CHECK_EQUAL(3, prot);

  // Run virtual query to check details
  // Need to define struct first.
  struct OrbisKernelVirtualQueryInfo {
    uint64_t start;
    uint64_t end;
    int64_t  offset;
    int32_t  prot;
    int32_t  memory_type;
    uint8_t  is_flexible  : 1;
    uint8_t  is_direct    : 1;
    uint8_t  is_stack     : 1;
    uint8_t  is_pooled    : 1;
    uint8_t  is_committed : 1;
    char     name[32];
  };

  OrbisKernelVirtualQueryInfo info;
  memset(&info, 0, sizeof(info));
  result = sceKernelVirtualQuery(output_addr, 0, &info, sizeof(info));
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(start_addr, info.start);
  CHECK_EQUAL(end_addr, info.end);
  // VirtualQuery does not report file mmap offsets.
  CHECK_EQUAL(0, info.offset);
  CHECK_EQUAL(prot, info.prot);
  CHECK_EQUAL(1, info.is_flexible);
  CHECK_EQUAL(0, info.is_direct);
  CHECK_EQUAL(0, info.is_stack);
  CHECK_EQUAL(0, info.is_pooled);
  CHECK_EQUAL(1, info.is_committed);

  // Run a memcpy to copy from this memory to a buffer.
  memcpy(mem_buf, reinterpret_cast<void*>(start_addr), sizeof(mem_buf));

  // Compare memory buffer to file contents.
  result = memcmp(mem_buf, &read_buf[offset - 0x2000], sizeof(mem_buf));
  CHECK_EQUAL(0, result);

  // Unmap test memory
  result = sceKernelMunmap(start_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // Close file
  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);

  // Open test file
  fd = sceKernelOpen("/download0/test_file.txt", 0x602, 0666);
  CHECK(fd > 0);

  // Run a sceKernelRead on /dev/urandom to get random numbers
  int32_t random_fd = sceKernelOpen("/dev/urandom", 0, 0777);
  CHECK(random_fd > 0);
  bytes_read = sceKernelRead(random_fd, read_buf, sizeof(read_buf));
  CHECK_EQUAL(sizeof(read_buf), bytes_read);

  // Use sceKernelWrite to write the random data to the file.
  bytes_read = sceKernelWrite(fd, read_buf, sizeof(read_buf));
  CHECK_EQUAL(sizeof(read_buf), bytes_read);

  // Seek back to the beginning of the file.
  bytes_read = sceKernelLseek(fd, 0, 0);
  CHECK_EQUAL(0, bytes_read);

  // mmap file to memory
  // Using flags 0, prot read-write.
  addr        = 0;
  output_addr = 0;
  offset      = 0;
  result      = sceKernelMmap(addr, 0x10000, 3, 0, fd, offset, &output_addr);
  CHECK_EQUAL(0, result);

  // Run a memcpy to copy from this memory to a buffer, then compare it to the file data.
  memcpy(mem_buf, reinterpret_cast<void*>(output_addr), sizeof(mem_buf));
  result = memcmp(mem_buf, read_buf, sizeof(mem_buf));
  CHECK_EQUAL(0, result);

  // Unmap test memory
  result = sceKernelMunmap(output_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // Test misaligned file size behavior.
  // This should leave the file contents intact.
  result = sceKernelFtruncate(fd, 0xf000);

  // mmap file to memory
  // Using flags 0, prot read-write.
  result = sceKernelMmap(addr, 0x10000, 3, 0, fd, offset, &output_addr);
  CHECK_EQUAL(0, result);

  // Run sceKernelVirtualQuery to make sure memory area is as expected.
  memset(&info, 0, sizeof(info));
  result = sceKernelVirtualQuery(output_addr, 0, &info, sizeof(info));
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(output_addr, info.start);
  CHECK_EQUAL(output_addr + 0x10000, info.end);
  CHECK_EQUAL(0, info.offset);
  CHECK_EQUAL(3, info.prot);
  CHECK_EQUAL(1, info.is_flexible);
  CHECK_EQUAL(0, info.is_direct);
  CHECK_EQUAL(0, info.is_stack);
  CHECK_EQUAL(0, info.is_pooled);
  CHECK_EQUAL(1, info.is_committed);

  // Run a memcpy to copy from this memory to a buffer, then compare it to the file data.
  memcpy(mem_buf, reinterpret_cast<void*>(output_addr), sizeof(mem_buf));
  // Only compare the parts that are in the file. Outside the file, memory contents differ from old file contents.
  result = memcmp(mem_buf, read_buf, 0xf000);
  CHECK_EQUAL(0, result);

  // Bytes outside the file contents are zeroed.
  char test_buf[0x1000];
  memset(test_buf, 0, 0x1000);
  result = memcmp(&mem_buf[0xf000], test_buf, 0x1000);
  CHECK_EQUAL(0, result);

  // Unmap test memory
  result = sceKernelMunmap(output_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // Alignment behavior allows this call to succeed too, since offset aligns down and size aligns up.
  // Remember that output_addr is misaligned here due to the offset.
  result = sceKernelMmap(addr, 0xf000, 3, 0, fd, 0x1000, &output_addr);
  CHECK_EQUAL(0, result);

  // Run sceKernelVirtualQuery to make sure memory area is as expected.
  memset(&info, 0, sizeof(info));
  result = sceKernelVirtualQuery(output_addr, 0, &info, sizeof(info));
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(output_addr - 0x1000, info.start);
  CHECK_EQUAL(output_addr + 0xf000, info.end);
  CHECK_EQUAL(0, info.offset);
  CHECK_EQUAL(3, info.prot);
  CHECK_EQUAL(1, info.is_flexible);
  CHECK_EQUAL(0, info.is_direct);
  CHECK_EQUAL(0, info.is_stack);
  CHECK_EQUAL(0, info.is_pooled);
  CHECK_EQUAL(1, info.is_committed);

  // Run a memcpy to copy from this memory to a buffer, then compare it to the file data.
  memcpy(mem_buf, reinterpret_cast<void*>(output_addr - 0x1000), sizeof(mem_buf));
  // Only compare the parts that are in the file. Outside the file, memory contents differ from old file contents.
  result = memcmp(mem_buf, read_buf, 0xf000);
  CHECK_EQUAL(0, result);

  // Bytes outside the file contents are zeroed.
  memset(test_buf, 0, 0x1000);
  result = memcmp(&mem_buf[0xf000], test_buf, 0x1000);
  CHECK_EQUAL(0, result);

  // Unmap test memory
  result = sceKernelMunmap(output_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // Now test MAP_SHARED behavior (safely).
  // This flag copies changes from memory to the file.
  // Note that using this flag with read-only files is dangerous, instantly crashes PS4s and causes data corruption.
  result = sceKernelMmap(addr, 0x8000, 3, 1, fd, offset, &output_addr);
  CHECK_EQUAL(0, result);

  // Right now the data we get will be the same random values.
  // Do a memory write to write 0s to the start of the file.
  memset(reinterpret_cast<void*>(output_addr), 0, 0x8000);

  // Lseek to the start of the file, then read the first 0x8000 bytes.
  result = sceKernelLseek(fd, 0, 0);
  CHECK_EQUAL(0, result);
  bytes_read = sceKernelRead(fd, read_buf, 0x8000);
  CHECK_EQUAL(0x8000, bytes_read);

  // Compare the memory to the raw file contents.
  result = memcmp(read_buf, reinterpret_cast<void*>(output_addr), 0x8000);
  CHECK_EQUAL(0, result);

  // Unmap test memory
  result = sceKernelMunmap(output_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // Close test file
  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);

  // Test closing a file while it's mmapped
  fd = sceKernelOpen("/download0/test_file.txt", 0x2, 0666);
  CHECK(fd > 0);

  // mmap file to memory
  result = sceKernelMmap(addr, 0x8000, 3, 0, fd, offset, &output_addr);
  CHECK_EQUAL(0, result);

  // Check file contents
  memset(read_buf, 0, 0x8000);
  bytes_read = sceKernelRead(fd, read_buf, 0x8000);
  CHECK_EQUAL(0x8000, bytes_read);

  // Close test file
  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);

  // Compare the memory to the raw file contents.
  result = memcmp(read_buf, reinterpret_cast<void*>(output_addr), 0x8000);
  CHECK_EQUAL(0, result);

  // Unmap test memory
  result = sceKernelMunmap(output_addr, 0x8000);
  CHECK_EQUAL(0, result);

  // Some devices can be mmapped, these can follow different rules.
  // The easiest device I've found to mmap is /dev/gc.
  fd = sceKernelOpen("/dev/gc", 0x602, 0666);
  CHECK(fd > 0);

  // Perform device file mmap.
  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &output_addr);
  CHECK_EQUAL(0, result);

  // Run sceKernelVirtualQuery to make sure memory area is as expected.
  memset(&info, 0, sizeof(info));
  result = sceKernelVirtualQuery(output_addr, 0, &info, sizeof(info));
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(output_addr, info.start);
  CHECK_EQUAL(output_addr + 0x4000, info.end);
  CHECK_EQUAL(0, info.offset);
  CHECK_EQUAL(3, info.prot);
  CHECK_EQUAL(0, info.is_flexible);
  CHECK_EQUAL(0, info.is_direct);
  CHECK_EQUAL(0, info.is_stack);
  CHECK_EQUAL(0, info.is_pooled);
  CHECK_EQUAL(0, info.is_committed);

  // Device file mmaps are supposed to automatically append MAP_SHARED.
  memset(reinterpret_cast<void*>(output_addr), 1, 0x1000);

  // Read contents of memory into a buffer.
  char dev_buf[0x4000];
  memcpy(dev_buf, reinterpret_cast<void*>(output_addr), 0x4000);

  // Since we can't sceKernelRead our way through this, we can instead check for MAP_SHARED by unmapping, then remapping the file.
  // If changes persist, this memory is likely backed in some form. If not, we can assume MAP_SHARED was not appended.
  result = sceKernelMunmap(output_addr, 0x4000);
  CHECK_EQUAL(0, result);

  // Perform device file mmap.
  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &output_addr);
  CHECK_EQUAL(0, result);

  // Compare the device memory before and after. If the changes from before are still present, the mapping must have MAP_SHARED.
  result = memcmp(reinterpret_cast<void*>(output_addr), dev_buf, 0x4000);
  CHECK_EQUAL(0, result);

  result = sceKernelMunmap(output_addr, 0x4000);
  CHECK_EQUAL(0, result);

  // Device file mmaps fail with EINVAL if you map with flags Private (2)
  result = sceKernelMmap(0, 0x4000, 3, 2, fd, 0, &output_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);
}

TEST(MemoryTests, FlexibleTest) {
  // This test is to validate memory behaviors unique to flexible memory mappings.
  // Start by testing available flexible memory size.
  std::list<uint64_t> addresses;
  uint64_t            addr_out = 0;
  int32_t             result   = 0;
  while (result == 0) {
    result = sceKernelMapFlexibleMemory(&addr_out, 0x4000, 3, 0);
    if (result < 0) {
      // Out of flex mem
      CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
    } else {
      // Mapped flex mem successfully.
      CHECK_EQUAL(0, result);
      // Add the mapping address to addresses, need to unmap later to clean up.
      addresses.emplace_back(addr_out);
    }
  }

  // After all these mappings, available flex size should be 0.
  uint64_t avail_flex_size = 0;
  result                   = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0, avail_flex_size);

  // sceKernelMmap with MAP_ANON uses the flexible budget.
  uint64_t test_addr;
  result = sceKernelMmap(0, 0x4000, 3, 0x1000, -1, 0, &test_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // sceKernelMmap with files also uses the flexible budget.
  int32_t fd = sceKernelOpen("/app0/assets/misc/test_file.txt", 0, 0666);
  CHECK(fd > 0);
  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &test_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);

  // Make sure download dir file has enough space to mmap before trying.
  fd = sceKernelOpen("/download0/test_file.txt", 0x602, 0666);
  CHECK(fd > 0);
  char test_buf[0x100000];
  memset(test_buf, 0, sizeof(test_buf));
  int64_t bytes = sceKernelWrite(fd, test_buf, sizeof(test_buf));
  CHECK_EQUAL(sizeof(test_buf), bytes);

  // mmap download dir file.
  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &test_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);

  // Files in /data don't count towards the budget.
  fd = sceKernelOpen("/data/test_file.txt", 0x602, 0666);
  CHECK(fd > 0);
  memset(test_buf, 0, sizeof(test_buf));
  bytes = sceKernelWrite(fd, test_buf, sizeof(test_buf));
  CHECK_EQUAL(sizeof(test_buf), bytes);

  // mmap data dir file.
  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &test_addr);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(test_addr, 0x4000);
  CHECK_EQUAL(0, result);
  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);

  // Device mmaps do not use the flexible budget.
  fd = sceKernelOpen("/dev/gc", 0x602, 0666);
  CHECK(fd > 0);
  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &test_addr);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(test_addr, 0x4000);
  CHECK_EQUAL(0, result);
  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);

  // mmaps from system folders do not use the flexible budget.
  const char* sys_folder = sceKernelGetFsSandboxRandomWord();
  char        path[128];
  memset(path, 0, sizeof(path));
  snprintf(path, sizeof(path), "/%s/common/lib/libc.sprx", sys_folder);
  fd = sceKernelOpen(path, 0, 0666);
  CHECK(fd > 0);

  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &test_addr);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(test_addr, 0x4000);
  CHECK_EQUAL(0, result);
  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);

  // We can't directly correlate flex size to mapped memory,
  // since my list of addresses will also cut into the flex mem budget.

  // Unmap all of the flexible memory used here.
  for (uint64_t addr: addresses) {
    result = sceKernelMunmap(addr, 0x4000);
    CHECK_EQUAL(0, result);
  }
  addresses.clear();

  // Available flex size should be greater than 0 now
  result = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  CHECK_EQUAL(0, result);
  CHECK(avail_flex_size != 0);

  // Flexible memory is not backed in any way, so the contents are garbage before initializing.
  result = sceKernelMapFlexibleMemory(&addr_out, 0x10000, 3, 0);
  CHECK_EQUAL(0, result);

  // Write 1's to the full memory range.
  memset(reinterpret_cast<void*>(addr_out), 1, 0x10000);

  // Ensure the full range has 1's
  memset(test_buf, 1, sizeof(test_buf));
  result = memcmp(test_buf, reinterpret_cast<void*>(addr_out), 0x10000);
  CHECK_EQUAL(0, result);

  // Unmap the middle of the range
  result = sceKernelMunmap(addr_out + 0x4000, 0x8000);
  CHECK_EQUAL(0, result);

  // Ensure the remaining mapped areas are still filled with 1's
  result = memcmp(test_buf, reinterpret_cast<void*>(addr_out), 0x4000);
  CHECK_EQUAL(0, result);
  result = memcmp(&test_buf[0xc000], reinterpret_cast<void*>(addr_out + 0xc000), 0x4000);
  CHECK_EQUAL(0, result);

  // Remap the unmapped area, use flag fixed to ensure correct placement.
  uint64_t new_addr = addr_out + 0x4000;
  result            = sceKernelMapFlexibleMemory(&new_addr, 0x8000, 3, 0x10);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(addr_out + 0x4000, new_addr);

  // The new memory area will not have the contents of the old memory.
  result = memcmp(&test_buf[0x4000], reinterpret_cast<void*>(new_addr), 0x8000);
  CHECK(result != 0);

  // Re-fill the area with 1's
  memset(reinterpret_cast<void*>(addr_out), 1, 0x10000);
  result = memcmp(test_buf, reinterpret_cast<void*>(addr_out), 0x10000);
  CHECK_EQUAL(0, result);

  // Overwrite the whole memory area, this should destroy the contents within.
  new_addr = addr_out;
  result   = sceKernelMapFlexibleMemory(&new_addr, 0x10000, 3, 0x10);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(new_addr, addr_out);

  result = memcmp(test_buf, reinterpret_cast<void*>(addr_out), 0x10000);
  CHECK(result != 0);

  result = sceKernelMunmap(new_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // Get a base budget value to calculate with.
  uint64_t avail_flex_size_before = 0;
  result                          = sceKernelAvailableFlexibleMemorySize(&avail_flex_size_before);
  CHECK_EQUAL(0, result);
  CHECK(avail_flex_size_before != 0);

  // Note: Seems like allocations are not directly consuming the budget.
  // Using these larger sized mappings allows me to validate budget behavior, but I will need to test this issue.

  // Test sceKernelMapFlexibleMemory to see how it impacts the budget.
  result = sceKernelMapFlexibleMemory(&new_addr, 0x100000, 3, 0x0);
  CHECK_EQUAL(0, result);

  // This should reduce available size by 0x100000.
  result = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(avail_flex_size_before - 0x100000, avail_flex_size);

  // Unmap the memory
  result = sceKernelMunmap(new_addr, 0x100000);
  CHECK_EQUAL(0, result);

  // This should bring available back to normal.
  result = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(avail_flex_size_before, avail_flex_size);

  // Test mmap with MAP_ANON to see how it impacts the budget.
  result = sceKernelMmap(0, 0x100000, 3, 0x1000, -1, 0, &new_addr);
  CHECK_EQUAL(0, result);

  // This should reduce available size by 0x100000.
  result = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(avail_flex_size_before - 0x100000, avail_flex_size);

  // Unmap the memory
  result = sceKernelMunmap(new_addr, 0x100000);
  CHECK_EQUAL(0, result);

  // This should bring available back to normal.
  result = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(avail_flex_size_before, avail_flex_size);

  // Test file mmap from download dir to see how it impacts the budget.
  fd = sceKernelOpen("/download0/test_file.txt", 0, 0666);
  CHECK(fd > 0);
  result = sceKernelMmap(0, 0x100000, 3, 0, fd, 0, &new_addr);
  CHECK_EQUAL(0, result);

  // This should reduce available size by 0x10000.
  result = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(avail_flex_size_before - 0x100000, avail_flex_size);

  // Unmap the memory
  result = sceKernelMunmap(new_addr, 0x100000);
  CHECK_EQUAL(0, result);

  // Close the file
  result = sceKernelClose(fd);
  CHECK_EQUAL(0, result);

  // This should bring available back to normal.
  result = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(avail_flex_size_before, avail_flex_size);
}

TEST(MemoryTests, DirectTest) {
  // This test is to validate memory behaviors somewhat unique to direct memory mappings.
  // This will focus on behavioral edge cases, since all the error checks involved are checked in MapMemoryTest.

  // First, sceKernelMapDirectMemory on physical addresses that are not mapped.
  // Once again, due to the SceGnmDriver mapping, 0 through 0x10000 is allocated.
  uint64_t addr = 0;
  int32_t result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, 0x10000, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);
  CHECK_EQUAL(0, addr);

  // Since it's a different syscall, check sceKernelMapDirectMemory2 as well.
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 0, 3, 0, 0x10000, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);
  CHECK_EQUAL(0, addr);

  // Ensure backing behavior is correct when two mappings share a physical address.
  int64_t phys_addr = 0;
  result = sceKernelAllocateMainDirectMemory(0x10000, 0x10000, 0, &phys_addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0x10000, phys_addr);

  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);

  // While we're here, make sure stored memory info matches expectations for direct memory.
  struct OrbisKernelVirtualQueryInfo {
    uint64_t start;
    uint64_t end;
    int64_t  offset;
    int32_t  prot;
    int32_t  memory_type;
    uint8_t  is_flexible  : 1;
    uint8_t  is_direct    : 1;
    uint8_t  is_stack     : 1;
    uint8_t  is_pooled    : 1;
    uint8_t  is_committed : 1;
    char     name[32];
  };
  OrbisKernelVirtualQueryInfo info;
  memset(&info, 0, sizeof(info));
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(addr, info.start);
  CHECK_EQUAL(addr + 0x10000, info.end);
  // Offset is set to physical address.
  CHECK_EQUAL(phys_addr, info.offset);
  CHECK_EQUAL(3, info.prot);
  CHECK_EQUAL(0, info.memory_type);
  CHECK_EQUAL(0, info.is_flexible);
  CHECK_EQUAL(1, info.is_direct);
  CHECK_EQUAL(0, info.is_stack);
  CHECK_EQUAL(0, info.is_pooled);
  CHECK_EQUAL(1, info.is_committed);

  uint64_t addr2 = 0;
  result = sceKernelMapDirectMemory(&addr2, 0x10000, 3, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);

  // Both calls should succeed, and writes to the first address will be mirrored in the second.
  memset(reinterpret_cast<void*>(addr), 1, 0x10000);
  result = memcmp(reinterpret_cast<void*>(addr), reinterpret_cast<void*>(addr2), 0x10000);
  CHECK_EQUAL(0, result);

  // Store the current state of this memory
  char test_buf[0x10000];
  memcpy(test_buf, reinterpret_cast<void*>(addr2), 0x10000);

  // Both addresses should be unmapped by sceKernelCheckedReleaseDirectMemory
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // Ensure physical memory is properly released
  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // Reallocate memory
  result = sceKernelAllocateMainDirectMemory(0x10000, 0x10000, 0, &phys_addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0x10000, phys_addr);

  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);

  // Backing contents should remain the same despite the direct memory release.
  result = memcmp(reinterpret_cast<void*>(addr), test_buf, 0x10000);
  CHECK_EQUAL(0, result);

  // Partially release page. In theory, this should only unmap the part of mapped to these physical addresses.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr + 0x4000, 0x4000);
  CHECK_EQUAL(0, result);

  // Use sceKernelQueryMemoryProtection to validate expected addresses.
  uint64_t start_addr;
  uint64_t end_addr;
  int32_t prot;
  result = sceKernelQueryMemoryProtection(addr, &start_addr, &end_addr, &prot);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(addr, start_addr);
  CHECK_EQUAL(addr + 0x4000, end_addr);
  CHECK_EQUAL(3, prot);

  // We unmapped from 0x4000 to 0x8000 in the mapping. Query past that gap
  result = sceKernelQueryMemoryProtection(end_addr + 0x4000, &start_addr, &end_addr, &prot);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(addr + 0x8000, start_addr);
  CHECK_EQUAL(addr + 0x10000, end_addr);
  CHECK_EQUAL(3, prot);

  // Unmap the full direct memory area.
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);

  // Remap the area, this should fail due to the missing physical address chunk.
  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // Since it's a different syscall, check sceKernelMapDirectMemory2 as well.
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 0, 3, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // Loop through all possible mtypes, ensure all are backed appropriately
  // Start by releasing any remaining physical memory from before. Use sceKernelReleaseDirectMemory to skip the unallocated addresses.
  result = sceKernelReleaseDirectMemory(phys_addr, 0x10000);
  CHECK_EQUAL(0, result);

  for (int32_t mtype = 0; mtype < 10; ++mtype) {
    // Allocate direct memory using specified mtype
    result = sceKernelAllocateMainDirectMemory(0x10000, 0x10000, mtype, &phys_addr);
    CHECK_EQUAL(0, result);

    // Map direct memory to this physical address space
    addr = 0;
    result = sceKernelMapDirectMemory(&addr, 0x10000, 0x33, 0, phys_addr, 0);
    CHECK_EQUAL(0, result);

    // Check for proper backing behavior. As per usual, copy bytes, unmap, remap, then check for bytes.
    memset(reinterpret_cast<void*>(addr), 1, 0x10000);
    memcpy(test_buf, reinterpret_cast<void*>(addr), 0x10000);

    result = sceKernelMunmap(addr, 0x10000);
    CHECK_EQUAL(0, result);

    result = sceKernelMapDirectMemory(&addr, 0x10000, 0x33, 0, phys_addr, 0);
    CHECK_EQUAL(0, result);

    // If this succeeds, this memory is backed.
    result = memcmp(reinterpret_cast<void*>(addr), test_buf, 0x10000);
    CHECK_EQUAL(0, result);

    // Release direct memory used for this test.
    result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
    CHECK_EQUAL(0, result);

    // While we're here, we can test sceKernelMapDirectMemory2's type-setting.
    for (int32_t test_mtype = 0; test_mtype < 10; ++test_mtype) {
      // Allocate direct memory with mtype
      result = sceKernelAllocateMainDirectMemory(0x10000, 0x10000, mtype, &phys_addr);
      CHECK_EQUAL(0, result);

      // Use sceKernelGetDirectMemoryType to ensure type is stored correctly
      int64_t phys_start;
      int64_t phys_end;
      int32_t out_mtype;
      result = sceKernelGetDirectMemoryType(phys_addr, &out_mtype, &phys_start, &phys_end);
      CHECK_EQUAL(0, result);
      if (out_mtype == 3) {
        // This is the mtype of SceGnmDriver's dmem allocation.
        // Allocations with this type will coalesce with it.
        CHECK_EQUAL(0, phys_start);
      } else {
        CHECK_EQUAL(phys_addr, phys_start);
      }
      CHECK_EQUAL(phys_addr + 0x10000, phys_end);
      CHECK_EQUAL(mtype, out_mtype);

      // Use sceKernelMapDirectMemory2 to change memory type.
      addr = 0;
      result = sceKernelMapDirectMemory2(&addr, 0x10000, test_mtype, 0x33, 0, phys_addr, 0);
      CHECK_EQUAL(0, result);

      // Use sceKernelGetDirectMemoryType to ensure type is updated correctly
      result = sceKernelGetDirectMemoryType(phys_addr, &out_mtype, &phys_start, &phys_end);
      CHECK_EQUAL(0, result);
      // sceKernelMapDirectMemory2 doesn't coalesce dmem areas after changing type.
      if (out_mtype == 3 && mtype == 3) {
        // This is the mtype of SceGnmDriver's dmem allocation.
        // Allocations with this type will coalesce with it.
        CHECK_EQUAL(0, phys_start);
      } else {
        CHECK_EQUAL(phys_addr, phys_start);
      }
      CHECK_EQUAL(phys_addr + 0x10000, phys_end);
      CHECK_EQUAL(test_mtype, out_mtype);

      // Use sceKernelMunmap to unmap memory
      result = sceKernelMunmap(addr, 0x10000);
      CHECK_EQUAL(0, result);

      // Use sceKernelGetDirectMemoryType to ensure type remains the same
      result = sceKernelGetDirectMemoryType(phys_addr, &out_mtype, &phys_start, &phys_end);
      CHECK_EQUAL(0, result);
      // sceKernelMapDirectMemory2 doesn't coalesce dmem areas after changing type.
      if (out_mtype == 3 && mtype == 3) {
        // This is the mtype of SceGnmDriver's dmem allocation.
        // Allocations with this type will coalesce with it.
        CHECK_EQUAL(0, phys_start);
      } else {
        CHECK_EQUAL(phys_addr, phys_start);
      }
      CHECK_EQUAL(phys_addr + 0x10000, phys_end);
      CHECK_EQUAL(test_mtype, out_mtype);

      // Use sceKernelCheckedReleaseDirectMemory to erase direct memory
      result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
      CHECK_EQUAL(0, result);
    }
  }

  // memory type 10 does not allow write permissions of any form.
  result = sceKernelAllocateMainDirectMemory(0x10000, 0, 10, &phys_addr);
  CHECK_EQUAL(0, result);

  addr = 0;
  result = sceKernelMapDirectMemory(&addr, 0x10000, 0, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);

  addr = 0;
  result = sceKernelMapDirectMemory(&addr, 0x10000, 1, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);

  result = sceKernelMapDirectMemory(&addr, 0x10000, 2, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  addr = 0;
  result = sceKernelMapDirectMemory(&addr, 0x10000, 0x10, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);

  addr = 0;
  result = sceKernelMapDirectMemory(&addr, 0x10000, 0x11, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);

  result = sceKernelMapDirectMemory(&addr, 0x10000, 0x20, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMapDirectMemory(&addr, 0x10000, 0x22, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMapDirectMemory(&addr, 0x10000, 0x30, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMapDirectMemory(&addr, 0x10000, 0x33, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // Make sure permissions are checked appropriately when changing type through sceKernelMapDirectMemory2
  result = sceKernelAllocateMainDirectMemory(0x10000, 0, 0, &phys_addr);
  CHECK_EQUAL(0, result);

  addr = 0;
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 10, 3, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 10, 0x20, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 10, 0x11, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);

  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);

  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  CHECK_EQUAL(0, result);
}

// These tests are from my old homebrew, I'll probably rewrite some later.
static void TestMemoryWrite(uint64_t addr, uint64_t size) {
  // This function writes 0's to the requested memory area.
  void* test_addr = malloc(size);
  memset(test_addr, 0, size);

  // This memcpy will crash if the specified memory area isn't writable.
  memcpy(reinterpret_cast<void*>(addr), test_addr, size);

  // Free memory after testing
  free(test_addr);
}

static bool TestZeroedMemory(uint64_t addr, uint64_t size) {
  // This function reads 0's from the requested memory area, and compares them to an area full of zeros.
  void* test_addr = malloc(size);
  memset(test_addr, 0, size);

  // This memcpy will crash if the specified memory area isn't writable.
  bool succ = memcmp(reinterpret_cast<void*>(addr), test_addr, size) == 0;

  // Free memory after testing
  free(test_addr);

  return succ;
}

TEST(MemoryTests, ExecutableTests) {
  // This tests to make sure executable memory behaves properly.
  // Allocate some direct memory for the direct memory tests
  uint64_t dmem_size = sceKernelGetDirectMemorySize();
  int64_t  phys_addr = 0;
  uint64_t size      = 0x100000;
  int32_t  result    = sceKernelAllocateDirectMemory(0, dmem_size, size, 0, 0, &phys_addr);
  CHECK_EQUAL(0, result);

  // Using the physical addresses, map direct memory to a flexible address.
  uint64_t addr = 0;
  int32_t  prot = 0x37;
  // Direct memory mapping functions do not allow for assigning executable permissions. Test this edge case.
  result = sceKernelMapDirectMemory(&addr, size, prot, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // Perform a successful direct memory mapping.
  prot   = 0x33;
  result = sceKernelMapDirectMemory(&addr, size, prot, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);

  // Run sceKernelQueryMemoryProtection to confirm the protection value assigned to this mapping.
  int32_t old_prot = 0;
  // We don't care about the other data, supply nullptrs for start and end.
  result = sceKernelQueryMemoryProtection(addr, nullptr, nullptr, &old_prot);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(prot, old_prot);

  // Use sceKernelMprotect to give the memory executable permissions.
  prot   = 0x37;
  result = sceKernelMprotect(addr, size, prot);
  CHECK_EQUAL(0, result);

  // Direct memory never actually exposes execute permissions.
  // Confirm this behavior by running sceKernelQueryMemoryProtection again.
  int32_t new_prot = 0;
  result           = sceKernelQueryMemoryProtection(addr, nullptr, nullptr, &new_prot);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(old_prot, new_prot);

  // Write a basic function into memory that returns 256.
  uint8_t bytes[] = {0x48, 0xc7, 0xc0, 00, 01, 00, 00, 0xc3};
  std::memcpy(reinterpret_cast<void*>(addr), bytes, 8);
  typedef uint64_t (*func)();
  func     test_func = reinterpret_cast<func>(addr);
  uint64_t res       = test_func();
  CHECK_EQUAL(256, res);

  // Unmap the direct memory used for this test.
  result = sceKernelMunmap(addr, size);
  CHECK_EQUAL(0, result);

  // Release direct memory allocated for this test.
  result = sceKernelReleaseDirectMemory(phys_addr, size);
  CHECK_EQUAL(0, result);

  // Run a similar test utilizing flexible memory instead. Flexible memory allows and exposes executable permissions.
  addr   = 0;
  result = sceKernelMapFlexibleMemory(&addr, size, prot, 0);
  CHECK_EQUAL(0, result);

  // Validate the protection given to the memory area
  result = sceKernelQueryMemoryProtection(addr, nullptr, nullptr, &old_prot);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(prot, old_prot);

  // Write a basic function into memory that returns 256.
  std::memcpy(reinterpret_cast<void*>(addr), bytes, 8);
  test_func = reinterpret_cast<func>(addr);
  res       = test_func();
  CHECK_EQUAL(256, res);

  // Unmap the flexible memory used for this test.
  result = sceKernelMunmap(addr, size);
  CHECK_EQUAL(0, result);
}

TEST(MemoryTests, AlignmentTests) {
  // This tests for behaviors relating to memory alignment
  uint64_t alignment = getpagesize();
  CHECK_EQUAL(0x4000, alignment);

  // Start by testing mmap behavior with misaligned address and size.
  uint64_t addr  = 0;
  uint64_t size  = 0x3000;
  int32_t  prot  = 0x3;
  int32_t  flags = 0x1000;

  int32_t result = sceKernelMmap(addr, size, prot, flags, -1, 0, &addr);
  CHECK_EQUAL(0, result);
  // Test to ensure addr is aligned
  uint64_t addr_value = addr;
  CHECK_EQUAL(0, addr_value % alignment);

  // Check the size of the mapping
  struct OrbisKernelVirtualQueryInfo {
    uint64_t start;
    uint64_t end;
    int64_t  offset;
    int32_t  prot;
    int32_t  memory_type;
    uint8_t  is_flexible  : 1;
    uint8_t  is_direct    : 1;
    uint8_t  is_stack     : 1;
    uint8_t  is_pooled    : 1;
    uint8_t  is_committed : 1;
    char     name[32];
  };

  OrbisKernelVirtualQueryInfo info;
  memset(&info, 0, sizeof(info));
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  CHECK_EQUAL(0, result);
  uint64_t calc_size = info.end - info.start;
  CHECK_EQUAL(0, calc_size % alignment);
  CHECK(calc_size > size);

  // Write to the full reported memory space to ensure the memory is actually mapped.
  // This will crash on emulators that don't emulate this behavior properly.
  TestMemoryWrite(addr, calc_size);

  // Unmap the flexible memory used for this test.
  result = sceKernelMunmap(addr, calc_size);
  CHECK_EQUAL(0, result);

  // Misaligned address with flags fixed should fail.
  uint64_t test_addr = addr_value + 0x1000;
  size               = 0x4000;
  flags              = 0x1010;
  result             = sceKernelMmap(test_addr, size, prot, flags, -1, 0, &test_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Without flags fixed, this should both addr and size up.
  size   = 0x3000;
  flags  = 0x1000;
  result = sceKernelMmap(test_addr, size, prot, flags, -1, 0, &test_addr);
  CHECK_EQUAL(0, result);

  // Test to ensure addr is aligned
  addr_value = test_addr;
  CHECK_EQUAL(0, addr_value % alignment);
  // Address rounds up, so it should be greater than addr
  CHECK(addr_value > addr);

  // Check the size of the mapping
  memset(&info, 0, sizeof(info));
  result = sceKernelVirtualQuery(test_addr, 0, &info, sizeof(info));
  CHECK_EQUAL(0, result);
  calc_size = info.end - info.start;
  CHECK_EQUAL(0, calc_size % alignment);
  // Size should align up
  CHECK_EQUAL(0x4000, calc_size);

  // Write to the full reported memory space to ensure the memory is actually mapped.
  // This will crash on emulators that don't emulate this behavior properly.
  TestMemoryWrite(test_addr, calc_size);

  // Unmap the flexible memory used for this test.
  result = sceKernelMunmap(test_addr, calc_size);
  CHECK_EQUAL(0, result);

  // Now test for unmap alignment behavior.
  // Start by mmap'ing 4 pages of memory. This should be enough to check for most edge cases.
  addr_value = 0x10000000000;
  addr       = addr_value;
  size       = 0x10000;
  flags      = 0x1010;
  result     = sceKernelMmap(addr, size, prot, flags, -1, 0, &addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(addr_value, addr);

  // Munmap should align down the address, and align up the length.
  // This should unmap only the second page of that mapping.
  uint64_t test_addr_value = addr_value + 0x7000;
  uint64_t test_size       = 0x1000;
  result                   = sceKernelMunmap(test_addr_value, test_size);
  CHECK_EQUAL(0, result);

  memset(&info, 0, sizeof(info));
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  CHECK_EQUAL(0, result);

  // Expected start addr is addr_value, expected end is addr_value + 0x4000.
  CHECK_EQUAL(addr, info.start);
  CHECK_EQUAL(addr_value + 0x4000, info.end);

  // Make sure all of this memory is actually mapped.
  TestMemoryWrite(info.start, alignment);

  // Run VirtualQuery on the remaining addresses that should be mapped.
  test_addr_value = addr_value + 0x8000;
  memset(&info, 0, sizeof(info));
  result = sceKernelVirtualQuery(test_addr_value, 0, &info, sizeof(info));
  CHECK_EQUAL(0, result);

  // Expected start addr is addr_value + 0x8000, expected end is addr_value + 0x10000.
  CHECK_EQUAL(addr_value + 0x8000, info.start);
  CHECK_EQUAL(addr_value + 0x10000, info.end);

  // Make sure all of this memory is actually mapped.
  TestMemoryWrite(info.start, alignment * 2);

  // Place a mapping where the unmap occurred.
  test_addr_value = addr_value + 0x4000;
  test_addr       = test_addr_value;
  result          = sceKernelMmap(test_addr, alignment, prot, flags, -1, 0, &test_addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(test_addr_value, test_addr);

  // Make sure all the memory is mapped
  TestMemoryWrite(addr_value, alignment * 4);

  // Unmap the memory used for this test.
  result = sceKernelMunmap(addr_value, alignment * 4);
  CHECK_EQUAL(0, result);
}

TEST(MemoryTests, FlagTests) {
  // Make sure no-overlap and fixed flags behave properly, since that's what most games use.
  uint64_t addr  = 0;
  uint64_t size  = 0x1000000;
  int32_t  flags = 0x10;

  // Fixed with addr nullptr should ignore fixed flags.
  int32_t result = sceKernelReserveVirtualRange(&addr, size, flags, 0);
  CHECK_EQUAL(0, result);
  CHECK(addr != 0);

  // Regardless of flags or reservations, mmap shouldn't map to the reserved page.
  uint64_t mmap_addr = 0;
  result             = sceKernelMmap(addr, size, 0x3, 0x1000, -1, 0, &mmap_addr);
  CHECK_EQUAL(0, result);
  CHECK(mmap_addr != addr);

  // Unmap the mmap
  result = sceKernelMunmap(mmap_addr, size);
  CHECK_EQUAL(0, result);

  // Perform a mapping with fixed | no-overwrite. This should fail.
  result = sceKernelMmap(addr, size, 0x3, 0x1090, -1, 0, &mmap_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // Perform a fixed mapping, this should overwrite the reserved page with usable memory.
  result = sceKernelMmap(addr, size, 0x3, 0x1010, -1, 0, &mmap_addr);
  CHECK_EQUAL(0, result);
  CHECK(mmap_addr == addr);

  // Perform another fixed mapping, this should overwrite the newly mapped page.
  result = sceKernelMmap(addr, size, 0x3, 0x1010, -1, 0, &mmap_addr);
  CHECK_EQUAL(0, result);
  CHECK(mmap_addr == addr);

  // Unmap the mmap
  result = sceKernelMunmap(mmap_addr, size);
  CHECK_EQUAL(0, result);
}

TEST(MemoryTests, ProtectTests) {
  // This tests potential edge cases involving mprotect.
  // Start by mapping memory with no permissions
  uint64_t addr   = 0;
  uint64_t size   = 0x100000;
  int32_t  flags  = 0;
  int32_t  prot   = 0;
  int32_t  result = sceKernelMapFlexibleMemory(&addr, size, prot, flags);
  CHECK_EQUAL(0, result);

  // Use sceKernelMprotect to protect the middle of this memory area.
  // For testing purposes, use misaligned address and size.
  uint64_t protect_addr = addr + 0x43000;
  uint64_t protect_size = 0x7D000;
  prot                  = 1;
  result                = sceKernelMprotect(protect_addr, protect_size, prot);
  CHECK_EQUAL(0, result);

  // Verify permissions and alignment using sceKernelQueryMemoryProtection.
  // Address is expected to align down, while the size should align up.
  uint64_t start_addr;
  uint64_t end_addr;
  int32_t  out_prot;
  result = sceKernelQueryMemoryProtection(protect_addr, &start_addr, &end_addr, &out_prot);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(addr + 0x40000, start_addr);
  CHECK_EQUAL(addr + 0xC0000, end_addr);
  CHECK_EQUAL(prot, out_prot);

  // Ensure we can read from this memory by calling TestZeroedMemory.
  // Ignore the return, all we care about is that reading doesn't crash.
  TestZeroedMemory(addr + 0x40000, 0x80000);

  // Use sceKernelMprotect to protect the middle of this memory area.
  // For this test, only provide write protections.
  protect_addr = addr + 0x40000;
  protect_size = 0x80000;
  prot         = 2;
  result       = sceKernelMprotect(protect_addr, protect_size, prot);
  CHECK_EQUAL(0, result);

  // Validate returned protection, libkernel adds read protection to write-only pages.
  out_prot = 0;
  result   = sceKernelQueryMemoryProtection(protect_addr, nullptr, nullptr, &out_prot);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(3, out_prot);

  // Call TestMemoryWrite to make sure writes work properly, then TestZeroedMemory to check reads.
  TestMemoryWrite(protect_addr, protect_size);
  CHECK(TestZeroedMemory(protect_addr, protect_size));

  // Use sceKernelMprotect to give the entire original mapping read permissions
  prot   = 1;
  result = sceKernelMprotect(addr, size, prot);
  CHECK_EQUAL(0, result);

  // Ensure prot applied properly.
  result = sceKernelQueryMemoryProtection(addr, nullptr, nullptr, &out_prot);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(prot, out_prot);

  // Ensure we can read from this memory by calling TestZeroedMemory.
  // Ignore the return, all we care about is that reading doesn't crash.
  TestZeroedMemory(addr, size);

  // Unmap memory used for test.
  result = sceKernelMunmap(addr, size);
  CHECK_EQUAL(0, result);
}