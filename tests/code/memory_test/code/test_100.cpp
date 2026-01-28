#include "test.h"

#include <CppUTest/TestHarness.h>
#include <cstdio>
#include <list>

TEST_GROUP (MemoryTests) {
  void setup() { // Before each test, call mem_scan to print out memory map information.
    // This will provide an indicator of how the memory map looks, which can help with debugging strange behavior during tests.
    printf("Before test:\n");
    mem_scan();
  }

  void teardown() {}
};

// These tests are at the top of the file so they run last.
// This is to prevent issues related to sceKernelEnableDmemAliasing, which afaik cannot be undone without direct syscall usage.
TEST(MemoryTests, MapMemoryTest) {
  // These tests assume a 16KB page size, so check getpagesize to validate this.
  int32_t page_size = getpagesize();
  LONGS_EQUAL(0x4000, page_size);

  // Most memory functions in libkernel rely on sceKernelMmap,
  // leading to overlap in some edge cases.
  // Start with testing libkernel's error returns for sceKernelMapFlexibleMemory
  // If size is less than page size, or size is not page aligned, return EINVAL
  uint64_t addr   = 0;
  int32_t  result = sceKernelMapFlexibleMemory(&addr, 0x5000, 0, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapFlexibleMemory(&addr, 0x2000, 0, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Protection should be a bitwise-or'ed combination of flags. It can be zero,
  // or include Read (1) | Write (2) | Execute (4) | GpuRead (0x10) | GpuWrite (0x20).
  // If a value is inputted for protection that doesn't match those flags, it returns EINVAL.
  result = sceKernelMapFlexibleMemory(&addr, 0x4000, 8, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapFlexibleMemory(&addr, 0x4000, 64, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // The only permitted flags are: Fixed (0x10) | NoOverwrite (0x80) | NoCoalesce (0x400000)
  // If flags is non-zero, and includes bits not contained in that list, then it returns EINVAL.
  result = sceKernelMapFlexibleMemory(&addr, 0x4000, 0, 1);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapFlexibleMemory(&addr, 0x4000, 0, 0x100);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapFlexibleMemory(&addr, 0x4000, 0, 0x2000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Other sceKernelMapFlexibleMemory Notes:
   * If address is not specified, it defaults to 0x200000000
   * Not possible to test here, but there's a condition it follows that instead defaults to 0x880000000
   * On SDK version < 1.70, flag Fixed with unspecified address would remove the Fixed flag.
   */

  // Now for edge cases in sceKernelMapDirectMemory. This function allows flags:
  // Fixed (0x10) | NoOverwrite (0x80) | DmemCompat (0x400) | Sanitizer (0x200000) | NoCoalesce (0x400000)
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 1, 0, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0x100, 0, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0x2000, 0, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Notes:
   * Flag DmemCompat is exclusive to libkernel code, mmap flag 0x400 is Stack.
   * For games compiled with sdk version < 2.50, DmemCompat does nothing.
   * For newer titles, it will redirect sceKernelMapDirectMemory to sceKernelMapDirectMemory2.
   * We can check for edge cases specific to the latter case when testing sys_mmap_dmem later.
   */

  // Protection should be a bitwise-or'ed combination of flags. It can be zero,
  // or include Read (1) | Write (2) | Execute (4) | GpuRead (0x10) | GpuWrite (0x20).
  // If a value is inputted for protection that doesn't match those flags, it returns EINVAL.
  // Note that the execute flag causes mmap to fail, that will come up during later edge case tests.
  result = sceKernelMapDirectMemory(&addr, 0x4000, 8, 0, 0, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapDirectMemory(&addr, 0x4000, 64, 0, 0, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Note:
   * The execute protection causes sceKernelMapDirectMemory calls to fail.
   * This will likely come up in sys_mmap tests.
   */

  // Allocate some direct memory to test with
  // Technically this isn't necessary at all, since the error returns should occur before any file mapping logic (and thus, before the dmem checks),
  // but this should make the tests less confusing to work with.
  int64_t dmem_start = 0x100000;
  int64_t dmem_size  = sceKernelGetDirectMemorySize();
  int64_t phys_addr  = 0;
  result             = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x8000, 0x4000, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Input address, length, phys_addr, and alignment all must be page-aligned.
  addr   = (uint64_t)0x200002000;
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  addr   = 0x200000000;
  result = sceKernelMapDirectMemory(&addr, 0x2000, 0, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0, phys_addr + 0x2000, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0, phys_addr, 0x2000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Additionally, alignment must be a power of two.
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0, phys_addr, 0xc000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Finally, alignment must be less than 0x100000000
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0, 0, phys_addr, 0x100000000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Release allocated direct memory
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x8000);
  UNSIGNED_INT_EQUALS(0, result);

  /**
   * Other sceKernelMapDirectMemory Notes:
   * If address is not specified, it defaults to 0x200000000
   * Not possible to test here, but there's a condition it follows that instead defaults to 0x880000000
   * On SDK version < 1.70, flag Fixed with unspecified address would remove the Fixed flag.
   */

  // Now for sceKernelReserveVirtualRange edge cases.
  // Alignment must be power of two
  addr   = 0x200000000;
  result = sceKernelReserveVirtualRange(&addr, 0x4000, 0, 0xc000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Flags can only be Fixed, NoOverwrite, and NoCoalesce
  result = sceKernelReserveVirtualRange(&addr, 0x4000, 0x400, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelReserveVirtualRange(&addr, 0x4000, 0x1000, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Address input must be aligned.
  addr   = 0x200002000;
  result = sceKernelReserveVirtualRange(&addr, 0x4000, 0, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // length input must be aligned.
  addr   = 0x200000000;
  result = sceKernelReserveVirtualRange(&addr, 0x2000, 0, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Alignment input must be aligned.
  result = sceKernelReserveVirtualRange(&addr, 0x4000, 0, 0x2000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Alignment input must be less than 0x100000000
  result = sceKernelReserveVirtualRange(&addr, 0x4000, 0, 0x100000000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Other notes for sceKernelReserveVirtualRange:
   * If Fixed flag is specified and address is null, sdk ver 1.70 and below removes flag fixed.
   * Games compiled for newer firmwares will instead return EINVAL.
   */

  // Now for sys_mmap edge cases.
  // To test each of these, I'll use sceKernelMmap (which is just a wrapper for the syscall that converts errors to ORBIS errors)
  // If sys_mmap is called with the Sanitizer flag, then non-devkit consoles return EINVAL.
  addr              = 0x200000000;
  uint64_t addr_out = 0;

  // For this test call, use flags Sanitizer (0x200000) | System (0x2000) | Anon (0x1000) | NoOverwrite (0x80) | Fixed (0x10)
  // This combination of flags is what sceKernelMapSanitizerShadowMemory uses.
  addr   = 0x300000000;
  result = sceKernelMmap(addr, 0x4000, 0, 0x203090, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If length == 0, return EINVAL
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0, 0, 0x1000, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If flags include Void (0x100) or Anon (0x1000), and phys_addr is non-zero or fd is not -1, return EINVAL.
  // Note: Void flag means this is reserving memory, sceKernelReserveVirtualRange uses it internally.
  result = sceKernelMmap(addr, 0x4000, 0, 0x1000, -1, 0x4000, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMmap(addr, 0x4000, 0, 0x1000, 0, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMmap(addr, 0x4000, 0, 0x100, -1, 0x4000, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMmap(addr, 0x4000, 0, 0x100, 0, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Successful call with anon flag for comparison.
  result = sceKernelMmap(addr, 0x4000, 0, 0x1000, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // Successful call with void flag for comparison.
  result = sceKernelMmap(addr, 0x4000, 0, 0x100, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // If flags include Stack (0x400) and prot does not include CpuReadWrite or fd is not -1, return EINVAL.
  addr   = 0xfc00000000;
  result = sceKernelMmap(addr, 0x4000, 0, 0x400, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, 0, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Successful stack mappings.
  addr   = 0xfb00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0xfc00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // Compiled SDK version 3.00 or above prohibit stack mappings above 0xfc00000000.
  // Since this test homebrew has SDK version 1.00, this should succeed.
  addr   = 0xff00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0x200400000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // Stack flag does not allow memory overwriting.
  addr   = 0xfb00000000;
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMmap(addr, 0x4000, 3, 0x400, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  /**
   * Other notes here:
   * Stack and Void flags append Anon flag internally.
   * Stack, Anon, and Void flags skip fget_mmap call (which gets the file object for the fd input)
   * If Fixed is not specified and address is 0, then it's set to 0x200000000
   * Length parameter is aligned up to the nearest page.
   */

  // If flag Fixed (0x10) is specified, address must have the same offset as phys_addr.
  addr   = 0x200002000;
  result = sceKernelMmap(addr, 0x4000, 0, 0x1010, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Notes:
   * This is where sys_mmap calls fget_mmap to get a file for mmap.
   * I still need to decompile everything here, especially the switch case for file type.
   * Insert error cases here if there are any errors that I can expose through homebrew.
   */

  // If an unopened file descriptor is specified, then returns EBADF.
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x4000, 1, 0, 100, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EBADF, result);

  int32_t fd = sceKernelOpen("/app0/assets/misc/test_file.txt", 0, 0666);
  CHECK(fd > 0);

  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelMmap(addr, 0x4000, 7, 0, fd, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // Offset is where in the file we want to map.
  // The file is ~500KB large, so offset 0x10000 should be valid.
  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0x10000, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // Try file mmap with read-write file
  fd = sceKernelOpen("/download0/test_file.txt", 0x602, 0666);
  CHECK(fd > 0);

  // Write empty data to the file
  char buf[0x10000];
  memset(buf, 0, sizeof(buf));
  result = sceKernelWrite(fd, buf, 0x8000);
  LONGS_EQUAL(0x8000, result);

  // Read only file mmap for read-write file
  result = sceKernelMmap(addr, 0x4000, 1, 0, fd, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // Read-write file mmap on read-write file
  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // Read-write-execute file mmap on read-write file
  result = sceKernelMmap(addr, 0x4000, 7, 0, fd, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // file mmap with offset == file size.
  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0x8000, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // file mmap with offset + size greater than file size
  result = sceKernelMmap(addr, 0x8000, 3, 0, fd, 0x4000, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // file mmap with offset 0 + size greater than file size
  result = sceKernelMmap(addr, 0x10000, 3, 0, fd, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  fd = sceKernelOpen("/download0/test_file.txt", 0x0, 0666);
  CHECK(fd > 0);

  // Read-only file mmap on read-only file
  result = sceKernelMmap(addr, 0x4000, 1, 0, fd, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // Read-write file mmap on read-only file
  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // Read-write-execute file mmap on read-only file
  result = sceKernelMmap(addr, 0x4000, 7, 0, fd, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // file mmap with valid offset.
  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0x4000, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // file mmap with offset == file size.
  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0x8000, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // file mmap with offset + size greater than file size
  result = sceKernelMmap(addr, 0x8000, 3, 0, fd, 0x4000, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // file mmap with offset 0 + size greater than file size
  result = sceKernelMmap(addr, 0x10000, 3, 0, fd, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // Try file mmap with read-write file
  fd = sceKernelOpen("/download0/test_file.txt", 0x602, 0666);
  CHECK(fd > 0);

  // Write empty data to the file
  result = sceKernelWrite(fd, buf, 0xf000);
  LONGS_EQUAL(0xf000, result);

  // Checks for file size in both size and offset parameters are aligned up.
  // Here, we shouldn't see errors unless I try size + offset > 0x10000
  // file mmap with offset == file size.
  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0x10000, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // file mmap with offset + size greater than file size
  result = sceKernelMmap(addr, 0xc000, 3, 0, fd, 0x8000, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // file mmap with offset 0 + size greater than file size
  result = sceKernelMmap(addr, 0x14000, 3, 0, fd, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // file mmap with size + offset greater than file size, but less than page-aligned file size.
  result = sceKernelMmap(addr, 0x8000, 3, 0, fd, 0x8000, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x8000);
  UNSIGNED_INT_EQUALS(0, result);

  // file mmap with size greater than file size, but less than page-aligned file size.
  result = sceKernelMmap(addr, 0x10000, 3, 0, fd, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr_out, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // file mmap on directories fails with EINVAL
  fd = sceKernelOpen("/download0", 0, 0666);
  CHECK(fd > 0);

  result = sceKernelMmap(addr, 0x4000, 3, 0, fd, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  /**
   * Notes:
   * File mmaps to GPU cause the PS4 to crash, this should get investigated.
   *
   * Based on decompilation, Sanitizer flag with a valid address (below a hardcoded 0x800000000000) restricts prot here.
   * Specifically, if address input > 0xfc00000000, prot is restricted to GpuReadWrite.
   * If address input is still zero here (Fixed flag with null address input?), then address defaults to 0xfc00000000.
   * sys_mmap calls vm_mmap2 with offset aligned down to the nearest page.
   */

  /**
   * Notes for vm_mmap2:
   * Starts with a check for size == 0, returning EINVAL. This can't be tested because sys_mmap had an earlier return for this case.
   * Then checks if our mapping will fit in the virtual memory limit? (if not return ENOMEM)
   * Then checks if phys_addr is aligned, returning EINVAL if it isn't. sys_mmap aligned phys_addr for us though.
   * If flags Fixed is specified, and address input is misaligned, return EINVAL. Again, sys_mmap checked this for us.
   * A bunch of checks specific to file mmaps, most of which are un-important for homebrew testing (mostly internal stuff like devices)
   */

  // Run some behavior tests on mmap here.
  // Start with expected flag behaviors.
  // If address is null and fixed is specified, returns EINVAL.
  addr     = 0;
  addr_out = 0;
  result   = sceKernelMmap(addr, 0x10000, 3, 0x1010, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // When fixed is specified, address will match the input address exactly.
  addr     = 0x300000000;
  addr_out = 0;
  result   = sceKernelMmap(addr, 0x10000, 3, 0x1010, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, addr_out);

  // If a mapping already exists at the address, and no overwrite is not specified, then it will overwrite the mapping.
  // This is tested more thoroughly in the flexible memory tests.
  result = sceKernelMmap(addr, 0x10000, 3, 0x110, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, addr_out);

  // If no overwrite is specified and the area contains a mapping, mmap returns an error.
  result = sceKernelMmap(addr, 0x10000, 3, 0x1090, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // Make sure this check is properly applied if the range is partially mapped.
  result = sceKernelMmap(addr - 0x4000, 0x10000, 3, 0x1090, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // Make sure this check is properly applied if the range is partially mapped.
  result = sceKernelMmap(addr + 0x4000, 0x10000, 3, 0x1090, -1, 0, &addr_out);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // When fixed is not specified, address will search for a free area, starting at a base of 0x200000000.
  // libc mappings follow this behavior, so we can't hardcode an address here, instead search manually for a free address to check against.
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
  info = {};

  addr = 0x200000000;
  // VirtualQuery with flags 0 means there must be a mapping here.
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  while (result != ORBIS_KERNEL_ERROR_EACCES) {
    // VirtualQuery returns EACCES when failing to find a mapping. Abuse this to find the first free address mmap should use.
    // Since we're only mapping one page, we can safely assume any gap is large enough to map.
    addr   = info.end;
    result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  }

  // If VirtualQuery works properly, addr should be our first free address.
  uint64_t expected_addr = addr;
  addr                   = 0;
  result                 = sceKernelMmap(addr, 0x4000, 0, 0x1000, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);
  // If this succeeds, sceKernelMmap is using the correct base address for addr == nullptr.
  LONGS_EQUAL(expected_addr, addr);

  result = sceKernelMunmap(addr, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // If fixed is not specified, but address is free, then the mapping will use the specified address.
  // Use a hardcoded, but presumably free address for this.
  addr          = 0x300000000;
  expected_addr = addr;
  result        = sceKernelMmap(addr, 0x4000, 0, 0x1000, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(expected_addr, addr);

  // When fixed is not specified, mmap will search to higher addresses, not lower ones.
  addr          = 0x300000000;
  expected_addr = 0x300004000;
  result        = sceKernelMmap(addr, 0x4000, 0, 0x1000, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(expected_addr, addr);

  result = sceKernelMunmap(addr - 0x4000, 0x8000);
  UNSIGNED_INT_EQUALS(0, result);

  /**
   * Run mmap with various remaining valid flags to ensure all are at least usable when they should be.
   * Many of these are either related to multi-process, dmem, or other forms of memory, and won't have a noticeable impact on anon mappings.
   * MAP_2MB_ALIGN and MAP_OPTIMAL_SPACE are removed during sys_mmap, so they have no impact here.
   */

  // Test anon unbacked mmap with shared (1)
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x10000, 3, 0x1001, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run VirtualQuery to see if mapping behaved as expected
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Test writing data to memory
  memset(buf, 0, sizeof(buf));
  memcpy(reinterpret_cast<void*>(addr), buf, 0x10000);
  result = memcmp(reinterpret_cast<void*>(addr), buf, 0x10000);
  LONGS_EQUAL(0, result);

  // Unmap after testing.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // While the shared flag would make the data persist for a file mapping, it does not do anything for anon mappings.
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x10000, 3, 0x1001, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);
  result = memcmp(reinterpret_cast<void*>(addr), buf, 0x10000);
  LONGS_EQUAL(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Test anon unbacked mmap with private (2)
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x10000, 3, 0x1002, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run VirtualQuery to see if mapping behaved as expected
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Test writing data to memory
  memset(buf, 0, sizeof(buf));
  memcpy(reinterpret_cast<void*>(addr), buf, 0x10000);
  result = memcmp(reinterpret_cast<void*>(addr), buf, 0x10000);
  LONGS_EQUAL(0, result);

  // Unmap after testing.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Test anon unbacked mmap with void (0x100)
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x10000, 3, 0x1100, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run VirtualQuery to see if mapping behaved as expected
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  // void flag forces prot = 0
  LONGS_EQUAL(0, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(0, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(0, info.is_committed);

  // Unmap after testing.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Test anon mmap with has semaphore (0x200)
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x10000, 3, 0x1200, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run VirtualQuery to see if mapping behaved as expected
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Test writing data to memory
  memset(buf, 0, sizeof(buf));
  memcpy(reinterpret_cast<void*>(addr), buf, 0x10000);
  result = memcmp(reinterpret_cast<void*>(addr), buf, 0x10000);
  LONGS_EQUAL(0, result);

  // Unmap after testing.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Test anon mmap with no sync (0x800)
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x10000, 3, 0x1800, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run VirtualQuery to see if mapping behaved as expected
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Test writing data to memory
  memset(buf, 0, sizeof(buf));
  memcpy(reinterpret_cast<void*>(addr), buf, 0x10000);
  result = memcmp(reinterpret_cast<void*>(addr), buf, 0x10000);
  LONGS_EQUAL(0, result);

  // Unmap after testing.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Test anon mmap with system (0x2000)
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x10000, 3, 0x3000, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run VirtualQuery to see if mapping behaved as expected
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Test writing data to memory
  memset(buf, 0, sizeof(buf));
  memcpy(reinterpret_cast<void*>(addr), buf, 0x10000);
  result = memcmp(reinterpret_cast<void*>(addr), buf, 0x10000);
  LONGS_EQUAL(0, result);

  // MAP_SYSTEM would normally place this memory under a different budget, test that in FlexibleTest

  // Unmap after testing.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Test anon mmap with all available (0x4000)
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x10000, 3, 0x5000, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run VirtualQuery to see if mapping behaved as expected
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Test writing data to memory
  memset(buf, 0, sizeof(buf));
  memcpy(reinterpret_cast<void*>(addr), buf, 0x10000);
  result = memcmp(reinterpret_cast<void*>(addr), buf, 0x10000);
  LONGS_EQUAL(0, result);

  // Unmap after testing.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Test anon mmap with 2mb align (0x10000)
  addr   = 0x300004000;
  result = sceKernelMmap(addr, 0x10000, 3, 0x11000, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run VirtualQuery to see if mapping behaved as expected
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // The 2mb align flag is removed in mmap, so this has no impact on address alignment.
  CHECK(addr % 0x200000 != 0);

  // Test writing data to memory
  memset(buf, 0, sizeof(buf));
  memcpy(reinterpret_cast<void*>(addr), buf, 0x10000);
  result = memcmp(reinterpret_cast<void*>(addr), buf, 0x10000);
  LONGS_EQUAL(0, result);

  // Unmap after testing.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Test anon mmap with no core (0x20000)
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x10000, 3, 0x21000, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run VirtualQuery to see if mapping behaved as expected
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Test writing data to memory
  memset(buf, 0, sizeof(buf));
  memcpy(reinterpret_cast<void*>(addr), buf, 0x10000);
  result = memcmp(reinterpret_cast<void*>(addr), buf, 0x10000);
  LONGS_EQUAL(0, result);

  // Unmap after testing.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Test anon mmap with prefault read (0x40000)
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x10000, 3, 0x41000, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run VirtualQuery to see if mapping behaved as expected
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Test writing data to memory
  memset(buf, 0, sizeof(buf));
  memcpy(reinterpret_cast<void*>(addr), buf, 0x10000);
  result = memcmp(reinterpret_cast<void*>(addr), buf, 0x10000);
  LONGS_EQUAL(0, result);

  // Unmap after testing.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Test anon mmap with self (0x80000)
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x10000, 3, 0x81000, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run VirtualQuery to see if mapping behaved as expected
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Test writing data to memory
  memset(buf, 0, sizeof(buf));
  memcpy(reinterpret_cast<void*>(addr), buf, 0x10000);
  result = memcmp(reinterpret_cast<void*>(addr), buf, 0x10000);
  LONGS_EQUAL(0, result);

  // Unmap after testing.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Test anon mmap with optimal space (0x100000)
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x10000, 3, 0x101000, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run VirtualQuery to see if mapping behaved as expected
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Test writing data to memory
  memset(buf, 0, sizeof(buf));
  memcpy(reinterpret_cast<void*>(addr), buf, 0x10000);
  result = memcmp(reinterpret_cast<void*>(addr), buf, 0x10000);
  LONGS_EQUAL(0, result);

  // Unmap after testing.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Test anon mmap with writable wb garlic (0x800000)
  addr   = 0x200000000;
  result = sceKernelMmap(addr, 0x10000, 3, 0x801000, -1, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run VirtualQuery to see if mapping behaved as expected
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Test writing data to memory
  memset(buf, 0, sizeof(buf));
  memcpy(reinterpret_cast<void*>(addr), buf, 0x10000);
  result = memcmp(reinterpret_cast<void*>(addr), buf, 0x10000);
  LONGS_EQUAL(0, result);

  // Unmap after testing.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Executable protections cause dmem file mmaps to fail.
  result = sceKernelMapDirectMemory(&addr, 0x4000, 0x7, 0, 0, 0x2000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Continued notes:
   * vm_map_find causes ENOMEM return if there are no valid memory areas to map.
   * If vm_map_find returns any address between 0x80000000 and 0x200000000, ENOMEM is returned.
   */

  /**
   * Notes for sceKernelMapDirectMemory2:
   * Alignment must be a power of 2, page aligned, and less than 0x100000000
   * If alignment is less than page size, it's increased to the page size.
   * Remaining behavior seems to rely on checks in sys_mmap_dmem.
   */

  // To run sys_mmap_dmem without worries from phys addresses, allocate some dmem first.
  phys_addr = 0;
  result    = sceKernelAllocateMainDirectMemory(0x4000, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);

  addr = 0x200000000;
  // Not power of 2
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0, phys_addr, 0xc000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  // Not page aligned
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0, phys_addr, 0x100);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  // Too large.
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0, phys_addr, 0x100000000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  /**
   * Notes for sys_mmap_dmem:
   * Starts with parameter checks.
   * Address, phys_addr, and length must be page aligned. Length = 0 is not allowed.
   * Flags must only contain Fixed, NoOverwrite, Sanitizer, NoCoalesce, and the alignment bits.
   * Prot contains only CpuRead, CpuWrite, GpuRead, or GpuWrite.
   * mtype must be less than 10
   * If any of these error checks fail, the function returns EINVAL.
   *
   * sys_mmap_dmem also follows the same address input logic as sys_mmap.
   * If address is null and map Fixed is specified, returns EINVAL.
   *
   * sys_mmap_dmem has an extra check for physical addresses overlapping.
   * This check is removed if sceKernelEnableDmemAliasing is called.
   */

  // Misaligned addr
  addr   = 0x200002000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Misaligned phys_addr
  addr   = 0x200000000;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0, phys_addr - 0x2000, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Misaligned length
  result = sceKernelMapDirectMemory2(&addr, 0x2000, 0, 1, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Zero length
  result = sceKernelMapDirectMemory2(&addr, 0, 0, 1, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Invalid flags
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 1, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Invalid prot
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 0x7, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 0x8, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Invalid mtype
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 11, 1, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // -1 mtype skips assigning type.
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -1, 1, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);

  // Overlapping phys addr causes EBUSY
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EBUSY, result);

  // Try enabling dmem aliasing
  result = sceKernelEnableDmemAliasing();
  UNSIGNED_INT_EQUALS(0, result);

  // Overlapping phys addr works now.
  uint64_t addr2 = 0x200000000;
  result         = sceKernelMapDirectMemory2(&addr2, 0x4000, 0, 1, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr2, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // Lower negatives are invalid.
  result = sceKernelMapDirectMemory2(&addr, 0x4000, -2, 1, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Map Sanitizer
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0x200000, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // null address with map fixed
  addr   = 0;
  result = sceKernelMapDirectMemory2(&addr, 0x4000, 0, 1, 0x10, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // Might continue down this rabbit hole at some point, but I don't see too much of a point?
  // I've gleamed the error cases they start with, everything else will just come from testing, as always.
}

TEST(MemoryTests, DeviceFileTest) {
  // This function is for testing any lower-level behavior through directly opening device files.
  // This is kept as a separate test since, no matter how messy a game is, it generally wont be manipulating low level hardware behavior.

  // Test for the writable wb garlic flag (0x800000), this flag is used by a libkernel function that OpenOrbis doesn't have.
  // To get around this, we can instead open /dev/dmem1 ourselves to get a file descriptor for mmap.
  int32_t dmem1_fd = sceKernelOpen("/dev/dmem1", 0x2, 0777);
  CHECK(dmem1_fd > 0);

  // Allocate some direct memory for direct memory mapping.
  int64_t phys_addr = 0;
  int32_t result    = sceKernelAllocateMainDirectMemory(0x10000, 0x10000, 10, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Start with a typical mmap to verify we're mapping to the dmem file.
  // Like libkernel, specify MAP_SHARED flag, though this is theoretically not required.
  uint64_t addr = 0;
  result        = sceKernelMmap(addr, 0x10000, 1, 0x1, dmem1_fd, phys_addr, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // To confirm this actually mapped the expected direct memory, check if sceKernelReleaseDirectMemory can unmap it.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Define OrbisKernelVirtualQueryInfo struct
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
  info = {};

  // If the memory unmapped properly, VirtualQuery will return EACCES.
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // Now test the MAP_WRITABLE_WB_GARLIC flag.
  result = sceKernelAllocateMainDirectMemory(0x10000, 0x10000, 10, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Read-only CPU mapping normally works.
  addr   = 0;
  result = sceKernelMmap(addr, 0x10000, 1, 0x800001, dmem1_fd, phys_addr, &addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run VirtualQuery on this mapping to make sure it has the correct types.
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0x10000, info.offset);
  LONGS_EQUAL(1, info.prot);
  LONGS_EQUAL(10, info.memory_type);
  LONGS_EQUAL(0, info.is_flexible);
  LONGS_EQUAL(1, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Despite the flag, write prot on CPU still returns EACCESS for this mtype.
  addr   = 0;
  result = sceKernelMmap(addr, 0x10000, 2, 0x800001, dmem1_fd, phys_addr, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMmap(addr, 0x10000, 3, 0x800001, dmem1_fd, phys_addr, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // Read-only GPU mapping normally works.
  result = sceKernelMmap(addr, 0x10000, 0x10, 0x800001, dmem1_fd, phys_addr, &addr);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // write-only GPU should work due to the flag, normally this fails.
  result = sceKernelMmap(addr, 0x10000, 0x20, 0x800001, dmem1_fd, phys_addr, &addr);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // read-write GPU should work due to the flag, normally this fails.
  result = sceKernelMmap(addr, 0x10000, 0x30, 0x800001, dmem1_fd, phys_addr, &addr);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Release direct memory.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Close dmem1 file
  result = sceKernelClose(dmem1_fd);
  UNSIGNED_INT_EQUALS(0, result);

  // Some devices can be mmapped, these can follow different rules.
  int32_t fd = sceKernelOpen("/dev/gc", 0x602, 0666);
  CHECK(fd > 0);

  // Perform device file mmap.
  uint64_t output_addr = 0;
  result               = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &output_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run sceKernelVirtualQuery to make sure memory area is as expected.
  info   = {};
  result = sceKernelVirtualQuery(output_addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(output_addr, info.start);
  LONGS_EQUAL(output_addr + 0x4000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(0, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(0, info.is_committed);

  // Device file mmaps are supposed to automatically append MAP_SHARED.
  memset(reinterpret_cast<void*>(output_addr), 1, 0x1000);

  // Read contents of memory into a buffer.
  char dev_buf[0x4000];
  memcpy(dev_buf, reinterpret_cast<void*>(output_addr), 0x4000);

  // Since we can't sceKernelRead our way through this, we can instead check for MAP_SHARED by unmapping, then remapping the file.
  // If changes persist, this memory is likely backed in some form. If not, we can assume MAP_SHARED was not appended.
  result = sceKernelMunmap(output_addr, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // Perform device file mmap.
  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &output_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Compare the device memory before and after. If the changes from before are still present, the mapping must have MAP_SHARED.
  result = memcmp(reinterpret_cast<void*>(output_addr), dev_buf, 0x4000);
  LONGS_EQUAL(0, result);

  result = sceKernelMunmap(output_addr, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // Device file mmaps fail with EINVAL if you map with flags Private (2)
  result = sceKernelMmap(0, 0x4000, 3, 2, fd, 0, &output_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // Test budget behaviors that wouldn't come up in normal games.
  std::list<uint64_t> addresses;
  uint64_t            addr_out = 0;
  while (result == 0) {
    result = sceKernelMapFlexibleMemory(&addr_out, 0x4000, 3, 0);
    if (result < 0) {
      // Out of flex mem
      UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
    } else {
      // Mapped flex mem successfully.
      UNSIGNED_INT_EQUALS(0, result);
      // Add the mapping address to addresses, need to unmap later to clean up.
      addresses.emplace_back(addr_out);
    }
  }

  // After all these mappings, available flex size should be 0.
  uint64_t avail_flex_size = 0;
  result                   = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0, avail_flex_size);

  // Device mmaps do not use the flexible budget.
  fd = sceKernelOpen("/dev/gc", 0x602, 0666);
  CHECK(fd > 0);
  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &output_addr);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(output_addr, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // mmaps from system folders do not use the flexible budget.
  const char* sys_folder = sceKernelGetFsSandboxRandomWord();
  char        path[128];
  memset(path, 0, sizeof(path));
  snprintf(path, sizeof(path), "/%s/common/lib/libc.sprx", sys_folder);
  fd = sceKernelOpen(path, 0, 0666);
  CHECK(fd > 0);

  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &output_addr);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(output_addr, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // Unmap all of the flexible memory used here.
  for (uint64_t addr: addresses) {
    result = sceKernelMunmap(addr, 0x4000);
    UNSIGNED_INT_EQUALS(0, result);
  }
  addresses.clear();
}

TEST(MemoryTests, AvailableDirectMemoryTest) {
  // Not exactly a way for this one to fail. Just need it to test other functions.
  uint64_t dmem_size = sceKernelGetDirectMemorySize();
  CHECK(dmem_size != 0);

  // Test sceKernelAvailableDirectMemorySize.
  // Typical use case:
  int64_t  phys_addr = 0;
  uint64_t size      = 0;
  int32_t  result    = sceKernelAvailableDirectMemorySize(0, dmem_size, 0, &phys_addr, &size);
  // Check for success
  UNSIGNED_INT_EQUALS(0, result);
  // Check for known quirks on success.
  // Specifically, the dmem map already has a mapping (which comes from libSceGnmDriver).
  // Many games are built with this assumption in mind, though most don't need it emulated.
  // This mapping will be from physical addresses 0 to 0x10000 under these circumstances.
  LONGS_EQUAL(0x10000, phys_addr);
  LONGS_EQUAL(dmem_size - 0x10000, size);

  // Now test for potential edge cases.
  // Based on decompilation, this function should accept both phys_addr and size of nullptr.
  result = sceKernelAvailableDirectMemorySize(0, dmem_size, 0, nullptr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);

  // If there is no size to output, this function returns ENOMEM
  result = sceKernelAvailableDirectMemorySize(0, 0, 0, nullptr, nullptr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // The alignment check is very lenient, they run (align - 1) & align to check.
  // While this alignment value is far lower than the usually acceptable amount, this still works.
  result = sceKernelAvailableDirectMemorySize(0, dmem_size, 0x100, nullptr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);

  // If you use a value that triggers the check, it returns EINVAL
  result = sceKernelAvailableDirectMemorySize(0, dmem_size, 0x11111, nullptr, nullptr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // There is no specific error for start > end, it still returns ENOMEM.
  result = sceKernelAvailableDirectMemorySize(dmem_size, 0, 0, nullptr, nullptr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // There's also no specific error for the range being outside the dmem map, it returns ENOMEM.
  result = sceKernelAvailableDirectMemorySize(dmem_size, dmem_size * 2, 0, nullptr, nullptr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  result = sceKernelAvailableDirectMemorySize(dmem_size * 5, dmem_size * 6, 0, nullptr, nullptr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOMEM, result);

  // Start and end phys addresses are limited internally, with no error return for them.
  // Make sure size is appropriately restricted.
  result = sceKernelAvailableDirectMemorySize(0, dmem_size * 5, 0, nullptr, &size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(dmem_size - 0x10000, size);

  // phys_addr and size are restricted based on the start and end.
  // So in a case like this, where start is after the start of the free dmem area, size and phys_addr are restricted.
  result = sceKernelAvailableDirectMemorySize(dmem_size / 2, dmem_size, 0, &phys_addr, &size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(dmem_size / 2, phys_addr);
  LONGS_EQUAL(dmem_size / 2, size);

  // phys_addr and size are restricted based on the start and end.
  // So in a case like this, where end is before the end of the free dmem area, size and phys_addr are restricted.
  result = sceKernelAvailableDirectMemorySize(0, dmem_size / 2, 0, &phys_addr, &size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0x10000, phys_addr);
  LONGS_EQUAL((dmem_size / 2) - 0x10000, size);

  // start is rounded up to alignment.
  result = sceKernelAvailableDirectMemorySize(0x1c001, dmem_size, 0, &phys_addr, &size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0x20000, phys_addr);
  LONGS_EQUAL(dmem_size - 0x20000, size);

  // end ignores alignment
  result = sceKernelAvailableDirectMemorySize(0x10000, dmem_size - 0x3fff, 0, &phys_addr, &size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0x10000, phys_addr);
  LONGS_EQUAL(dmem_size - 0x13fff, size);

  // Alignments below the system requirements are rounded up to the system-wide page size of 0x4000.
  // This can be tested with the search start, as that is rounded up to alignment.
  result = sceKernelAvailableDirectMemorySize(0x1c001, dmem_size, 0x100, &phys_addr, &size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0x20000, phys_addr);
  LONGS_EQUAL(dmem_size - 0x20000, size);

  // phys_addr is aligned up to alignment, size is aligned down to compensate.
  result = sceKernelAvailableDirectMemorySize(0, dmem_size, 0x20000, &phys_addr, &size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0x20000, phys_addr);
  LONGS_EQUAL(dmem_size - 0x20000, size);

  // returned statistics are always from the largest area of free space.
  // Do an allocation in the middle of the dmem map to demonstrate this.
  int64_t phys_addr_out = 0;
  result                = sceKernelAllocateDirectMemory(dmem_size / 2, dmem_size, 0x100000, 0, 0, &phys_addr_out);
  UNSIGNED_INT_EQUALS(0, result);
  // Nothing else in that area of the dmem map, so this should be the returned phys address.
  LONGS_EQUAL(dmem_size / 2, phys_addr_out);

  result = sceKernelAvailableDirectMemorySize(0, dmem_size, 0, &phys_addr, &size);
  UNSIGNED_INT_EQUALS(0, result);
  // The larger area should be from 0x10000 to dmem_size / 2 here.
  LONGS_EQUAL(0x10000, phys_addr);
  LONGS_EQUAL((dmem_size / 2) - 0x10000, size);

  // Release the direct memory used for testing this, we want a clean dmem_map.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr_out, 0x100000);
  UNSIGNED_INT_EQUALS(0, result);

  // Free mappings should coalesce, make sure sceKernelAvailableDirectMemorySize still returns the expected values.
  result = sceKernelAvailableDirectMemorySize(0, dmem_size, 0, &phys_addr, &size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0x10000, phys_addr);
  LONGS_EQUAL(dmem_size - 0x10000, size);
}

TEST(MemoryTests, AllocateDirectMemoryTest) {
  // Test sceKernelAllocateDirectMemory
  // Start with the typical working case.
  uint64_t dmem_size  = sceKernelGetDirectMemorySize();
  int64_t  dmem_start = 0x100000;
  int64_t  phys_addr  = 0;
  int32_t  result     = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x10000, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // When there is no direct memory at the start, phys_addr equals start.
  LONGS_EQUAL(dmem_start, phys_addr);

  // Keep dmem_map clean.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // If either start or end is less than zero, returns EINVAL
  phys_addr = 0;
  result    = sceKernelAllocateDirectMemory(-1, dmem_size, 0x10000, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelAllocateDirectMemory(0, -1, 0x10000, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If length is zero, returns EINVAL
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If there is more than one active bit in align, returns EINVAL
  // 0xc000 is page-aligned, but should fail this.
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0x10000, 0xc000, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If alignment or length is not page aligned, returns EINVAL
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0x1000, 0x10000, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0x10000, 0x1000, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If type > 10, return EINVAL
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0x10000, 0, 11, &phys_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If type < 0, return EINVAL
  result = sceKernelAllocateDirectMemory(0, dmem_size, 0x10000, 0, -1, &phys_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // If limited end <= start, return EAGAIN
  result = sceKernelAllocateDirectMemory(0x100000, 0x10000, 0x10000, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EAGAIN, result);

  // If limited end < length, return EAGAIN
  result = sceKernelAllocateDirectMemory(0, 0x100000, 0x200000, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EAGAIN, result);

  // If limited end - length < start, return EAGAIN
  result = sceKernelAllocateDirectMemory(0x10000, 0x40000, 0x40000, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EAGAIN, result);

  // There is no check for if phys_addr is null in firmware. This call WILL crash the application.
  // result = sceKernelAllocateDirectMemory(0, dmem_size, 0x20000, 0, 0, nullptr);

  // Now on to edge cases that don't cause immediate returns.
  result = sceKernelAllocateDirectMemory(dmem_start, dmem_start + 0x10000, 0x10000, 0, 0, &phys_addr);
  // This call fails with EAGAIN, since there's no direct memory in that area.
  result = sceKernelAllocateDirectMemory(dmem_start, dmem_start + 0x10000, 0x10000, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EAGAIN, result);

  result = sceKernelReleaseDirectMemory(dmem_start, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // phys_addr will be aligned up to alignment.
  dmem_start = 0x110000;
  result     = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x20000, 0x20000, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(dmem_start + 0x10000, phys_addr);

  // Keep dmem_map clean.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x20000);
  UNSIGNED_INT_EQUALS(0, result);

  // size is not affected by alignment
  dmem_start = 0x100000;
  result     = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x10000, 0x20000, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(dmem_start, phys_addr);

  // Run sceKernelGetDirectMemoryType to confirm mapping dimensions.
  int32_t out_type;
  int64_t out_start;
  int64_t out_end;
  result = sceKernelGetDirectMemoryType(phys_addr, &out_type, &out_start, &out_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr, out_start);
  LONGS_EQUAL(phys_addr + 0x10000, out_end);

  // Keep dmem_map clean.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // If start is later than the start of the free pages, phys_addr = start
  result = sceKernelAllocateDirectMemory(dmem_size / 2, dmem_size, 0x10000, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(dmem_size / 2, phys_addr);

  // Keep dmem_map clean.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // If align would force phys_addr up to a mapped page, then that area is unsuitable.
  dmem_start = 0x100000;
  result     = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x4000, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(dmem_start, phys_addr);

  int64_t phys_addr2 = 0;
  result             = sceKernelAllocateDirectMemory(dmem_start - 0x10000, dmem_size, 0x20000, 0x20000, 0, &phys_addr2);
  UNSIGNED_INT_EQUALS(0, result);
  // Since dmem_start through dmem_start + 0x4000 is mapped, and alignment is 0x20000, the returned phys_addr is aligned up to dmem_start + 0x20000.
  LONGS_EQUAL(dmem_start + 0x20000, phys_addr2);

  // Keep dmem_map clean.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelCheckedReleaseDirectMemory(phys_addr2, 0x20000);
  UNSIGNED_INT_EQUALS(0, result);

  // Firmware doesn't align search start, which allows this call to create a misaligned dmem page.
  // A notable side effect of this behavior is crashing the entire console.
  // result = sceKernelAllocateDirectMemory(0x10001, dmem_size, 0x4000, 0, 0, &phys_addr);
}

TEST(MemoryTests, ReleaseDirectMemoryTest) {
  // Both checked and unchecked return an error if addr or length aren't aligned to page size.
  int64_t  dmem_start = 0x100000;
  uint64_t dmem_size  = sceKernelGetDirectMemorySize();
  int32_t  result     = sceKernelReleaseDirectMemory(dmem_start, 0x3000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelReleaseDirectMemory(dmem_start + 0x3000, 0x10000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Releasing free dmem causes error ENOENT on checked release
  result = sceKernelCheckedReleaseDirectMemory(dmem_start, 0x10000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOENT, result);

  // Releasing free dmem "succeeds" on unchecked release
  result = sceKernelReleaseDirectMemory(dmem_start, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Allocate a little dmem to test with
  int64_t phys_addr = 0;
  result            = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x20000, 0x20000, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(dmem_start, phys_addr);
  int64_t phys_addr2 = 0;
  result             = sceKernelAllocateDirectMemory(dmem_start + 0x30000, dmem_size, 0x10000, 0x10000, 0, &phys_addr2);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(dmem_start + 0x30000, phys_addr2);

  // Now we've got 0x100000-0x120000 and 0x130000-0x140000 allocated.
  // Checked fails if anything in the range is free.
  // Make sure you're checking the end of the range.
  result = sceKernelCheckedReleaseDirectMemory(dmem_start, 0x30000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOENT, result);
  // Check for the start of the range.
  result = sceKernelCheckedReleaseDirectMemory(dmem_start + 0x20000, 0x20000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOENT, result);
  // Check for the middle of the range, first 0x10000 and last 0x10000 are allocated, middle 0x10000 is free.
  result = sceKernelCheckedReleaseDirectMemory(dmem_start + 0x10000, 0x30000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOENT, result);

  // Because of how checked fails, the allocations are safe.
  int32_t out_type;
  int64_t out_start;
  int64_t out_end;
  result = sceKernelGetDirectMemoryType(phys_addr, &out_type, &out_start, &out_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr, out_start);
  LONGS_EQUAL(phys_addr + 0x20000, out_end);

  result = sceKernelGetDirectMemoryType(phys_addr2, &out_type, &out_start, &out_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr2, out_start);
  LONGS_EQUAL(phys_addr2 + 0x10000, out_end);

  // Unchecked will deallocate both free areas
  result = sceKernelReleaseDirectMemory(phys_addr, 0x40000);
  UNSIGNED_INT_EQUALS(0, result);

  // As a result of the unchecked release, this sceKernelGetDirectMemoryType call fails.
  result = sceKernelGetDirectMemoryType(phys_addr, &out_type, &out_start, &out_end);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOENT, result);
  result = sceKernelGetDirectMemoryType(phys_addr2, &out_type, &out_start, &out_end);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOENT, result);

  // Release should unmap any mappings that reference the physical block.
  // Allocate a little dmem to test with
  result = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x10000, 0x20000, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(dmem_start, phys_addr);

  uint64_t addr = 0;
  result        = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelReleaseDirectMemory(phys_addr, 0x30000);
  UNSIGNED_INT_EQUALS(0, result);

  uint64_t start_addr;
  uint64_t end_addr;
  int32_t  out_prot;
  // Since the mapping was unmapped through ReleaseDirectMemory, QueryMemoryProtection should return EACCES.
  result = sceKernelQueryMemoryProtection(addr, &start_addr, &end_addr, &out_prot);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
}

// Test for various edge cases related to file mmaps
TEST(MemoryTests, FileMappingTest) {
  // Start with behaviors of read-only mmaps
  // Open test file
  int32_t fd = sceKernelOpen("/app0/assets/misc/test_file.txt", 0, 0666);
  CHECK(fd > 0);

  // Run a sceKernelRead to read from the file directly.
  char    read_buf[0x40000];
  int64_t bytes_read = sceKernelRead(fd, read_buf, sizeof(read_buf));
  LONGS_EQUAL(sizeof(read_buf), bytes_read);

  // mmap file to memory
  // Using flags 0, prot read-write.
  uint64_t addr        = 0;
  uint64_t output_addr = 0;
  uint64_t offset      = 0;
  int32_t  result      = sceKernelMmap(addr, 0x10000, 3, 0, fd, offset, &output_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // With this mmap, we should be able to read from the file.
  // Run a memcpy to copy from this memory to a buffer.
  char mem_buf[0x10000];
  memcpy(mem_buf, reinterpret_cast<void*>(output_addr), sizeof(mem_buf));

  // Compare memory buffer to file contents.
  result = memcmp(mem_buf, &read_buf[offset], sizeof(mem_buf));
  UNSIGNED_INT_EQUALS(0, result);

  // Write zeros to the memory area, then read back the new data.
  memset(reinterpret_cast<void*>(output_addr), 0, sizeof(mem_buf));
  memcpy(mem_buf, reinterpret_cast<void*>(output_addr), sizeof(mem_buf));

  // Re-read file contents
  bytes_read = sceKernelLseek(fd, 0, 0);
  LONGS_EQUAL(0, bytes_read);
  bytes_read = sceKernelRead(fd, read_buf, sizeof(read_buf));
  LONGS_EQUAL(sizeof(read_buf), bytes_read);

  // Compare memory buffer to file contents.
  result = memcmp(mem_buf, &read_buf[offset], sizeof(mem_buf));
  // File contents should not be modified by the memory write.
  CHECK(result != 0);

  // Unmap test memory
  result = sceKernelMunmap(output_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // mmap with offset
  offset = 0x4000;
  result = sceKernelMmap(addr, 0x10000, 3, 0, fd, offset, &output_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run a memcpy to copy from this memory to a buffer.
  memcpy(mem_buf, reinterpret_cast<void*>(output_addr), sizeof(mem_buf));

  // Compare memory buffer to file contents.
  result = memcmp(mem_buf, &read_buf[offset], sizeof(mem_buf));
  LONGS_EQUAL(0, result);

  // Unmap test memory
  result = sceKernelMunmap(output_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Check for alignment behavior.
  offset = 0x6000;
  result = sceKernelMmap(addr, 0xe000, 3, 0, fd, offset, &output_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Size should round up to 0x10000, offset should round down to 0x4000.
  uint64_t start_addr;
  uint64_t end_addr;
  int32_t  prot;
  result = sceKernelQueryMemoryProtection(output_addr, &start_addr, &end_addr, &prot);
  UNSIGNED_INT_EQUALS(0, result);
  // mmap's address output is increased by page offset.
  // This is not reflected in the actual mapping.
  LONGS_EQUAL(output_addr - 0x2000, start_addr);
  LONGS_EQUAL(output_addr + 0xe000, end_addr);
  LONGS_EQUAL(3, prot);

  // Run virtual query to check details
  // Need to define struct first.
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
  info   = {};
  result = sceKernelVirtualQuery(output_addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(start_addr, info.start);
  LONGS_EQUAL(end_addr, info.end);
  // VirtualQuery does not report file mmap offsets.
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(prot, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Run a memcpy to copy from this memory to a buffer.
  memcpy(mem_buf, reinterpret_cast<void*>(start_addr), sizeof(mem_buf));

  // Compare memory buffer to file contents.
  result = memcmp(mem_buf, &read_buf[offset - 0x2000], sizeof(mem_buf));
  LONGS_EQUAL(0, result);

  // Unmap test memory
  result = sceKernelMunmap(start_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Close file
  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // Open test file
  fd = sceKernelOpen("/download0/test_file.txt", 0x602, 0666);
  CHECK(fd > 0);

  // Run a sceKernelRead on /dev/urandom to get random numbers
  int32_t random_fd = sceKernelOpen("/dev/urandom", 0, 0777);
  CHECK(random_fd > 0);
  bytes_read = sceKernelRead(random_fd, read_buf, sizeof(read_buf));
  LONGS_EQUAL(sizeof(read_buf), bytes_read);

  // Use sceKernelWrite to write the random data to the file.
  bytes_read = sceKernelWrite(fd, read_buf, sizeof(read_buf));
  LONGS_EQUAL(sizeof(read_buf), bytes_read);

  // Seek back to the beginning of the file.
  bytes_read = sceKernelLseek(fd, 0, 0);
  LONGS_EQUAL(0, bytes_read);

  // mmap file to memory
  // Using flags 0, prot read-write.
  addr        = 0;
  output_addr = 0;
  offset      = 0;
  result      = sceKernelMmap(addr, 0x10000, 3, 0, fd, offset, &output_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run a memcpy to copy from this memory to a buffer, then compare it to the file data.
  memcpy(mem_buf, reinterpret_cast<void*>(output_addr), sizeof(mem_buf));
  result = memcmp(mem_buf, read_buf, sizeof(mem_buf));
  LONGS_EQUAL(0, result);

  // Unmap test memory
  result = sceKernelMunmap(output_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Test misaligned file size behavior.
  // This should leave the file contents intact.
  result = sceKernelFtruncate(fd, 0xf000);

  // mmap file to memory
  // Using flags 0, prot read-write.
  result = sceKernelMmap(addr, 0x10000, 3, 0, fd, offset, &output_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run sceKernelVirtualQuery to make sure memory area is as expected.
  info   = {};
  result = sceKernelVirtualQuery(output_addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(output_addr, info.start);
  LONGS_EQUAL(output_addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Run a memcpy to copy from this memory to a buffer, then compare it to the file data.
  memcpy(mem_buf, reinterpret_cast<void*>(output_addr), sizeof(mem_buf));
  // Only compare the parts that are in the file. Outside the file, memory contents differ from old file contents.
  result = memcmp(mem_buf, read_buf, 0xf000);
  LONGS_EQUAL(0, result);

  // Bytes outside the file contents are zeroed.
  char test_buf[0x1000];
  memset(test_buf, 0, 0x1000);
  result = memcmp(&mem_buf[0xf000], test_buf, 0x1000);
  LONGS_EQUAL(0, result);

  // Unmap test memory
  result = sceKernelMunmap(output_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Alignment behavior allows this call to succeed too, since offset aligns down and size aligns up.
  // Remember that output_addr is misaligned here due to the offset.
  result = sceKernelMmap(addr, 0xf000, 3, 0, fd, 0x1000, &output_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Run sceKernelVirtualQuery to make sure memory area is as expected.
  info   = {};
  result = sceKernelVirtualQuery(output_addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(output_addr - 0x1000, info.start);
  LONGS_EQUAL(output_addr + 0xf000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Run a memcpy to copy from this memory to a buffer, then compare it to the file data.
  memcpy(mem_buf, reinterpret_cast<void*>(output_addr - 0x1000), sizeof(mem_buf));
  // Only compare the parts that are in the file. Outside the file, memory contents differ from old file contents.
  result = memcmp(mem_buf, read_buf, 0xf000);
  LONGS_EQUAL(0, result);

  // Bytes outside the file contents are zeroed.
  memset(test_buf, 0, 0x1000);
  result = memcmp(&mem_buf[0xf000], test_buf, 0x1000);
  LONGS_EQUAL(0, result);

  // Unmap test memory
  result = sceKernelMunmap(output_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Now test MAP_SHARED behavior (safely).
  // This flag copies changes from memory to the file.
  // Note that using this flag with read-only files is dangerous, instantly crashes PS4s and causes data corruption.
  result = sceKernelMmap(addr, 0x8000, 3, 1, fd, offset, &output_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Right now the data we get will be the same random values.
  // Do a memory write to write 0s to the start of the file.
  memset(reinterpret_cast<void*>(output_addr), 0, 0x8000);

  // Lseek to the start of the file, then read the first 0x8000 bytes.
  result = sceKernelLseek(fd, 0, 0);
  UNSIGNED_INT_EQUALS(0, result);
  bytes_read = sceKernelRead(fd, read_buf, 0x8000);
  LONGS_EQUAL(0x8000, bytes_read);

  // Compare the memory to the raw file contents.
  result = memcmp(read_buf, reinterpret_cast<void*>(output_addr), 0x8000);
  LONGS_EQUAL(0, result);

  // Unmap test memory
  result = sceKernelMunmap(output_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Close test file
  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // Test closing a file while it's mmapped
  fd = sceKernelOpen("/download0/test_file.txt", 0x2, 0666);
  CHECK(fd > 0);

  // mmap file to memory
  result = sceKernelMmap(addr, 0x8000, 3, 0, fd, offset, &output_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Check file contents
  memset(read_buf, 0, 0x8000);
  bytes_read = sceKernelRead(fd, read_buf, 0x8000);
  LONGS_EQUAL(0x8000, bytes_read);

  // Close test file
  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // Compare the memory to the raw file contents.
  result = memcmp(read_buf, reinterpret_cast<void*>(output_addr), 0x8000);
  LONGS_EQUAL(0, result);

  // Unmap test memory
  result = sceKernelMunmap(output_addr, 0x8000);
  UNSIGNED_INT_EQUALS(0, result);
}

TEST(MemoryTests, FlexibleTest) {
  // This test is to validate memory behaviors unique to flexible memory mappings.
  // Start by testing available flexible memory size.
  std::list<uint64_t> addresses;
  uint64_t            addr_out = 0;
  int32_t             result   = 0;
  while (result == 0) {
    result = sceKernelMapFlexibleMemory(&addr_out, 0x4000, 3, 0);
    if (result < 0) {
      // Out of flex mem
      UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
    } else {
      // Mapped flex mem successfully.
      UNSIGNED_INT_EQUALS(0, result);
      // Add the mapping address to addresses, need to unmap later to clean up.
      addresses.emplace_back(addr_out);
    }
  }

  // After all these mappings, available flex size should be 0.
  uint64_t avail_flex_size = 0;
  result                   = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0, avail_flex_size);

  // sceKernelMmap with MAP_ANON uses the flexible budget.
  uint64_t test_addr;
  result = sceKernelMmap(0, 0x4000, 3, 0x1000, -1, 0, &test_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Test budget behavior with MAP_STACK
  test_addr = 0xfb00000000;
  result    = sceKernelMmap(test_addr, 0x4000, 3, 0x400, -1, 0, &test_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // sceKernelMmap with MAP_VOID
  result = sceKernelMmap(test_addr, 0x4000, 3, 0x100, -1, 0, &test_addr);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(test_addr, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // sceKernelMmap with files also uses the flexible budget.
  int32_t fd = sceKernelOpen("/app0/assets/misc/test_file.txt", 0, 0666);
  CHECK(fd > 0);
  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &test_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // Make sure download dir file has enough space to mmap before trying.
  fd = sceKernelOpen("/download0/test_file.txt", 0x602, 0666);
  CHECK(fd > 0);
  char test_buf[0x100000];
  memset(test_buf, 0, sizeof(test_buf));
  int64_t bytes = sceKernelWrite(fd, test_buf, sizeof(test_buf));
  LONGS_EQUAL(sizeof(test_buf), bytes);

  // mmap download dir file.
  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &test_addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // Files in system folders like /data don't count towards the budget.
  fd = sceKernelOpen("/data/test_file.txt", 0x602, 0666);
  CHECK(fd > 0);
  memset(test_buf, 0, sizeof(test_buf));
  bytes = sceKernelWrite(fd, test_buf, sizeof(test_buf));
  LONGS_EQUAL(sizeof(test_buf), bytes);

  // mmap data dir file.
  result = sceKernelMmap(0, 0x4000, 3, 0, fd, 0, &test_addr);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(test_addr, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // We can't directly correlate flex size to mapped memory,
  // since my list of addresses will also cut into the flex mem budget.

  // Unmap all of the flexible memory used here.
  for (uint64_t addr: addresses) {
    result = sceKernelMunmap(addr, 0x4000);
    UNSIGNED_INT_EQUALS(0, result);
  }
  addresses.clear();

  // Available flex size should be greater than 0 now
  result = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK(avail_flex_size != 0);

  // Flexible memory is not backed in any way, so the contents are garbage before initializing.
  addr_out = 0x300000000;
  result   = sceKernelMapFlexibleMemory(&addr_out, 0x10000, 3, 0);
  UNSIGNED_INT_EQUALS(0, result);

  // While we're here, make sure stored memory info matches expectations for flexible memory.
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
  info   = {};
  result = sceKernelVirtualQuery(addr_out, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr_out, info.start);
  LONGS_EQUAL(addr_out + 0x10000, info.end);
  LONGS_EQUAL(0, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(1, info.is_flexible);
  LONGS_EQUAL(0, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Write 1's to the full memory range.
  memset(reinterpret_cast<void*>(addr_out), 1, 0x10000);

  // Ensure the full range has 1's
  memset(test_buf, 1, sizeof(test_buf));
  result = memcmp(test_buf, reinterpret_cast<void*>(addr_out), 0x10000);
  LONGS_EQUAL(0, result);

  // Unmap the middle of the range
  result = sceKernelMunmap(addr_out + 0x4000, 0x8000);
  UNSIGNED_INT_EQUALS(0, result);

  // Ensure the remaining mapped areas are still filled with 1's
  result = memcmp(test_buf, reinterpret_cast<void*>(addr_out), 0x4000);
  LONGS_EQUAL(0, result);
  result = memcmp(&test_buf[0xc000], reinterpret_cast<void*>(addr_out + 0xc000), 0x4000);
  LONGS_EQUAL(0, result);

  // Remap the unmapped area, use flag fixed to ensure correct placement.
  uint64_t new_addr = addr_out + 0x4000;
  result            = sceKernelMapFlexibleMemory(&new_addr, 0x8000, 3, 0x10);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr_out + 0x4000, new_addr);

  // The new memory area will not have the contents of the old memory.
  result = memcmp(&test_buf[0x4000], reinterpret_cast<void*>(new_addr), 0x8000);
  CHECK(result != 0);

  // Re-fill the area with 1's
  memset(reinterpret_cast<void*>(addr_out), 1, 0x10000);
  result = memcmp(test_buf, reinterpret_cast<void*>(addr_out), 0x10000);
  LONGS_EQUAL(0, result);

  // Overwrite the whole memory area, this should destroy the contents within.
  new_addr = addr_out;
  result   = sceKernelMapFlexibleMemory(&new_addr, 0x10000, 3, 0x10);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(new_addr, addr_out);

  result = memcmp(test_buf, reinterpret_cast<void*>(addr_out), 0x10000);
  CHECK(result != 0);

  result = sceKernelMunmap(new_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Get a base budget value to calculate with.
  uint64_t avail_flex_size_before = 0;
  result                          = sceKernelAvailableFlexibleMemorySize(&avail_flex_size_before);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK(avail_flex_size_before != 0);

  // Note: Seems like allocations are not directly consuming the budget.
  // Using these larger sized mappings allows me to validate budget behavior, but I will need to test this issue.

  // Test sceKernelMapFlexibleMemory to see how it impacts the budget.
  result = sceKernelMapFlexibleMemory(&new_addr, 0x100000, 3, 0x0);
  UNSIGNED_INT_EQUALS(0, result);

  // This should reduce available size by 0x100000.
  result = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(avail_flex_size_before - 0x100000, avail_flex_size);

  // Unmap the memory
  result = sceKernelMunmap(new_addr, 0x100000);
  UNSIGNED_INT_EQUALS(0, result);

  // This should bring available back to normal.
  result = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(avail_flex_size_before, avail_flex_size);

  // Test mmap with MAP_ANON to see how it impacts the budget.
  result = sceKernelMmap(0, 0x100000, 3, 0x1000, -1, 0, &new_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // This should reduce available size by 0x100000.
  result = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(avail_flex_size_before - 0x100000, avail_flex_size);

  // Unmap the memory
  result = sceKernelMunmap(new_addr, 0x100000);
  UNSIGNED_INT_EQUALS(0, result);

  // This should bring available back to normal.
  result = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(avail_flex_size_before, avail_flex_size);

  // Test file mmap from download dir to see how it impacts the budget.
  fd = sceKernelOpen("/download0/test_file.txt", 0, 0666);
  CHECK(fd > 0);
  result = sceKernelMmap(0, 0x100000, 3, 0, fd, 0, &new_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // This should reduce available size by 0x10000.
  result = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(avail_flex_size_before - 0x100000, avail_flex_size);

  // Unmap the memory
  result = sceKernelMunmap(new_addr, 0x100000);
  UNSIGNED_INT_EQUALS(0, result);

  // Close the file
  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // This should bring available back to normal.
  result = sceKernelAvailableFlexibleMemorySize(&avail_flex_size);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(avail_flex_size_before, avail_flex_size);
}

TEST(MemoryTests, DirectTest) {
  // This test is to validate memory behaviors somewhat unique to direct memory mappings.
  // This will focus on behavioral edge cases, since all the error checks involved are checked in MapMemoryTest.

  // First, sceKernelMapDirectMemory on physical addresses that are not mapped.
  // Once again, due to the SceGnmDriver mapping, 0 through 0x10000 is allocated.
  uint64_t addr   = 0;
  int32_t  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, 0x10000, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  LONGS_EQUAL(0, addr);

  // Since it's a different syscall, check sceKernelMapDirectMemory2 as well.
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 0, 3, 0, 0x10000, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  LONGS_EQUAL(0, addr);

  // Allocate some direct memory to test with.
  int64_t phys_addr = 0;
  result            = sceKernelAllocateMainDirectMemory(0x10000, 0x10000, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);

  // Perform a direct memory mapping.
  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);

  // While we're here, make sure stored memory info matches expectations for direct memory.
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
  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  // Offset is set to physical address.
  LONGS_EQUAL(phys_addr, info.offset);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  LONGS_EQUAL(0, info.is_flexible);
  LONGS_EQUAL(1, info.is_direct);
  LONGS_EQUAL(0, info.is_stack);
  LONGS_EQUAL(0, info.is_pooled);
  LONGS_EQUAL(1, info.is_committed);

  // Perform a second mapping to the same physical address, this should mirror the first one.
  uint64_t addr2 = 0;
  result         = sceKernelMapDirectMemory(&addr2, 0x10000, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);

  // Both calls should succeed, and writes to the first address will be mirrored to the second.
  memset(reinterpret_cast<void*>(addr), 1, 0x10000);
  result = memcmp(reinterpret_cast<void*>(addr), reinterpret_cast<void*>(addr2), 0x10000);
  LONGS_EQUAL(0, result);

  // Store the current state of this memory
  char test_buf[0x10000];
  memcpy(test_buf, reinterpret_cast<void*>(addr2), 0x10000);

  // Both addresses should be unmapped by sceKernelCheckedReleaseDirectMemory
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Ensure physical memory is properly released
  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // Reallocate memory
  result = sceKernelAllocateMainDirectMemory(0x10000, 0x10000, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);

  // Backing contents should remain the same despite the direct memory release.
  result = memcmp(reinterpret_cast<void*>(addr), test_buf, 0x10000);
  LONGS_EQUAL(0, result);

  // Partially release page. In theory, this should only unmap the part of mapped to these physical addresses.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr + 0x4000, 0x4000);
  UNSIGNED_INT_EQUALS(0, result);

  // Use sceKernelQueryMemoryProtection to validate expected addresses.
  uint64_t start_addr;
  uint64_t end_addr;
  int32_t  prot;
  result = sceKernelQueryMemoryProtection(addr, &start_addr, &end_addr, &prot);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, start_addr);
  LONGS_EQUAL(addr + 0x4000, end_addr);
  LONGS_EQUAL(3, prot);

  // We unmapped from 0x4000 to 0x8000 in the mapping. Query past that gap
  result = sceKernelQueryMemoryProtection(end_addr + 0x4000, &start_addr, &end_addr, &prot);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr + 0x8000, start_addr);
  LONGS_EQUAL(addr + 0x10000, end_addr);
  LONGS_EQUAL(3, prot);

  // Unmap the full direct memory area.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Remap the area, this should fail due to the missing physical address chunk.
  result = sceKernelMapDirectMemory(&addr, 0x10000, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // Since it's a different syscall, check sceKernelMapDirectMemory2 as well.
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 0, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // Release any remaining physical memory from before. Use sceKernelReleaseDirectMemory to skip the unallocated addresses.
  result = sceKernelReleaseDirectMemory(phys_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Loop through all possible mtypes, ensure all are backed appropriately
  int64_t dmem_start = 0x100000;
  int64_t dmem_size  = sceKernelGetDirectMemorySize();
  for (int32_t mtype = 0; mtype < 10; ++mtype) {
    // Allocate direct memory using specified mtype
    result = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x10000, 0x10000, mtype, &phys_addr);
    UNSIGNED_INT_EQUALS(0, result);

    // Map direct memory to this physical address space
    addr   = 0;
    result = sceKernelMapDirectMemory(&addr, 0x10000, 0x33, 0, phys_addr, 0);
    UNSIGNED_INT_EQUALS(0, result);

    // Check for proper backing behavior. As per usual, copy bytes, unmap, remap, then check for bytes.
    memset(reinterpret_cast<void*>(addr), 1, 0x10000);
    memcpy(test_buf, reinterpret_cast<void*>(addr), 0x10000);

    result = sceKernelMunmap(addr, 0x10000);
    UNSIGNED_INT_EQUALS(0, result);

    result = sceKernelMapDirectMemory(&addr, 0x10000, 0x33, 0, phys_addr, 0);
    UNSIGNED_INT_EQUALS(0, result);

    // If this succeeds, this memory is backed.
    result = memcmp(reinterpret_cast<void*>(addr), test_buf, 0x10000);
    LONGS_EQUAL(0, result);

    // Release direct memory used for this test.
    result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
    UNSIGNED_INT_EQUALS(0, result);

    // While we're here, we can test sceKernelMapDirectMemory2's type-setting.
    // The following code was written with the SceGnmDriver dmem allocation in mind,
    // instead of rewriting the code to remove that assumption, perform an extra dmem allocation with the same type and size.
    int64_t phys_addr2 = 0;
    result             = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x10000, 0x10000, 3, &phys_addr2);
    UNSIGNED_INT_EQUALS(0, result);

    for (int32_t test_mtype = 0; test_mtype < 10; ++test_mtype) {
      // Allocate direct memory with mtype
      result = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x10000, 0x10000, mtype, &phys_addr);
      UNSIGNED_INT_EQUALS(0, result);

      // Use sceKernelGetDirectMemoryType to ensure type is stored correctly
      int64_t phys_start;
      int64_t phys_end;
      int32_t out_mtype;
      result = sceKernelGetDirectMemoryType(phys_addr, &out_mtype, &phys_start, &phys_end);
      UNSIGNED_INT_EQUALS(0, result);
      if (out_mtype == 3) {
        // This is the mtype of our extra allocation.
        // Allocations with this type will coalesce with it.
        LONGS_EQUAL(phys_addr2, phys_start);
      } else {
        LONGS_EQUAL(phys_addr, phys_start);
      }
      LONGS_EQUAL(phys_addr + 0x10000, phys_end);
      LONGS_EQUAL(mtype, out_mtype);

      // Use sceKernelMapDirectMemory2 to change memory type.
      addr   = 0;
      result = sceKernelMapDirectMemory2(&addr, 0x10000, test_mtype, 0x33, 0, phys_addr, 0);
      UNSIGNED_INT_EQUALS(0, result);

      // Use sceKernelGetDirectMemoryType to ensure type is updated correctly
      result = sceKernelGetDirectMemoryType(phys_addr, &out_mtype, &phys_start, &phys_end);
      UNSIGNED_INT_EQUALS(0, result);
      // sceKernelMapDirectMemory2 doesn't coalesce dmem areas after changing type.
      if (out_mtype == 3 && mtype == 3) {
        // This is the mtype of our extra allocation.
        // Allocations with this type will coalesce with it.
        LONGS_EQUAL(phys_addr2, phys_start);
      } else {
        LONGS_EQUAL(phys_addr, phys_start);
      }
      LONGS_EQUAL(phys_addr + 0x10000, phys_end);
      LONGS_EQUAL(test_mtype, out_mtype);

      // Use sceKernelMunmap to unmap memory
      result = sceKernelMunmap(addr, 0x10000);
      UNSIGNED_INT_EQUALS(0, result);

      // Use sceKernelGetDirectMemoryType to ensure type remains the same
      result = sceKernelGetDirectMemoryType(phys_addr, &out_mtype, &phys_start, &phys_end);
      UNSIGNED_INT_EQUALS(0, result);
      // sceKernelMapDirectMemory2 doesn't coalesce dmem areas after changing type.
      if (out_mtype == 3 && mtype == 3) {
        // This is the mtype of our extra allocation.
        // Allocations with this type will coalesce with it.
        LONGS_EQUAL(phys_addr2, phys_start);
      } else {
        LONGS_EQUAL(phys_addr, phys_start);
      }
      LONGS_EQUAL(phys_addr + 0x10000, phys_end);
      LONGS_EQUAL(test_mtype, out_mtype);

      // Use sceKernelCheckedReleaseDirectMemory to erase direct memory
      result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
      UNSIGNED_INT_EQUALS(0, result);
    }

    // Release the extra allocation as well.
    result = sceKernelCheckedReleaseDirectMemory(phys_addr2, 0x10000);
    UNSIGNED_INT_EQUALS(0, result);
  }

  // memory type 10 does not allow write permissions of any form.
  result = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x10000, 0, 10, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0;
  result = sceKernelMapDirectMemory(&addr, 0x10000, 0, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0;
  result = sceKernelMapDirectMemory(&addr, 0x10000, 1, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelMapDirectMemory(&addr, 0x10000, 2, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  addr   = 0;
  result = sceKernelMapDirectMemory(&addr, 0x10000, 0x10, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0;
  result = sceKernelMapDirectMemory(&addr, 0x10000, 0x11, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelMapDirectMemory(&addr, 0x10000, 0x20, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMapDirectMemory(&addr, 0x10000, 0x22, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMapDirectMemory(&addr, 0x10000, 0x30, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMapDirectMemory(&addr, 0x10000, 0x33, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Make sure permissions are checked appropriately when changing type through sceKernelMapDirectMemory2
  result = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x10000, 0, 10, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = 0;
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 10, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 10, 0x20, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 10, 0x11, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Test dmem coalescing edge cases
  // We can get two "mergable" direct memory areas by utilizing sceKernelMapDirectMemory2.
  int64_t phys_addr2 = 0;

  // This will produce two separate direct memory areas.
  result = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x10000, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x10000, 0, 3, &phys_addr2);
  UNSIGNED_INT_EQUALS(0, result);

  // Use sceKernelMapDirectMemory2 to set the mtype of the first allocation to match the second.
  // Despite doing this, the two areas remain separate.
  addr   = 0;
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 3, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  // We don't need the dmem mapped for this test, so unmap it.
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Verify dmem map state
  int64_t phys_start;
  int64_t phys_end;
  int32_t out_mtype;
  result = sceKernelGetDirectMemoryType(phys_addr, &out_mtype, &phys_start, &phys_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr, phys_start);
  LONGS_EQUAL(phys_addr + 0x10000, phys_end);
  LONGS_EQUAL(3, out_mtype);
  result = sceKernelGetDirectMemoryType(phys_addr2, &out_mtype, &phys_start, &phys_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr2, phys_start);
  LONGS_EQUAL(phys_addr2 + 0x10000, phys_end);
  LONGS_EQUAL(3, out_mtype);

  // Try to map both areas in one sceKernelMapDirectMemory call.
  result = sceKernelMapDirectMemory(&addr, 0x20000, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x20000);
  UNSIGNED_INT_EQUALS(0, result);

  // That mapping will not coalesce the dmem areas.
  result = sceKernelGetDirectMemoryType(phys_addr, &out_mtype, &phys_start, &phys_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr, phys_start);
  LONGS_EQUAL(phys_addr + 0x10000, phys_end);
  LONGS_EQUAL(3, out_mtype);
  result = sceKernelGetDirectMemoryType(phys_addr2, &out_mtype, &phys_start, &phys_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr2, phys_start);
  LONGS_EQUAL(phys_addr2 + 0x10000, phys_end);
  LONGS_EQUAL(3, out_mtype);

  // Ensure checked release works properly under these conditions.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x20000);
  UNSIGNED_INT_EQUALS(0, result);

  phys_addr          = 0;
  phys_addr2         = 0;
  int64_t phys_addr3 = 0;
  int64_t phys_addr4 = 0;
  int64_t phys_addr5 = 0;
  // Set up four separate allocations, unmergable due to memory type.
  result = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x10000, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelAllocateDirectMemory(dmem_start + 0x10000, dmem_size, 0x10000, 0, 3, &phys_addr2);
  UNSIGNED_INT_EQUALS(0, result);
  // Leave a gap here, this will be where we allocate the third area.
  result = sceKernelAllocateDirectMemory(dmem_start + 0x30000, dmem_size, 0x10000, 0, 0, &phys_addr4);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelAllocateDirectMemory(dmem_start + 0x40000, dmem_size, 0x10000, 0, 3, &phys_addr5);
  UNSIGNED_INT_EQUALS(0, result);

  // Use sceKernelMapDirectMemory2 to set the mtypes of the first and third allocations to match the second and fourth.
  addr   = 0;
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 3, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);
  addr   = 0;
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 3, 3, 0, phys_addr4, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Now allocate a fifth area that is mergable with all nearby areas. This will merge 2, 3, and 4, but leave 1 and 5 separate.
  result = sceKernelAllocateDirectMemory(dmem_start, dmem_size, 0x10000, 0, 3, &phys_addr3);
  UNSIGNED_INT_EQUALS(0, result);

  // Use sceKernelGetDirectMemoryType to check.
  result = sceKernelGetDirectMemoryType(phys_addr, &out_mtype, &phys_start, &phys_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr, phys_start);
  LONGS_EQUAL(phys_addr2, phys_end);
  LONGS_EQUAL(3, out_mtype);
  result = sceKernelGetDirectMemoryType(phys_addr3, &out_mtype, &phys_start, &phys_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr2, phys_start);
  LONGS_EQUAL(phys_addr5, phys_end);
  LONGS_EQUAL(3, out_mtype);
  result = sceKernelGetDirectMemoryType(phys_addr5, &out_mtype, &phys_start, &phys_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr5, phys_start);
  LONGS_EQUAL(phys_addr5 + 0x10000, phys_end);
  LONGS_EQUAL(3, out_mtype);

  // This should release all allocations.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x50000);
  UNSIGNED_INT_EQUALS(0, result);

  // Set up two unmerged and unmapped allocations to test sceKernelMapDirectMemory2 with.
  // Release our allocation, then set the unmerged dmem areas back up.
  result = sceKernelAllocateMainDirectMemory(0x10000, 0, 0, &phys_addr);
  UNSIGNED_INT_EQUALS(0, result);
  addr   = 0;
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 3, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelAllocateMainDirectMemory(0x10000, 0, 0, &phys_addr2);
  UNSIGNED_INT_EQUALS(0, result);
  addr   = 0;
  result = sceKernelMapDirectMemory2(&addr, 0x10000, 3, 3, 0, phys_addr2, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Now we have two 0x10000 dmem pages with the same type, but unmerged. Validate this.
  result = sceKernelGetDirectMemoryType(phys_addr, &out_mtype, &phys_start, &phys_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr, phys_start);
  LONGS_EQUAL(phys_addr + 0x10000, phys_end);
  LONGS_EQUAL(3, out_mtype);
  result = sceKernelGetDirectMemoryType(phys_addr2, &out_mtype, &phys_start, &phys_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr2, phys_start);
  LONGS_EQUAL(phys_addr2 + 0x10000, phys_end);
  LONGS_EQUAL(3, out_mtype);

  // Run sceKernelMapDirectMemory2 to map both unallocated pages. Use -1 for mtype param to skip changing mtype.
  result = sceKernelMapDirectMemory2(&addr, 0x20000, -1, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x20000);
  UNSIGNED_INT_EQUALS(0, result);

  // This will not merge the areas.
  result = sceKernelGetDirectMemoryType(phys_addr, &out_mtype, &phys_start, &phys_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr, phys_start);
  LONGS_EQUAL(phys_addr + 0x10000, phys_end);
  LONGS_EQUAL(3, out_mtype);
  result = sceKernelGetDirectMemoryType(phys_addr2, &out_mtype, &phys_start, &phys_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr2, phys_start);
  LONGS_EQUAL(phys_addr2 + 0x10000, phys_end);
  LONGS_EQUAL(3, out_mtype);

  // Run sceKernelMapDirectMemory2 to map and change type for both areas.
  result = sceKernelMapDirectMemory2(&addr, 0x20000, 0, 3, 0, phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMunmap(addr, 0x20000);
  UNSIGNED_INT_EQUALS(0, result);

  // This does not merge the areas.
  result = sceKernelGetDirectMemoryType(phys_addr, &out_mtype, &phys_start, &phys_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr, phys_start);
  LONGS_EQUAL(phys_addr + 0x10000, phys_end);
  LONGS_EQUAL(0, out_mtype);
  result = sceKernelGetDirectMemoryType(phys_addr2, &out_mtype, &phys_start, &phys_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(phys_addr2, phys_start);
  LONGS_EQUAL(phys_addr2 + 0x10000, phys_end);
  LONGS_EQUAL(0, out_mtype);

  // This should release both allocations.
  result = sceKernelCheckedReleaseDirectMemory(phys_addr, 0x20000);
  UNSIGNED_INT_EQUALS(0, result);
}

TEST(MemoryTests, CoalescingTest) {
  /*
  Notes about coalescing logic:
    Coalescing is skipped entirely for memory pools.
    Coalescing is checked for both the previous and next entries.
    Only coalesce when protections, max_protections, inheritance, wired_count, vm_container, budget_type, cred, ext_flags, obj_entry_id, name all match.
    Additionally, address and offset must be sequential.
    Firmwares below 5.50 also require the same calling address (anon_addr).
  */
  // Define lambdas for memory calls. This (along with the optnone attribute) is needed to ensure a static calling address
  // (which is one of the things checked when merging vmem areas).
  auto map_func = [](uint64_t* addr, uint64_t size, int32_t fd, uint64_t offset, int32_t flags, int32_t prot = 0x33) __attribute__((optnone)) {
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
    if (fd != -1) {
      // Can't GPU map files.
      prot &= 0x3;
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
    info           = {};
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

  // Test memory coalescing behaviors.
  // For safety, start by reserving a 0xa0000 sized chunk of virtual memory
  uint64_t addr   = 0x2000000000;
  int32_t  result = sceKernelReserveVirtualRange(&addr, 0xa0000, 0, 0);
  UNSIGNED_INT_EQUALS(0, result);

  // To test coalescing in it's fullest, we'll map outside-in
  uint64_t base_addr = addr;
  result             = map_func(&addr, 0x20000, -1, 0, 0x1010);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x80000;
  result = map_func(&addr, 0x20000, -1, 0, 0x1010);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x20000;
  result = map_func(&addr, 0x20000, -1, 0, 0x1010);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x60000;
  result = map_func(&addr, 0x20000, -1, 0, 0x1010);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // None of these mappings should combine.
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
  result = map_func(&addr, 0x20000, -1, 0, 0x1010);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Nothing combines, we now have 5 separate areas.
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

  // Test using sceKernelSetVirtualRangeName, see if that merges anything.
  result = sceKernelSetVirtualRangeName(base_addr, 0xa0000, "Mapping");
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Nothing combines, all areas are named separately.
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

  // Test using sceKernelMprotect, see if anything merges.
  result = sceKernelMprotect(base_addr, 0xa0000, 0x3);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Nothing combines, all areas now have reduced prot.
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

  // Test using sceKernelMlock, see if anything merges.
  result = sceKernelMlock(base_addr, 0xa0000);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Nothing changes.
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

  // Unmap testing memory.
  result = unmap_func(base_addr, 0xa0000);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Perform memory reservations instead.
  addr   = base_addr;
  result = map_func(&addr, 0x20000, -1, 0, 0x111);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x80000;
  result = map_func(&addr, 0x20000, -1, 0, 0x111);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x20000;
  result = map_func(&addr, 0x20000, -1, 0, 0x111);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x60000;
  result = map_func(&addr, 0x20000, -1, 0, 0x111);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Check the state of the vmem areas.
  // Reserved areas coalesce normally.
  // sceKernelQueryMemoryProtection returns errors with reserved memory, use sceKernelVirtualQuery instead.
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
  info   = {};
  result = sceKernelVirtualQuery(base_addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, info.start);
  LONGS_EQUAL(base_addr + 0x40000, info.end);

  info   = {};
  result = sceKernelVirtualQuery(base_addr + 0x80000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x60000, info.start);
  LONGS_EQUAL(base_addr + 0xa0000, info.end);

  // Test making a mapping in between these other mappings.
  addr   = base_addr + 0x40000;
  result = map_func(&addr, 0x20000, -1, 0, 0x111);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // All reserved areas should coalesce together here.
  info   = {};
  result = sceKernelVirtualQuery(base_addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, info.start);
  LONGS_EQUAL(base_addr + 0xa0000, info.end);

  // Unmap testing memory.
  result = unmap_func(base_addr, 0xa0000);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Perform memory reservations without MAP_SHARED. This appears to skip some coalescing logic?
  addr   = base_addr;
  result = map_func(&addr, 0x20000, -1, 0, 0x110);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x80000;
  result = map_func(&addr, 0x20000, -1, 0, 0x110);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x20000;
  result = map_func(&addr, 0x20000, -1, 0, 0x110);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x60000;
  result = map_func(&addr, 0x20000, -1, 0, 0x110);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Check the state of the vmem areas.
  // sceKernelQueryMemoryProtection returns errors with reserved memory, use sceKernelVirtualQuery instead.
  info   = {};
  result = sceKernelVirtualQuery(base_addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, info.start);
  LONGS_EQUAL(base_addr + 0x40000, info.end);

  info   = {};
  result = sceKernelVirtualQuery(base_addr + 0x80000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x60000, info.start);
  LONGS_EQUAL(base_addr + 0xa0000, info.end);

  // Test making a mapping in between these other mappings.
  addr   = base_addr + 0x40000;
  result = map_func(&addr, 0x20000, -1, 0, 0x110);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Without MAP_SHARED, the new mapping only merges with the previous mapping, the later mapping remains unmerged.
  info   = {};
  result = sceKernelVirtualQuery(base_addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, info.start);
  LONGS_EQUAL(base_addr + 0x60000, info.end);

  info   = {};
  result = sceKernelVirtualQuery(base_addr + 0x60000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x60000, info.start);
  LONGS_EQUAL(base_addr + 0xa0000, info.end);

  // Unmap testing memory.
  result = unmap_func(base_addr, 0xa0000);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Now test with direct memory.
  // With SDK version 1.00, this will not coalesce.
  addr   = base_addr;
  result = map_func(&addr, 0x20000, -1, 0x100000, 0x10);
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

  mem_scan();

  // Mappings remain separate.
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

  // Test using sceKernelSetVirtualRangeName, see if that merges anything.
  result = sceKernelSetVirtualRangeName(base_addr, 0xa0000, "Mapping");
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Mappings remain separate, name is applied to all.
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

  // Test using sceKernelMprotect, see if anything merges.
  result = sceKernelMprotect(base_addr, 0xa0000, 0x3);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Mappings remain separate, protection is applied to all.
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

  // Test using sceKernelMlock, see if anything merges.
  result = sceKernelMlock(base_addr, 0xa0000);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Mappings remain separate.
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

  // Unmap testing memory.
  result = unmap_func(base_addr, 0xa0000);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Test coalescing with a file mmap.
  int32_t fd = sceKernelOpen("/download0/test_file.txt", 0x602, 0666);
  CHECK(fd > 0);

  result = sceKernelFtruncate(fd, 0x80000);
  UNSIGNED_INT_EQUALS(0, result);

  // file mmaps coalesce so long as MAP_SHARED is provided. Otherwise, they remain separate.
  // Start by testing without MAP_SHARED.
  addr   = base_addr;
  result = map_func(&addr, 0x4000, fd, 0x20000, 0x10);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x10000;
  result = map_func(&addr, 0x4000, fd, 0x30000, 0x10);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x4000;
  result = map_func(&addr, 0x4000, fd, 0x24000, 0x10);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0xc000;
  result = map_func(&addr, 0x4000, fd, 0x2c000, 0x10);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0x4000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x4000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x4000, start_addr);
  LONGS_EQUAL(base_addr + 0x8000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0xc000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0xc000, start_addr);
  LONGS_EQUAL(base_addr + 0x10000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x10000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x10000, start_addr);
  LONGS_EQUAL(base_addr + 0x14000, end_addr);

  // Test making a mapping in between these other mappings.
  addr   = base_addr + 0x8000;
  result = map_func(&addr, 0x4000, fd, 0x28000, 0x10);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Mappings remain separate.
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0x4000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x4000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x4000, start_addr);
  LONGS_EQUAL(base_addr + 0x8000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x8000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x8000, start_addr);
  LONGS_EQUAL(base_addr + 0xc000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0xc000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0xc000, start_addr);
  LONGS_EQUAL(base_addr + 0x10000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x10000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x10000, start_addr);
  LONGS_EQUAL(base_addr + 0x14000, end_addr);

  // Test using sceKernelSetVirtualRangeName, see if that merges anything.
  result = sceKernelSetVirtualRangeName(base_addr, 0x14000, "Mapping");
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Mappings remain separate.
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0x4000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x4000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x4000, start_addr);
  LONGS_EQUAL(base_addr + 0x8000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x8000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x8000, start_addr);
  LONGS_EQUAL(base_addr + 0xc000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0xc000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0xc000, start_addr);
  LONGS_EQUAL(base_addr + 0x10000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x10000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x10000, start_addr);
  LONGS_EQUAL(base_addr + 0x14000, end_addr);

  // Test using sceKernelMprotect, see if anything merges.
  result = sceKernelMprotect(base_addr, 0x14000, 0x1);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Mappings remain separate, protection is applied to all.
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0x4000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x4000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x4000, start_addr);
  LONGS_EQUAL(base_addr + 0x8000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x8000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x8000, start_addr);
  LONGS_EQUAL(base_addr + 0xc000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0xc000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0xc000, start_addr);
  LONGS_EQUAL(base_addr + 0x10000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x10000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x10000, start_addr);
  LONGS_EQUAL(base_addr + 0x14000, end_addr);

  // Test using sceKernelMlock, see if anything merges.
  result = sceKernelMlock(base_addr, 0x14000);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Mappings remain separate.
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0x4000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x4000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x4000, start_addr);
  LONGS_EQUAL(base_addr + 0x8000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x8000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x8000, start_addr);
  LONGS_EQUAL(base_addr + 0xc000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0xc000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0xc000, start_addr);
  LONGS_EQUAL(base_addr + 0x10000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x10000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x10000, start_addr);
  LONGS_EQUAL(base_addr + 0x14000, end_addr);

  // Unmap testing memory.
  result = unmap_func(base_addr, 0x14000);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // file mmaps coalesce so long as MAP_SHARED is provided. Otherwise, they remain separate.
  // Test with MAP_SHARED here.
  addr   = base_addr;
  result = map_func(&addr, 0x4000, fd, 0x20000, 0x11);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x10000;
  result = map_func(&addr, 0x4000, fd, 0x30000, 0x11);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x4000;
  result = map_func(&addr, 0x4000, fd, 0x24000, 0x11);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0xc000;
  result = map_func(&addr, 0x4000, fd, 0x2c000, 0x11);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0x8000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0xc000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0xc000, start_addr);
  LONGS_EQUAL(base_addr + 0x14000, end_addr);

  // Test making a mapping in between these other mappings.
  addr   = base_addr + 0x8000;
  result = map_func(&addr, 0x4000, fd, 0x28000, 0x11);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Mappings all combine together.
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0x14000, end_addr);

  // Unmap testing memory.
  result = unmap_func(base_addr, 0x14000);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Check behavior for read-only file mmaps without MAP_SHARED
  // This case also merges, like how MAP_SHARED behaves.
  addr   = base_addr;
  result = map_func(&addr, 0x4000, fd, 0x20000, 0x10, 0x1);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x10000;
  result = map_func(&addr, 0x4000, fd, 0x30000, 0x10, 0x1);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x4000;
  result = map_func(&addr, 0x4000, fd, 0x24000, 0x10, 0x1);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0xc000;
  result = map_func(&addr, 0x4000, fd, 0x2c000, 0x10, 0x1);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0x8000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0xc000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0xc000, start_addr);
  LONGS_EQUAL(base_addr + 0x14000, end_addr);

  // Test making a mapping in between these other mappings.
  addr   = base_addr + 0x8000;
  result = map_func(&addr, 0x4000, fd, 0x28000, 0x10, 0x1);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Mappings all combine together.
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0x14000, end_addr);

  // Unmap testing memory.
  result = unmap_func(base_addr, 0x14000);
  UNSIGNED_INT_EQUALS(0, result);

  sceKernelClose(fd);

  // Now test the NoCoalesce (0x400000) flag
  // With this, even reserved memory will not coalesce.
  addr   = base_addr;
  result = map_func(&addr, 0x20000, -1, 0, 0x400111);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x80000;
  result = map_func(&addr, 0x20000, -1, 0, 0x400111);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x20000;
  result = map_func(&addr, 0x20000, -1, 0, 0x400111);
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x60000;
  result = map_func(&addr, 0x20000, -1, 0, 0x400111);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Check the state of the vmem areas.
  // Due to the NoCoalesce flag, these won't combine
  info   = {};
  result = sceKernelVirtualQuery(base_addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, info.start);
  LONGS_EQUAL(base_addr + 0x20000, info.end);

  info   = {};
  result = sceKernelVirtualQuery(base_addr + 0x20000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x20000, info.start);
  LONGS_EQUAL(base_addr + 0x40000, info.end);

  info   = {};
  result = sceKernelVirtualQuery(base_addr + 0x60000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x60000, info.start);
  LONGS_EQUAL(base_addr + 0x80000, info.end);

  info   = {};
  result = sceKernelVirtualQuery(base_addr + 0x80000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x80000, info.start);
  LONGS_EQUAL(base_addr + 0xa0000, info.end);

  // Test making a mapping in between these other mappings.
  // Omit the NoCoalesce flag, this still shouldn't coalesce due to the surrounding areas.
  addr   = base_addr + 0x40000;
  result = map_func(&addr, 0x20000, -1, 0, 0x111);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // The mappings remain separate.
  info   = {};
  result = sceKernelVirtualQuery(base_addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, info.start);
  LONGS_EQUAL(base_addr + 0x20000, info.end);

  info   = {};
  result = sceKernelVirtualQuery(base_addr + 0x20000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x20000, info.start);
  LONGS_EQUAL(base_addr + 0x40000, info.end);

  info   = {};
  result = sceKernelVirtualQuery(base_addr + 0x40000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x40000, info.start);
  LONGS_EQUAL(base_addr + 0x60000, info.end);

  info   = {};
  result = sceKernelVirtualQuery(base_addr + 0x60000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x60000, info.start);
  LONGS_EQUAL(base_addr + 0x80000, info.end);

  info   = {};
  result = sceKernelVirtualQuery(base_addr + 0x80000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x80000, info.start);
  LONGS_EQUAL(base_addr + 0xa0000, info.end);

  // Test using sceKernelSetVirtualRangeName, see if that merges anything.
  result = sceKernelSetVirtualRangeName(base_addr, 0xa0000, "Mapping");
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Even there, mappings shouldn't coalesce.
  info   = {};
  result = sceKernelVirtualQuery(base_addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, info.start);
  LONGS_EQUAL(base_addr + 0x20000, info.end);

  info   = {};
  result = sceKernelVirtualQuery(base_addr + 0x20000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x20000, info.start);
  LONGS_EQUAL(base_addr + 0x40000, info.end);

  info   = {};
  result = sceKernelVirtualQuery(base_addr + 0x40000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x40000, info.start);
  LONGS_EQUAL(base_addr + 0x60000, info.end);

  info   = {};
  result = sceKernelVirtualQuery(base_addr + 0x60000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x60000, info.start);
  LONGS_EQUAL(base_addr + 0x80000, info.end);

  info   = {};
  result = sceKernelVirtualQuery(base_addr + 0x80000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x80000, info.start);
  LONGS_EQUAL(base_addr + 0xa0000, info.end);

  // Unmap testing memory.
  result = unmap_func(base_addr, 0xa0000);
  UNSIGNED_INT_EQUALS(0, result);
}

TEST(MemoryTests, ProtectTest) {
  // Set up some basic memory this test can use.
  // Allocate a decent chunk of direct memory, this area should be free, and isn't touched by other tests.
  int64_t  dmem_start = 0x300000;
  uint64_t dmem_size  = 0x100000;
  int64_t  dmem_phys_addr;
  int32_t  result = sceKernelAllocateDirectMemory(dmem_start, dmem_start + dmem_size, dmem_size, 0, 0, &dmem_phys_addr);
  UNSIGNED_INT_EQUALS(0, result);
  // The elevated dmem_start should ensure this area is free, so dmem_phys_addr should equal dmem_start.
  LONGS_EQUAL(dmem_start, dmem_phys_addr);

  // Reserve a chunk of memory for testing with.
  // Use a fixed base address that should be free.
  uint64_t vmem_start = 0x4000000000;
  uint64_t vmem_size  = 0x1000000;
  uint64_t addr       = vmem_start;
  result              = sceKernelReserveVirtualRange(&addr, vmem_size, 0x10, 0);
  UNSIGNED_INT_EQUALS(0, result);
  // MAP_FIXED should ensure this is the same.
  LONGS_EQUAL(vmem_start, addr);

  // Start with some "normal" behavior to ensure edge case tests are based on valid calls.
  // Map some direct memory, give it write permissions, write some test data, then protect with read and read that data back.
  // This will test basic functionality of mprotect.
  addr   = vmem_start + 0x20000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0, 0x10, dmem_start + 0x20000, 0);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, vmem_start + 0x20000);

  // For simplicity, use sceKernelVirtualQuery to validate prot.
  // While not necessary for most mappings, some tests will need it.
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

  // Verify the lack of protection.
  OrbisKernelVirtualQueryInfo info = {};
  result                           = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0, info.prot);

  // Use mprotect to set the full memory area to read-write.
  // A future edge case to test is that read is forcibly appended to write mappings (which is why I'm using read-write prot here).
  result = sceKernelMprotect(addr, 0x20000, 0x3);
  UNSIGNED_INT_EQUALS(0, result);

  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(3, info.prot);

  // Now write to the new memory area.
  const char* test_str = "This is a test of memory writing";
  strcpy((char*)addr, test_str);

  // Now protect with no prot
  result = sceKernelMprotect(addr, 0x20000, 0x0);
  UNSIGNED_INT_EQUALS(0, result);

  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0, info.prot);

  // Now protect with read prot.
  // If protection is handled properly, the string I copied should still be there.
  result = sceKernelMprotect(addr, 0x20000, 0x1);
  UNSIGNED_INT_EQUALS(0, result);

  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(1, info.prot);

  // Now read from the memory
  result = strcmp((char*)addr, test_str);
  UNSIGNED_INT_EQUALS(0, result);

  // Replace mapped memory with a reserved area before proceeding.
  // Since this compiles with SDK ver 1.00, the only way to ensure all mappings coalesce into one is to re-reserve the whole area.
  addr   = vmem_start;
  result = sceKernelReserveVirtualRange(&addr, vmem_size, 0x10, 0);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(vmem_start, addr);

  // Memory reserved through sceKernelReserveVirtualRange (or mmap with MAP_VOID) restricts maximum prot to 0.
  // That said, this call still splits the vmem area.
  result = sceKernelMprotect(addr + 0x20000, 0x20000, 0x1);
  UNSIGNED_INT_EQUALS(0, result);

  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x20000, info.end);
  LONGS_EQUAL(0, info.prot);

  info   = {};
  result = sceKernelVirtualQuery(addr + 0x20000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr + 0x20000, info.start);
  LONGS_EQUAL(addr + 0x40000, info.end);
  LONGS_EQUAL(0, info.prot);

  info   = {};
  result = sceKernelVirtualQuery(addr + 0x40000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr + 0x40000, info.start);
  LONGS_EQUAL(addr + vmem_size, info.end);
  LONGS_EQUAL(0, info.prot);

  // Note: Attempting to give GPU protections to a reserved memory area causes a full system crash.
  // Only uncomment this if you're willing to unplug your PS4 just to get it to shut off
  // result = sceKernelMprotect(addr, 0x20000, 0x10);

  // This should split the memory area up further.
  result = sceKernelMprotect(addr + 0x28000, 0x10000, 0x7);
  UNSIGNED_INT_EQUALS(0, result);

  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x20000, info.end);
  LONGS_EQUAL(0, info.prot);

  info   = {};
  result = sceKernelVirtualQuery(addr + 0x20000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr + 0x20000, info.start);
  LONGS_EQUAL(addr + 0x28000, info.end);
  LONGS_EQUAL(0, info.prot);

  info   = {};
  result = sceKernelVirtualQuery(addr + 0x28000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr + 0x28000, info.start);
  LONGS_EQUAL(addr + 0x38000, info.end);
  LONGS_EQUAL(0, info.prot);

  info   = {};
  result = sceKernelVirtualQuery(addr + 0x38000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr + 0x38000, info.start);
  LONGS_EQUAL(addr + 0x40000, info.end);
  LONGS_EQUAL(0, info.prot);

  info   = {};
  result = sceKernelVirtualQuery(addr + 0x40000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr + 0x40000, info.start);
  LONGS_EQUAL(addr + vmem_size, info.end);
  LONGS_EQUAL(0, info.prot);

  // This should re-merge the inner mappings
  result = sceKernelMprotect(addr + 0x28000, 0x10000, 0x1);
  UNSIGNED_INT_EQUALS(0, result);

  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x20000, info.end);
  LONGS_EQUAL(0, info.prot);

  info   = {};
  result = sceKernelVirtualQuery(addr + 0x20000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr + 0x20000, info.start);
  LONGS_EQUAL(addr + 0x40000, info.end);
  LONGS_EQUAL(0, info.prot);

  info   = {};
  result = sceKernelVirtualQuery(addr + 0x40000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr + 0x40000, info.start);
  LONGS_EQUAL(addr + vmem_size, info.end);
  LONGS_EQUAL(0, info.prot);

  // This mprotect will re-coalesce the full reservation.
  result = sceKernelMprotect(addr + 0x20000, 0x20000, 0x0);
  UNSIGNED_INT_EQUALS(0, result);

  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + vmem_size, info.end);
  LONGS_EQUAL(0, info.prot);

  // This mprotect should do absolutely nothing.
  result = sceKernelMprotect(addr + 0x28000, 0x10000, 0x0);
  UNSIGNED_INT_EQUALS(0, result);

  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + vmem_size, info.end);
  LONGS_EQUAL(0, info.prot);

  // Now test executable mappings. These are rarely seen in retail games, but utilized by some homebrew.
  addr   = vmem_start + 0x100000;
  result = sceKernelMapFlexibleMemory(&addr, 0x100000, 0x3, 0x10);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(vmem_start + 0x100000, addr);

  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x100000, info.end);
  LONGS_EQUAL(3, info.prot);

  // Write a basic function into memory that returns 256.
  uint8_t bytes[] = {0x48, 0xc7, 0xc0, 00, 01, 00, 00, 0xc3};
  memcpy(reinterpret_cast<void*>(addr), bytes, 8);
  typedef int32_t (*func)();
  func test_func = reinterpret_cast<func>(addr);

  // Now use mprotect to turn this flexible mapping into an executable memory area.
  result = sceKernelMprotect(addr, 0x100000, 0x4);
  UNSIGNED_INT_EQUALS(0, result);

  // Now run the function from memory, make sure it returns the expected value.
  result = test_func();
  LONGS_EQUAL(0x100, result);

  // Replace mapped memory with a reserved area before proceeding.
  addr   = vmem_start;
  result = sceKernelReserveVirtualRange(&addr, vmem_size, 0x10, 0);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(vmem_start, addr);

  // Now test direct memory behavior.
  // As MapMemoryTest shows, direct memory mappings with executable protections aren't allowed.
  // However, you can protect the direct memory, and it will be executable.
  addr   = vmem_start + 0x100000;
  result = sceKernelMapDirectMemory(&addr, 0x10000, 0x3, 0x10, dmem_start, 0);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(vmem_start + 0x100000, addr);

  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(3, info.prot);

  // Write a function into this memory.
  memcpy(reinterpret_cast<void*>(addr), bytes, 8);
  test_func = reinterpret_cast<func>(addr);

  // Now use mprotect to turn this direct mapping into an executable area.
  result = sceKernelMprotect(addr, 0x100000, 0x4);
  UNSIGNED_INT_EQUALS(0, result);

  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(0, info.prot);

  // Now run the function from memory, make sure it returns the expected value.
  result = test_func();
  LONGS_EQUAL(0x100, result);

  // Replace mapped memory with a reserved area before proceeding.
  addr   = vmem_start;
  result = sceKernelReserveVirtualRange(&addr, vmem_size, 0x10, 0);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(vmem_start, addr);

  // In addition, we should be able to execute from files too.
  // use sceKernelWrite to write our function to the file data, then mmap the file with execute protections and run the function.
  int32_t fd = sceKernelOpen("/download0/test_file.txt", 0x602, 0666);
  CHECK(fd > 0);

  int64_t bytes_written = sceKernelWrite(fd, bytes, 8);
  LONGS_EQUAL(8, bytes_written);

  // The function should be at the start of the file.
  // While the file mmap succeeds, the execute protection isn't applied.
  result = sceKernelMmap(vmem_start, 0x4000, 0x7, 0x11, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(vmem_start, addr);

  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x4000, info.end);
  LONGS_EQUAL(3, info.prot);

  // Use mprotect to apply execute protection.
  result = sceKernelMprotect(addr, 0x100000, 0x4);
  UNSIGNED_INT_EQUALS(0, result);

  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x4000, info.end);
  LONGS_EQUAL(0, info.prot);

  // Run the function from memory. Since it's at the start of the file, it should be at the start of the memory area.
  test_func = reinterpret_cast<func>(addr);
  result    = test_func();
  LONGS_EQUAL(0x100, result);

  // Replace mapped memory with a reserved area before proceeding.
  addr   = vmem_start;
  result = sceKernelReserveVirtualRange(&addr, vmem_size, 0x10, 0);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(vmem_start, addr);

  // Close the test file.
  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // If an mprotect call covers multiple memory areas, it will protect all of them.

  // Start by splitting the reserved area up again.
  for (int32_t i = 0; i < 0x10; i++) {
    uint64_t test_addr = vmem_start + ((vmem_size / 0x10) * i);
    // Do some weird arithmetic to ensure we never accidentally duplicate prots
    // Since PS4 appends read to all write-only protections.
    int32_t test_prot = ((i * 2) + 1) % 7;
    result            = sceKernelMprotect(test_addr, vmem_size / 10, test_prot);
    UNSIGNED_INT_EQUALS(0, result);
  }

  // Verify the splitting occurred as expected.
  for (int32_t i = 0; i < 0x10; i++) {
    uint64_t test_addr = vmem_start + ((vmem_size / 0x10) * i);
    info               = {};
    result             = sceKernelVirtualQuery(test_addr, 0, &info, sizeof(info));
    UNSIGNED_INT_EQUALS(0, result);
    LONGS_EQUAL(test_addr, info.start);
    LONGS_EQUAL(0, info.prot);
  }

  // Perform an mprotect that will merge the whole area together.
  // Include extra space in the call to ensure this behavior still happens if free areas are encountered.
  result = sceKernelMprotect(vmem_start - vmem_size, vmem_size * 3, 0);
  UNSIGNED_INT_EQUALS(0, result);
  info   = {};
  result = sceKernelVirtualQuery(vmem_start, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(vmem_start, info.start);
  LONGS_EQUAL(vmem_start + vmem_size, info.end);
  LONGS_EQUAL(0, info.prot);

  // Finally, perform a flexible mapping, then protect the whole map and check it.
  // The full map mprotect will include this mapping.
  addr   = vmem_start + 0x100000;
  result = sceKernelMapFlexibleMemory(&addr, 0x100000, 3, 0x10);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(vmem_start + 0x100000, addr);
  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x100000, info.end);
  LONGS_EQUAL(3, info.prot);

  // This does mark various mappings in the memory map with RWX prot.
  // Not all mappings are marked though, so this needs further investigation.
  result = sceKernelMprotect(0, 0x5000000000, 0x7);
  UNSIGNED_INT_EQUALS(0, result);

  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x100000, info.end);
  LONGS_EQUAL(7, info.prot);

  // Replace mapped memory with a reserved area before proceeding.
  addr   = vmem_start;
  result = sceKernelReserveVirtualRange(&addr, vmem_size, 0x10, 0);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(vmem_start, addr);

  // The PS4 appends read to all write only mappings.
  result = sceKernelMapFlexibleMemory(&addr, 0x100000, 0, 0x10);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(vmem_start, addr);
  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x100000, info.end);
  LONGS_EQUAL(0, info.prot);

  // Write-only protection.
  result = sceKernelMprotect(addr, 0x10000, 0x2);
  UNSIGNED_INT_EQUALS(0, result);
  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  // Read is appended.
  LONGS_EQUAL(3, info.prot);

  // Write | Execute protection.
  result = sceKernelMprotect(addr, 0x10000, 0x6);
  UNSIGNED_INT_EQUALS(0, result);
  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  // Read is appended.
  LONGS_EQUAL(7, info.prot);

  // Finally, test parameters.
  // Start with the parameter alignment.
  result = sceKernelMprotect(addr, 0xf000, 0x3);
  UNSIGNED_INT_EQUALS(0, result);
  info = {};
  // Alignment should align size up to 0x10000
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(3, info.prot);

  result = sceKernelMprotect(addr + 0x3000, 0x10000, 0x7);
  UNSIGNED_INT_EQUALS(0, result);
  info = {};
  // Alignment should align addr + 0x3000 down to addr, and that offset means size is aligned up to 0x14000
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x14000, info.end);
  LONGS_EQUAL(7, info.prot);

  // Because size rounding follows the formula (addr & page_mask) + page_mask + size & ~page_mask
  // addr + 0x3000 is rounded down to addr, and size gets rounded up to 0x14000 here.
  result = sceKernelMprotect(addr + 0x3000, 0xf000, 0x3);
  UNSIGNED_INT_EQUALS(0, result);
  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x14000, info.end);
  LONGS_EQUAL(3, info.prot);

  // mprotect simply ignores invalid protection flags.
  result = sceKernelMprotect(addr, 0x10000, 0xf);
  UNSIGNED_INT_EQUALS(0, result);
  info = {};
  // Alignment should align addr down to 0x1000, size up to 0x10000
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(7, info.prot);

  // Clean up memory used.
  result = sceKernelMunmap(vmem_start, vmem_size);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelCheckedReleaseDirectMemory(dmem_start, dmem_size);
  UNSIGNED_INT_EQUALS(0, result);

  // At this point, the memory designated by vmem_start is free.
  // mprotect succeeds on free memory, but does nothing to it (not that you can actually check for that)
  result = sceKernelMprotect(vmem_start, vmem_size, 0x3);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelMprotect(vmem_start, vmem_size, 0x10);
  UNSIGNED_INT_EQUALS(0, result);
}

TEST(MemoryTests, TypeProtectTest) {
  /**
   * This function uses completely separate code from mprotect.
   *
   * Initial errors:
   *  If mtype is greater than 10, returns EINVAL
   *  If prot & 0xc8 != 0, returns EINVAL (invalid prot check? 0xc8 = 0x8 | 0x40 | 0x80)
   *    Bits higher than that are simply ignored, so prot 0x100 results in protecting with prot 0.
   *  If mtype == 10, and prot contains execute or write permissions, then returns EPERM.
   *  Blockpool logic, like in most places, diverges part-way through here.
   */

  // Like with the mprotect tests, allocate a chunk of direct memory, and a chunk of virtual memory space for testing.
  // Set up some basic memory this test can use.
  // Allocate a decent chunk of direct memory, this area should be free, and isn't touched by other tests.
  int64_t  dmem_start = 0x400000;
  uint64_t dmem_size  = 0x100000;
  int64_t  dmem_phys_addr;
  int32_t  result = sceKernelAllocateDirectMemory(dmem_start, dmem_start + dmem_size, dmem_size, 0, 0, &dmem_phys_addr);
  UNSIGNED_INT_EQUALS(0, result);
  // The elevated dmem_start should ensure this area is free, so dmem_phys_addr should equal dmem_start.
  LONGS_EQUAL(dmem_start, dmem_phys_addr);

  // Reserve a chunk of memory for testing with.
  // Use a fixed base address that should be free.
  uint64_t vmem_start = 0x5000000000;
  uint64_t vmem_size  = 0x1000000;
  uint64_t addr       = vmem_start;
  result              = sceKernelReserveVirtualRange(&addr, vmem_size, 0x10, 0);
  UNSIGNED_INT_EQUALS(0, result);
  // MAP_FIXED should ensure this is the same.
  LONGS_EQUAL(vmem_start, addr);

  // As a basic test case, perform a direct memory mapping, then mtypeprotect it.
  addr   = vmem_start + 0x100000;
  result = sceKernelMapDirectMemory(&addr, 0x10000, 0, 0x10, dmem_phys_addr, 0);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(vmem_start + 0x100000, addr);

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

  // Verify the lack of protection.
  OrbisKernelVirtualQueryInfo info = {};
  result                           = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0, info.prot);
  LONGS_EQUAL(0, info.memory_type);

  // Perform both type-setting and prot setting here to ensure it fully functions.
  result = sceKernelMtypeprotect(addr, 0x10000, 3, 0x3);
  UNSIGNED_INT_EQUALS(0, result);

  // Both prot and type can be validated through sceKernelVirtualQuery calls.
  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(3, info.memory_type);

  // Use sceKernelGetDirectMemoryType to confirm the type changing applied properly.
  int32_t out_type;
  int64_t out_start;
  int64_t out_end;
  result = sceKernelGetDirectMemoryType(dmem_phys_addr, &out_type, &out_start, &out_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(3, out_type);
  LONGS_EQUAL(dmem_phys_addr, out_start);
  LONGS_EQUAL(dmem_phys_addr + 0x10000, out_end);
  result = sceKernelGetDirectMemoryType(dmem_phys_addr + 0x10000, &out_type, &out_start, &out_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0, out_type);
  LONGS_EQUAL(dmem_phys_addr + 0x10000, out_start);
  LONGS_EQUAL(dmem_phys_addr + 0x100000, out_end);

  // If the memory is now read-write, we can both write and read from it. Test this.
  const char* test_str = "This is a test of memory writing";
  strcpy((char*)addr, test_str);

  // Reduce prot to read-only, change mtype again.
  result = sceKernelMtypeprotect(addr, 0x10000, 0, 0x1);
  UNSIGNED_INT_EQUALS(0, result);

  result = strcmp((char*)addr, test_str);
  LONGS_EQUAL(0, result);

  // Basic sanity check complete, now we can test the basic edge cases.

  // For basic parameter testing, use the memory mapped in the prior test.
  // Test invalid type parameters.
  result = sceKernelMtypeprotect(addr, 0x10000, -1, 0x1);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMtypeprotect(addr, 0x10000, 11, 0x1);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Test type 10 with write prots
  result = sceKernelMtypeprotect(addr, 0x10000, 10, 0x2);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMtypeprotect(addr, 0x10000, 10, 0x20);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // Test invalid prot params?
  result = sceKernelMtypeprotect(addr, 0x10000, 0, 0x8);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMtypeprotect(addr, 0x10000, 0, 0x40);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);
  result = sceKernelMtypeprotect(addr, 0x10000, 0, 0x80);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EINVAL, result);

  // Higher bits are ignored, protect still succeeds.
  result = sceKernelMtypeprotect(addr, 0x10000, 0, 0x101);
  UNSIGNED_INT_EQUALS(0, result);
  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(1, info.prot);
  LONGS_EQUAL(0, info.memory_type);

  // Try mtypeprotect to mtype 10, then mprotect to prot write.
  result = sceKernelMtypeprotect(addr, 0x10000, 10, 0x1);
  UNSIGNED_INT_EQUALS(0, result);
  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(1, info.prot);
  LONGS_EQUAL(10, info.memory_type);

  // Mprotect fails since the physical memory has type 10, which can't be mapped with write perms.
  result = sceKernelMprotect(addr, 0x10000, 0x3);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // Type changing occurs before prot, so this succeeds.
  result = sceKernelMtypeprotect(addr, 0x10000, 0, 0x3);
  UNSIGNED_INT_EQUALS(0, result);
  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);

  // Now for the "fun" stuff, try splitting direct memory by setting types.
  // We can use the direct memory area we already have for this.
  result = sceKernelMtypeprotect(addr + 0x4000, 0x8000, 3, 0x3);
  UNSIGNED_INT_EQUALS(0, result);

  // mem_scan to help visualize
  mem_scan();

  // With this, our 1 direct memory mapping should split into three.
  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x4000, info.end);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);
  info   = {};
  result = sceKernelVirtualQuery(addr + 0x4000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr + 0x4000, info.start);
  LONGS_EQUAL(addr + 0xc000, info.end);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(3, info.memory_type);
  info   = {};
  result = sceKernelVirtualQuery(addr + 0xc000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr + 0xc000, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(0, info.memory_type);

  // Additionally, there should be 3 dmem areas within this test's dmem range.
  result = sceKernelGetDirectMemoryType(dmem_phys_addr, &out_type, &out_start, &out_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0, out_type);
  LONGS_EQUAL(dmem_phys_addr, out_start);
  LONGS_EQUAL(dmem_phys_addr + 0x4000, out_end);
  result = sceKernelGetDirectMemoryType(dmem_phys_addr + 0x4000, &out_type, &out_start, &out_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(3, out_type);
  LONGS_EQUAL(dmem_phys_addr + 0x4000, out_start);
  LONGS_EQUAL(dmem_phys_addr + 0xc000, out_end);
  // physical areas coalesce when type-changing through mtypeprotect.
  result = sceKernelGetDirectMemoryType(dmem_phys_addr + 0xc000, &out_type, &out_start, &out_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0, out_type);
  LONGS_EQUAL(dmem_phys_addr + 0xc000, out_start);
  LONGS_EQUAL(dmem_phys_addr + 0x100000, out_end);

  // Now try mtypeprotect over the whole area with type = 1
  result = sceKernelMtypeprotect(addr, 0x10000, 1, 0x3);
  UNSIGNED_INT_EQUALS(0, result);

  // mem_scan to help visualize
  mem_scan();

  // With this, the areas should all receive the updated type, but won't remerge.
  info   = {};
  result = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr, info.start);
  LONGS_EQUAL(addr + 0x4000, info.end);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(1, info.memory_type);
  info   = {};
  result = sceKernelVirtualQuery(addr + 0x4000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr + 0x4000, info.start);
  LONGS_EQUAL(addr + 0xc000, info.end);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(1, info.memory_type);
  info   = {};
  result = sceKernelVirtualQuery(addr + 0xc000, 0, &info, sizeof(info));
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(addr + 0xc000, info.start);
  LONGS_EQUAL(addr + 0x10000, info.end);
  LONGS_EQUAL(3, info.prot);
  LONGS_EQUAL(1, info.memory_type);

  // All physical areas should coalese.
  result = sceKernelGetDirectMemoryType(dmem_phys_addr, &out_type, &out_start, &out_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(1, out_type);
  LONGS_EQUAL(dmem_phys_addr, out_start);
  LONGS_EQUAL(dmem_phys_addr + 0x10000, out_end);
  result = sceKernelGetDirectMemoryType(dmem_phys_addr + 0x10000, &out_type, &out_start, &out_end);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(0, out_type);
  LONGS_EQUAL(dmem_phys_addr + 0x10000, out_start);
  LONGS_EQUAL(dmem_phys_addr + 0x100000, out_end);

  // Test Mtypeprotect with memory that is not direct.
  // Overwrite the direct memory mapping with flexible memory
  result = sceKernelMapFlexibleMemory(&addr, 0x10000, 0x0, 0x10);
  UNSIGNED_INT_EQUALS(0, result);

  // mtype check occurs first, so this fails.
  result = sceKernelMtypeprotect(addr, 0x10000, 10, 0x33);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // Mtypeprotect fails on memory that isn't direct.
  result = sceKernelMtypeprotect(addr, 0x10000, 0, 0x3);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOTSUP, result);

  // Mtypeprotect fails on memory that isn't direct.
  result = sceKernelMtypeprotect(addr, 0x10000, 10, 0x0);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOTSUP, result);

  result = sceKernelReserveVirtualRange(&addr, 0x10000, 0x10, 0);
  UNSIGNED_INT_EQUALS(0, result);

  // Mtypeprotect on reserved memory fails
  result = sceKernelMtypeprotect(addr, 0x10000, 10, 0x3);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  result = sceKernelMunmap(addr, 0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Mtypeprotect on free memory fails
  result = sceKernelMtypeprotect(addr, 0x10000, 10, 0x3);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // Clean up memory used.
  result = sceKernelMunmap(vmem_start, vmem_size);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelCheckedReleaseDirectMemory(dmem_start, dmem_size);
  UNSIGNED_INT_EQUALS(0, result);
}
