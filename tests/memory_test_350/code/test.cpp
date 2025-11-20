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

// Test behavior that changed in firmware 3.50
TEST(MemoryTests, FW350Test) {
  // Starting with firmware 3.50, out of memory errors return ORBIS_KERNEL_ERROR_ENOMEM instead of ORBIS_KERNEL_ERROR_EINVAL.
  // Start by using the full flexible budget.
  std::list<uint64_t> addresses;
  uint64_t            addr_out = 0;
  int32_t             result   = 0;
  while (result == 0) {
    result = sceKernelMapFlexibleMemory(&addr_out, 0x4000, 3, 0);
    if (result < 0) {
      // Out of flex mem
      CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);
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
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // sceKernelMmap with files also uses the flexible budget.
  int32_t fd = sceKernelOpen("/app0/assets/misc/test_file.txt", 0, 0666);
  CHECK(fd > 0);
  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &test_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);
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
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);
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

// Test behavior that changed in firmware 7.00
TEST(MemoryTests, FW700Test) {
  // Starting with firmware 7.00, vm_map_set_name_str received a fix to a vm_map_simplify_entry call
  // This difference should not be visible here, since this homebrew compiles with SDK version 3.50

  // Test memory coalescing behaviors.
  // To avoid issues with memory names, use sceKernelSetVirtualRangeName
  uint64_t addr   = 0x300000000;
  int32_t  result = sceKernelReserveVirtualRange(&addr, 0x10000, 0x10, 0);
  CHECK_EQUAL(0, result);
  // Keep track of this address.
  uint64_t addr1 = addr;
  uint64_t addr2 = addr1 + 0x10000;
  result         = sceKernelReserveVirtualRange(&addr2, 0x10000, 0x10, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelSetVirtualRangeName(addr1, 0x20000, "mapping");
  CHECK_EQUAL(0, result);

  // The two memory areas should not combine by default.
  uint64_t start;
  uint64_t end;
  int32_t  prot;
  result = sceKernelQueryMemoryProtection(addr1, &start, &end, &prot);
  CHECK_EQUAL(0, result);
  // Start should be addr1
  CHECK_EQUAL(addr1, start);
  // End should be addr2 + size.
  CHECK_EQUAL(addr1 + 0x10000, end);
  CHECK_EQUAL(0, prot);

  result = sceKernelQueryMemoryProtection(addr2, &start, &end, &prot);
  CHECK_EQUAL(0, result);
  // Start should be addr1
  CHECK_EQUAL(addr2, start);
  // End should be addr2 + size.
  CHECK_EQUAL(addr2 + 0x10000, end);
  CHECK_EQUAL(0, prot);

  // Unmap testing memory
  result = sceKernelMunmap(addr1, 0x20000);
  CHECK_EQUAL(0, result);
}