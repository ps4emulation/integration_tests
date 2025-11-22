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

// Test behavior that changed in firmware 1.70
TEST(MemoryTests, FW170Test) {
  int64_t phys_addr = 0;
  int32_t result    = sceKernelAllocateMainDirectMemory(0x4000, 0, 0, &phys_addr);

  // Null address with MAP_FIXED is now prohibited from the libkernel side.
  uint64_t addr = 0;
  result        = sceKernelMapDirectMemory(&addr, 0x4000, 3, 0x10, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelMapNamedDirectMemory(&addr, 0x4000, 3, 0x10, phys_addr, 0, "name");
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelMapFlexibleMemory(&addr, 0x4000, 3, 0x10);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelMapNamedFlexibleMemory(&addr, 0x4000, 3, 0x10, "name");
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelReserveVirtualRange(&addr, 0x4000, 0x10, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelMapNamedSystemFlexibleMemory(&addr, 0x4000, 3, 0x10, "name");
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Note: Pool 1 should match sceKernelMapDirectMemory behavior.
  result = sceKernelInternalMapDirectMemory(1, &addr, 0x4000, 3, 0x10, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelInternalMapNamedDirectMemory(1, &addr, 0x4000, 3, 0x10, phys_addr, 0, "name");
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // mmap and sceKernelMemoryPoolReserve both still reach the mmap syscall, but error there like fw 1.00 does.
  result = sceKernelMmap(addr, 0x4000, 3, 0x1010, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelMemoryPoolReserve(addr, 0x4000, 0, 0x10, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelReleaseDirectMemory(phys_addr, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);
}

// Test behavior that changed in firmware 2.00, these differences should not be present here.
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

  // In firmware 2.00, direct memory mappings coalesce.
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

  // On firmware 2.00, there will be 2 mappings. Here, there should be 4.
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

  result = sceKernelQueryMemoryProtection(base_addr + 0x60000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x60000, start_addr);
  LONGS_EQUAL(base_addr + 0x80000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x80000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x80000, start_addr);
  LONGS_EQUAL(base_addr + 0xa0000, end_addr);

  // Test making a mapping in between these other mappings.
  addr   = base_addr + 0x40000;
  result = map_func(&addr, 0x20000, -1, 0x140000, 0x10);
  UNSIGNED_INT_EQUALS(0, result);

  // There should now be 5 mappings.
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

  // Mappings split through mprotect and mtypeprotect will not re-coalesce on older SDK versions.
  result = sceKernelMprotect(base_addr + 0x4000, 0x8000, 0x3);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0x4000, end_addr);
  result = sceKernelQueryMemoryProtection(base_addr + 0x4000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x4000, start_addr);
  LONGS_EQUAL(base_addr + 0xc000, end_addr);
  result = sceKernelQueryMemoryProtection(base_addr + 0xc000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0xc000, start_addr);
  LONGS_EQUAL(base_addr + 0x20000, end_addr);

  result = sceKernelMprotect(base_addr, 0x20000, 0x33);
  UNSIGNED_INT_EQUALS(0, result);

  // Mappings remain separate.
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0x4000, end_addr);
  result = sceKernelQueryMemoryProtection(base_addr + 0x4000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x4000, start_addr);
  LONGS_EQUAL(base_addr + 0xc000, end_addr);
  result = sceKernelQueryMemoryProtection(base_addr + 0xc000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0xc000, start_addr);
  LONGS_EQUAL(base_addr + 0x20000, end_addr);

  // Areas split due to changed mtype
  result = sceKernelMtypeprotect(base_addr + 0x24000, 0x8000, 3, 0x33);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelQueryMemoryProtection(base_addr + 0x20000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x20000, start_addr);
  LONGS_EQUAL(base_addr + 0x24000, end_addr);
  result = sceKernelQueryMemoryProtection(base_addr + 0x24000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x24000, start_addr);
  LONGS_EQUAL(base_addr + 0x2c000, end_addr);
  result = sceKernelQueryMemoryProtection(base_addr + 0x2c000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x2c000, start_addr);
  LONGS_EQUAL(base_addr + 0x40000, end_addr);

  // Areas do not recombine.
  result = sceKernelMtypeprotect(base_addr + 0x20000, 0x20000, 0, 0x33);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelQueryMemoryProtection(base_addr + 0x20000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x20000, start_addr);
  LONGS_EQUAL(base_addr + 0x24000, end_addr);
  result = sceKernelQueryMemoryProtection(base_addr + 0x24000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x24000, start_addr);
  LONGS_EQUAL(base_addr + 0x2c000, end_addr);
  result = sceKernelQueryMemoryProtection(base_addr + 0x2c000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x2c000, start_addr);
  LONGS_EQUAL(base_addr + 0x40000, end_addr);

  // Areas split due to changed prot
  result = sceKernelMtypeprotect(base_addr + 0x44000, 0x8000, 0, 0x3);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelQueryMemoryProtection(base_addr + 0x40000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x40000, start_addr);
  LONGS_EQUAL(base_addr + 0x44000, end_addr);
  result = sceKernelQueryMemoryProtection(base_addr + 0x44000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x44000, start_addr);
  LONGS_EQUAL(base_addr + 0x4c000, end_addr);
  result = sceKernelQueryMemoryProtection(base_addr + 0x4c000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x4c000, start_addr);
  LONGS_EQUAL(base_addr + 0x60000, end_addr);

  // Areas do not recombine.
  result = sceKernelMtypeprotect(base_addr + 0x40000, 0x20000, 0, 0x33);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelQueryMemoryProtection(base_addr + 0x40000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x40000, start_addr);
  LONGS_EQUAL(base_addr + 0x44000, end_addr);
  result = sceKernelQueryMemoryProtection(base_addr + 0x44000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x44000, start_addr);
  LONGS_EQUAL(base_addr + 0x4c000, end_addr);
  result = sceKernelQueryMemoryProtection(base_addr + 0x4c000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x4c000, start_addr);
  LONGS_EQUAL(base_addr + 0x60000, end_addr);

  // Unmap testing memory.
  result = unmap_func(base_addr, 0xa0000);
  UNSIGNED_INT_EQUALS(0, result);
}