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

// Test behavior that changed in firmware 5.50, the differences will not be present here.
TEST(MemoryTests, FW550Test) {
  // Starting with firmware 5.50, memory mappings performed with different call addresses will now merge.
  int64_t phys_addr;
  int32_t result = sceKernelAllocateDirectMemory(0x100000, 0x1a0000, 0xa0000, 0, 0, &phys_addr);
  CHECK_EQUAL(0, result);

  uint64_t base_addr = 0x2000000000;
  uint64_t addr      = base_addr;
  result             = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x100000, 0);
  CHECK_EQUAL(0, result);

  addr   = base_addr + 0x80000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x180000, 0);
  CHECK_EQUAL(0, result);

  addr   = base_addr + 0x20000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x120000, 0);
  CHECK_EQUAL(0, result);

  addr   = base_addr + 0x60000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x160000, 0);
  CHECK_EQUAL(0, result);

  addr   = base_addr + 0x40000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x140000, 0);
  CHECK_EQUAL(0, result);

  mem_scan();

  // There should be five observable mappings.
  uint64_t start_addr;
  uint64_t end_addr;
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(base_addr, start_addr);
  CHECK_EQUAL(base_addr + 0x20000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x20000, &start_addr, &end_addr, nullptr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(base_addr + 0x20000, start_addr);
  CHECK_EQUAL(base_addr + 0x40000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x40000, &start_addr, &end_addr, nullptr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(base_addr + 0x40000, start_addr);
  CHECK_EQUAL(base_addr + 0x60000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x60000, &start_addr, &end_addr, nullptr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(base_addr + 0x60000, start_addr);
  CHECK_EQUAL(base_addr + 0x80000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x80000, &start_addr, &end_addr, nullptr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(base_addr + 0x80000, start_addr);
  CHECK_EQUAL(base_addr + 0xa0000, end_addr);

  // Now call sceKernelSetVirtualRangeName
  result = sceKernelSetVirtualRangeName(base_addr, 0xa0000, "Mapping");
  CHECK_EQUAL(0, result);

  mem_scan();

  // Because of the older SDK version, the mappings remain separate.
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(base_addr, start_addr);
  CHECK_EQUAL(base_addr + 0x20000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x20000, &start_addr, &end_addr, nullptr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(base_addr + 0x20000, start_addr);
  CHECK_EQUAL(base_addr + 0x40000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x40000, &start_addr, &end_addr, nullptr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(base_addr + 0x40000, start_addr);
  CHECK_EQUAL(base_addr + 0x60000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x60000, &start_addr, &end_addr, nullptr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(base_addr + 0x60000, start_addr);
  CHECK_EQUAL(base_addr + 0x80000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x80000, &start_addr, &end_addr, nullptr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(base_addr + 0x80000, start_addr);
  CHECK_EQUAL(base_addr + 0xa0000, end_addr);

  // Unmap testing memory.
  result = sceKernelReleaseDirectMemory(phys_addr, 0xa0000);
  CHECK_EQUAL(0, result);
}