#include "fs_test.h"

#include <fcntl.h>
#include <orbis/UserService.h>
#include <orbis/libkernel.h>
#include <sstream>
#include <string>
#include <vector>

namespace FS_Test {
namespace oi = OrbisInternals;

off_t GetSize(int fd) {
  OrbisKernelStat st;
  if (int status = sceKernelFstat(fd, &st); status < 0) return status;
  return st.st_size;
}

off_t GetSize(const char* path) {
  int fd   = sceKernelOpen(path, O_RDONLY, 0777);
  int size = GetSize(fd);
  sceKernelClose(fd);
  return size;
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

void ElEsDashElAy(const char* path) {
  Log("<< ls -la [", path, "] >>");
  std::error_code ec {};

  for (const auto& entry: fs::directory_iterator(path, ec)) {
    struct OrbisKernelStat st {};
    std::string            pathstr = entry.path().string();
    int                    fd      = 0;

    if (fd = sceKernelOpen(entry.path().c_str(), 0x0, 0777); fd < 0) {
      LogError("Cannot open ", entry.path());
      continue;
    }
    if (sceKernelFstat(fd, &st) == -1) {
      LogError("Cannot stat ", entry.path());
      continue;
    }

    char     timebuf[64];
    std::tm* t = std::localtime(&st.st_mtime);
    std::strftime(timebuf, sizeof(timebuf), "%EY-%m-%d %H:%M", t);

    Log(file_mode(st.st_mode), right('0' + to_octal(st.st_mode), 8), std::dec, right(STR(st.st_nlink), 3), st.st_uid, ":", st.st_gid, right(STR(st.st_size), 8),
        timebuf, pathstr);

    // uncomment for hex dump
    // std::cout << "\t\t";
    // for (auto q = 0; q < sizeof(st); ++q)
    // {
    //     std::cout << " " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>((reinterpret_cast<const unsigned char *>(&st)[q]));
    //     if ((q + 1) % 32 == 0)
    //         std::cout << std::endl
    //                   << "\t\t";
    // }
    // std::cout << std::endl;

    if (sceKernelClose(fd) < 0) LogError("Can't close [", path, "]");
  }

  LogSuccess(">> ls -la [", path, "] <<");
  return;
}

std::string file_mode(OrbisKernelMode mode) {
  std::string s;

  if (S_ISREG(mode))
    s += '-';
  else if (S_ISDIR(mode))
    s += 'd';
  else if (S_ISLNK(mode))
    s += 'l';
  else if (S_ISCHR(mode))
    s += 'c';
  else if (S_ISBLK(mode))
    s += 'b';
  else if (S_ISFIFO(mode))
    s += 'p';
  else if (S_ISSOCK(mode))
    s += 's';
  else
    s += '?';

  // owner
  s += (mode & S_IRUSR) ? 'r' : '-';
  s += (mode & S_IWUSR) ? 'w' : '-';
  s += (mode & S_IXUSR) ? 'x' : '-';

  // group
  s += (mode & S_IRGRP) ? 'r' : '-';
  s += (mode & S_IWGRP) ? 'w' : '-';
  s += (mode & S_IXGRP) ? 'x' : '-';

  // other
  s += (mode & S_IROTH) ? 'r' : '-';
  s += (mode & S_IWOTH) ? 'w' : '-';
  s += (mode & S_IXOTH) ? 'x' : '-';

  return s;
}

u16 dumpDirRecursive(fs::path path, int depth) {
  if (0 == depth) {
    Log("Listing dirents of [", path.string(), "]");
  }

  std::string depEnt = "|--";
  for (u8 q = 0; q < depth; q++) {
    depEnt = depEnt + "|--";
  }
  depEnt[depEnt.length() - 1] = '>';

  //   Log(depDir.c_str(), path_str.c_str());

  s32 fd = sceKernelOpen(path.c_str(), 0, 0777);
  if (fd < 0) {
    Log("\t\t\t\t", depEnt.c_str(), "//NO ACCESS//");
    return 0;
  }

  char* buf    = new char[DIRENT_BUFFER_SIZE];
  char* bufptr = buf;
  s64   idx    = 0;
  s64   read_bytes;

  read_bytes = sceKernelGetdirentries(fd, buf, DIRENT_BUFFER_SIZE, &idx);
  if (read_bytes <= 0) Log("\t\t\t\t", depEnt.c_str(), "//FAILED//");

  u16 last_reclen = 0;

  while (read_bytes > 0) {
    bufptr       = buf;
    char* endptr = buf + read_bytes;
    while (bufptr < endptr) {
      oi::FolderDirent* entry = (oi::FolderDirent*)bufptr;
      bufptr += static_cast<s64>(entry->d_reclen);
      if (entry->d_reclen == 0) {
        Log("\t\t\t\t", depEnt.c_str(), "//BAD RECLEN//");
        break;
      }

      std::string ftype {};
      switch (entry->d_type) {
        default: ftype = std::to_string(entry->d_type); break;
        case 2: ftype = "DEV"; break;
        case 4: ftype = "DIR"; break;
        case 8: ftype = "FIL"; break;
        case 10: ftype = "LNK"; break;
        case 12: ftype = "SOC"; break;
      }

      std::string tree = depEnt + std::string(entry->d_name);

      Log("[", center(ftype, 3), "][", right(STR(entry->d_fileno), 10), "][", right(STR(entry->d_namlen), 3), "][", right(STR(entry->d_reclen), 3), "]", tree);

      last_reclen = entry->d_reclen;

      if (ftype != "DIR") continue;
      // preserved: parent and child may percieve each other differently
      if (strncmp(".", entry->d_name, 1) == 0) continue;
      if (strncmp("..", entry->d_name, 2) == 0) continue;

      std::string child(entry->d_name);
      dumpDirRecursive(path / child, depth + 1);
    }
    // move unread data to the beginning of the buffer
    s64 diff = endptr - bufptr;
    // shouldn't, but shit happens
    if (diff < 0) {
      Log("XDXDXDXDXDXDXDXDXDXD ", diff);
      diff = 0;
    }
    // memmove(buf, bufptr, diff);
    // read into after saved remainder
    //  read_bytes = sceKernelGetdirentries(fd, buf + diff, DIRENT_BUFFER_SIZE - diff, &idx) + diff;
    read_bytes = sceKernelGetdirentries(fd, buf, DIRENT_BUFFER_SIZE, &idx);
  }

  sceKernelClose(fd);
  delete[] buf;

  if (0 == depth) {
    LogSuccess("Listing dirents of [", path.string(), "]");
  }
  return last_reclen;
}

u16 dumpDir(int fd) {
  std::string depEnt = "|->";

  char* buf    = new char[DIRENT_BUFFER_SIZE];
  char* bufptr = buf;
  s64   idx    = 0;
  s64   read_bytes;

  read_bytes = sceKernelGetdirentries(fd, buf, DIRENT_BUFFER_SIZE, &idx);
  if (read_bytes <= 0) Log("\t\t\t\t", depEnt.c_str(), "//FAILED//");

  u16 last_reclen = 0;

  while (read_bytes > 0) {
    bufptr       = buf;
    char* endptr = buf + read_bytes;
    while (bufptr < endptr) {
      oi::FolderDirent* entry = (oi::FolderDirent*)bufptr;
      bufptr += static_cast<s64>(entry->d_reclen);
      if (entry->d_reclen == 0) {
        Log("\t\t\t\t", depEnt.c_str(), "//BAD RECLEN//");
        break;
      }

      std::string ftype {};
      switch (entry->d_type) {
        default: ftype = std::to_string(entry->d_type); break;
        case 2: ftype = "DEV"; break;
        case 4: ftype = "DIR"; break;
        case 8: ftype = "FIL"; break;
        case 10: ftype = "LNK"; break;
        case 12: ftype = "SOC"; break;
      }

      std::string tree = depEnt + std::string(entry->d_name);

      Log("[", center(ftype, 3), "][", right(STR(entry->d_fileno), 10), "][", right(STR(entry->d_namlen), 3), "][", right(STR(entry->d_reclen), 3), "]", tree);

      last_reclen = entry->d_reclen;

      if (ftype != "DIR") continue;
      // preserved: parent and child may percieve each other differently
      if (strncmp(".", entry->d_name, 1) == 0) continue;
      if (strncmp("..", entry->d_name, 2) == 0) continue;
    }
    // move unread data to the beginning of the buffer
    s64 diff = endptr - bufptr;
    // shouldn't, but shit happens
    if (diff < 0) {
      Log("XDXDXDXDXDXDXDXDXDXD ", diff);
      diff = 0;
    }
    memmove(buf, bufptr, diff);
    // read into after saved remainder
    read_bytes = sceKernelGetdirentries(fd, buf + diff, DIRENT_BUFFER_SIZE - diff, &idx) + diff;
  }

  delete[] buf;

  return last_reclen;
}

void PrintStatInfo(const struct stat* info) {
  Log("stat", "info.st_dev =", info->st_dev);
  Log("stat", "info.st_ino =", info->st_ino);
  Log("stat", "info.st_mode =", "0" + to_octal(info->st_mode));
  Log("stat", "info.st_nlink =", info->st_nlink);
  Log("stat", "info.st_uid =", info->st_uid);
  Log("stat", "info.st_gid =", info->st_gid);
  Log("stat", "info.st_rdev =", info->st_rdev);
  Log("stat", "info.st_atim.tv_sec =", info->st_atim.tv_sec);
  Log("stat", "info.st_atim.tv_nsec =", info->st_atim.tv_nsec);
  Log("stat", "info.st_mtim.tv_sec =", info->st_mtim.tv_sec);
  Log("stat", "info.st_mtim.tv_nsec =", info->st_mtim.tv_nsec);
  Log("stat", "info.st_ctim.tv_sec =", info->st_ctim.tv_sec);
  Log("stat", "info.st_ctim.tv_nsec =", info->st_ctim.tv_nsec);
  Log("stat", "info.st_size = ", info->st_size);
  Log("stat", "info.st_blocks =", info->st_blocks);
  Log("stat", "info.st_blksize =", info->st_blksize);
  Log("stat", "info.st_flags =", info->st_flags);
  Log("stat", "info.st_gen =", info->st_gen);
  Log("stat", "info.st_birthtim.tv_sec =", info->st_birthtim.tv_sec);
  Log("stat", "info.st_birthtim.tv_nsec =", info->st_birthtim.tv_nsec);
}

void PrintStatInfo(const OrbisKernelStat* info) {
  Log("stat", "info.st_dev =", info->st_dev);
  Log("stat", "info.st_ino =", info->st_ino);
  Log("stat", "info.st_mode =", "0" + to_octal(info->st_mode));
  Log("stat", "info.st_nlink =", info->st_nlink);
  Log("stat", "info.st_uid =", info->st_uid);
  Log("stat", "info.st_gid =", info->st_gid);
  Log("stat", "info.st_rdev =", info->st_rdev);
  Log("stat", "info.st_atim.tv_sec =", info->st_atim.tv_sec);
  Log("stat", "info.st_atim.tv_nsec =", info->st_atim.tv_nsec);
  Log("stat", "info.st_mtim.tv_sec =", info->st_mtim.tv_sec);
  Log("stat", "info.st_mtim.tv_nsec =", info->st_mtim.tv_nsec);
  Log("stat", "info.st_ctim.tv_sec =", info->st_ctim.tv_sec);
  Log("stat", "info.st_ctim.tv_nsec =", info->st_ctim.tv_nsec);
  Log("stat", "info.st_size = ", info->st_size);
  Log("stat", "info.st_blocks =", info->st_blocks);
  Log("stat", "info.st_blksize =", info->st_blksize);
  Log("stat", "info.st_flags =", info->st_flags);
  Log("stat", "info.st_gen =", info->st_gen);
  Log("stat", "info.st_lspare =", info->st_lspare);
  Log("stat", "info.st_birthtim.tv_sec =", info->st_birthtim.tv_sec);
  Log("stat", "info.st_birthtim.tv_nsec =", info->st_birthtim.tv_nsec);
}

bool StatCmp(const OrbisKernelStat* lhs, const OrbisKernelStat* rhs) {
  if (nullptr == rhs) return true;

  bool was_error = false;
  was_error |= lhs->st_mode != rhs->st_mode;
  was_error |= lhs->st_nlink != rhs->st_nlink;
  was_error |= lhs->st_uid != rhs->st_uid;
  was_error |= lhs->st_gid != rhs->st_gid;
  was_error |= lhs->st_size != rhs->st_size;
  was_error |= lhs->st_blocks != rhs->st_blocks;
  was_error |= lhs->st_blksize != rhs->st_blksize;
  was_error |= lhs->st_flags != rhs->st_flags;

  Log("---- OrbisKernelStat comparsion ----");
  Log("st_mode   \tLHS = ", right("0" + to_octal(lhs->st_mode), 7), "\t|\tRHS = ", right("0" + to_octal(rhs->st_mode), 7));
  // nlink can differ between localizations, constant in RO locations
  Log("st_nlink  \tLHS = ", right(STR(lhs->st_nlink), 7), "\t|\tRHS = ", right(STR(rhs->st_nlink), 7));
  Log("st_uid    \tLHS = ", right(STR(lhs->st_uid), 7), "\t|\tRHS = ", right(STR(rhs->st_uid), 7));
  Log("st_gid    \tLHS = ", right(STR(lhs->st_gid), 7), "\t|\tRHS = ", right(STR(rhs->st_gid), 7));
  Log("st_size   \tLHS = ", right(STR(lhs->st_size), 7), "\t|\tRHS = ", right(STR(rhs->st_size), 7));
  Log("st_blocks \tLHS = ", right(STR(lhs->st_blocks), 7), "\t|\tRHS = ", right(STR(rhs->st_blocks), 7));
  Log("st_blksize\tLHS = ", right(STR(lhs->st_blksize), 7), "\t|\tRHS = ", right(STR(rhs->st_blksize), 7));
  Log("st_flags  \tLHS = ", right(STR(lhs->st_flags), 7), "\t|\tRHS = ", right(STR(rhs->st_flags), 7));
  return !was_error;
}

s8 GetDir(fs::path path, fs::path leaf, oi::PfsDirent* dirent) {
  const char* target_file_name        = leaf.c_str();
  const u16   target_file_name_length = leaf.string().size();
  char        buffer[DIRENT_PFS_BUFFER_SIZE];
  char*       bufptr;
  char*       bufend;
  u64         total_read {0};
  s64         diff;
  bool        found {false};

  int fd = sceKernelOpen(path.c_str(), 0, 0777);
  if (fd < 0) {
    LogError("[PFS] Cannot open [", target_file_name, "]");
    return -1;
  }

  s64 read_bytes = sceKernelRead(fd, buffer, DIRENT_PFS_BUFFER_SIZE);

  // redundant
  while (read_bytes > 0 && !found) {
    total_read += read_bytes;
    bufptr = buffer;
    bufend = buffer + read_bytes;

    while (bufptr < bufend) {
      oi::PfsDirent* entry = (oi::PfsDirent*)bufptr;

      if (entry->d_reclen >= 0) bufptr += static_cast<s64>(entry->d_reclen);

      if (entry->d_reclen <= 0) {
        LogError("[PFS] //BAD RECLEN// at offset [", std::hex, total_read, "]");
        break;
      }

      if (entry->d_namlen == 0) {
        LogError("[PFS] //BAD FILENAME// at offset [", std::hex, total_read, "]");
        break;
      }

      if (entry->d_namlen != target_file_name_length) continue;

      if (strncmp(entry->d_name, target_file_name, target_file_name_length) == 0) {
        memcpy(dirent, entry, entry->d_reclen);
        found = true;
        break;
      }
    }

    if (read_bytes < DIRENT_PFS_BUFFER_SIZE) break;

    if (found) break;

    read_bytes = sceKernelRead(fd, buffer, DIRENT_PFS_BUFFER_SIZE);
  }

  fd = sceKernelClose(fd);
  if (fd < 0) {
    LogError("[PFS] Cannot close", target_file_name);
    return -1;
  }

  return found;
}

s8 GetDir(fs::path path, fs::path leaf, oi::FolderDirent* dirent) {
  const char* target_file_name        = leaf.c_str();
  const u16   target_file_name_length = leaf.string().size();
  char        buffer[DIRENT_BUFFER_SIZE];
  char*       bufptr;
  char*       bufend;
  u64         total_read {0};
  s64         diff;
  bool        found {false};

  int fd = sceKernelOpen(path.c_str(), 0, 0777);
  if (fd < 0) {
    LogError("[Normal] Cannot open [", target_file_name, "]");
    return -1;
  }

  s64 read_bytes = sceKernelGetdirentries(fd, buffer, DIRENT_BUFFER_SIZE, nullptr);

  // redundant
  while (read_bytes > 0 && !found) {
    total_read += read_bytes;
    bufptr = buffer;
    bufend = buffer + read_bytes;

    while (bufptr < bufend) {
      oi::FolderDirent* entry = (oi::FolderDirent*)bufptr;
      bufptr += static_cast<s64>(entry->d_reclen);

      if (entry->d_reclen == 0) {
        LogError("[Normal] //BAD RECLEN// at offset [", std::hex, total_read, "]");
        break;
      }

      if (entry->d_namlen == 0) {
        LogError("[Normal] //BAD FILENAME// at offset [", std::hex, total_read, "]");
        break;
      }

      if (entry->d_namlen != target_file_name_length) continue;

      if (strncmp(entry->d_name, target_file_name, target_file_name_length) == 0) {
        memcpy(dirent, entry, sizeof(oi::FolderDirent));
        found = true;
        break;
      }
    }

    if (found) break;

    // read into after saved remainder
    read_bytes = sceKernelGetdirentries(fd, buffer, DIRENT_BUFFER_SIZE, nullptr);
  }

  fd = sceKernelClose(fd);
  if (fd < 0) {
    LogError("[Normal] Cannot close", target_file_name);
    return -1;
  }

  return found;
}
} // namespace FS_Test