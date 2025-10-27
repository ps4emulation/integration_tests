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

// Flexible memory functions
int32_t sceKernelMapFlexibleMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags);
int32_t sceKernelMapNamedFlexibleMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags, const char* name);
int32_t sceKernelMapNamedSystemFlexibleMemory(uint64_t* addr, uint64_t size, int32_t prot, int32_t flags, const char* name);

// Reserve memory function
int32_t sceKernelReserveVirtualRange(uint64_t* addr, uint64_t size, int32_t flags, uint64_t alignment);

// Generic memory functions
int32_t sceKernelMmap(uint64_t addr, uint64_t size, int32_t prot, int32_t flags, int32_t fd, int64_t offset, uint64_t* out_addr);
int32_t sceKernelMunmap(uint64_t addr, uint64_t size);
int32_t sceKernelVirtualQuery(uint64_t addr, int32_t flags, void* info, uint64_t info_size);

// Memory pool functions
int32_t sceKernelMemoryPoolReserve(uint64_t addr_in, uint64_t len, uint64_t alignment, int32_t flags, uint64_t* addr_out);
}

// Some error codes
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

// Test behavior that changed in firmware 1.70
TEST(MemoryTests, FW170Test) {
  // Null address with MAP_FIXED is now prohibited from the libkernel side.
  uint64_t addr   = 0;
  int32_t  result = sceKernelMapDirectMemory(&addr, 0x4000, 3, 0x10, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelMapNamedDirectMemory(&addr, 0x4000, 3, 0x10, 0, 0, "name");
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelMapFlexibleMemory(&addr, 0x4000, 3, 0x10);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelMapNamedFlexibleMemory(&addr, 0x4000, 3, 0x10, "name");
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelReserveVirtualRange(&addr, 0x4000, 0x10, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelMapNamedSystemFlexibleMemory(&addr, 0x4000, 3, 0x10, "name");
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Note: Pool 1 should match sceKernelMapDirectMemory behavior.
  result = sceKernelInternalMapDirectMemory(1, &addr, 0x4000, 3, 0x10, 0, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelInternalMapNamedDirectMemory(1, &addr, 0x4000, 3, 0x10, 0, 0, "name");
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // mmap and sceKernelMemoryPoolReserve both still reach the mmap syscall, but error there like fw 1.00 does.
  result = sceKernelMmap(addr, 0x4000, 3, 0x1010, -1, 0, &addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelMemoryPoolReserve(addr, 0x4000, 0, 0x10, &addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
}

// Test behavior that changed in firmware 2.50, to verify the changes aren't visible here.
TEST(MemoryTests, FW250Test) {
  // Starting with firmware 2.50, several direct memory functions that previously called sys_mmap will now use sceKernelMapDirectMemory2 instead.
  // This behavior will not be present here, which can be identified through overlapping dmem use.

  // All of these mappings would fail on firmware 2.50, but succeed on anything below that.
  uint64_t addr   = 0;
  int32_t  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, 0, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);
  result = sceKernelMapNamedDirectMemory(&addr, 0x10000, 3, 0, 0, 0, "name");
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);
  result = sceKernelInternalMapDirectMemory(1, &addr, 0x10000, 3, 0, 0, 0);
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);
  result = sceKernelInternalMapNamedDirectMemory(1, &addr, 0x10000, 3, 0, 0, 0, "name");
  CHECK_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  CHECK_EQUAL(0, result);
}