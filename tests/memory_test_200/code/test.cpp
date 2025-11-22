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

// Test behavior that changed in firmware 2.00
TEST(MemoryTests, FW200Test) {
  auto map_func = [](uint64_t* addr, uint64_t size, int32_t fd, uint64_t offset, int32_t flags) __attribute__((optnone)) {
    // This function is effectively an mmap wrapper, except for the case of mapping direct memory.
    if (fd == -1 && offset != 0) {
      // Used for a direct memory mapping, use sceKernelAllocateDirectMemory and sceKernelMapDirectMemory.
      int64_t phys_addr = 0;
      int32_t result    = sceKernelAllocateDirectMemory(offset, offset + size, size, 0, 0, &phys_addr);
      if (result < 0) {
        return result;
      }
      result = sceKernelMapDirectMemory(addr, size, 0x33, flags, phys_addr, 0);
      if (result < 0) {
        sceKernelReleaseDirectMemory(phys_addr, size);
      }
      return result;
    }
    int32_t prot = 0x33;
    if (fd != -1) {
      // Can't GPU map files.
      prot = 0x3;
    }
    if ((flags & 0x100) == 1) {
      // Reserved memory has no protection, mmap just ignores prot.
      prot = 0;
    }
    return sceKernelMmap(*addr, size, prot, flags, fd, offset, addr);
  };

  auto unmap_func = [](uint64_t addr, uint64_t size) __attribute__((optnone)) {
    // We can use a VirtualQuery to see if this is direct or not
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
    int32_t result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
    if (info.is_direct == 1) {
      // Just release the direct memory, which will unmap for us.
      int64_t offset = (addr - info.start) + info.offset;
      result         = sceKernelReleaseDirectMemory(offset, size);
    } else {
      result = sceKernelMunmap(addr, size);
    }
    return result;
  };

  // In this firmware, direct memory mappings now coalesce.
  uint64_t base_addr = 0x2000000000;
  uint64_t addr      = base_addr;
  int32_t  result    = map_func(&addr, 0x20000, -1, 0x100000, 0x10);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x80000;
  result = map_func(&addr, 0x20000, -1, 0x180000, 0x10);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x20000;
  result = map_func(&addr, 0x20000, -1, 0x120000, 0x10);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x60000;
  result = map_func(&addr, 0x20000, -1, 0x160000, 0x10);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // There should be two observable mappings.
  uint64_t start_addr;
  uint64_t end_addr;
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0x40000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x60000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x60000, start_addr);
  LONGS_EQUAL(base_addr + 0xa0000, end_addr);

  // Test making a mapping in between these other mappings.
  addr   = base_addr + 0x40000;
  result = map_func(&addr, 0x20000, -1, 0x140000, 0x10);
  UNSIGNED_INT_EQUALS(0, result);

  // Mappings all merge together.
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0xa0000, end_addr);

  // Unmap testing memory.
  result = unmap_func(base_addr, 0xa0000);
  UNSIGNED_INT_EQUALS(0, result);
}

// Test behavior that changed in firmware 2.50, to verify the changes aren't visible here.
TEST(MemoryTests, FW250Test) {
  int64_t phys_addr = 0;
  int32_t result    = sceKernelAllocateMainDirectMemory(0x10000, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);
  uint64_t addr = 0;
  result        = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  // Starting with firmware 2.50, several direct memory functions that previously called sys_mmap will now use sceKernelMapDirectMemory2 instead.
  // This behavior will not be present here, which can be identified through overlapping dmem use.

  // All of these mappings would fail on firmware 2.50, but succeed on anything below that.
  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMapNamedDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0, "name");
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelInternalMapDirectMemory(1, &addr, 0x10000, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelInternalMapNamedDirectMemory(1, &addr, 0x10000, 3, 0, phys_addr, 0, "name");
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // This will unmap all memory tied to this physical area.
  result = sceKernelReleaseDirectMemory(phys_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);
}