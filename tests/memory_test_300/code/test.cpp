#include "CppUTest/TestHarness.h"

#include <list>
#include <stdio.h>
#include <string>

extern "C" {
// Direct memory functions
int32_t sceKernelAllocateMainDirectMemory(uint64_t size, uint64_t alignment, int32_t type, int64_t* phys_addr);
int32_t sceKernelMapDirectMemory2(uint64_t* addr, uint64_t size, int32_t type, int32_t prot, int32_t flags, int64_t phys_addr, uint64_t alignment);
int32_t sceKernelCheckedReleaseDirectMemory(int64_t phys_addr, uint64_t size);

// Flexible memory functions
int32_t sceKernelAvailableFlexibleMemorySize(uint64_t* size);
int32_t sceKernelMapFlexibleMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags);

// Generic memory functions
int32_t sceKernelMmap(uint64_t addr, uint64_t size, int32_t prot, int32_t flags, int32_t fd, int64_t offset, uint64_t* out_addr);
int32_t sceKernelMunmap(uint64_t addr, uint64_t size);
int32_t sceKernelVirtualQuery(uint64_t addr, int32_t flags, void* info, uint64_t info_size);

// Filesystem functions
int32_t sceKernelOpen(const char* path, int32_t flags, uint16_t mode);
int64_t sceKernelWrite(int32_t fd, const void* buf, uint64_t size);
int32_t sceKernelClose(int32_t fd);
}

// Some error codes
#define ORBIS_KERNEL_ERROR_ENOMEM (0x8002000C)
#define ORBIS_KERNEL_ERROR_EINVAL (0x80020016)

void mem_scan() {
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
}

TEST_GROUP (MemoryTests) {
  void setup() { // Before each test, call mem_scan to print out memory map information.
    // This will provide an indicator of how the memory map looks, which can help with debugging strange behavior during tests.
    printf("Before test:\n");
    mem_scan();
  }

  void teardown() {}
};

// Test behavior that changed in firmware 3.00
TEST(MemoryTests, FW300Test) {
  // Starting with firmware 3.00, mmap calls with MAP_STACK and address greater than 0xfc00000000 should fail with ENOMEM
  uint64_t addr     = 0xfb00000000;
  uint64_t addr_out = 0;
  int32_t  result   = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xfc00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xfc00004000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  addr   = 0xff00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // mmap calls with MAP_FIXED have a similar error condition,
  // if addr + size > 0xfc00000000 and SDK version is at or above 3.00, then return EINVAL.
  // Since this homebrew is compiled with SDK version set to 2.50, these should all succeed.
  // Note: MAP_SANITIZER does not hit these error returns, that behavior cannot be tested on retail hardware.

  addr   = 0xfb00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1010, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xfbffffc000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1010, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xfc00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1010, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  addr   = 0xfc00004000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1010, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  addr   = 0xff00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1010, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // mmap calls without MAP_FIXED also have a similar edge case, but with ENOMEM returns.
  addr   = 0xfb00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1000, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xfbffffc000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1000, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xfc00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1000, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  addr   = 0xfc00004000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1000, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  addr   = 0xff00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1000, -1, 0, &addr_out);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // sys_mmap_dmem also has an equivalent check.
  // To test this, we need to allocate some direct memory.
  int64_t phys_addr = 0;
  result            = sceKernelAllocateMainDirectMemory(0x4000, 0x4000, 0, &phys_addr);
  CHECK_EQUAL(0, result);

  addr   = 0xfb00000000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -1, 3, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xfbffffc000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -1, 3, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xfc00000000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -1, 3, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  addr   = 0xfc00004000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -1, 3, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  addr   = 0xff00000000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -1, 3, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x4000);
  CHECK_EQUAL(0, result);
}

// Test behavior that changed in firmware 3.50
TEST(MemoryTests, FW350Test) {
  // Starting with firmware 3.50, out of memory errors return ORBIS_KERNEL_ERROR_ENOMEM instead of ORBIS_KERNEL_ERROR_EINVAL.
  // This should not be visible here, since this homebrew compiles with SDK version 3.00
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

  // Unmap memory used by this test
  for (uint64_t addr: addresses) {
    result = sceKernelMunmap(addr, 0x4000);
    CHECK_EQUAL(0, result);
  }

  // Clear list of addresses
  addresses.clear();
}