#include "CppUTest/TestHarness.h"

#include <list>
#include <stdio.h>
#include <string>

extern "C" {
// Direct memory functions
int32_t sceKernelMapDirectMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags, int64_t phys_addr, uint64_t alignment);
int32_t sceKernelMapNamedDirectMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags, int64_t phys_addr, uint64_t alignment, const char* name);
int32_t sceKernelInternalMapDirectMemory(int32_t pool, uint64_t* addr, uint64_t size, int32_t prot, int32_t flags, int64_t phys_addr, uint64_t alignment);
int32_t sceKernelInternalMapNamedDirectMemory(int32_t pool, uint64_t* addr, uint64_t size, int32_t prot, int32_t flags, int64_t phys_addr, uint64_t alignment,
                                              const char* name);

// Generic memory functions
int32_t sceKernelMmap(uint64_t addr, uint64_t size, int32_t prot, int32_t flags, int32_t fd, int64_t offset, uint64_t* out_addr);
int32_t sceKernelMunmap(uint64_t addr, uint64_t size);
int32_t sceKernelVirtualQuery(uint64_t addr, int32_t flags, void* info, uint64_t info_size);
}

// Some error codes
#define ORBIS_KERNEL_ERROR_EBUSY (0x80020010)

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

TEST_GROUP(MemoryTests) {
    void setup() {// Before each test, call mem_scan to print out memory map information.
                  // This will provide an indicator of how the memory map looks, which can help with debugging strange behavior during tests.
                  printf("Before test:\n");
mem_scan();
}

void teardown() {}
}
;

// Test behavior that changed in firmware 2.50
TEST(MemoryTests, FW250Test) {
  // Starting with firmware 2.50, several direct memory functions that previously called sys_mmap will now use sceKernelMapDirectMemory2 instead.
  // This includes sceKernelMapDirectMemory, sceKernelMapNamedDirectMemory, sceKernelInternalMapDirectMemory, and sceKernelInternalMapNamedDirectMemory.
  // This behavior is reverted if you use flag MAP_STACK (0x400).

  // We can abuse the overlapping dmem error case sceKernelMapDirectMemory2 has to identify this.
  uint64_t addr   = 0;
  int32_t  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EBUSY, result);
  result = sceKernelMapNamedDirectMemory(&addr, 0x10000, 3, 0, 0, 0, "name");
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EBUSY, result);
  result = sceKernelInternalMapDirectMemory(1, &addr, 0x10000, 3, 0, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EBUSY, result);
  result = sceKernelInternalMapNamedDirectMemory(1, &addr, 0x10000, 3, 0, 0, 0, "name");
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EBUSY, result);

  // Then check the flag behavior. If MAP_STACK behaves appropriately, these mappings will all succeed instead.
  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0x400, 0, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);
  result = sceKernelMapNamedDirectMemory(&addr, 0x10000, 3, 0x400, 0, 0, "name");
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);
  result = sceKernelInternalMapDirectMemory(1, &addr, 0x10000, 3, 0x400, 0, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);
  result = sceKernelInternalMapNamedDirectMemory(1, &addr, 0x10000, 3, 0x400, 0, 0, "name");
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
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
}