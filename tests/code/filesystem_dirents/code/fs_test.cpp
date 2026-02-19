#include "fs_test.h"

#include "orbis/UserService.h"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

namespace FS_Test {
namespace oi = OrbisInternals;

void Drop(char* buffer, size_t size) {
  std::stringstream out;
  for (int b = 1; b <= size; b++) {
    out << std::setw(2) << std::hex << (0xFF & static_cast<unsigned int>(buffer[b - 1])) << " ";
    if ((b % 64) == 0) {
      out.flush();
      Log(out.str());
      std::stringstream().swap(out);
    }
  }
  Log(out.str(), "\n");
}

bool DropRead(int dir_fd, int dump_fd, char* buffer, size_t size) {
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

  s64 tbw = sceKernelWrite(dump_fd, buffer, tbr);
  if (tbw != tbr) LogError("Written", tbw, "bytes out of", tbr, "bytes read");
  return true;
}

bool DropDirents(int dir_fd, int dump_fd, char* buffer, size_t size, s64* idx) {
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

  s64 tbw = sceKernelWrite(dump_fd, buffer, tbr);
  if (tbw != tbr) LogError("Written", tbw, "bytes out of", tbr, "bytes read");
  return true;
}

void DumpDirents(int fd, int buffer_size, s64 offset, bool is_pfs = false) {
  char* buffer = new char[buffer_size] {0};

  fs::path read_path =
      "/data/enderman/dumps/read_" + (is_pfs ? std::string("PFS_") : std::string("")) + std::to_string(buffer_size) + '+' + std::to_string(offset) + ".bin";
  fs::path dirent_path =
      "/data/enderman/dumps/dirent_" + (is_pfs ? std::string("PFS_") : std::string("")) + std::to_string(buffer_size) + '+' + std::to_string(offset) + ".bin";

  int read_fd   = sceKernelOpen(read_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  int dirent_fd = sceKernelOpen(dirent_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);

  LogTest("Read", is_pfs ? "PFS" : "normal", "directory, fd =", fd, "size =", buffer_size, "offset =", offset);
  u16 max_loops = 0; // 65536 iterations lmao
  if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp < 0) LogError("Lseek failed:", _tmp);
  while (--max_loops && DropRead(fd, read_fd, buffer, buffer_size))
    ;
  if (0 == max_loops) LogError("Aborted after 255 loops");

  s64 idx   = 0;
  max_loops = 0;
  if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp < 0) LogError("Lseek failed:", _tmp);
  while (--max_loops && DropDirents(fd, dirent_fd, buffer, buffer_size, &idx))
    ;
  if (0 == max_loops) LogError("Aborted after 255 loops");

  sceKernelClose(read_fd);
  sceKernelClose(dirent_fd);
}

void RunTests() {
  RegenerateDir("/data/enderman");
  sceKernelMkdir("/data/enderman/dumps", 0777);
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname01", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname02", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname03", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname04", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname05", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname06", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname07", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname08", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname09", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname10", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname11", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname12", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname13", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname14", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname15", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname16", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname17", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname18", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname19", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname10", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname21", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname22", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname23", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname24", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname25", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname26", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname27", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname28", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname29", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname30", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname31", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname32", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname33", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname34", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname35", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname36", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname37", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname38", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname39", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname40", O_CREAT | O_WRONLY | O_TRUNC, 0777));

#define FUCKING_LSEEK_OFFSET 1024

  Log("---------------------");
  Log("Dump normal directory");
  Log("---------------------");

  int fd = sceKernelOpen("/data/enderman", O_DIRECTORY | O_RDONLY, 0777);
  Log("Directory opened with fd=", fd);

  Log("LSeek END+0=", sceKernelLseek(fd, 0, 2));
  Log("LSeek END+123456=", sceKernelLseek(fd, 123456, 2));
  Log("LSeek END+100=", sceKernelLseek(fd, 100, 2));
  Log("LSeek END-100000=", sceKernelLseek(fd, -100000, 2));

  DumpDirents(fd, 512, 0);
  DumpDirents(fd, 512, 5);
  DumpDirents(fd, 512, 30);
  DumpDirents(fd, 768, 0);
  DumpDirents(fd, 768, 5);
  DumpDirents(fd, 768, 30);
  DumpDirents(fd, 1024, 0);
  DumpDirents(fd, 1024, 5);
  DumpDirents(fd, 1024, 30);

  sceKernelClose(fd);

  Log("------------------");
  Log("Dump PFS directory");
  Log("------------------");
  fd = sceKernelOpen("/app0/assets/misc", O_DIRECTORY | O_RDONLY, 0777);

  Log("Directory opened with fd=", fd);

  Log("LSeek END+0=", sceKernelLseek(fd, 0, 2));
  Log("LSeek END+123456=", sceKernelLseek(fd, 123456, 2));
  Log("LSeek END+100=", sceKernelLseek(fd, 100, 2));
  Log("LSeek END-100000=", sceKernelLseek(fd, -100000, 2));

  DumpDirents(fd, 16, 0, true);
  DumpDirents(fd, 16, 5, true);
  DumpDirents(fd, 16, 30, true);
  DumpDirents(fd, 64, 0, true);
  DumpDirents(fd, 64, 5, true);
  DumpDirents(fd, 64, 30, true);
  DumpDirents(fd, 128, 0, true);
  DumpDirents(fd, 128, 5, true);
  DumpDirents(fd, 128, 33, true);
  DumpDirents(fd, 256, 0, true);
  DumpDirents(fd, 256, 5, true);
  DumpDirents(fd, 256, 30, true);
  DumpDirents(fd, 65536, 0, true);
  DumpDirents(fd, 65536, 5, true);
  DumpDirents(fd, 65536, 30, true);

  sceKernelClose(fd);
}

bool RegenerateDir(const char* path) {
  Obliterate(path);
  sceKernelMkdir(path, 0777);
  return true;
}

} // namespace FS_Test