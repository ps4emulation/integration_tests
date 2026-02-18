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
  if (0 == max_loops) LogError("Aborted");

  s64 idx   = 0;
  max_loops = 0;
  if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp < 0) LogError("Lseek failed:", _tmp);
  while (--max_loops && DropDirents(fd, dirent_fd, buffer, buffer_size, &idx))
    ;
  if (0 == max_loops) LogError("Aborted");

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
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname41", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname42", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname43", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname44", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname45", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname46", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname47", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname48", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname49", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname50", O_CREAT | O_WRONLY | O_TRUNC, 0777));

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

  DumpDirents(fd, 16, 0);
  DumpDirents(fd, 16, 7);
  DumpDirents(fd, 16, 47);
  DumpDirents(fd, 16, 123);
  DumpDirents(fd, 16, 128);
  DumpDirents(fd, 23, 0);
  DumpDirents(fd, 23, 7);
  DumpDirents(fd, 23, 47);
  DumpDirents(fd, 23, 123);
  DumpDirents(fd, 23, 128);
  DumpDirents(fd, 64, 0);
  DumpDirents(fd, 64, 7);
  DumpDirents(fd, 64, 47);
  DumpDirents(fd, 64, 123);
  DumpDirents(fd, 64, 128);
  DumpDirents(fd, 123, 0);
  DumpDirents(fd, 123, 7);
  DumpDirents(fd, 123, 47);
  DumpDirents(fd, 123, 123);
  DumpDirents(fd, 123, 128);
  DumpDirents(fd, 128, 0);
  DumpDirents(fd, 128, 7);
  DumpDirents(fd, 128, 47);
  DumpDirents(fd, 128, 123);
  DumpDirents(fd, 128, 128);
  DumpDirents(fd, 199, 0);
  DumpDirents(fd, 199, 7);
  DumpDirents(fd, 199, 47);
  DumpDirents(fd, 199, 123);
  DumpDirents(fd, 199, 128);
  DumpDirents(fd, 256, 0);
  DumpDirents(fd, 256, 7);
  DumpDirents(fd, 256, 47);
  DumpDirents(fd, 256, 123);
  DumpDirents(fd, 256, 128);
  DumpDirents(fd, 512, 0);
  DumpDirents(fd, 512, 7);
  DumpDirents(fd, 512, 47);
  DumpDirents(fd, 512, 123);
  DumpDirents(fd, 512, 128);
  DumpDirents(fd, 567, 0);
  DumpDirents(fd, 567, 7);
  DumpDirents(fd, 567, 47);
  DumpDirents(fd, 567, 123);
  DumpDirents(fd, 567, 128);
  DumpDirents(fd, 999, 0);
  DumpDirents(fd, 999, 7);
  DumpDirents(fd, 999, 47);
  DumpDirents(fd, 999, 123);
  DumpDirents(fd, 999, 128);
  DumpDirents(fd, 1024, 0);
  DumpDirents(fd, 1024, 7);
  DumpDirents(fd, 1024, 47);
  DumpDirents(fd, 1024, 123);
  DumpDirents(fd, 1024, 128);
  DumpDirents(fd, 1555, 0);
  DumpDirents(fd, 1555, 7);
  DumpDirents(fd, 1555, 47);
  DumpDirents(fd, 1555, 123);
  DumpDirents(fd, 1555, 128);
  DumpDirents(fd, 2048, 0);
  DumpDirents(fd, 2048, 7);
  DumpDirents(fd, 2048, 47);
  DumpDirents(fd, 2048, 123);
  DumpDirents(fd, 2048, 128);
  DumpDirents(fd, 2123, 0);
  DumpDirents(fd, 2123, 7);
  DumpDirents(fd, 2123, 47);
  DumpDirents(fd, 2123, 123);
  DumpDirents(fd, 2123, 128);

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

  DumpDirents(fd, 16, 0, true);
  DumpDirents(fd, 16, 7, true);
  DumpDirents(fd, 16, 47, true);
  DumpDirents(fd, 16, 123, true);
  DumpDirents(fd, 16, 128, true);
  DumpDirents(fd, 23, 0, true);
  DumpDirents(fd, 23, 7, true);
  DumpDirents(fd, 23, 47, true);
  DumpDirents(fd, 23, 123, true);
  DumpDirents(fd, 23, 128, true);
  DumpDirents(fd, 64, 0, true);
  DumpDirents(fd, 64, 7, true);
  DumpDirents(fd, 64, 47, true);
  DumpDirents(fd, 64, 123, true);
  DumpDirents(fd, 64, 128, true);
  DumpDirents(fd, 123, 0, true);
  DumpDirents(fd, 123, 7, true);
  DumpDirents(fd, 123, 47, true);
  DumpDirents(fd, 123, 123, true);
  DumpDirents(fd, 123, 128, true);
  DumpDirents(fd, 128, 0, true);
  DumpDirents(fd, 128, 7, true);
  DumpDirents(fd, 128, 47, true);
  DumpDirents(fd, 128, 123, true);
  DumpDirents(fd, 128, 128, true);
  DumpDirents(fd, 199, 0, true);
  DumpDirents(fd, 199, 7, true);
  DumpDirents(fd, 199, 47, true);
  DumpDirents(fd, 199, 123, true);
  DumpDirents(fd, 199, 128, true);
  DumpDirents(fd, 256, 0, true);
  DumpDirents(fd, 256, 7, true);
  DumpDirents(fd, 256, 47, true);
  DumpDirents(fd, 256, 123, true);
  DumpDirents(fd, 256, 128, true);
  DumpDirents(fd, 512, 0, true);
  DumpDirents(fd, 512, 7, true);
  DumpDirents(fd, 512, 47, true);
  DumpDirents(fd, 512, 123, true);
  DumpDirents(fd, 512, 128, true);
  DumpDirents(fd, 567, 0, true);
  DumpDirents(fd, 567, 7, true);
  DumpDirents(fd, 567, 47, true);
  DumpDirents(fd, 567, 123, true);
  DumpDirents(fd, 567, 128, true);
  DumpDirents(fd, 999, 0, true);
  DumpDirents(fd, 999, 7, true);
  DumpDirents(fd, 999, 47, true);
  DumpDirents(fd, 999, 123, true);
  DumpDirents(fd, 999, 128, true);
  DumpDirents(fd, 1024, 0, true);
  DumpDirents(fd, 1024, 7, true);
  DumpDirents(fd, 1024, 47, true);
  DumpDirents(fd, 1024, 123, true);
  DumpDirents(fd, 1024, 128, true);
  DumpDirents(fd, 65536, 0, true);
  DumpDirents(fd, 65536, 7, true);
  DumpDirents(fd, 65536, 47, true);
  DumpDirents(fd, 65536, 123, true);
  DumpDirents(fd, 65536, 128, true);

  sceKernelClose(fd);
}

bool RegenerateDir(const char* path) {
  Obliterate(path);
  sceKernelMkdir(path, 0777);
  return true;
}

} // namespace FS_Test