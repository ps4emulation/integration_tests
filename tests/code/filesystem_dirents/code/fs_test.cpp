#include "fs_test.h"

#include "orbis/UserService.h"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace FS_Test {

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
std::vector<u32> read_sizes_pfs {65535, 65536, 65537};
std::vector<u16> read_offsets {0, 1, 5, 10, 21, 37, 127, 128, 129, 400, 500, 512, 768, 1024, 111, 666, 420, 1234, 96, 42};

namespace fs = std::filesystem;
namespace oi = OrbisInternals;

bool DumpByRead(int dir_fd, int dump_fd, char* buffer, size_t size);
bool DumpByDirent(int dir_fd, int dump_fd, char* buffer, size_t size, s64* idx);
void DumpDirectory(int fd, int buffer_size, s64 offset, bool is_pfs = false);

void RunTests() {
  RegenerateDir("/data/enderman");
  sceKernelMkdir("/data/enderman/dumps", 0777);
  std::string nf_path = "/data/enderman/filewithaverylongname";
  char        nf_num[4] {0};
  for (u8 idx = 1; idx <= 50; idx++) {
    snprintf(nf_num, 4, "%02d", idx);
    touch(nf_path + std::string(nf_num));
  }

  Log("---------------------");
  Log("Dump normal directory");
  Log("---------------------");

  int fd = sceKernelOpen("/data/enderman", O_DIRECTORY | O_RDONLY, 0777);
  Log("Directory opened with fd=", fd);

  Log("LSeek START+0=", sceKernelLseek(fd, 0, 0));
  Log("LSeek START-123=", sceKernelLseek(fd, -123, 0));
  Log("LSeek START+123456=", sceKernelLseek(fd, 123456, 0));
  Log("LSeek START+60=", sceKernelLseek(fd, 60, 0));
  Log("LSeek CUR+0=", sceKernelLseek(fd, 0, 1));
  Log("LSeek CUR+24=", sceKernelLseek(fd, 24, 1));
  Log("LSeek CUR-24=", sceKernelLseek(fd, -24, 1));
  Log("LSeek CUR-6666=", sceKernelLseek(fd, -6666, 1));
  Log("LSeek CUR+123456=", sceKernelLseek(fd, 123456, 1));
  Log("LSeek END+0=", sceKernelLseek(fd, 0, 2));
  Log("LSeek END+123456=", sceKernelLseek(fd, 123456, 2));
  Log("LSeek END+100=", sceKernelLseek(fd, 100, 2));
  Log("LSeek END-100=", sceKernelLseek(fd, -100, 2));
  Log("LSeek END-100000=", sceKernelLseek(fd, -100000, 2));

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

  Log("Directory opened with fd=", fd);

  Log("LSeek START+0=", sceKernelLseek(fd, 0, 0));
  Log("LSeek START-123=", sceKernelLseek(fd, -123, 0));
  Log("LSeek START+123456=", sceKernelLseek(fd, 123456, 0));
  Log("LSeek START+60=", sceKernelLseek(fd, 60, 0));
  Log("LSeek CUR+0=", sceKernelLseek(fd, 0, 1));
  Log("LSeek CUR+24=", sceKernelLseek(fd, 24, 1));
  Log("LSeek CUR-24=", sceKernelLseek(fd, -24, 1));
  Log("LSeek CUR-6666=", sceKernelLseek(fd, -6666, 1));
  Log("LSeek CUR+123456=", sceKernelLseek(fd, 123456, 1));
  Log("LSeek END+0=", sceKernelLseek(fd, 0, 2));
  Log("LSeek END+123456=", sceKernelLseek(fd, 123456, 2));
  Log("LSeek END+100=", sceKernelLseek(fd, 100, 2));
  Log("LSeek END-100=", sceKernelLseek(fd, -100, 2));
  Log("LSeek END-100000=", sceKernelLseek(fd, -100000, 2));

  for (auto read_size: read_sizes) {
    for (auto read_offset: read_offsets) {
      DumpDirectory(fd, read_size, read_offset, true);
    }
  }

  sceKernelClose(fd);
}

bool DumpByRead(int dir_fd, int dump_fd, char* buffer, size_t size) {
  memset(buffer, 0xAA, size);

  s64 tbr = sceKernelRead(dir_fd, buffer, size);
  // Log("Read got", tbr, "/", size, "bytes, ptr =", sceKernelLseek(dir_fd, 0, 1));

  if (tbr < 0) {
    LogError("Read finished with error:", tbr);
    return false;
  }
  if (tbr == 0) {
    LogSuccess("Read finished");
    return false;
  }

  if (s64 tbw = sceKernelWrite(dump_fd, buffer, size); tbw != size) LogError("Written", tbw, "bytes out of", size, "bytes");
  return true;
}

bool DumpByDirent(int dir_fd, int dump_fd, char* buffer, size_t size, s64* idx) {
  // magic to determine how many trailing elements were cut
  memset(buffer, 0xAA, size);

  s64 tbr = sceKernelGetdirentries(dir_fd, buffer, size, idx);
  // Log("Dirent got", tbr, "/", size, "bytes, ptr =", sceKernelLseek(dir_fd, 0, 1), "idx =", *idx);

  if (tbr < 0) {
    LogError("Dirent finished with error:", tbr);
    return false;
  }
  if (tbr == 0) {
    LogSuccess("Dirent finished");
    return false;
  }

  if (s64 tbw = sceKernelWrite(dump_fd, buffer, size); tbw != size) LogError("Written", tbw, "bytes out of", size, "bytes");
  return true;
}

void DumpDirectory(int fd, int buffer_size, s64 offset, bool is_pfs) {
  char* buffer = new char[buffer_size] {0};

  fs::path read_path =
      "/data/enderman/dumps/read_" + (is_pfs ? std::string("PFS_") : std::string("")) + std::to_string(buffer_size) + '+' + std::to_string(offset) + ".bin";
  fs::path dirent_path =
      "/data/enderman/dumps/dirent_" + (is_pfs ? std::string("PFS_") : std::string("")) + std::to_string(buffer_size) + '+' + std::to_string(offset) + ".bin";

  LogTest("Read", is_pfs ? "PFS" : "normal", "directory, fd =", fd, "buffer size =", buffer_size, "starting offset =", offset);

  u16 max_loops = 0; // 65536 iterations lmao
  int read_fd   = sceKernelOpen(read_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp != offset) LogError("Lseek failed:", _tmp);
  while (--max_loops && DumpByRead(fd, read_fd, buffer, buffer_size))
    ;
  if (0 == max_loops) LogError("Aborted");
  sceKernelClose(read_fd);

  s64 idx       = 0;
  max_loops     = 0;
  int dirent_fd = sceKernelOpen(dirent_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp != offset) LogError("Lseek failed:", _tmp);
  while (--max_loops && DumpByDirent(fd, dirent_fd, buffer, buffer_size, &idx))
    ;
  if (0 == max_loops) LogError("Aborted");

  sceKernelClose(dirent_fd);
  delete[] buffer;
}

} // namespace FS_Test