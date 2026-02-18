#ifndef FS_TEST_H
#define FS_TEST_H

#include "log.h"

#include <filesystem>
#include <orbis/AppContent.h>
#include <orbis/Sysmodule.h>
#include <orbis/libkernel.h>
#include <sys/stat.h>

namespace fs = std::filesystem;

namespace FS_Test {
#define DIRENT_BUFFER_SIZE 512

using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

namespace OrbisInternals {
enum class OpenFlags : s32 {
  ReadOnly  = 0x0,
  WriteOnly = 0x1,
  ReadWrite = 0x2,
  NonBlock  = 0x4,
  Append    = 0x8,
  Fsync     = 0x80,
  Sync      = 0x80,
  Create    = 0x200,
  Truncate  = 0x400,
  Excl      = 0x800,
  Dsync     = 0x1000,
  Direct    = 0x10000,
  Directory = 0x20000,
};

enum class SeekWhence : s32 {
  SeekSet = 0,
  SeekCur = 1,
  SeekEnd = 2,
  // The following two are unsupported on Orbis, with unique error behavior when used.
  SeekHole = 3,
  SeekData = 4,
};

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

bool RegenerateDir(const char* path);

bool StatCmp(const OrbisKernelStat* lhs, const OrbisKernelStat* rhs);

u16 dumpDirRecursive(fs::path path, int depth = 0);
u16 dumpDir(int fd);

void PrintStatInfo(const struct stat* info);
void PrintStatInfo(const OrbisKernelStat* info);
s8   GetDir(fs::path path, fs::path leaf, OrbisInternals::FolderDirent* dirent);
s8   GetDir(fs::path path, fs::path leaf, OrbisInternals::PfsDirent* dirent);

std::string file_mode(OrbisKernelMode mode);
void        Obliterate(const char* path);
void        ElEsDashElAy(const char* path);
int32_t     touch(const char* path);
off_t       GetSize(const char* path);
off_t       GetSize(int fd);

} // namespace FS_Test
#endif // FS_TEST_H
