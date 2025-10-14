#include "CppUTest/TestHarness.h"

#include <stdio.h>
#include <string>

extern "C" {
uint64_t sceKernelGetDirectMemorySize();

int32_t  sceKernelAvailableDirectMemorySize(int64_t start, int64_t end, uint64_t alignment, int64_t* phys_addr, uint64_t* size);
int32_t  sceKernelAllocateDirectMemory(int64_t start, int64_t end, uint64_t size, uint64_t alignment, int32_t type, int64_t* phys_addr);
int32_t  sceKernelAllocateMainDirectMemory(uint64_t size, uint64_t alignment, int32_t type, int64_t* phys_addr);
int32_t  sceKernelMapDirectMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags, int64_t phys_addr, uint64_t alignment);
int32_t  sceKernelGetDirectMemoryType(int64_t phys_addr, int32_t* type, int64_t* start, int64_t* end);
int32_t  sceKernelCheckedReleaseDirectMemory(int64_t phys_addr, uint64_t size);
int32_t  sceKernelReleaseDirectMemory(int64_t phys_addr, uint64_t size);

int32_t  sceKernelAvailableFlexibleMemorySize(uint64_t* size);
int32_t  sceKernelMapFlexibleMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags);
int32_t  sceKernelReleaseFlexibleMemory(uint64_t* addr, uint64_t size);

int32_t  sceKernelReserveVirtualRange(uint64_t* addr, uint64_t size, int32_t flags, uint64_t alignment);

int32_t  sceKernelMmap(uint64_t addr, uint64_t size, int32_t prot, int32_t flags, int32_t fd, int64_t phys_addr, uint64_t* out_addr);
int32_t  sceKernelMunmap(uint64_t addr, uint64_t size);

int32_t  sceKernelMemoryPoolExpand(int64_t start, int64_t end, uint64_t len, uint64_t alignment, int64_t* phys_addr);
int32_t  sceKernelMemoryPoolReserve(uint64_t addr_in, uint64_t len, uint64_t alignment, int32_t flags, uint64_t addr_out);
int32_t  sceKernelMemoryPoolCommit(uint64_t addr, uint64_t len, int32_t type, int32_t prot, int32_t flags);
int32_t  sceKernelMemoryPoolDecommit(uint64_t addr, uint64_t len, int32_t flags);

int32_t  sceKernelOpen(const char* path, int32_t flags, uint16_t mode);
int64_t  sceKernelWrite(int32_t fd, const void* buf, uint64_t size);
int32_t  sceKernelClose(int32_t fd);
}

// Some error codes
#define ORBIS_KERNEL_ERROR_UNKNOWN (0x80020000)
#define ORBIS_KERNEL_ERROR_EPERM (0x80020001)
#define ORBIS_KERNEL_ERROR_ENOENT (0x80020002)
#define ORBIS_KERNEL_ERROR_ESRCH (0x80020003)
#define ORBIS_KERNEL_ERROR_EINTR (0x80020004)
#define ORBIS_KERNEL_ERROR_EIO (0x80020005)
#define ORBIS_KERNEL_ERROR_ENXIO (0x80020006)
#define ORBIS_KERNEL_ERROR_E2BIG (0x80020007)
#define ORBIS_KERNEL_ERROR_ENOEXEC (0x80020008)
#define ORBIS_KERNEL_ERROR_EBADF (0x80020009)
#define ORBIS_KERNEL_ERROR_ECHILD (0x8002000A)
#define ORBIS_KERNEL_ERROR_EDEADLK (0x8002000B)
#define ORBIS_KERNEL_ERROR_ENOMEM (0x8002000C)
#define ORBIS_KERNEL_ERROR_EACCES (0x8002000D)
#define ORBIS_KERNEL_ERROR_EFAULT (0x8002000E)
#define ORBIS_KERNEL_ERROR_ENOTBLK (0x8002000F)
#define ORBIS_KERNEL_ERROR_EBUSY (0x80020010)
#define ORBIS_KERNEL_ERROR_EEXIST (0x80020011)
#define ORBIS_KERNEL_ERROR_EXDEV (0x80020012)
#define ORBIS_KERNEL_ERROR_ENODEV (0x80020013)
#define ORBIS_KERNEL_ERROR_ENOTDIR (0x80020014)
#define ORBIS_KERNEL_ERROR_EISDIR (0x80020015)
#define ORBIS_KERNEL_ERROR_EINVAL (0x80020016)
#define ORBIS_KERNEL_ERROR_ENFILE (0x80020017)
#define ORBIS_KERNEL_ERROR_EMFILE (0x80020018)
#define ORBIS_KERNEL_ERROR_ENOTTY (0x80020019)
#define ORBIS_KERNEL_ERROR_ETXTBSY (0x8002001A)
#define ORBIS_KERNEL_ERROR_EFBIG (0x8002001B)
#define ORBIS_KERNEL_ERROR_ENOSPC (0x8002001C)
#define ORBIS_KERNEL_ERROR_ESPIPE (0x8002001D)
#define ORBIS_KERNEL_ERROR_EROFS (0x8002001E)
#define ORBIS_KERNEL_ERROR_EMLINK (0x8002001F)
#define ORBIS_KERNEL_ERROR_EPIPE (0x80020020)
#define ORBIS_KERNEL_ERROR_EDOM (0x80020021)
#define ORBIS_KERNEL_ERROR_ERANGE (0x80020022)
#define ORBIS_KERNEL_ERROR_EAGAIN (0x80020023)
#define ORBIS_KERNEL_ERROR_EWOULDBLOCK (0x80020023)
#define ORBIS_KERNEL_ERROR_EINPROGRESS (0x80020024)
#define ORBIS_KERNEL_ERROR_EALREADY (0x80020025)
#define ORBIS_KERNEL_ERROR_ENOTSOCK (0x80020026)
#define ORBIS_KERNEL_ERROR_EDESTADDRREQ (0x80020027)
#define ORBIS_KERNEL_ERROR_EMSGSIZE (0x80020028)
#define ORBIS_KERNEL_ERROR_EPROTOTYPE (0x80020029)
#define ORBIS_KERNEL_ERROR_ENOPROTOOPT (0x8002002A)
#define ORBIS_KERNEL_ERROR_EPROTONOSUPPORT (0x8002002B)
#define ORBIS_KERNEL_ERROR_ESOCKTNOSUPPORT (0x8002002C)
#define ORBIS_KERNEL_ERROR_ENOTSUP (0x8002002D)
#define ORBIS_KERNEL_ERROR_EOPNOTSUPP (0x8002002D)
#define ORBIS_KERNEL_ERROR_EPFNOSUPPORT (0x8002002E)
#define ORBIS_KERNEL_ERROR_EAFNOSUPPORT (0x8002002F)
#define ORBIS_KERNEL_ERROR_EADDRINUSE (0x80020030)
#define ORBIS_KERNEL_ERROR_EADDRNOTAVAIL (0x80020031)
#define ORBIS_KERNEL_ERROR_ENETDOWN (0x80020032)
#define ORBIS_KERNEL_ERROR_ENETUNREACH (0x80020033)
#define ORBIS_KERNEL_ERROR_ENETRESET (0x80020034)
#define ORBIS_KERNEL_ERROR_ECONNABORTED (0x80020035)
#define ORBIS_KERNEL_ERROR_ECONNRESET (0x80020036)
#define ORBIS_KERNEL_ERROR_ENOBUFS (0x80020037)
#define ORBIS_KERNEL_ERROR_EISCONN (0x80020038)
#define ORBIS_KERNEL_ERROR_ENOTCONN (0x80020039)
#define ORBIS_KERNEL_ERROR_ESHUTDOWN (0x8002003A)
#define ORBIS_KERNEL_ERROR_ETOOMANYREFS (0x8002003B)
#define ORBIS_KERNEL_ERROR_ETIMEDOUT (0x8002003C)
#define ORBIS_KERNEL_ERROR_ECONNREFUSED (0x8002003D)
#define ORBIS_KERNEL_ERROR_ELOOP (0x8002003E)
#define ORBIS_KERNEL_ERROR_ENAMETOOLONG (0x8002003F)
#define ORBIS_KERNEL_ERROR_EHOSTDOWN (0x80020040)
#define ORBIS_KERNEL_ERROR_EHOSTUNREACH (0x80020041)
#define ORBIS_KERNEL_ERROR_ENOTEMPTY (0x80020042)
#define ORBIS_KERNEL_ERROR_EPROCLIM (0x80020043)
#define ORBIS_KERNEL_ERROR_EUSERS (0x80020044)
#define ORBIS_KERNEL_ERROR_EDQUOT (0x80020045)
#define ORBIS_KERNEL_ERROR_ESTALE (0x80020046)
#define ORBIS_KERNEL_ERROR_EREMOTE (0x80020047)
#define ORBIS_KERNEL_ERROR_EBADRPC (0x80020048)
#define ORBIS_KERNEL_ERROR_ERPCMISMATCH (0x80020049)
#define ORBIS_KERNEL_ERROR_EPROGUNAVAIL (0x8002004A)
#define ORBIS_KERNEL_ERROR_EPROGMISMATCH (0x8002004B)
#define ORBIS_KERNEL_ERROR_EPROCUNAVAIL (0x8002004C)
#define ORBIS_KERNEL_ERROR_ENOLCK (0x8002004D)
#define ORBIS_KERNEL_ERROR_ENOSYS (0x8002004E)
#define ORBIS_KERNEL_ERROR_EFTYPE (0x8002004F)
#define ORBIS_KERNEL_ERROR_EAUTH (0x80020050)
#define ORBIS_KERNEL_ERROR_ENEEDAUTH (0x80020051)
#define ORBIS_KERNEL_ERROR_EIDRM (0x80020052)
#define ORBIS_KERNEL_ERROR_ENOMSG (0x80020053)
#define ORBIS_KERNEL_ERROR_EOVERFLOW (0x80020054)
#define ORBIS_KERNEL_ERROR_ECANCELED (0x80020055)
#define ORBIS_KERNEL_ERROR_EILSEQ (0x80020056)
#define ORBIS_KERNEL_ERROR_ENOATTR (0x80020057)
#define ORBIS_KERNEL_ERROR_EDOOFUS (0x80020058)
#define ORBIS_KERNEL_ERROR_EBADMSG (0x80020059)
#define ORBIS_KERNEL_ERROR_EMULTIHOP (0x8002005A)
#define ORBIS_KERNEL_ERROR_ENOLINK (0x8002005B)
#define ORBIS_KERNEL_ERROR_EPROTO (0x8002005C)
#define ORBIS_KERNEL_ERROR_ENOTCAPABLE (0x8002005D)
#define ORBIS_KERNEL_ERROR_ECAPMODE (0x8002005E)
#define ORBIS_KERNEL_ERROR_ENOBLK (0x8002005F)
#define ORBIS_KERNEL_ERROR_EICV (0x80020060)
#define ORBIS_KERNEL_ERROR_ENOPLAYGOENT (0x80020061)

TEST_GROUP(MemoryUnitTests) {void setup() {

} void teardown() {

}};

TEST(MemoryUnitTests, AvailableDirectMemoryTest) {
  // Not exactly a way for this one to fail. Just need it to test other functions.
  uint64_t dmem_size = sceKernelGetDirectMemorySize();
  CHECK(dmem_size != 0);

  // Test sceKernelAvailableDirectMemorySize.
  // Typical use case:
  int64_t phys_addr = 0;
  uint64_t size = 0;
  int32_t result = sceKernelAvailableDirectMemorySize(0, dmem_size, 0, &phys_addr, &size);
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
  result = sceKernelAllocateDirectMemory(dmem_size / 2, dmem_size, 0x100000, 0, 0, &phys_addr_out);
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

TEST(MemoryUnitTests, AllocateDirectMemoryTest) {
  // Test sceKernelAllocateDirectMemory
  // Start with the typical working case.
  uint64_t dmem_size = sceKernelGetDirectMemorySize();
  int64_t phys_addr = 0;
  int32_t result = sceKernelAllocateDirectMemory(0, dmem_size, 0x10000, 0, 0, &phys_addr);
  CHECK_EQUAL(0, result);
  // Due to libSceGnmDriver, the first phys_addr we expect to see from sceKernelAllocateDirectMemory is 0x10000.
  CHECK_EQUAL(0x10000, phys_addr);

  // Keep dmem_map clean.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  CHECK_EQUAL(0, result);

  // If either start or end is less than zero, returns EINVAL
  phys_addr = 0;
  result = sceKernelAllocateDirectMemory(-1, dmem_size, 0x10000, 0, 0, &phys_addr);
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

TEST(MemoryUnitTests, ReleaseDirectMemoryTest) {
  // Both checked and unchecked return an error if addr or length aren't aligned to page size.
  int64_t phys_addr = 0x10000;
  int32_t result = sceKernelReleaseDirectMemory(phys_addr, 0x3000);
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
}

TEST(MemoryUnitTests, MapMemoryTest) {
  // Most memory functions in libkernel rely on sceKernelMmap,
  // leading to overlap in some edge cases.
  // Start with testing libkernel's error returns for sceKernelMapFlexibleMemory
  // If size is less than page size, or size is not page aligned, return EINVAL
  uint64_t addr = 0;
  int32_t result = sceKernelMapFlexibleMemory(&addr, 0x5000, 0, 0);
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
  addr = (uint64_t)0x200002000;
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  addr = 0;
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

  // Now for sys_mmap edge cases.
  // To test each of these, I'll use sceKernelMmap (which is just a wrapper for the syscall that converts errors to ORBIS errors)
  // If sys_mmap is called with the Sanitizer flag, then non-devkit consoles return EINVAL.
  addr = 0x200000000;
  uint64_t addr_out = 0;

  // For this test call, use flags Sanitizer (0x200000) | System (0x2000) | Anon (0x1000) | NoOverwrite (0x80) | Fixed (0x10)
  // This combination of flags is what sceKernelMapSanitizerShadowMemory uses.
  addr = 0x300000000;
  result = sceKernelMmap(addr, 0x4000, 0, 0x203090, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If length == 0, return EINVAL
  addr = 0x200000000;
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
  result = sceKernelMmap(addr, 0x4000, 0, 0x400, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, 0, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Note: I can't get stack flag to succeed, it probably has some other restriction I'll find deeper in the code.
  // Calling with the "valid" parameters assumed here causes ENOMEM.

  /**
   * Other notes here:
   * Stack and Void flags append Anon flag internally.
   * Stack, Anon, and Void flags skip fget_mmap call (which gets the file object for the fd input)
   * If Fixed is not specified and address is 0, then it's set to 0x200000000
   * Length parameter is aligned up to the nearest page.
   */

  // If flag Fixed (0x10) is specified, address must have the same offset as phys_addr.
  // This effectively prevents misaligned mappings, unless you manually opened a dmem file yourself.
  addr = 0x200002000;
  result = sceKernelMmap(addr, 0x4000, 0, 0x1010, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Notes:
   * This is where sys_mmap calls fget_mmap to get a file for mmap.
   * I still need to decompile everything here, especially the switch case for file type.
   * Insert error cases here if there are any errors that I can expose through homebrew.
   */

  // If an unopened file descriptor is specified, then returns EBADF.
  result = sceKernelMmap(addr, 0x4000, 1, 0, 100, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EBADF, result);

  // If prot dictates a mapping with read access, but the file doesn't have read access, then returns EACCES
  int32_t fd = sceKernelOpen("/download0/test.txt", 0x601, 0777);
  CHECK(fd > 0);
  
  // For testing purposes, write 16KB of empty space to the file.
  char buf[16384];
  memset(buf, 0, sizeof(buf));
  int64_t bytes_written = sceKernelWrite(fd, buf, sizeof(buf));
  CHECK_EQUAL(16384, bytes_written);

  // This sceKernelMmap should try mapping the file to memory with read permissions, but fail since the file is opened as write-only.
  result = sceKernelMmap(addr, 0x4000, 1, 0, fd, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // GPU file mmaps are prone to crashing PS4.
  // result = sceKernelMmap(addr, 0x4000, 0x10, 0, fd, 0, &addr_out);
  // CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // This sceKernelClose should succeed.
  result = sceKernelClose(fd);
  CHECK_EQUAL(result, 0);

  // If prot dictates a mapping with write access, but the file doesn't have write access, then returns EACCES
  fd = sceKernelOpen("/download0/test.txt", 0x600, 0777);
  CHECK(fd > 0);

  // This sceKernelMmap should try mapping the file to memory with read permissions, but fail since the file is opened as write-only.
  result = sceKernelMmap(addr, 0x4000, 2, 0, fd, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // GPU file mmaps are prone to crashing PS4.
  // result = sceKernelMmap(addr, 0x4000, 0x20, 0, fd, 0, &addr_out);
  // CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  result = sceKernelClose(fd);
  CHECK_EQUAL(result, 0);

  // Success? for reference
  fd = sceKernelOpen("/download0/test.txt", 0x602, 0777);
  CHECK(fd > 0);

  //Read-write file with CpuReadWrite prot, this should succeed (here as a sanity check, not ready for actual success tests yet).
  result = sceKernelMmap(addr, 0x4000, 0x3, 0, fd, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  // GPU file mmaps are prone to crashing PS4.
  // result = sceKernelMmap(addr, 0x4000, 0x30, 0, fd, 0, &addr_out);
  // CHECK_EQUAL(0, result);
  // result = sceKernelMunmap(addr_out, 0x4000);
  // CHECK_EQUAL(0, result);

  result = sceKernelClose(fd);
  CHECK_EQUAL(result, 0);

  /**
   * Notes:
   * Somewhere later down the line, CpuRead prot is appended to mappings with CpuWrite.
   * (This begs the question, will CpuWrite on a file mmap actually succeed for write-only files, or is there another check later?)
   * 
   * Based on decompilation, Sanitizer flag with a valid address (below a hardcoded 0x800000000000) restricts prot here.
   * Specifically, if address input > 0xfc00000000, prot is restricted to GpuReadWrite.
   * If address input is still zero here (Fixed flag with null address input?), then address defaults to 0xfc00000000.
   */

  // Next stop: Hell. Also called, vm_mmap2.
}