#include "CppUTest/TestHarness.h"

#include <orbis/_types/kernel.h>
#include <orbis/libkernel.h>
#include <stdio.h>
#include <string>
#include <unistd.h>

TEST_GROUP(MemoryTests) {void setup() {

} void teardown() {

}};

static void TestMemoryWrite(void* addr, uint64_t size) {
  // This function writes 0's to the requested memory area.
  void* test_addr = malloc(size);
  memset(test_addr, 0, size);

  // This memcpy will crash if the specified memory area isn't writable.
  memcpy(addr, test_addr, size);

  // Free memory after testing
  free(test_addr);
}

static bool TestZeroedMemory(void* addr, uint64_t size) {
  // This function reads 0's from the requested memory area, and compares them to an area full of zeros.
  void* test_addr = malloc(size);
  memset(test_addr, 0, size);

  // This memcpy will crash if the specified memory area isn't writable.
  bool succ = memcmp(addr, test_addr, size) == 0;

  // Free memory after testing
  free(test_addr);

  return succ;
}

TEST(MemoryTests, ExecutableTests) {
  // This tests to make sure executable memory behaves properly.
  // Allocate some direct memory for the direct memory tests
  uint64_t dmem_size = sceKernelGetDirectMemorySize();
  int64_t  phys_addr = 0;
  uint64_t size      = 0x100000;
  int32_t  result    = sceKernelAllocateDirectMemory(0, dmem_size, size, 0, 0, &phys_addr);
  CHECK_EQUAL(0, result);

  // Using the physical addresses, map direct memory to a flexible address.
  void*   addr = nullptr;
  int32_t prot = 0x37;
  // Direct memory mapping functions do not allow for assigning executable permissions. Test this edge case.
  result = sceKernelMapDirectMemory(&addr, size, prot, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EACCES, result);

  // Perform a successful direct memory mapping.
  prot   = 0x33;
  result = sceKernelMapDirectMemory(&addr, size, prot, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);

  // Run sceKernelQueryMemoryProtection to confirm the protection value assigned to this mapping.
  int32_t old_prot = 0;
  // We don't care about the other data, supply nullptrs for start and end.
  result = sceKernelQueryMemoryProtection(addr, nullptr, nullptr, &old_prot);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(prot, old_prot);

  // Use sceKernelMprotect to give the memory executable permissions.
  prot   = 0x37;
  result = sceKernelMprotect(addr, size, prot);
  CHECK_EQUAL(0, result);

  // Direct memory never actually exposes execute permissions.
  // Confirm this behavior by running sceKernelQueryMemoryProtection again.
  int32_t new_prot = 0;
  result           = sceKernelQueryMemoryProtection(addr, nullptr, nullptr, &new_prot);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(old_prot, new_prot);

  // Write a basic function into memory that returns 256.
  uint8_t bytes[] = {0x48, 0xc7, 0xc0, 00, 01, 00, 00, 0xc3};
  std::memcpy(addr, bytes, 8);
  typedef uint64_t (*func)();
  func     test_func = reinterpret_cast<func>(addr);
  uint64_t res       = test_func();
  CHECK_EQUAL(256, res);

  // Unmap the direct memory used for this test.
  result = sceKernelMunmap(addr, size);
  CHECK_EQUAL(0, result);

  // Release direct memory allocated for this test.
  result = sceKernelReleaseDirectMemory(phys_addr, size);
  CHECK_EQUAL(0, result);

  // Run a similar test utilizing flexible memory instead. Flexible memory allows and exposes executable permissions.
  addr   = nullptr;
  result = sceKernelMapFlexibleMemory(&addr, size, prot, 0);
  CHECK_EQUAL(0, result);

  // Validate the protection given to the memory area
  result = sceKernelQueryMemoryProtection(addr, nullptr, nullptr, &old_prot);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(prot, old_prot);

  // Write a basic function into memory that returns 256.
  std::memcpy(addr, bytes, 8);
  test_func = reinterpret_cast<func>(addr);
  res       = test_func();
  CHECK_EQUAL(256, res);

  // Unmap the flexible memory used for this test.
  result = sceKernelMunmap(addr, size);
  CHECK_EQUAL(0, result);
}

TEST(MemoryTests, AlignmentTests) {
  // This tests for behaviors relating to memory alignment
  uint64_t alignment = getpagesize();
  CHECK_EQUAL(0x4000, alignment);

  // Start by testing mmap behavior with misaligned address and size.
  void*    addr  = nullptr;
  uint64_t size  = 0x3000;
  int32_t  prot  = 0x3;
  int32_t  flags = 0x1000;

  int32_t result = sceKernelMmap(addr, size, prot, flags, -1, 0, &addr);
  CHECK_EQUAL(0, result);
  // Test to ensure addr is aligned
  uint64_t addr_value = reinterpret_cast<uint64_t>(addr);
  CHECK_EQUAL(0, addr_value % alignment);

  // Check the size of the mapping
  OrbisKernelVirtualQueryInfo info;
  memset(&info, 0, sizeof(info));
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  CHECK_EQUAL(0, result);
  uint64_t calc_size = reinterpret_cast<uint64_t>(info.end_addr) - reinterpret_cast<uint64_t>(info.start_addr);
  CHECK_EQUAL(0, calc_size % alignment);
  CHECK(calc_size > size);

  // Write to the full reported memory space to ensure the memory is actually mapped.
  // This will crash on emulators that don't emulate this behavior properly.
  TestMemoryWrite(addr, calc_size);

  // Unmap the flexible memory used for this test.
  result = sceKernelMunmap(addr, calc_size);
  CHECK_EQUAL(0, result);

  // Misaligned address with flags fixed should fail.
  void* test_addr = reinterpret_cast<void*>(addr_value + 0x1000);
  size            = 0x4000;
  flags           = 0x1010;
  result          = sceKernelMmap(test_addr, size, prot, flags, -1, 0, &test_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Without flags fixed, this should both addr and size up.
  size   = 0x3000;
  flags  = 0x1000;
  result = sceKernelMmap(test_addr, size, prot, flags, -1, 0, &test_addr);
  CHECK_EQUAL(0, result);

  // Test to ensure addr is aligned
  addr_value = reinterpret_cast<uint64_t>(test_addr);
  CHECK_EQUAL(0, addr_value % alignment);
  // Address rounds up, so it should be greater than addr
  CHECK(addr_value > reinterpret_cast<uint64_t>(addr));

  // Check the size of the mapping
  memset(&info, 0, sizeof(info));
  result = sceKernelVirtualQuery(test_addr, 0, &info, sizeof(info));
  CHECK_EQUAL(0, result);
  calc_size = reinterpret_cast<uint64_t>(info.end_addr) - reinterpret_cast<uint64_t>(info.start_addr);
  CHECK_EQUAL(0, calc_size % alignment);
  // Size should align up
  CHECK_EQUAL(0x4000, calc_size);

  // Write to the full reported memory space to ensure the memory is actually mapped.
  // This will crash on emulators that don't emulate this behavior properly.
  TestMemoryWrite(test_addr, calc_size);

  // Unmap the flexible memory used for this test.
  result = sceKernelMunmap(test_addr, calc_size);
  CHECK_EQUAL(0, result);

  // Now test for unmap alignment behavior.
  // Start by mmap'ing 4 pages of memory. This should be enough to check for most edge cases.
  addr_value = 0x10000000000;
  addr       = reinterpret_cast<void*>(addr_value);
  size       = 0x10000;
  flags      = 0x1010;
  result     = sceKernelMmap(addr, size, prot, flags, -1, 0, &addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(reinterpret_cast<void*>(addr_value), addr);

  // Munmap should align down the address, and align up the length.
  // This should unmap only the second page of that mapping.
  uint64_t test_addr_value = addr_value + 0x7000;
  uint64_t test_size       = 0x1000;
  result                   = sceKernelMunmap(reinterpret_cast<void*>(test_addr_value), test_size);
  CHECK_EQUAL(0, result);

  memset(&info, 0, sizeof(info));
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  CHECK_EQUAL(0, result);

  // Expected start addr is addr_value, expected end is addr_value + 0x4000.
  CHECK_EQUAL(addr, info.start_addr);
  CHECK_EQUAL(reinterpret_cast<void*>(addr_value + 0x4000), info.end_addr);

  // Make sure all of this memory is actually mapped.
  TestMemoryWrite(info.start_addr, alignment);

  // Run VirtualQuery on the remaining addresses that should be mapped.
  test_addr_value = addr_value + 0x8000;
  memset(&info, 0, sizeof(info));
  result = sceKernelVirtualQuery(reinterpret_cast<void*>(test_addr_value), 0, &info, sizeof(info));
  CHECK_EQUAL(0, result);

  // Expected start addr is addr_value + 0x8000, expected end is addr_value + 0x10000.
  CHECK_EQUAL(reinterpret_cast<void*>(addr_value + 0x8000), info.start_addr);
  CHECK_EQUAL(reinterpret_cast<void*>(addr_value + 0x10000), info.end_addr);

  // Make sure all of this memory is actually mapped.
  TestMemoryWrite(info.start_addr, alignment * 2);

  // Place a mapping where the unmap occurred.
  test_addr_value = addr_value + 0x4000;
  test_addr       = reinterpret_cast<void*>(test_addr_value);
  result          = sceKernelMmap(test_addr, alignment, prot, flags, -1, 0, &test_addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(reinterpret_cast<void*>(test_addr_value), test_addr);

  // Make sure all the memory is mapped
  TestMemoryWrite(reinterpret_cast<void*>(addr_value), alignment * 4);

  // Unmap the memory used for this test.
  result = sceKernelMunmap(reinterpret_cast<void*>(addr_value), alignment * 4);
  CHECK_EQUAL(0, result);
}

TEST(MemoryTests, LimitTests) {
  // This test will test a couple "limits" based around game behaviors I've witnessed.
  // Various EA games like to hardcode the address 0x500010000000 in a fixed mapping.
  void*    addr  = reinterpret_cast<void*>(0x500010000000);
  uint64_t size  = 0x100000;
  int32_t  prot  = 0x3;
  int32_t  flags = 0x1010;

  // Map the memory at the address
  int32_t result = sceKernelMmap(addr, size, prot, flags, -1, 0, &addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(reinterpret_cast<void*>(0x500010000000), addr);

  // Ensure the full page is writable.
  TestMemoryWrite(addr, size);

  // Unmap the memory.
  result = sceKernelMunmap(addr, size);
  CHECK_EQUAL(0, result);

  // Some Saint's Row games map to a low fixed addr, make sure this is free too.
  addr = reinterpret_cast<void*>(0x10410000);
  size = 0x1000000;
  // Specify no-overwrite too, just like they do.
  flags = 0x1090;

  result = sceKernelMmap(addr, size, prot, flags, -1, 0, &addr);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(reinterpret_cast<void*>(0x10410000), addr);

  // Ensure the full page is writable.
  TestMemoryWrite(addr, size);

  // Unmap the memory.
  result = sceKernelMunmap(addr, size);
  CHECK_EQUAL(0, result);

  // Ensure there's a proper limit attached to flexible memory
  int32_t max_iterations = 1000;
  void*   base_addr      = nullptr;
  addr                   = reinterpret_cast<void*>(0x1000000000);
  uint64_t mapped_size   = 0;
  flags                  = 0x1000;
  for (int32_t i = 1; i <= max_iterations; i++) {
    CHECK(i != max_iterations);
    // Run mmap in a loop to use all flexible memory.
    result = sceKernelMapFlexibleMemory(&addr, size, prot, flags);
    if (base_addr == nullptr) {
      base_addr = addr;
    }
    if (result == 0) {
      mapped_size += size;
    } else {
      CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
      break;
    }
  }

  // Unmap the flexible memory
  result = sceKernelMunmap(base_addr, mapped_size);

  // Ensure there's a proper limit attached to system memory
  max_iterations = 1000;
  base_addr      = nullptr;
  addr           = reinterpret_cast<void*>(0x1000000000);
  mapped_size    = 0;
  flags          = 0x3000; // Add system flag.
  for (int32_t i = 1; i <= max_iterations; i++) {
    CHECK(i != max_iterations);
    // Run mmap in a loop to use all flexible memory.
    result = sceKernelMmap(addr, size, prot, flags, -1, 0, &addr);
    if (base_addr == nullptr) {
      base_addr = addr;
    }
    if (result == 0) {
      mapped_size += size;
    } else {
      CHECK_EQUAL(result, ORBIS_KERNEL_ERROR_EINVAL);
      CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, result);
      break;
    }
  }

  // Unmap the flexible memory
  result = sceKernelMunmap(base_addr, mapped_size);

  // Ensure there's a proper limit attached to direct memory
  uint64_t dmem_size     = sceKernelGetDirectMemorySize();
  max_iterations         = 1000;
  base_addr              = nullptr;
  int64_t base_phys_addr = 0;
  addr                   = reinterpret_cast<void*>(0x1000000000);
  mapped_size            = 0;
  flags                  = 0;
  for (int32_t i = 1; i <= max_iterations; i++) {
    CHECK(i != max_iterations);
    // Run mmap in a loop to use all flexible memory.
    int64_t phys_addr = 0;
    result            = sceKernelAllocateDirectMemory(0, dmem_size, size, 0, 0, &phys_addr);
    if (result != 0) {
      CHECK_EQUAL(ORBIS_KERNEL_ERROR_EAGAIN, result);
      break;
    }
    if (base_phys_addr == 0) {
      base_phys_addr = phys_addr;
    }

    result = sceKernelMapDirectMemory(&addr, size, prot, flags, phys_addr, 0);
    // sceKernelMapDirectMemory should never fail in this loop
    CHECK_EQUAL(0, result);
    if (base_addr == nullptr) {
      base_addr = addr;
    }
    if (result == 0) {
      mapped_size += size;
    }
  }

  // Unmap the memory from this test.
  result = sceKernelMunmap(base_addr, mapped_size);
  CHECK_EQUAL(0, result);
  result = sceKernelReleaseDirectMemory(base_phys_addr, mapped_size);
  CHECK_EQUAL(0, result);
}

TEST(MemoryTests, FlagTests) {
  // Make sure no-overlap and fixed flags behave properly, since that's what most games use.
  void*    addr  = nullptr;
  uint64_t size  = 0x1000000;
  int32_t  flags = 0x10;

  // Fixed with addr nullptr should ignore fixed flags.
  int32_t result = sceKernelReserveVirtualRange(&addr, size, flags, 0);
  CHECK_EQUAL(0, result);
  CHECK(addr != nullptr);

  // Regardless of flags or reservations, mmap shouldn't map to the reserved page.
  void* mmap_addr = nullptr;
  result          = sceKernelMmap(addr, size, 0x3, 0x1000, -1, 0, &mmap_addr);
  CHECK_EQUAL(0, result);
  CHECK(mmap_addr != addr);

  // Unmap the mmap
  result = sceKernelMunmap(mmap_addr, size);
  CHECK_EQUAL(0, result);

  // Perform a mapping with fixed | no-overwrite. This should fail.
  result = sceKernelMmap(addr, size, 0x3, 0x1090, -1, 0, &mmap_addr);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // Perform a fixed mapping, this should overwrite the reserved page with usable memory.
  result = sceKernelMmap(addr, size, 0x3, 0x1010, -1, 0, &mmap_addr);
  CHECK_EQUAL(0, result);
  CHECK(mmap_addr == addr);

  // Perform another fixed mapping, this should overwrite the newly mapped page.
  result = sceKernelMmap(addr, size, 0x3, 0x1010, -1, 0, &mmap_addr);
  CHECK_EQUAL(0, result);
  CHECK(mmap_addr == addr);

  // Unmap the mmap
  result = sceKernelMunmap(mmap_addr, size);
  CHECK_EQUAL(0, result);
}

TEST(MemoryTests, BackingTests) {
  // This tests to ensure memory is handled properly for direct and flexible mappings.
  // Allocate some dmem for the test.
  uint64_t dmem_size = sceKernelGetDirectMemorySize();
  int64_t  phys_addr = 0;
  uint64_t size      = 0x1000000;
  int32_t  result    = sceKernelAllocateDirectMemory(0, dmem_size, size, 0, 0, &phys_addr);
  CHECK_EQUAL(0, result);

  // Map an area of dmem
  void* addr = nullptr;
  result     = sceKernelMapDirectMemory(&addr, size, 0x3, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);

  // Write a bunch of zeros to it.
  TestMemoryWrite(addr, size);

  // Make sure those zeros are all written
  CHECK(TestZeroedMemory(addr, size));

  // Unmap the dmem
  result = sceKernelMunmap(addr, size);
  CHECK_EQUAL(0, result);

  // Re-map the physical space
  addr   = nullptr;
  result = sceKernelMapDirectMemory(&addr, size, 0x3, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);

  // Ensure the zeros are still present
  CHECK(TestZeroedMemory(addr, size));

  // Attempt to map a second mapping to this space through sceKernelMapDirectMemory2. This should fail.
  result = sceKernelMapDirectMemory2(&addr, size, 0, 0x3, 0, phys_addr, 0);
  CHECK_EQUAL(ORBIS_KERNEL_ERROR_EBUSY, result);

  // Use sceKernelMapDirectMemory to successfully map.
  void* addr_two = nullptr;
  result         = sceKernelMapDirectMemory(&addr_two, size, 0x3, 0, phys_addr, 0);
  CHECK_EQUAL(0, result);

  // Ensure the zeros are present in the new memory.
  CHECK(TestZeroedMemory(addr_two, size));

  // Unmap the first area.
  result = sceKernelMunmap(addr, size);
  CHECK_EQUAL(0, result);

  // Ensure the zeros are present in the new memory.
  CHECK(TestZeroedMemory(addr_two, size));

  // Unmap the second area.
  result = sceKernelMunmap(addr_two, size);
  CHECK_EQUAL(0, result);

  // Release the direct memory
  result = sceKernelReleaseDirectMemory(phys_addr, size);
  CHECK_EQUAL(0, result);
}

TEST(MemoryTests, ProtectTests) {
  // This tests potential edge cases involving mprotect.
  // Start by mapping memory with no permissions
  void*    addr   = nullptr;
  uint64_t size   = 0x100000;
  int32_t  flags  = 0;
  int32_t  prot   = 0;
  int32_t  result = sceKernelMapFlexibleMemory(&addr, size, prot, flags);
  CHECK_EQUAL(0, result);

  uint64_t addr_value = reinterpret_cast<uint64_t>(addr);

  // Use sceKernelMprotect to protect the middle of this memory area.
  // For testing purposes, use misaligned address and size.
  void*    protect_addr = reinterpret_cast<void*>(addr_value + 0x43000);
  uint64_t protect_size = 0x7D000;
  prot                  = 1;
  result                = sceKernelMprotect(protect_addr, protect_size, prot);
  CHECK_EQUAL(0, result);

  // Verify permissions and alignment using sceKernelQueryMemoryProtection.
  // Address is expected to align down, while the size should align up.
  void*   start_addr;
  void*   end_addr;
  int32_t out_prot;
  result = sceKernelQueryMemoryProtection(protect_addr, &start_addr, &end_addr, &out_prot);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(reinterpret_cast<void*>(addr_value + 0x40000), start_addr);
  CHECK_EQUAL(reinterpret_cast<void*>(addr_value + 0xC0000), end_addr);
  CHECK_EQUAL(prot, out_prot);

  // Ensure we can read from this memory by calling TestZeroedMemory.
  // Ignore the return, all we care about is that reading doesn't crash.
  TestZeroedMemory(reinterpret_cast<void*>(addr_value + 0x40000), 0x80000);

  // Use sceKernelMprotect to protect the middle of this memory area.
  // For this test, only provide write protections.
  protect_addr = reinterpret_cast<void*>(addr_value + 0x40000);
  protect_size = 0x80000;
  prot         = 2;
  result       = sceKernelMprotect(protect_addr, protect_size, prot);
  CHECK_EQUAL(0, result);

  // Validate returned protection, libkernel adds read protection to write-only pages.
  out_prot = 0;
  result   = sceKernelQueryMemoryProtection(protect_addr, nullptr, nullptr, &out_prot);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(3, out_prot);

  // Call TestMemoryWrite to make sure writes work properly, then TestZeroedMemory to check reads.
  TestMemoryWrite(protect_addr, protect_size);
  CHECK(TestZeroedMemory(protect_addr, protect_size));

  // Use sceKernelMprotect to give the entire original mapping read permissions
  prot   = 1;
  result = sceKernelMprotect(addr, size, prot);
  CHECK_EQUAL(0, result);

  // Ensure prot applied properly.
  result = sceKernelQueryMemoryProtection(addr, nullptr, nullptr, &out_prot);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(prot, out_prot);

  // Ensure we can read from this memory by calling TestZeroedMemory.
  // Ignore the return, all we care about is that reading doesn't crash.
  TestZeroedMemory(addr, size);

  // Unmap memory used for test.
  result = sceKernelMunmap(addr, size);
  CHECK_EQUAL(0, result);
}