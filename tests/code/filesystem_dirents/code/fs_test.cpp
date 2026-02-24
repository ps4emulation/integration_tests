#include "fs_test.h"

#include "orbis/UserService.h"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <string>

namespace FS_Test {

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

  DumpDirectory(fd, 16, 0);
  DumpDirectory(fd, 16, 7);
  DumpDirectory(fd, 16, 47);
  DumpDirectory(fd, 16, 123);
  DumpDirectory(fd, 16, 128);
  DumpDirectory(fd, 23, 0);
  DumpDirectory(fd, 23, 7);
  DumpDirectory(fd, 23, 47);
  DumpDirectory(fd, 23, 123);
  DumpDirectory(fd, 23, 128);
  DumpDirectory(fd, 64, 0);
  DumpDirectory(fd, 64, 7);
  DumpDirectory(fd, 64, 47);
  DumpDirectory(fd, 64, 123);
  DumpDirectory(fd, 64, 128);
  DumpDirectory(fd, 123, 0);
  DumpDirectory(fd, 123, 7);
  DumpDirectory(fd, 123, 47);
  DumpDirectory(fd, 123, 123);
  DumpDirectory(fd, 123, 128);
  DumpDirectory(fd, 128, 0);
  DumpDirectory(fd, 128, 7);
  DumpDirectory(fd, 128, 47);
  DumpDirectory(fd, 128, 123);
  DumpDirectory(fd, 128, 128);
  DumpDirectory(fd, 199, 0);
  DumpDirectory(fd, 199, 7);
  DumpDirectory(fd, 199, 47);
  DumpDirectory(fd, 199, 123);
  DumpDirectory(fd, 199, 128);
  DumpDirectory(fd, 256, 0);
  DumpDirectory(fd, 256, 7);
  DumpDirectory(fd, 256, 47);
  DumpDirectory(fd, 256, 123);
  DumpDirectory(fd, 256, 128);
  DumpDirectory(fd, 512, 0);
  DumpDirectory(fd, 512, 7);
  DumpDirectory(fd, 512, 47);
  DumpDirectory(fd, 512, 123);
  DumpDirectory(fd, 512, 128);
  DumpDirectory(fd, 567, 0);
  DumpDirectory(fd, 567, 7);
  DumpDirectory(fd, 567, 47);
  DumpDirectory(fd, 567, 123);
  DumpDirectory(fd, 567, 128);
  DumpDirectory(fd, 999, 0);
  DumpDirectory(fd, 999, 7);
  DumpDirectory(fd, 999, 47);
  DumpDirectory(fd, 999, 123);
  DumpDirectory(fd, 999, 128);
  DumpDirectory(fd, 1024, 0);
  DumpDirectory(fd, 1024, 7);
  DumpDirectory(fd, 1024, 47);
  DumpDirectory(fd, 1024, 123);
  DumpDirectory(fd, 1024, 128);
  DumpDirectory(fd, 1555, 0);
  DumpDirectory(fd, 1555, 7);
  DumpDirectory(fd, 1555, 47);
  DumpDirectory(fd, 1555, 123);
  DumpDirectory(fd, 1555, 128);
  DumpDirectory(fd, 2048, 0);
  DumpDirectory(fd, 2048, 7);
  DumpDirectory(fd, 2048, 47);
  DumpDirectory(fd, 2048, 123);
  DumpDirectory(fd, 2048, 128);
  DumpDirectory(fd, 2123, 0);
  DumpDirectory(fd, 2123, 7);
  DumpDirectory(fd, 2123, 47);
  DumpDirectory(fd, 2123, 123);
  DumpDirectory(fd, 2123, 128);

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

  DumpDirectory(fd, 16, 0, true);
  DumpDirectory(fd, 16, 7, true);
  DumpDirectory(fd, 16, 47, true);
  DumpDirectory(fd, 16, 123, true);
  DumpDirectory(fd, 16, 128, true);
  DumpDirectory(fd, 23, 0, true);
  DumpDirectory(fd, 23, 7, true);
  DumpDirectory(fd, 23, 47, true);
  DumpDirectory(fd, 23, 123, true);
  DumpDirectory(fd, 23, 128, true);
  DumpDirectory(fd, 64, 0, true);
  DumpDirectory(fd, 64, 7, true);
  DumpDirectory(fd, 64, 47, true);
  DumpDirectory(fd, 64, 123, true);
  DumpDirectory(fd, 64, 128, true);
  DumpDirectory(fd, 123, 0, true);
  DumpDirectory(fd, 123, 7, true);
  DumpDirectory(fd, 123, 47, true);
  DumpDirectory(fd, 123, 123, true);
  DumpDirectory(fd, 123, 128, true);
  DumpDirectory(fd, 128, 0, true);
  DumpDirectory(fd, 128, 7, true);
  DumpDirectory(fd, 128, 47, true);
  DumpDirectory(fd, 128, 123, true);
  DumpDirectory(fd, 128, 128, true);
  DumpDirectory(fd, 199, 0, true);
  DumpDirectory(fd, 199, 7, true);
  DumpDirectory(fd, 199, 47, true);
  DumpDirectory(fd, 199, 123, true);
  DumpDirectory(fd, 199, 128, true);
  DumpDirectory(fd, 256, 0, true);
  DumpDirectory(fd, 256, 7, true);
  DumpDirectory(fd, 256, 47, true);
  DumpDirectory(fd, 256, 123, true);
  DumpDirectory(fd, 256, 128, true);
  DumpDirectory(fd, 512, 0, true);
  DumpDirectory(fd, 512, 7, true);
  DumpDirectory(fd, 512, 47, true);
  DumpDirectory(fd, 512, 123, true);
  DumpDirectory(fd, 512, 128, true);
  DumpDirectory(fd, 567, 0, true);
  DumpDirectory(fd, 567, 7, true);
  DumpDirectory(fd, 567, 47, true);
  DumpDirectory(fd, 567, 123, true);
  DumpDirectory(fd, 567, 128, true);
  DumpDirectory(fd, 999, 0, true);
  DumpDirectory(fd, 999, 7, true);
  DumpDirectory(fd, 999, 47, true);
  DumpDirectory(fd, 999, 123, true);
  DumpDirectory(fd, 999, 128, true);
  DumpDirectory(fd, 1024, 0, true);
  DumpDirectory(fd, 1024, 7, true);
  DumpDirectory(fd, 1024, 47, true);
  DumpDirectory(fd, 1024, 123, true);
  DumpDirectory(fd, 1024, 128, true);
  DumpDirectory(fd, 65536, 0, true);
  DumpDirectory(fd, 65536, 7, true);
  DumpDirectory(fd, 65536, 47, true);
  DumpDirectory(fd, 65536, 123, true);
  DumpDirectory(fd, 65536, 128, true);

  sceKernelClose(fd);
}

bool DumpByRead(int dir_fd, int dump_fd, char* buffer, size_t size) {
  memset(buffer, 0xAA, size);

  s64 tbr = sceKernelRead(dir_fd, buffer, size);
  Log("Read got", tbr, "/", size, "bytes, ptr =", sceKernelLseek(dir_fd, 0, 1));

  if (tbr < 0) {
    LogError("Read finished with error:", tbr);
    return false;
  }
  if (tbr == 0) {
    LogSuccess("Read finished");
    return false;
  }

  s64 tbw = sceKernelWrite(dump_fd, buffer, size);
  if (tbw != tbr) LogError("Written", tbw, "bytes out of", size, "bytes read");
  return true;
}

bool DumpByDirent(int dir_fd, int dump_fd, char* buffer, size_t size, s64* idx) {
  // magic to determine how many trailing elements were cut
  memset(buffer, 0xAA, size);

  s64 tbr = sceKernelGetdirentries(dir_fd, buffer, size, idx);
  Log("Dirent got", tbr, "/", size, "bytes, ptr =", sceKernelLseek(dir_fd, 0, 1), "idx =", *idx);

  if (tbr < 0) {
    LogError("Dirent finished with error:", tbr);
    return false;
  }
  if (tbr == 0) {
    LogSuccess("Dirent finished");
    return false;
  }

  s64 tbw = sceKernelWrite(dump_fd, buffer, size);
  if (tbw != tbr) LogError("Written", tbw, "bytes out of", size, "bytes read");
  return true;
}

void DumpDirectory(int fd, int buffer_size, s64 offset, bool is_pfs) {
  char* buffer = new char[buffer_size] {0};

  fs::path read_path =
      "/data/enderman/dumps/read_" + (is_pfs ? std::string("PFS_") : std::string("")) + std::to_string(buffer_size) + '+' + std::to_string(offset) + ".bin";
  fs::path dirent_path =
      "/data/enderman/dumps/dirent_" + (is_pfs ? std::string("PFS_") : std::string("")) + std::to_string(buffer_size) + '+' + std::to_string(offset) + ".bin";

  LogTest("Read", is_pfs ? "PFS" : "normal", "directory, fd =", fd, "size =", buffer_size, "offset =", offset);

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