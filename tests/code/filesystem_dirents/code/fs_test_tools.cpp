#include "fs_test.h"

#include <filesystem>
#include <orbis/libkernel.h>
#include <vector>

namespace fs = std::filesystem;
namespace oi = OrbisInternals;

int32_t touch(const char* path) {
  return sceKernelClose(sceKernelOpen(path, O_CREAT | O_WRONLY | O_TRUNC, 0777));
}

int32_t touch(const std::string& path) {
  return sceKernelClose(sceKernelOpen(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0777));
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
    errno = 0;
    if (0 == sceKernelUnlink(pp)) continue;
    errno = 0;
    if (0 == sceKernelRmdir(pp)) continue;
    LogError("Cannot remove [", pp, "] ( errno =", errno, ")");
  }
  if (0 != sceKernelRmdir(path)) LogError("Cannot remove [", path, "] ( errno =", errno, ")");

  LogSuccess(">> rm -rf [", path, "] <<");
  return;
}

void RegenerateDir(const char* path) {
  Obliterate(path);
  sceKernelMkdir(path, 0777);
}
