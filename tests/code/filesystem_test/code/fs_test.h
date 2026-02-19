#ifndef FS_TEST_H
#define FS_TEST_H

#define UNSIGNED_INT_EQUALS(expected, actual) UNSIGNED_LONGS_EQUAL_LOCATION((uint32_t)expected, (uint32_t)actual, NULLPTR, __FILE__, __LINE__)
#define UNSIGNED_INT_EQUALS_TEXT(expected, actual, text) UNSIGNED_LONGS_EQUAL_LOCATION((uint32_t)expected, (uint32_t)actual, text, __FILE__, __LINE__)

#include "log.h"

#include <filesystem>
#include <orbis/AppContent.h>
#include <orbis/Sysmodule.h>
#include <orbis/libkernel.h>
#include <sys/stat.h>

namespace fs = std::filesystem;

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
s32  TestMovements(const char* from, const char* to);
bool TestRelatives(fs::path path, bool expected_mountpoint = false);
bool CompareNormalVsPFS(fs::path path, fs::path leaf, s32 expected_normal_reclen = -1, s32 expected_pfs_reclen = -1);
bool RegenerateDir(const char* path);
bool TestDirEnts(void);
bool TestStat(fs::path path, const OrbisKernelStat* original = nullptr);
bool TestLStat(fs::path path);
bool PrepareCursedFileop(void);
bool TestFileTouch(const char* path);
int  TestOpenFlags(const char* path, int32_t flags, const char* flags_str, int* errno_return = nullptr);
bool TestFileRW(const char* path, u16 to_test);
bool TestFileOps(const char* path);

bool StatCmp(const OrbisKernelStat* lhs, const OrbisKernelStat* rhs);

u16 dumpDirRecursive(fs::path path, int depth = 0);
u16 dumpDir(int fd);

void PrintStatInfo(const struct stat* info);
void PrintStatInfo(const OrbisKernelStat* info);
s8   GetDir(fs::path path, fs::path leaf, OrbisInternals::FolderDirent* dirent);
s8   GetDir(fs::path path, fs::path leaf, OrbisInternals::PfsDirent* dirent);

std::string file_mode(OrbisKernelMode mode);
ino_t       get_fileno(const char* path);
ino_t       get_fileno(int fd);
void        Obliterate(const char* path);
void        ElEsDashElAy(const char* path);
int32_t     touch(const char* path);
int32_t     touch(const std::string& path);
off_t       GetSize(const char* path);
off_t       GetSize(int fd);
int         exists(const char* path);

#endif // FS_TEST_H
