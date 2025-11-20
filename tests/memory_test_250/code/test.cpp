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

// Test behavior that changed in firmware 2.50
TEST(MemoryTests, FW250Test) {
  int64_t phys_addr = 0;
  int32_t result    = sceKernelAllocateMainDirectMemory(0x10000, 0, 0, &phys_addr);
  CHECK_EQUAL(0, result);
  uint64_t addr = 0;
  result        = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);
  // Starting with firmware 2.50, several direct memory functions that previously called sys_mmap will now use sceKernelMapDirectMemory2 instead.
  // This includes sceKernelMapDirectMemory, sceKernelMapNamedDirectMemory, sceKernelInternalMapDirectMemory, and sceKernelInternalMapNamedDirectMemory.
  // This behavior is reverted if you use flag MAP_STACK (0x400).

  // We can abuse the overlapping dmem error case sceKernelMapDirectMemory2 has to identify this.
  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EBUSY, result);
  result = sceKernelMapNamedDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0, "name");
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EBUSY, result);
  result = sceKernelInternalMapDirectMemory(1, &addr, 0x10000, 3, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EBUSY, result);
  result = sceKernelInternalMapNamedDirectMemory(1, &addr, 0x10000, 3, 0, phys_addr, 0, "name");
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EBUSY, result);

  // Then check the flag behavior. If MAP_STACK behaves appropriately, these mappings will all succeed instead.
  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0x400, phys_addr, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);
  result = sceKernelMapNamedDirectMemory(&addr, 0x10000, 3, 0x400, phys_addr, 0, "name");
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);
  result = sceKernelInternalMapDirectMemory(1, &addr, 0x10000, 3, 0x400, phys_addr, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);
  result = sceKernelInternalMapNamedDirectMemory(1, &addr, 0x10000, 3, 0x400, phys_addr, 0, "name");
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);

  // This will unmap all memory tied to this physical area.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  CHECK_EQUAL(0, result);
}

// Test behavior that changed in firmware 3.00
TEST(MemoryTests, FW300Test) {
  // Starting with firmware 3.00, mmap calls with MAP_STACK and address greater than 0xfc00000000 should fail with ENOMEM
  // Since this homebrew compiles with SDK version 2.50, these differences should not be present here.
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
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xff00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

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
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xfc00004000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1010, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xff00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1010, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

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
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xfc00004000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1000, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xff00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x1000, -1, 0, &addr_out);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  CHECK_EQUAL(0, result);

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
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xfc00004000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -1, 3, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x4000);
  CHECK_EQUAL(0, result);

  addr   = 0xff00000000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -1, 3, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x4000);
  CHECK_EQUAL(0, result);

  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x4000);
  CHECK_EQUAL(0, result);
}