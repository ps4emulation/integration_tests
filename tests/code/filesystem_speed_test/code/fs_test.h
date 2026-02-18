#ifndef FS_TEST_H
#define FS_TEST_H

#include "log.h"

#include <chrono>
#include <filesystem>
#include <orbis/AppContent.h>
#include <orbis/Sysmodule.h>
#include <orbis/libkernel.h>
#include <sys/stat.h>

namespace fs = std::filesystem;

namespace FS_Test {
#define DIRENT_PFS_BUFFER_SIZE 65536
#define DIRENT_BUFFER_SIZE     512

using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

namespace OrbisInternals {
typedef struct PfsDirent {
  s32  d_fileno;
  s32  d_type;
  s32  d_namlen;
  s32  d_reclen;
  char d_name[256];
} PfsDirent;

typedef struct FolderDirent {
  u32  d_fileno;
  u16  d_reclen;
  u8   d_type;
  u8   d_namlen;
  char d_name[256];
} FolderDirent;
} // namespace OrbisInternals

void RunTests(void);
void RegenerateDir(const char* path);

ino_t   get_fileno(const char* path);
ino_t   get_fileno(int fd);
void    Obliterate(const char* path);
int32_t touch(const char* path);
off_t   GetSize(const char* path);
off_t   GetSize(int fd);
int     exists(const char* path);

u64 tick();

} // namespace FS_Test
#endif // FS_TEST_H
