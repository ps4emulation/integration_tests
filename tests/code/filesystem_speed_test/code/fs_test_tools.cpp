#include "fs_test.h"

#include <fcntl.h>
#include <orbis/UserService.h>
#include <orbis/libkernel.h>
#include <sstream>
#include <string>
#include <vector>

namespace FS_Test {
namespace oi = OrbisInternals;

void RegenerateDir(const char* path) {
  Obliterate(path);
  sceKernelMkdir(path, 0777);
}

off_t GetSize(int fd) {
  OrbisKernelStat st;
  if (int status = sceKernelFstat(fd, &st); status < 0) return status;
  return st.st_size;
}

off_t GetSize(const char* path) {
  OrbisKernelStat st;
  if (int status = sceKernelStat(path, &st); status < 0) return status;
  return st.st_size;
}

int32_t touch(const char* path) {
  return sceKernelClose(sceKernelOpen(path, O_RDWR | O_CREAT | O_TRUNC, 0777));
}

ino_t get_fileno(int fd) {
  struct OrbisKernelStat st {};
  int                    status = sceKernelFstat(fd, &st);
  return (status == 0) * st.st_ino;
}

ino_t get_fileno(const char* path) {
  struct OrbisKernelStat st {};
  int                    fd = sceKernelOpen(path, O_RDONLY, 0777);
  if (fd < 0) return 0;
  int status = sceKernelFstat(fd, &st);
  sceKernelClose(fd);
  return (status == 0) * st.st_ino;
}

int exists(const char* path) {
  struct OrbisKernelStat ost {};
  return sceKernelStat(path, &ost);
}

void Obliterate(const char* path) {
  Log("<< rm -rf [", path, "] >>");
  std::error_code ec {};

  std::vector<std::string> entries;
  for (auto& p: fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied, ec))
    entries.push_back(p.path().string());

  for (auto it = entries.rbegin(); it != entries.rend(); ++it) {
    if (ec) {
      LogError("Exception: [", ec.value(), "] :", ec.message());
      ec.clear();
      continue;
    }

    const char* pp = it->c_str();

    // see what sticks

    struct OrbisKernelStat st {0};

    errno = 0;
    sceKernelStat(pp, &st);
    if (2 == errno)
      // not found, good
      continue;

    errno = 0;
    if (S_ISDIR(st.st_mode)) sceKernelRmdir(pp);
    if (S_ISREG(st.st_mode)) sceKernelUnlink(pp);

    if (errno != 0) LogError("Cannot remove [", pp, "] ( errno =", errno, ")");
  }

  errno = 0;
  sceKernelRmdir(path);

  if (!(ENOENT == errno || 0 == errno)) LogError("Cannot remove [", path, "] ( errno =", errno, ")");

  LogSuccess(">> rm -rf [", path, "] <<");
  return;
}

u64 tick() {
  using namespace std::chrono;
  return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}
} // namespace FS_Test