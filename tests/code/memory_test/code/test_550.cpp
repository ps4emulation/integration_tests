#include "test.h"

#include <CppUTest/TestHarness.h>
#include <cstdio>

TEST_GROUP (MemoryTests) {
  void setup() { // Before each test, call mem_scan to print out memory map information.
    // This will provide an indicator of how the memory map looks, which can help with debugging strange behavior during tests.
    printf("Before test:\n");
    mem_scan();
  }

  void teardown() {}
};

// Test behavior that changed in firmware 5.50
TEST(MemoryTests, FW550Test) {
  // Starting with firmware 5.50, memory mappings performed with different call addresses will now merge.
  // This ignores the anon names, but not other memory names.
  int64_t phys_addr;
  int32_t result = sceKernelAllocateDirectMemory(0x100000, 0x1a0000, 0xa0000, 0, 0, &phys_addr);

  uint64_t base_addr = 0x2000000000;
  uint64_t addr      = base_addr;
  result             = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x100000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelSetVirtualRangeName(addr, 0x20000, "Mapping1");
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x80000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x180000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelSetVirtualRangeName(addr, 0x20000, "Mapping5");
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x20000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x120000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelSetVirtualRangeName(addr, 0x20000, "Mapping2");
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x60000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x160000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelSetVirtualRangeName(addr, 0x20000, "Mapping4");
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x40000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x140000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelSetVirtualRangeName(addr, 0x20000, "Mapping3");
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // There should be five observable mappings.
  uint64_t start_addr;
  uint64_t end_addr;
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0x20000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x20000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x20000, start_addr);
  LONGS_EQUAL(base_addr + 0x40000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x40000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x40000, start_addr);
  LONGS_EQUAL(base_addr + 0x60000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x60000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x60000, start_addr);
  LONGS_EQUAL(base_addr + 0x80000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x80000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x80000, start_addr);
  LONGS_EQUAL(base_addr + 0xa0000, end_addr);

  // Now call sceKernelSetVirtualRangeName
  result = sceKernelSetVirtualRangeName(base_addr, 0xa0000, "Mapping");
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Mappings all merge together.
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0xa0000, end_addr);

  // Unmap testing memory.
  result = sceKernelReleaseDirectMemory(phys_addr, 0xa0000);
  UNSIGNED_INT_EQUALS(0, result);

  // Additionally, mappings with different anon names also end up merging.
  // The memory name preserved will always be the name of the newest mapping.
  result = sceKernelAllocateDirectMemory(0x100000, 0x1a0000, 0xa0000, 0, 0, &phys_addr);
  addr   = base_addr;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x100000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  addr   = base_addr + 0x80000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x180000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  addr   = base_addr + 0x20000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x120000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  addr   = base_addr + 0x60000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x160000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  addr   = base_addr + 0x40000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x140000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0xa0000, end_addr);

  // Unmap testing memory.
  result = sceKernelReleaseDirectMemory(phys_addr, 0xa0000);
  UNSIGNED_INT_EQUALS(0, result);
}

TEST(MemoryTests, TLOU2Test) {
  // Check for something that came up in fpPS4.
  // I'll probably rewrite this test at some point.
  int64_t first_phys_addr = 0;
  int32_t result          = sceKernelAllocateMainDirectMemory(0x100000000, 0x10000, 0, &first_phys_addr);
  UNSIGNED_INT_EQUALS(0, result);

  uint64_t first_addr = 0x1600000000;
  result              = sceKernelMapDirectMemory(&first_addr, 0x1D200000, 3, 0, first_phys_addr, 0x200000);
  UNSIGNED_INT_EQUALS(0, result);

  uint64_t second_addr = 0x3700000000;
  for (int32_t i = 0; i < 48; ++i) {
    uint64_t unmap_addr = first_addr + (i * 0x100000);
    result              = sceKernelMunmap(unmap_addr, 0x100000);
    UNSIGNED_INT_EQUALS(0, result);

    uint64_t map_addr = second_addr + (i * 0x100000);
    result            = sceKernelMapDirectMemory(&map_addr, 0x100000, 3, 0x90, first_phys_addr + (i * 0x100000), 0x100000);
    UNSIGNED_INT_EQUALS(0, result);
  }

  first_addr  = 0x1603600000;
  second_addr = 0x4100000000;

  for (int32_t i = 0; i < 10; ++i) {
    uint64_t unmap_addr = first_addr + (i * 0x100000);
    result              = sceKernelMunmap(unmap_addr, 0x100000);

    uint64_t map_addr = second_addr + (i * 0x100000);
    result            = sceKernelMapDirectMemory(&map_addr, 0x100000, 3, 0x90, first_phys_addr + 0x3600000 + (i * 0x100000), 0x100000);
    UNSIGNED_INT_EQUALS(0, result);
  }

  result = sceKernelMunmap(second_addr, 0xa00000);
  UNSIGNED_INT_EQUALS(0, result);

  for (int32_t i = 0; i < 10; ++i) {
    uint64_t map_addr = first_addr + (i * 0x100000);
    result            = sceKernelMapDirectMemory(&map_addr, 0x100000, 3, 0x90, first_phys_addr + 0x3600000 + (i * 0x100000), 0x100000);
    UNSIGNED_INT_EQUALS(0, result);
  }

  result = sceKernelCheckedReleaseDirectMemory(first_phys_addr, 0x100000000);
  UNSIGNED_INT_EQUALS(0, result);
}
