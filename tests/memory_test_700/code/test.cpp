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

// Test behavior that changed in firmware 7.00
TEST(MemoryTests, FW700Test) {
  // Starting with firmware 7.00, vm_map_set_name_str received a fix to a vm_map_simplify_entry call
  // This is probably where VMAs started coalescing properly? Needs testing.

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

  // The two memory areas should combine into one mapping.
  uint64_t start;
  uint64_t end;
  int32_t  prot;
  result = sceKernelQueryMemoryProtection(addr1, &start, &end, &prot);
  CHECK_EQUAL(0, result);
  // Start should be addr1
  CHECK_EQUAL(addr1, start);
  // End should be addr2 + size.
  CHECK_EQUAL(addr1 + 0x20000, end);
  CHECK_EQUAL(0, prot);
  // Unmap testing memory
  result = sceKernelMunmap(addr1, 0x20000);
  CHECK_EQUAL(0, result);

  // Use the MAP_NO_COALESCE flag to prevent coalescing
  addr   = 0x300000000;
  result = sceKernelReserveVirtualRange(&addr, 0x10000, 0x400010, 0);
  CHECK_EQUAL(0, result);
  addr1  = addr;
  addr2  = addr1 + 0x10000;
  result = sceKernelReserveVirtualRange(&addr2, 0x10000, 0x400010, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelSetVirtualRangeName(addr1, 0x20000, "mapping");
  CHECK_EQUAL(0, result);

  // We should have two separate mappings.
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

TEST(MemoryTests, TLOU2Test) {
  // Check for something that came up in fpPS4.
  // I'll probably rewrite this test at some point.
  int64_t first_phys_addr = 0;
  int32_t result          = sceKernelAllocateMainDirectMemory(0x100000000, 0x10000, 0, &first_phys_addr);
  CHECK_EQUAL(0, result);

  uint64_t first_addr = 0x1600000000;
  result              = sceKernelMapDirectMemory(&first_addr, 0x1D200000, 3, 0, first_phys_addr, 0x200000);
  CHECK_EQUAL(0, result);

  uint64_t second_addr = 0x3700000000;
  for (int32_t i = 0; i < 48; ++i) {
    uint64_t unmap_addr = first_addr + (i * 0x100000);
    result              = sceKernelMunmap(unmap_addr, 0x100000);
    CHECK_EQUAL(0, result);

    uint64_t map_addr = second_addr + (i * 0x100000);
    result            = sceKernelMapDirectMemory(&map_addr, 0x100000, 3, 0x90, first_phys_addr + (i * 0x100000), 0x100000);
    CHECK_EQUAL(0, result);
  }

  first_addr  = 0x1603600000;
  second_addr = 0x4100000000;

  for (int32_t i = 0; i < 10; ++i) {
    uint64_t unmap_addr = first_addr + (i * 0x100000);
    result              = sceKernelMunmap(unmap_addr, 0x100000);

    uint64_t map_addr = second_addr + (i * 0x100000);
    result            = sceKernelMapDirectMemory(&map_addr, 0x100000, 3, 0x90, first_phys_addr + 0x3600000 + (i * 0x100000), 0x100000);
    CHECK_EQUAL(0, result);
  }

  result = sceKernelMunmap(second_addr, 0xa00000);
  CHECK_EQUAL(0, result);

  for (int32_t i = 0; i < 10; ++i) {
    uint64_t map_addr = first_addr + (i * 0x100000);
    result            = sceKernelMapDirectMemory(&map_addr, 0x100000, 3, 0x90, first_phys_addr + 0x3600000 + (i * 0x100000), 0x100000);
    CHECK_EQUAL(0, result);
  }

  result = sceKernelCheckedReleaseDirectMemory(first_phys_addr, 0x100000000);
  CHECK_EQUAL(0, result);
}