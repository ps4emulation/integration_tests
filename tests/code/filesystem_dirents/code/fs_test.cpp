#include "fs_test.h"

#include "orbis/UserService.h"

#include <CppUTest/TestHarness.h>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

std::vector<u32> read_sizes {// 8
                             7, 8, 9,
                             // 16
                             15, 16, 17,
                             // 32
                             31, 32, 33,
                             // 64
                             63, 64, 65,
                             // 128
                             127, 128, 129,
                             // 256
                             255, 256, 257,
                             // 512
                             511, 512, 513,
                             // 1024
                             1023, 1024, 1025,
                             // 2048
                             2047, 2048, 2049,
                             // 4096
                             4095, 4096, 4097,
                             // 65536
                             65535, 65536, 65537,
                             // cursed
                             2137, 21, 37, 69, 420, 42, 123, 222, 666, 911, 112, 997,
                             // something for zoomers
                             67};
std::vector<u16> read_offsets {0, 1, 5, 10, 21, 37, 127, 128, 129, 400, 500, 512, 768, 1024, 111, 666, 420, 1234, 96, 42};

namespace fs = std::filesystem;
namespace oi = OrbisInternals;

s64  DumpByRead(int dir_fd, int dump_fd, char* buffer, size_t size);
s64  DumpByDirent(int dir_fd, int dump_fd, char* buffer, size_t size, s64* idx);
void DumpDirectory(int fd, int buffer_size, s64 offset, bool is_pfs = false);

TEST_GROUP (DirentTests) {
  void setup() {}
  void teardown() {}
};

TEST(DirentTests, LseekRegularTests) {
  int fd = sceKernelOpen("/data/enderman", O_DIRECTORY | O_RDONLY, 0777);
  CHECK_COMPARE_TEXT(fd, >, 0, "Unable to open /data/enderman");

  int status;

  errno  = 0;
  status = sceKernelLseek(fd, 0, 0);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(0, status, "START+0");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -123, 0);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "START-123");
  UNSIGNED_INT_EQUALS(EINVAL, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 123456, 0);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(123456, status, "START+123456");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 60, 0);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(60, status, "START+60");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 0, 1);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(60, status, "CUR+0");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 24, 1);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(84, status, "CUR+24");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -24, 1);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(60, status, "CUR-24");
  UNSIGNED_INT_EQUALS(EINVAL, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -6666, 1);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "CUR-6666");
  UNSIGNED_INT_EQUALS(EINVAL, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 123456, 1);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(123516, status, "CUR+123456");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 0, 2);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(2048, status, "END+0");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 123456, 2);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(125504, status, "END+123456");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 100, 2);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(2148, status, "END+100");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -100, 2);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(1948, status, "END-100");
  UNSIGNED_INT_EQUALS(EINVAL, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -100000, 2);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "END-100000");
  UNSIGNED_INT_EQUALS(EINVAL, errno);

  sceKernelClose(fd);
}

TEST(DirentTests, LseekPFSTests) {
  int fd = sceKernelOpen("/app0/assets/misc", O_DIRECTORY | O_RDONLY, 0777);
  CHECK_COMPARE_TEXT(fd, >, 0, "Unable to open /app0/assets/misc");

  s64 status;

  errno  = 0;
  status = sceKernelLseek(fd, 0, 0);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(0, status, "START+0");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -123, 0);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "START-123");
  UNSIGNED_INT_EQUALS(EINVAL, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 123456, 0);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(123456, status, "START+123456");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 60, 0);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(60, status, "START+60");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 0, 1);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(60, status, "CUR+0");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 24, 1);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(84, status, "CUR+24");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -24, 1);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(60, status, "CUR-24");
  UNSIGNED_INT_EQUALS(EINVAL, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -6666, 1);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "CUR-6666");
  UNSIGNED_INT_EQUALS(EINVAL, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 123456, 1);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(123516, status, "CUR+123456");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 0, 2);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(2048, status, "END+0");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 123456, 2);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(125504, status, "END+123456");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 100, 2);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(2148, status, "END+100");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -100, 2);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(1948, status, "END-100");
  UNSIGNED_INT_EQUALS(EINVAL, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -100000, 2);
  UNSIGNED_LONGLONGS_EQUAL_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "END-100000");
  UNSIGNED_INT_EQUALS(EINVAL, errno);

  sceKernelClose(fd);
}

void RunTests() {
  std::string nf_path        = "/data/enderman/filewithaverylongname";
  std::string nf_path_longer = "/data/enderman/filewithunnecesarilylongnamejusttomesswitheveryone";
  char        nf_num[4] {0};
  for (u8 idx = 1; idx <= 50; idx++) {
    snprintf(nf_num, 4, "%02d", idx);
    touch(nf_path + std::string(nf_num));
    touch(nf_path_longer + std::string(nf_num));
  }

  Log("---------------------");
  Log("Dump normal directory");
  Log("---------------------");

  int fd = sceKernelOpen("/data/enderman", O_DIRECTORY | O_RDONLY, 0777);
  for (auto read_size: read_sizes) {
    for (auto read_offset: read_offsets) {
      DumpDirectory(fd, read_size, read_offset);
    }
  }
  sceKernelClose(fd);

  Log("------------------");
  Log("Dump PFS directory");
  Log("------------------");
  fd = sceKernelOpen("/app0/assets/misc", O_DIRECTORY | O_RDONLY, 0777);
  for (auto read_size: read_sizes) {
    for (auto read_offset: read_offsets) {
      DumpDirectory(fd, read_size, read_offset, true);
    }
  }
  sceKernelClose(fd);
}

s64 DumpByRead(int dir_fd, int dump_fd, char* buffer, size_t size) {
  memset(buffer, 0xAA, size);

  s64 tbr = sceKernelRead(dir_fd, buffer, size);
  // Log("Read got", tbr, "/", size, "bytes, ptr =", sceKernelLseek(dir_fd, 0, 1));

  if (tbr < 0) {
    LogError("Read finished with error:", tbr);
    return 0;
  }
  if (tbr == 0) {
    return 0;
  }

  if (s64 tbw = sceKernelWrite(dump_fd, buffer, tbr); tbw != tbr) LogError("Written", tbw, "bytes out of", tbr, "bytes");
  return tbr;
}

s64 DumpByDirent(int dir_fd, int dump_fd, char* buffer, size_t size, s64* idx) {
  // magic to determine how many trailing elements were cut
  memset(buffer, 0xAA, size);

  s64 tbr = sceKernelGetdirentries(dir_fd, buffer, size, idx);
  // Log("Dirent got", tbr, "/", size, "bytes, ptr =", sceKernelLseek(dir_fd, 0, 1), "idx =", *idx);

  if (tbr < 0) {
    LogError("Dirent finished with error:", tbr);
    return 0;
  }
  if (tbr == 0) {
    return 0;
  }

  if (s64 tbw = sceKernelWrite(dump_fd, buffer, tbr); tbw != tbr) LogError("Written", tbw, "bytes out of", tbr, "bytes");
  return tbr;
}

void DumpDirectory(int fd, int buffer_size, s64 offset, bool is_pfs) {
  char* buffer = new char[buffer_size] {0};

  std::string file_basename =  (is_pfs ? std::string("PFS_") : std::string("")) + std::to_string(buffer_size) + '+' + std::to_string(offset);

  fs::path read_path =
      "/data/enderman/dumps/read_" +file_basename + ".bin";
  fs::path dirent_path =
      "/data/enderman/dumps/dirent_" + file_basename + ".bin";
  fs::path read_cue_path =
      "/data/enderman/dumps/read_" + file_basename+ ".cue";
  fs::path dirent_cue_path =
      "/data/enderman/dumps/dirent_" + file_basename + ".cue";

  LogTest(is_pfs ? "PFS" : "normal", "directory, fd =", fd, "buffer size =", buffer_size, "starting offset =", offset);

  s64 tbr         = 0;
  u16 max_loops   = 0; // 65536 iterations lmao
  int fd_read     = sceKernelOpen(read_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  int fd_read_cue = sceKernelOpen(read_cue_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp != offset) LogError("Lseek failed:", _tmp);
  while (--max_loops) {
    tbr = DumpByRead(fd, fd_read, buffer, buffer_size);
    sceKernelWrite(fd_read_cue, reinterpret_cast<u8*>(&tbr), sizeof(s64) / sizeof(u8));
    if (tbr <= 0) break;
  }
  if (0 == max_loops) LogError("Aborted");
  sceKernelClose(fd_read);
  sceKernelClose(fd_read_cue);

  s64 idx           = 0;
  max_loops         = 0;
  int fd_dirent     = sceKernelOpen(dirent_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  int fd_dirent_cue = sceKernelOpen(dirent_cue_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp != offset) LogError("Lseek failed:", _tmp);
  while (--max_loops) {
    tbr = DumpByDirent(fd, fd_dirent, buffer, buffer_size, &idx);
    sceKernelWrite(fd_dirent_cue, reinterpret_cast<u8*>(&tbr), sizeof(s64) / sizeof(u8));
    sceKernelWrite(fd_dirent_cue, reinterpret_cast<u8*>(&idx), sizeof(s64) / sizeof(u8));
    if (tbr <= 0) break;
  }
  if (0 == max_loops) LogError("Aborted");

  sceKernelClose(fd_dirent);
  sceKernelClose(fd_dirent_cue);

  delete[] buffer;
}
