#include "test.h"

#include "CppUTest/TestHarness.h"

#include <list>
#include <stdio.h>
#include <string>

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
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0xfc00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0xfc00004000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  addr   = 0xff00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // mmap calls with MAP_FIXED have a similar error condition,
  // if addr + size > 0xfc00000000 and SDK version is at or above 3.00, then return EINVAL.
  // Note: MAP_SANITIZER does not hit these error returns, that behavior cannot be tested on retail hardware.

  addr   = 0xfb00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1010, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0xfbffffc000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1010, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0xfc00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1010, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  addr   = 0xfc00004000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1010, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  addr   = 0xff00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1010, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // mmap calls without MAP_FIXED also have a similar edge case, but with ENOMEM returns.
  addr   = 0xfb00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1000, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0xfbffffc000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1000, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0xfc00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1000, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  addr   = 0xfc00004000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1000, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  addr   = 0xff00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1000, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // sys_mmap_dmem also has an equivalent check.
  // To test this, we need to allocate some direct memory.
  int64_t phys_addr = 0;
  result            = sceKernelAllocateMainDirectMemory(0x4000, 0x4000, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0xfb00000000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -1, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0xfbffffc000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -1, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0xfc00000000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -1, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  addr   = 0xfc00004000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -1, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  addr   = 0xff00000000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -1, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);
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
      UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
    } else {
      // Mapped flex mem successfully.
      UNSIGNED_INT_EQUALS(0, result);
      // Add the mapping address to addresses, need to unmap later to clean up.
      addresses.emplace_back(addr_out);
    }
  }

  // After all these mappings, available flex size should be 0.
  uint64_t avail_flex_size = 0;
  result                   = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0, avail_flex_size);

  // sceKernelMmap with MAP_ANON uses the flexible budget.
  uint64_t test_addr;
  result = sceKernelMmap(0, 0x4000, 3, 0x1000, -1, 0, &test_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // sceKernelMmap with files also uses the flexible budget.
  int32_t fd = sceKernelOpen("/app0/assets/misc/test_file.txt", 0, 0666);
  CHECK(fd > 0);
  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &test_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // Make sure download dir file has enough space to mmap before trying.
  fd = sceKernelOpen("/download0/test_file.txt", 0x602, 0666);
  CHECK(fd > 0);
  char test_buf[0x100000];
  memset(test_buf, 0, sizeof(test_buf));
  int64_t bytes = sceKernelWrite(fd, test_buf, sizeof(test_buf));
  LONGS_EQUAL(sizeof(test_buf), bytes);

  // mmap download dir file.
  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &test_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // Unmap memory used by this test
  for (uint64_t addr: addresses) {
    result = sceKernelMunmap(addr, 0x4000);
    UNSIGNED_INT_EQUALS(0, result);
  }

  // Clear list of addresses
  addresses.clear();
}