#include "fs_test.h"

#include "fs_constants.h"

#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <orbis/UserService.h>
#include <sstream>
#include <string>
#include <unistd.h>

namespace fs = std::filesystem;

namespace FS_Test {
namespace oi = OrbisInternals;

void RunTests() {
  RegenerateDir("/data/therapist");

  Log();
  Log("<<<< TEST SUITE STARTING >>>>");
  Log("\tSome test components may (and will) fail. This is expected.");
  Log("\tValidity of the test is determined by last message emitted by test case.");
  Log();

  Log();
  Log("\t<<<< DUMP DIRENTS >>>>");
  Log();
  dumpDirRecursive("/app0");

  //

  Log();
  Log("\t<<<< RELATIVE FILENO >>>>");
  Log();
  //
  // fileno=0 is *always* reserved for removed/nonexistent files
  // fileno=1 is *always* reserved for the superblock partitions
  // fileno=2 is *always* reserved for mount-root partitions
  TEST_CASE(TestRelatives("/"), "Test complete. Expected values: ~700, 3, -2", "Can't test relatives of /");
  TEST_CASE(TestRelatives("/app0", true), "Test complete", "Can't test relatives of /app0");
  TEST_CASE(TestRelatives("/app0/sce_sys"), "Test complete", "Can't test relatives of /app0/sce_sys");
  TEST_CASE(TestRelatives("/av_contents"), "Test complete", "Can't test relatives of /av_contents");
  TEST_CASE(TestRelatives("/data", true), "Test complete", "Can't test relatives of /data/therapist");
  TEST_CASE(!TestRelatives("/dev"), "Test complete. Expected values: -2, -2, ~800", "/dev should NOT be accessible");

  TEST_CASE(TestRelatives("/host", true), "/host is a superblock partition. Expected values: 9, 1, ~700", "Can't test relatives of /host");
  TEST_CASE(TestRelatives("/hostapp", true), "/hostapp is a superblock partition. Expected values: 10, 1, ~700", "Can't test relatives of /hostapp");

  TEST_CASE(TestRelatives("/system_tmp", true), "Test complete", "Can't test relatives of /system_tmp");
  TEST_CASE(!TestRelatives("/this_should_fail"), "Test complete", "Accessed nonexistent directory");

  //

  Log();
  Log("\t<<<< NORMAL AND PFS DIRENT >>>>");
  Log("\tNormal dirents are on LHS, PFS is on RHS.");
  Log("\tThe last element of the path is the accessed dirent,");
  Log("\tso it can refer to a file, directory or relative entry [ . ], [ .. ]");
  Log();
  //
  //  PFS works only with /app0
  //  Also, don't try to open /host with PFS
  TEST_CASE(!CompareNormalVsPFS("/", "."), "Test complete [PFS should fail]", "Cannot access [ . ] at [ / ]");
  TEST_CASE(!CompareNormalVsPFS("/", "app0"), "Test complete [PFS should fail]", "Cannot access [ app0 ] at [ / ]");
  TEST_CASE(!CompareNormalVsPFS("/", "data"), "Test complete [PFS should fail]", "Cannot access [ data ] at [ / ]");
  TEST_CASE(CompareNormalVsPFS("/app0", "."), "Test complete", "Cannot access [ . ] at [ /app0 ]");
  TEST_CASE(CompareNormalVsPFS("/app0", ".."), "Test complete", "Cannot access [ .. ] at [ /app0 ]");
  TEST_CASE(CompareNormalVsPFS("/app0", "sce_sys"), "Test complete", "Cannot access [ sce_sys ] at [ /app0 ]");
  TEST_CASE(CompareNormalVsPFS("/app0", "eboot.bin"), "Test complete", "Cannot access [ eboot.bin ] at [ /app0 ]");
  TEST_CASE(!CompareNormalVsPFS("/data/therapist", "."), "Test complete [PFS should fail]", "Cannot access [ . ] at [ /data/therapist ]");
  TEST_CASE(!CompareNormalVsPFS("/this_should_fail", "."), "Test complete [All should fail]", "Accessed [ . ] at [ /this_should_fail ]");

  //

  Log();
  Log("\t<<<< Example directory listings >>>>");
  Log("\t   This is made to mimic `ls -la`.");
  Log("\t       UID, GID are always 0");
  Log();

  ElEsDashElAy("/");
  ElEsDashElAy("/app0");

  //
  // Dirents
  //

  Log();
  Log("\t<<<< DIRENTS >>>>");
  Log();

  bool dirent_prepared = RegenerateDir("/data/therapist/tmp_dirent");
  TEST_CASE(dirent_prepared, "Prepared for dirents", "Can't setup dirents");
  if (dirent_prepared) {
    TEST_CASE(TestDirEnts(), "Test complete", "You won't see this, this test doesn't have a negative return");
  }

  //
  // Stat
  //

  Log();
  Log("\t<<<< STAT >>>>");
  Log("\tLHS - emulated, RHS - OG");
  Log();

  TEST_CASE(TestStat("/", &DumpedConstants::stat_root), "Test complete", "Stat comparsion failed");
  TEST_CASE(TestStat("/app0", &DumpedConstants::stat_root_app0), "Test complete", "Stat comparsion failed");
  TEST_CASE(TestStat("/app0/eboot.bin", &DumpedConstants::stat_root_app0_eboot), "Test complete", "Stat comparsion failed");
  TEST_CASE(TestStat("/app0/assets/misc/file.txt", &DumpedConstants::stat_root_app0_assets_misc_file), "Test complete", "Stat comparsion failed");
  TEST_CASE(TestStat("/data/therapist", &DumpedConstants::stat_root_data), "Test complete", "Stat comparsion failed");
  TEST_CASE(TestStat("/dev", &DumpedConstants::stat_root_dev), "Test complete", "Stat comparsion failed");
  TEST_CASE(TestStat("/dev/deci_stderr", &DumpedConstants::stat_root_dev_deci_stderr), "Test complete", "Stat comparsion failed");
  TEST_CASE(TestLStat("/dev/deci_stderr"), "Test complete", "Stat comparsion failed");
  TEST_CASE(TestStat("/dev/deci_stdout", &DumpedConstants::stat_root_dev_deci_stdout), "Test complete", "Stat comparsion failed");
  TEST_CASE(TestStat("/dev/stdin", nullptr), "Test complete [stat should fail]", "Stat comparsion failed");
  TEST_CASE(TestStat("/dev/stdout", nullptr), "Test complete", "Stat worked on /dev/stdout");
  TEST_CASE(!TestLStat("/dev/stdout"), "Test complete", "LStat worked on /dev/stdout");
  TEST_CASE(TestStat("/dev/random", &DumpedConstants::stat_root_dev_random), "Test complete", "Stat comparsion failed");
  TEST_CASE(TestStat("/dev/urandom", &DumpedConstants::stat_root_dev_urandom), "Test complete", "Stat comparsion failed");
  TEST_CASE(TestStat("/host", &DumpedConstants::stat_root_host), "Test complete", "Stat comparsion failed");
  TEST_CASE(TestStat("/hostapp", &DumpedConstants::stat_root_hostapp), "Test complete", "Stat comparsion failed");
  TEST_CASE(TestStat("/av_contents", &DumpedConstants::stat_root_av_contents), "Test complete", "Stat comparsion failed");
  TEST_CASE(!TestLStat("/av_contents"), "Test complete", "LStat worked on /av_contents");

  //
  // CURSED FILE CREATION
  //

  Log();
  Log("\t<<<< CURSED FILE CREATION >>>>");
  Log();

  bool cursed_fileop_prepared = PrepareCursedFileop();
  TEST_CASE(cursed_fileop_prepared, "Prepared for cursed fileop", "Can't setup cursed fileops");
  if (cursed_fileop_prepared) {
    TEST_CASE(TestFileTouch("/data/therapist/tmp_cursed/Aursed/AAursed/a_aa_file.txt"), "Test complete", "Test failed");
    TEST_CASE(TestFileTouch("/data/therapist/tmp_cursed/Aursed/a_file.txt"), "Test complete", "Test failed");
    TEST_CASE(TestFileTouch("/data/therapist/tmp_cursed/Aursed/././a_d_d_file.txt"), "Test complete", "Test failed");
    TEST_CASE(TestFileTouch("/data/therapist/tmp_cursed/Bursed/../Aursed/AAursed/b_dd_a_aa_file.txt"), "Test complete", "Test failed");
    TEST_CASE(TestFileTouch("/data/therapist/tmp_cursed/Aursed/AAursed/../../Cursed/../Bursed/aa_dd_dd_c_dd_b_file.txt"), "Test complete", "Test failed");
    TEST_CASE(TestFileTouch("/data/therapist/tmp_cursed/Aursed/AAursed/../../Cursed/CCursed/../../Bursed/BBursed/a_aa_dd_dd_c_cc_dd_dd_b_bb_file.txt"),
              "Test complete", "Test failed");
    Log("errno for tests below should equal 9 (EBADF) and 2 (ENOENT)");
    TEST_CASE(!TestFileTouch("/data/therapist/tmp_cursed/../tmp_cursed/../../././data/therapist/../data/therapist/././tmp_cursed/Cursed/../../../data/"
                             "therapist/tmp_cursed/Cursed/idfk.txt"),
              "Test complete", "Test failed");
    Log("errno for tests below should equal 9 (EBADF) and 22 (EINVAL)");
    TEST_CASE(!TestFileTouch("../../../data/therapist/tmp_cursed/escape_from_app0_file.txt"), "Test complete: Can't", "Test failed: Can",
              "escape from curdir with relatives");
    TEST_CASE(!TestFileTouch("app0_file.txt"), "Test complete: File not", "Test failed: File", "written to (RO?) curdir");
  }

  //
  // File open tests
  //

  Log();
  Log("\t<<<< File open tests >>>>");
  Log("\tAcual flag values for Orbis are different from sys/fcntl.h");
  Log("\tNumerical values are correct as of today, macro flags are not");
  Log();

  bool file_open_tests_prepared = RegenerateDir("/data/therapist/tmp_open"); // the same conditions
  if (file_open_tests_prepared) {
    // No modifiers
    // O_RDONLY
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_ro.txt", O_RDONLY, "O_RDONLY");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", 2, ")");
    // O_WRONLY
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_wo.txt", O_WRONLY, "O_WRONLY");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", 2, ")");
    // O_RDWR
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_rw.txt", O_RDWR, "O_RDWR");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", 2, ")");
    // O_RDONLY | O_WRONLY | O_RDWR
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_rowo.txt", O_RDONLY | O_WRONLY | O_RDWR, "O_RDONLY | O_WRONLY | O_RDWR");
              _errno == EINVAL, "Pass (EINVAL)", "Fail", "( errno =", _errno, "should be =", 22, ")");

    // O_TRUNC
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_rot.txt", O_TRUNC, "O_RDONLY | O_TRUNC");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", ENOENT, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_wot.txt", O_WRONLY | O_TRUNC, "O_WRONLY | O_TRUNC");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", ENOENT, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_rwt.txt", O_RDWR | O_TRUNC, "O_RDWR | O_TRUNC");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", ENOENT, ")");
    // O_CREAT
    // these create a file
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_roc.txt", O_RDONLY | O_CREAT, "O_RDONLY | O_CREAT");
              _errno == 0, "Pass", "Fail", "( errno =", _errno, "should be =", 0, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_woc.txt", O_WRONLY | O_CREAT, "O_WRONLY | O_CREAT");
              _errno == 0, "Pass", "Fail", "( errno =", _errno, "should be =", 0, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_rwc.txt", O_RDWR | O_CREAT, "O_RDWR | O_CREAT");
              _errno == 0, "Pass", "Fail", "( errno =", _errno, "should be =", 0, ")");
    // O_CREAT | O_EXCL
    // exists, not creating
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_roc.txt", O_RDONLY | O_CREAT | O_EXCL, "O_RDONLY | O_CREAT | O_EXCL");
              _errno == 17, "Pass (EEXIST)", "Fail", "( errno =", _errno, "should be =", EEXIST, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_woc.txt", O_WRONLY | O_CREAT | O_EXCL, "O_WRONLY | O_CREAT | O_EXCL");
              _errno == 17, "Pass (EEXIST)", "Fail", "( errno =", _errno, "should be =", EEXIST, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_rwc.txt", O_RDWR | O_CREAT | O_EXCL, "O_RDWR | O_CREAT | O_EXCL");
              _errno == 17, "Pass (EEXIST)", "Fail", "( errno =", _errno, "should be =", EEXIST, ")");
    // O_APPEND
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_roa.txt", O_RDONLY | O_APPEND, "O_RDONLY | O_APPEND");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", ENOENT, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_woa.txt", O_WRONLY | O_APPEND, "O_WRONLY | O_APPEND");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", ENOENT, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_rwa.txt", O_RDWR | O_APPEND, "O_RDWR | O_APPEND");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", ENOENT, ")");
    // O_TRUNC | O_APPEND
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_rota.txt", O_RDONLY | O_TRUNC | O_APPEND, "O_RDONLY | O_TRUNC | O_APPEND");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", ENOENT, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_wota.txt", O_WRONLY | O_TRUNC | O_APPEND, "O_WRONLY | O_TRUNC | O_APPEND");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", ENOENT, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_rwta.txt", O_RDWR | O_TRUNC | O_APPEND, "O_RDWR | O_TRUNC | O_APPEND");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", ENOENT, ")");
    // O_DIRECTORY (nonexistent file is a target)
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_dir_ro.txt", O_RDONLY | O_DIRECTORY, "O_RDONLY | O_DIRECTORY");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", ENOENT, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_dir_wo.txt", O_WRONLY | O_DIRECTORY, "O_WRONLY | O_DIRECTORY");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", ENOENT, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_dir_rw.txt", O_RDWR | O_DIRECTORY, "O_RDWR | O_DIRECTORY");
              _errno == ENOENT, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", ENOENT, ")");
    // O_DIRECTORY (existing directory)
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/", 0 | 0x20000, "O_RDONLY | O_DIRECTORY");
              _errno == 0, "Pass", "Fail", "( errno =", _errno, "should be =", 0, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/", 0x1 | 0x20000, "O_WRONLY | O_DIRECTORY");
              _errno == 21, "Pass (EISDIR)", "Fail", "( errno =", _errno, "should be =", EISDIR, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/", 0x2 | 0x20000, "O_RDWR | O_DIRECTORY");
              _errno == 21, "Pass (EISDIR)", "Fail", "( errno =", _errno, "should be =", EISDIR, ")");
    // O_CREAT | O_DIRECTORY (unspecified type, directory)
    // these create a file
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_dir_rocd", 0 | 0x200 | 0x20000, "O_RDONLY | O_CREAT | O_DIRECTORY");
              _errno == 20, "Pass (ENOTDIR)", "Fail", "( errno =", _errno, "should be =", ENOTDIR, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_dir_wocd", 0x1 | 0x200 | 0x20000, "O_WRONLY | O_CREAT | O_DIRECTORY");
              _errno == 20, "Pass (ENOTDIR)", "Fail", "( errno =", _errno, "should be =", ENOTDIR, ")");
    TEST_CASE(int _errno = TestOpenFlags("/data/therapist/tmp_open/nonexistent_dir_rwcd", 0x2 | 0x200 | 0x20000, "O_RDWR | O_CREAT | O_DIRECTORY");
              _errno == 20, "Pass (ENOTDIR)", "Fail", "( errno =", _errno, "should be =", ENOTDIR, ")");
    // No modifiers, RO directory
    // O_RDONLY
    TEST_CASE(int _errno = TestOpenFlags("/app0/assets/misc/file.txt", 0, "O_RDONLY");
              _errno == 0, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", 0, ")");
    // O_WRONLY
    TEST_CASE(int _errno = TestOpenFlags("/app0/assets/misc/file.txt", 0x1, "O_WRONLY");
              _errno == 30, "Pass (EROFS)", "Fail", "( errno =", _errno, "should be =", EROFS, ")");
    // O_RDWR
    TEST_CASE(int _errno = TestOpenFlags("/app0/assets/misc/file.txt", 0x2, "O_RDWR");
              _errno == 30, "Pass (EROFS)", "Fail", "( errno =", _errno, "should be =", EROFS, ")");
    // O_RDONLY | O_WRONLY | O_RDWR
    TEST_CASE(int _errno = TestOpenFlags("/app0/assets/misc/file.txt", 0 | 0x1 | 0x2, "O_RDONLY | O_WRONLY | O_RDWR");
              _errno == EINVAL, "Pass (EINVAL)", "Fail", "( errno =", _errno, "should be =", EINVAL, ")");
    // Obviously bad ones, flags are irrelevant
    TEST_CASE(int _errno = TestOpenFlags("assets/misc/file.txt", 0, "O_RDONLY");
              _errno == EINVAL, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", EINVAL, ")");
    TEST_CASE(int _errno = TestOpenFlags("./assets/misc/file.txt", 0, "O_RDONLY");
              _errno == EINVAL, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", EINVAL, ")");
    TEST_CASE(int _errno = TestOpenFlags("", 0, "O_RDONLY"); _errno == EINVAL, "Pass (ENOENT)", "Fail", "( errno =", _errno, "should be =", EINVAL, ")");

    Log("RO+Truncate is undefined. Testing");
    int __fd = sceKernelOpen("/data/therapist/tmp_open/nonexistent_rot.txt", O_RDONLY | O_TRUNC, 0777);
    TEST_CASE(errno == ENOENT, "RO+TRUNC returned correct errno", "RO+TRUNC returned wrong errno", ":", errno, "( should be:", ENOENT, ")");
    TEST_CASE(exists("/data/therapist/tmp_open/nonexistent_rot.txt");
              errno == ENOENT, "RO+TRUNC file not created", "RO+TRUNC file created", "( errno =", errno, "should be =", ENOENT, ")")

    Log("RO+Truncate+Append is undefined. Testing");
    __fd = sceKernelOpen("/data/therapist/tmp_open/nonexistent_rota.txt", O_RDONLY | O_TRUNC | O_APPEND, 0777);
    TEST_CASE(errno == ENOENT, "ROTA returned correct errno", "ROTA returned wrong errno", ":", errno, "( should be:", ENOENT, ")");
    TEST_CASE(exists("/data/therapist/tmp_open/nonexistent_rota.txt");
              errno == ENOENT, "ROTA file not created", "ROTA file created", "( errno =", errno, "should be =", ENOENT, ")")
  }

  //
  // File ops tests
  //

  Log();
  Log("\t<<<< File ops tests >>>>");
  Log();

  bool file_op_tests_prepared = RegenerateDir("/data/therapist/tmp_rw2"); // the same conditions
  if (file_op_tests_prepared) {
    // short test, just depends on target location. the output is looong, don't get overboard
    TEST_CASE(!TestFileOps(""), "Pass", "Fail", "[open() should fail with errno = 22]");
    TEST_CASE(!TestFileOps("/app0/assets/misc/test.txt"), "Pass", "Fail", "[open should fail with errno = 30]");
    TEST_CASE(TestFileOps("/data/therapist/tmp_rw2/test.txt"), "Pass", "Fail");
    // a true trial. should fail immediately :)
    // TEST(TestFileOps("/app0/sce_sys/param.sfo", 0, "[No flags]"), "Pass", "Fail");
  }

  //
  // File R/W tests
  //

  Log();
  Log("\t<<<< File R/W tests >>>>");
  Log();

  bool file_rw_tests_prepared = RegenerateDir("/data/therapist/tmp_rw3"); // the same conditions
  if (file_rw_tests_prepared) {
    TEST_CASE(!TestFileRW("", 32), "Test complete", "Test failed", "[empty path, should fail with errno = 22 (EINVAL)]");
    touch("/data/therapist/tmp_rw3/rwtest.txt");
    TEST_CASE(TestFileRW("/data/therapist/tmp_rw3/rwtest.txt", 32), "Test complete", "Test failed", "[both buffers should hold the same values]");
    TEST_CASE(!TestFileRW("/data/therapist/tmp_rw3", 32), "Test complete:", "Test failed:", "[R/W on a directory should fail with errno = 21 (EISDIR)]");
    TEST_CASE(!TestFileRW("/app0/assets/misc/file_empty.txt", 32),
              "Test complete:", "Test failed:", "[R/W on a file in RO directory should fail with errno = 30 (EROFS)]");

    TEST_CASE(!TestFileRW("/dev/stdin", 32), "Test complete", "Test failed", "[Should fail with errno = 2 (ENOENT)]");
    TEST_CASE(!TestFileRW("/dev/stdout", 32), "Test complete", "Test failed", "[Should fail with errno = 2 (ENOENT)]");
    TEST_CASE(!TestFileRW("/dev/null", 32), "Test complete", "Test failed", "[Write should pass, read should return 0 bytes]");
    TEST_CASE(TestFileRW("/dev/random", 32), "Test complete", "Test failed", "[All should pass, random reads]");
    TEST_CASE(TestFileRW("/dev/urandom", 32), "Test complete", "Test failed", "[All should pass, random reads]");
    TEST_CASE(TestFileRW("/dev/zero", 32), "Test complete", "Test failed", "[All should pass, read buffer zeroed out]");
  }

  RegenerateDir("/data/therapist/tmd");
  for (u16 idx = 0; idx < 16; idx++) {
    std::string p = std::string("/data/therapist/tmd/AFileWithAReallyLongName-") + std::to_string(idx);
    touch(p.c_str());
  }
  for (u16 idx = 0; idx < 64; idx++) {
    std::string p = std::string("/data/therapist/tmd/SN-") + std::to_string(idx);
    touch(p.c_str());
  }
  dumpDirRecursive("/data/therapist/tmd");

  ///
  /// Case sensitivity
  ///

  Log();
  Log("\t<<<< Case sensitivity tests >>>>");
  Log();

  struct stat st;
  const char* case_insensitive_path           = "/data/therapist/cAsEinSenSiTive.hwdp";
  const char* case_insensitive_path_lower     = "/data/therapist/caseinsensitive.hwdp";
  const char* case_insensitive_path_upper     = "/DATA/THERAPIST/CASEINSENSITIVE.HWDP";
  const char* case_insensitive_path_upper_end = "/data/THERAPIST/CASEINSENSITIVE.HWDP";
  TEST_CASE(sceKernelClose(sceKernelOpen(case_insensitive_path, O_RDWR | O_TRUNC | O_CREAT, 0777)) == 0, "Test file created", "Test file not created");

  TEST_CASE(stat(case_insensitive_path, &st) == 0, "Data: 1:1 case sensitivity passed", "Data: Can't resolve 1:1 name", "( ", case_insensitive_path, " )");
  TEST_CASE(stat(case_insensitive_path_lower, &st) != 0 && errno == ENOENT, "Data: Data: Lowercase sensitivity passed", "Data: Resolved name in lowercase",
            "( ", case_insensitive_path_lower, " )");
  TEST_CASE(stat(case_insensitive_path_upper, &st) != 0 && errno == ENOENT, "Data: Uppercase sensitivity passed", "Data: Resolved name in uppercase", "( ",
            case_insensitive_path_upper, " )");
  TEST_CASE(stat(case_insensitive_path_upper_end, &st) != 0 && errno == ENOENT, "Data: Uppercase sensitivity (2nd half) passed",
            "Data: Resolved name (2nd half) in uppercase", "( ", case_insensitive_path_upper_end, " )");

  const char* case_insensitive_app0_path           = "/app0/assets/misc/cAsEinSEnsITiVE.HwDp";
  const char* case_insensitive_path_app0_lower     = "/app0/assets/misc/caseinsensitive.hwdp";
  const char* case_insensitive_path_app0_upper     = "/APP0/ASSETS/misc/CASEINSENSITIVE.HWDP";
  const char* case_insensitive_path_app0_upper_end = "/app0/ASSETS/misc/CASEINSENSITIVE.HWDP";
  TEST_CASE(stat(case_insensitive_app0_path, &st) == 0, "app0: 1:1 case sensitivity passed", "app0: Can't resolve 1:1 name", "( ", case_insensitive_app0_path,
            " )");
  TEST_CASE(stat(case_insensitive_path_app0_lower, &st) == 0, "app0: Data: Lowercase sensitivity passed", "app0: Can't resolve name in lowercase", "( ",
            case_insensitive_path_app0_lower, " )");
  TEST_CASE(stat(case_insensitive_path_app0_upper, &st) != 0 && errno == ENOENT, "app0: Uppercase (whole path) sensitivity passed",
            "app0: Resolved (whole path) name in uppercase", "( ", case_insensitive_path_app0_upper, " )");
  TEST_CASE(stat(case_insensitive_path_app0_upper_end, &st) == 0, "app0: Uppercase (app0 path) sensitivity passed",
            "app0: Can't resolve (app0 path) name in uppercase", "( ", case_insensitive_path_app0_upper_end, " )");

  ///
  /// Long names (ENAMETOOLONG)
  ///

  Log();
  Log("\tLong names (ENAMETOOLONG)");
  Log();

  sceKernelClose(sceKernelOpen("/data/therapist/dummy_target", O_CREAT | O_WRONLY, 0777));
  OrbisKernelStat st_long_orbis {};
  struct stat     st_long_posix {};

  const char* very_long_path = "/data/therapist/"
                               "thisisafilewithaverylongnameForrealthisfileissupposedtohaveareallylongnameforthistestcaseOtherwisewewontknowifitworkscorrec"
                               "tlyItsnotusedoftenbutcanbepro"
                               "blematicasthisbreaksdirentswhichcanstoreonly255charactersincludingnullterminatorThisisthelastsentence.goodluck";
  // sceKernel*
  TEST_CASE(int status = sceKernelOpen(very_long_path, O_RDONLY, 0777); errno == ENAMETOOLONG, "File name too long detected",
                                                                        "Didn't detect file name >255 characters", "sceKernelOpen(RO) ( errno =", errno,
                                                                        ", should be", ENAMETOOLONG, ")");
  TEST_CASE(int status = sceKernelOpen(very_long_path, O_WRONLY | O_RDWR | O_RDONLY, 0777);
            errno == EINVAL, "Cursed rw flags are more important than ENAMETOOLONG", "Didn't detect file name >255 characters",
            "sceKernelOpen(RDO|WRO|RW) ( errno =", errno, ", should be", EINVAL, ")");
  TEST_CASE(int status = sceKernelOpen(very_long_path, O_CREAT | O_RDONLY, 0777);
            errno == ENAMETOOLONG, "File creation flags are less important than ENAMETOOLONG", "Didn't detect file name >255 characters",
            "sceKernelOpen(CREAT|RDO) ( errno =", errno, ", should be", ENAMETOOLONG, ")");
  TEST_CASE(int status = sceKernelRename(very_long_path, "/data/therapist/dummy_target_2");
            errno == ENAMETOOLONG, "File name too long detected", "Didn't detect file name >255 characters", "sceKernelRename(long,normal) ( errno =", errno,
            ", should be", ENAMETOOLONG, ")");
  TEST_CASE(int status = sceKernelRename("/data/therapist/dummy_target", very_long_path);
            errno == ENAMETOOLONG, "File name too long detected", "Didn't detect file name >255 characters", "sceKernelRename(normal,long) ( errno =", errno,
            ", should be", ENAMETOOLONG, ")");
  TEST_CASE(int status = sceKernelCheckReachability(very_long_path); errno == ENAMETOOLONG, "File name too long detected",
                                                                     "Didn't detect file name >255 characters", "sceKernelCheckReachability() ( errno =", errno,
                                                                     ", should be", ENAMETOOLONG, ")");
  TEST_CASE(int status = sceKernelMkdir(very_long_path, 0777); errno == ENAMETOOLONG, "File name too long detected", "Didn't detect file name >255 characters ",
                                                               "sceKernelMkdir() ( errno =", errno, ", should be", ENAMETOOLONG, ")");

  TEST_CASE(int status = sceKernelStat(very_long_path, &st_long_orbis); errno == ENAMETOOLONG, "File name too long detected",
                                                                        "Didn't detect file name >255 characters", "sceKernelStat() ( errno =", errno,
                                                                        ", should be", ENAMETOOLONG, ")");
  TEST_CASE(int status = sceKernelUnlink(very_long_path); errno == ENAMETOOLONG, "File name too long detected", "Didn't detect file name >255 characters",
                                                          "sceKernelUnlink() ( errno =", errno, ", should be", ENAMETOOLONG, ")");

  // posix_*
  TEST_CASE(int status = open(very_long_path, O_RDONLY, 0777); errno == ENAMETOOLONG, "File name too long detected", "Didn't detect file name >255 characters ",
                                                               "open(RO) ( errno =", errno, ", should be", ENAMETOOLONG, ")");
  TEST_CASE(int status = open(very_long_path, O_WRONLY | O_RDWR | O_RDONLY, 0777); errno == EINVAL, "Cursed rw flags are more important than ENAMETOOLONG",
                                                                                   "Didn't detect file name >255 characters",
                                                                                   "open(RDO|WRO|RW) ( errno =", errno, ", should be", EINVAL, ")");
  TEST_CASE(int status = open(very_long_path, O_CREAT | O_RDONLY, 0777); errno == ENAMETOOLONG, "File creation flags are less important than ENAMETOOLONG",
                                                                         "Didn't detect file name >255 characters", "open(CREAT|RDO) ( errno =", errno,
                                                                         ", should be", ENAMETOOLONG, ")");
  TEST_CASE(int status = rename(very_long_path, "/data/therapist/dummy_target_2"); errno == ENAMETOOLONG, "File name too long detected",
                                                                                   "Didn't detect file name >255 characters",
                                                                                   "rename(long,normal) ( errno =", errno, ", should be", ENAMETOOLONG, ")");
  TEST_CASE(int status = rename("/data/therapist/dummy_target", very_long_path); errno == ENAMETOOLONG, "File name too long detected",
                                                                                 "Didn't detect file name >255 characters",
                                                                                 "rename(normal,long) ( errno =", errno, ", should be", ENAMETOOLONG, ")");
  TEST_CASE(int status = mkdir(very_long_path, 0777); errno == ENAMETOOLONG, "File name too long detected", "Didn't detect file name >255 characters",
                                                      "mkdir() ( errno =", errno, ", should be", ENAMETOOLONG, ")");
  TEST_CASE(int status = stat(very_long_path, &st_long_posix); errno == ENAMETOOLONG, "File name too long detected", "Didn't detect file name >255 characters ",
                                                               "stat() ( errno =", errno, ", should be", ENAMETOOLONG, ")");
  TEST_CASE(int status = unlink(very_long_path); errno == ENAMETOOLONG, "File name too long detected", "Didn't detect file name >255 characters",
                                                 "unlink() ( errno =", errno, ", should be", ENAMETOOLONG, ")");

  ///
  /// Moving files
  ///

  Log();
  Log("\t<<<< Moving files >>>>");
  Log();

  const char* movingFileA = "/data/therapist/moves/fileA";
  const char* movingFileB = "/data/therapist/moves/fileB";
  const char* movingFileC = "/data/therapist/moves/fileC";

  const char* movingDirectoryA = "/data/therapist/moves/dirA";
  const char* movingDirectoryB = "/data/therapist/moves/dirB";
  const char* movingDirectoryC = "/data/therapist/moves/dirC";
  const char* movingDirectoryD = "/data/therapist/moves/dirD";

  Obliterate("/data/therapist/moves");
  sceKernelMkdir("/data/therapist/moves", 0777);
  sceKernelMkdir(movingDirectoryA, 0777);
  sceKernelMkdir(movingDirectoryB, 0777);
  sceKernelMkdir(movingDirectoryC, 0777);
  sceKernelMkdir(movingDirectoryD, 0777);
  touch(movingFileA);
  touch(movingFileB);
  touch(movingFileC);

  ino_t fileno_movingDirectoryA = get_fileno(movingDirectoryA);
  ino_t fileno_movingDirectoryB = get_fileno(movingDirectoryB);
  ino_t fileno_movingDirectoryC = get_fileno(movingDirectoryC);
  ino_t fileno_movingDirectoryD = get_fileno(movingDirectoryD);
  ino_t fileno_movingFileA      = get_fileno(movingFileA);
  ino_t fileno_movingFileB      = get_fileno(movingFileB);
  ino_t fileno_movingFileC      = get_fileno(movingFileC);

  Log("fileno of: movingDirectoryA:\t", fileno_movingDirectoryA);
  Log("fileno of: movingDirectoryB:\t", fileno_movingDirectoryB);
  Log("fileno of: movingDirectoryC:\t", fileno_movingDirectoryC);
  Log("fileno of: movingDirectoryD:\t", fileno_movingDirectoryD);
  Log("fileno of: movingFileA:\t", fileno_movingFileA);
  Log("fileno of: movingFileB:\t", fileno_movingFileB);
  Log("fileno of: movingFileC:\t", fileno_movingFileC);

  const char* yeetFile = "/data/therapist/moves/yeet";
  const char* yeetDir  = "/data/therapist/moves/yeet_dir";

  TEST_CASE(int status = sceKernelRename(movingFileA, movingFileB);
            status == 0 && get_fileno(movingFileB) == fileno_movingFileA, "Moved", "Not moved", "file->(existent)file:", status);
  fileno_movingFileB = fileno_movingFileA;
  TEST_CASE(int status = sceKernelRename(movingFileC, yeetFile);
            status == 0 && get_fileno(yeetFile) == fileno_movingFileC, "Moved", "Not moved", "file->(nonexistent)file:", status);
  ino_t fileno_movingFileYeet = fileno_movingFileC;
  Log("fileno of: yeetFile:\t", fileno_movingFileYeet);

  TEST_CASE(int status = sceKernelRename(movingDirectoryA, movingDirectoryB);
            status == 0 && get_fileno(movingDirectoryB) == fileno_movingDirectoryA, "Moved", "Not moved", "dir->(existing)dir:", status);
  fileno_movingDirectoryB = fileno_movingDirectoryA;
  TEST_CASE(int status = sceKernelRename(movingDirectoryC, yeetDir);
            status == 0 && get_fileno(yeetDir) == fileno_movingDirectoryC, "Moved", "Not moved", "dir->(nonexistent)dir:", status);
  ino_t fileno_movingDirYeet = fileno_movingDirectoryC;
  Log("fileno of: yeetDir:\t", fileno_movingDirYeet);

  // no change in folder structure
  TEST_CASE(int status = sceKernelRename(yeetFile, yeetDir); errno == EISDIR, "Not moved", "Moved", "file->(existent)dir:", status);
  // no change either
  TEST_CASE(int status = sceKernelRename(yeetDir, yeetFile); errno == ENOTDIR, "Not moved", "Moved", "dir->(existent)file:", status);
  TEST_CASE(int status = sceKernelRename(yeetFile, "/data/therapist/moves/yeet_dir/yeeee");
            status == 0 && get_fileno("/data/therapist/moves/yeet_dir/yeeee") == fileno_movingFileYeet, "Moved", "Not moved",
            "file->(into existent)dir:", status);

  // move empty to not empty dir, no change
  sceKernelMkdir(movingDirectoryD, 0777);
  TEST_CASE(int status = sceKernelRename(movingDirectoryD, yeetDir);
            errno == ENOTEMPTY && get_fileno(yeetDir) == fileno_movingDirYeet, "Not moved", "Moved", "empty dir->not empty dir:", status);
  // not empty to empty, changed
  TEST_CASE(int status = sceKernelRename(yeetDir, movingDirectoryD);
            status == 0 && get_fileno(movingDirectoryD) == fileno_movingDirYeet, "Moved", "Not moved", "not empty dir->empty dir:", status);
  fileno_movingDirectoryD = fileno_movingDirYeet;

  ///
  /// Open fd abuse (moving/removing files with open fd)
  ///

  Log();
  Log("\t<<<< Open fd abuse (moving/removing files with open fd) >>>>");
  Log();

  const char* abused_file         = "/data/therapist/abuse.txt";
  const char* teststring1         = "0123456789\r\n";
  const char* teststring2         = "9876543210\r\n";
  const char* readback_string     = "0123456789\r\n9876543210\r\n";
  int64_t     readback_string_len = strlen(readback_string);
  char*       abused_buffer[256] {0};

  touch(abused_file);
  int  abused_fd     = sceKernelOpen(abused_file, O_RDWR, 0777);
  auto abused_fileno = get_fileno(abused_file);

  if (abused_fileno == 0) LogError("File not created");
  TEST_CASE(abused_fd >= 0, "Test file opened", "Test file not opened", "status =", abused_fd);

  if (int bw = sceKernelWrite(abused_fd, teststring1, strlen(teststring1)); bw < 0) LogError("Didn't write the whole string 1:", bw, "written");
  TEST_CASE(int status = sceKernelUnlink(abused_file); status == 0, "File", "File not", "unlinked ( status =", status, ")");
  if (get_fileno(abused_file) != 0) LogError("File not unlinked");

  if (int bw = sceKernelWrite(abused_fd, teststring2, strlen(teststring2)); bw < 0) LogError("Didn't write the whole string 2:", bw, "written");
  if (int lsr = sceKernelLseek(abused_fd, 0, 0); lsr != 0) LogError("Can't lseek:", lsr);
  if (int br = sceKernelRead(abused_fd, abused_buffer, 256); br < 0) LogError("Didn't read the whole string:", br, "read");
  TEST_CASE(memcmp(abused_buffer, readback_string, readback_string_len) == 0, "Readback string is correct", "Readback string is incorrect", "");

  Log("fstat() Fileno before removal =", abused_fileno, "after =", get_fileno(abused_fd));
  Log("stat() after removal =", exists(abused_file));

  if (int status = sceKernelClose(abused_fd); status != 0) LogError("File didn't close properly ( status =", status, ")");

  struct OrbisKernelStat ost;
  Log(sceKernelStat("/download0", &ost));
  Log(sceKernelMkdir("/download0/temp/", 0777));
}

bool TestFileOps(const char* path) {
  LogTest("Testing file operations on [", path, "]");
  OrbisKernelStat st {};
  int             _errno = 0;
  int             fd     = -1;
  // shhh, don't spoil it yet
  if (sceKernelStat(path, &st) > -1) sceKernelUnlink(path);

  ResetErrorCounter();
  errno = 0;
  TEST_CASE(sceKernelStat(path, &st) < 0, "Stat failed on nonexistent file", "Stat'd nonexistent file", "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(sceKernelTruncate(path, 0) < 0, "Truncate to 0 failed on nonexistent file", "Truncated nonexistent file to 0", "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(sceKernelTruncate(path, 10) < 0, "Truncate to 10 failed on nonexistent file", "Truncated nonexistent file to 10", "( errno =", errno, ")");

  errno = 0;
  TEST_CASE(fd = sceKernelOpen(path, 0x2 | 0x200, 0777); fd > -1, "File opened", "Can't open file", "[", path, "]", "( errno =", errno, ")");
  if (fd < 0) return false;

  // expand to 10 characters
  errno = 0;
  TEST_CASE(sceKernelFtruncate(fd, 10) == 0, "File expanded", "File not expanded", "( errno =", errno, ")");
  errno = 0;
  // new size
  TEST_CASE(int news = GetSize(fd); news == 10, "File resized", "File not resized", "(current size:", news, "should be:", 10, ") ( errno =", errno, ")");
  errno = 0;
  // truncating up doesn't change file ptr
  TEST_CASE(int newp = sceKernelLseek(fd, 0, 1); newp == 0, "ptr is valid", "ptr is invalid", "(lseek is", newp, "should be", 0, ") ( errno =", errno, ")");

  char readbuffer[256];

  const char teststr[]   = "If you can read this, unused gibberish";
  auto       teststr_len = strlen(teststr);

  // write test string
  errno = 0;
  TEST_CASE(int bw = sceKernelWrite(fd, teststr, teststr_len);
            bw == teststr_len, "Write successful", "Write failed", "(Written:", bw, ") should write", teststr_len, "( errno =", errno, ")");
  // check ptr after writing
  errno = 0;
  TEST_CASE(int curp = sceKernelLseek(fd, 0, 1);
            curp == teststr_len, "ptr is valid", "ptr is invalid", "(lseek is", curp, "should be:", teststr_len, ") ( errno =", errno, ")");
  // return to origin
  errno = 0;
  TEST_CASE(int curp = sceKernelLseek(fd, 0, 0); curp == 0, "ok:", "fail:", "origin+0 (lseek is", curp, "should be:", 0, ") ( errno =", errno, ")");
  errno = 0;
  memset(readbuffer, 0, 256);
  TEST_CASE(int br = sceKernelRead(fd, readbuffer, 256);
            br == teststr_len, "Read successful", "Read failed", "(Read:", br, "should read:", teststr_len, ") ( errno =", errno, ")");
  Log("\t", readbuffer);
  errno = 0;
  // check lseek after reading
  TEST_CASE(int curp = sceKernelLseek(fd, 0, 1);
            curp == teststr_len, "ptr is valid", "ptr is invalid", "(lseek is", curp, "should be:", teststr_len, ") ( errno =", errno, ")");

  // truncate to save only the beginning
  errno = 0;
  TEST_CASE(sceKernelFtruncate(fd, 20) == 0, "File truncated", "File not truncated", "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(int news = GetSize(path); news == 20, "File resized", "File not resized", "(current size:", news, "should be:", 20, ") ( errno =", errno, ")");
  errno = 0;
  TEST_CASE(int newp = sceKernelLseek(fd, 0, 1); newp == 38, "file pointer not changed after resizing", "file ptr changed after resizing", "(lseek is", newp,
                                                 "should be", 38, ") ( errno =", errno, ")");

  // overwrite 2nd half of the string
  const char teststr2[]   = "xxxxxxxxxxxxxxxxxxxx, remember to drink some water. This is saved for later.";
  auto       teststr2_len = strlen(teststr2);
  errno                   = 0;
  // lseek to the end of the file
  TEST_CASE(int newp = sceKernelLseek(fd, 0, 2); newp == 20, "ok:", "fail:", "end+0 (lseek is", newp, "should be", 20, ") ( errno =", errno, ")");
  errno = 0;
  // write the other half
  TEST_CASE(int bw = sceKernelWrite(fd, teststr2 + 20, teststr2_len - 20); bw == teststr2_len - 20, "2-nd write successful", "2-nd write failed",
                                                                           "(Written:", bw, ") should write", teststr2_len - 20, "( errno =", errno, ")");
  errno = 0;
  // good advice
  TEST_CASE(int curp = sceKernelLseek(fd, 0, 0); curp == 0, "ok:", "fail:", "origin+0 (lseek is", curp, "should be:", 0, ") ( errno =", errno, ")");
  errno = 0;
  memset(readbuffer, 0, 256);
  TEST_CASE(int br = sceKernelRead(fd, readbuffer, 51);
            br == 51, "Read successful", "Read failed", "(Read:", br, "should read:", 51, ") ( errno =", errno, ")");
  Log("\t", readbuffer);
  // last element
  errno = 0;
  TEST_CASE(int curp = sceKernelLseek(fd, -24, 2); curp == 52, "ok:", "fail:", "end-24 (lseek is", curp, "should be:", 52, ") ( errno =", errno, ")");
  errno = 0;
  memset(readbuffer, 0, 256);
  TEST_CASE(int br = sceKernelRead(fd, readbuffer, 256);
            br == 24, "Read successful", "Read failed", "(Read:", br, "should read:", 24, ") ( errno =", errno, ")");
  Log("\t", readbuffer);

  // random stuff
  // drink
  errno = 0;
  sceKernelLseek(fd, 0, 0);
  TEST_CASE(int curp = sceKernelLseek(fd, 34, 1); curp == 34, "ok:", "fail:", "current+34 (lseek is", curp, "should be:", 34, ") ( errno =", errno, ")");
  memset(readbuffer, 0, 256);
  TEST_CASE(int br = sceKernelRead(fd, readbuffer, 5); br == 5, "Read successful", "Read failed", "(Read:", br, "should read:", 5, ") ( errno =", errno, ")");
  Log("\t", readbuffer);

  // water
  errno = 0;
  TEST_CASE(int curp = sceKernelLseek(fd, 6, 1); curp == 45, "ok:", "fail:", "current+6 (lseek is", curp, "should be:", 45, ") ( errno =", errno, ")");
  memset(readbuffer, 0, 256);
  TEST_CASE(int br = sceKernelRead(fd, readbuffer, 5); br == 5, "Read successful", "Read failed", "(Read:", br, "should read:", 5, ") ( errno =", errno, ")");
  Log("\t", readbuffer);
  // remember
  TEST_CASE(int curp = sceKernelLseek(fd, -28, 1); curp == 22, "ok:", "fail:", "current-28 (lseek is", curp, "should be:", 22, ") ( errno =", errno, ")");
  memset(readbuffer, 0, 256);
  TEST_CASE(int br = sceKernelRead(fd, readbuffer, 8); br == 8, "Read successful", "Read failed", "(Read:", br, "should read:", 8, ") ( errno =", errno, ")");
  Log("\t", readbuffer);

  // OOB tests
  // TLDR - outcoming -OOB results in EINVAL, doesn't change ptr. +OOB is a fair game
  errno = 0;
  // absolute, 2137
  TEST_CASE(int curp = sceKernelLseek(fd, 2137, 0);
            curp == 2137, "ok:", "fail:", "origin+2137 (OOB) (lseek is", curp, "should be:", 2137, ") ( errno =", errno, ")");
  errno = 0;
  // error, no change in ptr
  TEST_CASE(sceKernelLseek(fd, -2137, 0) < 0 && errno == 22, "ok:", "fail:", "origin-2137 (OOB) (errno should be 22 - EINVAL) ( errno =", errno, ")");
  errno = 0;
  // n to the front of current ptr, 2137+2137
  TEST_CASE(int curp = sceKernelLseek(fd, 2137, 1);
            curp == 2137 * 2, "ok:", "fail:", "current+2137 (OOB) (lseek is", curp, "should be:", 4274, ") ( errno =", errno, ")");
  errno = 0;
  // n to the back of current ptr, 2137+2137-1234
  TEST_CASE(int curp = sceKernelLseek(fd, -1234, 1);
            curp == 3040, "ok:", "fail:", "current-1234 (OOB) (lseek is", curp, "should be:", 3040, ") ( errno =", errno, ")");
  errno = 0;
  // n to the back of current ptr, into the negative 2137+2137-1234-5000
  TEST_CASE(sceKernelLseek(fd, -5000, 1) < 0 && errno == 22, "ok:", "fail:", "current-5000 (into -OOB) (errno should be 22 - EINVAL) ( errno =", errno, ")");
  errno = 0;
  // n after the end, i.e. size+n
  auto oob_end_pos = GetSize(fd) + 2137;
  TEST_CASE(int curp = sceKernelLseek(fd, 2137, 2);
            curp == oob_end_pos, "ok:", "fail:", "end+2137 (OOB) (lseek is", curp, "should be:", oob_end_pos, ") ( errno =", errno, ")");
  errno = 0;
  // error, no change in ptr
  TEST_CASE(sceKernelLseek(fd, -2137, 2) < 0 && errno == 22, "ok:", "fail:", "end-2137 (OOB) (errno should be 22 - EINVAL) ( errno =", errno, ")");

  if (0 != sceKernelClose(fd)) LogError("Can't close [", path, "]", "( errno =", errno, ")");

  return GetErrorCounter() == 0;
}

int TestOpenFlags(const char* path, int32_t flags, const char* flags_str) {
  LogTest("Testing open() on [", path, "] with flags:", to_hex(flags), "(", flags_str, ")");

  errno      = 0;
  int fd     = sceKernelOpen(path, flags, 0777);
  int _errno = errno;
  // dummy string, some files need text to be created
  sceKernelWrite(fd, "TEST", 4);
  if (fd > -1) sceKernelClose(fd);
  return _errno;
}

bool TestFileRW(const char* path, u16 to_test) {
  LogTest("Testing r/w on [", path, "]");
  ResetErrorCounter();
  if (to_test < 8) {
    LogError("Must test at least 8 bytes!");
    return false;
  }

  int      fd       = -1;
  uint8_t* writebuf = new uint8_t[to_test + 2];
  uint8_t* readbuf  = new uint8_t[to_test + 2];
  memset(writebuf, 0xFF, to_test);
  memset(readbuf, 0xFF, to_test);

  char tmpchr = 'a';
  u16  cnt    = 0;
  while (cnt < to_test) {
    writebuf[cnt++] = tmpchr;
    if (++tmpchr == 'z') tmpchr = 'a';
  }
  writebuf[cnt++] = '\r';
  writebuf[cnt]   = '\n';

  errno = 0;
  fd    = sceKernelOpen(path, 0x2, 0777);
  TEST_CASE(fd > 0, "Opened", "Can't open", "[", path, "]", "( errno =", errno, ")");

  if (fd < 0) return false;

  errno  = 0;
  int bw = sceKernelWrite(fd, writebuf, to_test);
  TEST_CASE(bw == to_test, "Write succeded", "Write failed", "(", bw, " bytes written)", "( errno =", errno, ")");

  struct OrbisKernelStat st {};

  sceKernelFstat(fd, &st);
  // device is always 0 in size
  bool not_a_dev   = !S_ISCHR(st.st_mode);
  auto target_size = st.st_size * not_a_dev;

  errno = 0;
  TEST_CASE(int status = sceKernelLseek(fd, 10, 0); status == 10, "", "", "Lseek ORIGIN+10", "val =", status, "should be", 10, "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(int status = sceKernelLseek(fd, 1, 1); status == 11, "", "", "Lseek CURRENT+1", "val =", status, "should be", 11, "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(int status = sceKernelLseek(fd, -1, 1); status == 10, "", "", "Lseek CURRENT-1", "val =", status, "should be", 10, "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(int status = sceKernelLseek(fd, 0, 2);
            status == target_size, "", "", "Lseek END", "val =", status, "should be", target_size, "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(int status = sceKernelLseek(fd, -1, 2); status == (target_size - (1 * not_a_dev)), "", "", "Lseek END-1", "val =", status, "should be",
                                                    target_size - (1 * not_a_dev), "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(int status = sceKernelLseek(fd, 1, 2);
            status == (target_size + 1), "", "", "Lseek END+1", "val =", status, "should be", target_size + 1, "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(int status = sceKernelLseek(fd, 0, 0); status == 0, "", "", "Lseek ORIGIN+0", "val =", status, "should be", 0, "( errno =", errno, ")");

  errno  = 0;
  int br = sceKernelRead(fd, readbuf, to_test);
  TEST_CASE(br == to_test, "Read succeded", "Read failed", "(", br, " bytes read)", "( errno =", errno, ")")
  Log("Buffers are equal? :", (memcmp(writebuf, readbuf, to_test) == 0 ? "yes" : "no"));

  if (bw > 0) Log("Write preview:", right(STR(writebuf[0]), 3), right(STR(writebuf[1]), 3), right(STR(writebuf[2]), 3), right(STR(writebuf[3]), 3));
  if (br > 0) Log("Read  preview:", right(STR(readbuf[0]), 3), right(STR(readbuf[1]), 3), right(STR(readbuf[2]), 3), right(STR(readbuf[3]), 3));

  bool all_zeros = true;
  for (u16 i = 0; i < to_test; i++) {
    if (readbuf[i] == 0) continue;
    all_zeros = false;
    break;
  }

  Log("Is reading buffer full of zeros? :", (all_zeros ? "yes" : "no"));

  delete[] writebuf;
  delete[] readbuf;

  return GetErrorCounter() == 0;
}

bool TestFileTouch(const char* path) {
  LogTest("Cursed file operations: [", path, "]");
  ResetErrorCounter();
  OrbisKernelStat st {};

  errno = 0;
  TEST_CASE(0 == touch(path), "touched", "can'touch", "( errno =", errno, ")");

  errno = 0;
  TEST_CASE(0 == sceKernelStat(path, &st), "stat success", "stat fail", "( errno =", errno, ")")

  return GetErrorCounter() == 0;
}

bool PrepareCursedFileop(void) {
  Obliterate("/data/therapist/tmp_cursed");

  LogTest("Cursed file operations");
  bool is_ok = true;

  is_ok &= (0 == sceKernelMkdir("/data/therapist/tmp_cursed", 0777));
  is_ok &= (0 == sceKernelMkdir("/data/therapist/tmp_cursed/Aursed", 0777));
  is_ok &= (0 == sceKernelMkdir("/data/therapist/tmp_cursed/Aursed/AAursed", 0777));
  is_ok &= (0 == sceKernelMkdir("/data/therapist/tmp_cursed/Bursed", 0777));
  is_ok &= (0 == sceKernelMkdir("/data/therapist/tmp_cursed/Bursed/BBursed", 0777));
  is_ok &= (0 == sceKernelMkdir("/data/therapist/tmp_cursed/Cursed", 0777));
  is_ok &= (0 == sceKernelMkdir("/data/therapist/tmp_cursed/Cursed/CCursed", 0777));

  return is_ok;
}

bool TestStat(fs::path path, const OrbisKernelStat* original) {
  LogTest("Testing stat on [", path, "]");
  OrbisKernelStat st {};

  errno = 0;
  TEST_CASE(0 == sceKernelStat(path.c_str(), &st), "Stat successful", "Stat failed", "[", path, "]", "( errno =", errno, ")");

  if (nullptr == original) {
    Log("No comparsiton target provided for [", path, "]. Dumping your own :)");
    PrintStatInfo(&st);
    return true;
  }

  return StatCmp(&st, original);
}

bool TestLStat(fs::path path) {
  LogTest("Testing lstat on [", path, "]");
  struct stat st {};

  errno = 0;

  return 0 == lstat(path.c_str(), &st);
}

bool RegenerateDir(const char* path) {
  Obliterate(path);
  sceKernelMkdir(path, 0777);
  return true;
}

bool TestDirEnts() {
  //
  // Setup
  //
  LogTest("Testing dirents");

  s32  result = 0;
  char prebuffer[DIRENT_BUFFER_SIZE] {0};
  char postbuffer[DIRENT_BUFFER_SIZE] {0};

  ResetErrorCounter();

  //
  // Creating target directory
  //

  errno = 0;
  TEST_CASE(sceKernelMkdir("/data/therapist/tmp_dirent", 0777) == 0x80020011, "Creating [ /data/therapist/tmp_dirent ] failed (EEXIST)",
            "Created [ /data/therapist/tmp_dirent ] even though it existed before", "( errno =", errno, ")");

  //
  // Test opening first
  //

  errno  = 0;
  int fd = sceKernelOpen("/data/therapist/tmp_dirent", 0, 0777);
  TEST_CASE(fd > -1, "open() succeeded on", "can't open", "[ /data/therapist/tmp_dirent ] ( errno =", errno, ")");

  errno = 0;
  TEST_CASE(sceKernelClose(fd) > -1, "Closed", "Can't close", "[ /data/therapist/tmp_dirent ]", "( errno =", errno, ")");

  //
  // Create dummy files, read dir
  //

  errno             = 0;
  int tmp_creat_cnt = 0;
  // basically creat(), but it's not available in sceKernel
  tmp_creat_cnt += touch("/data/therapist/tmp_dirent/dummy_file_1");
  tmp_creat_cnt += touch("/data/therapist/tmp_dirent/dummy_file_2");
  tmp_creat_cnt += touch("/data/therapist/tmp_dirent/dummy_file_3");
  tmp_creat_cnt += touch("/data/therapist/tmp_dirent/dummy_file_4");
  TEST_CASE(tmp_creat_cnt == 0, "Dummy files (1-4) created", "Can't create dummy files in [ /data/therapist/tmp_dirent ]", "( errno =", errno, ")");

  errno = 0;
  fd    = sceKernelOpen("/data/therapist/tmp_dirent", 0, 0777);
  TEST_CASE(fd > -1, "open() succeeded on", "can't open", "[ /data/therapist/tmp_dirent ] ( errno =", errno, ")");

  errno = 0;
  TEST_CASE(sceKernelLseek(fd, 0, 0) == 0, "Seek to start", "Seek failed", "(dirents, pre)", "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(int status = sceKernelGetdirentries(fd, prebuffer, DIRENT_BUFFER_SIZE, nullptr);
            status == DIRENT_BUFFER_SIZE, "Direntries read correctly", "Can't get direntries", "( read bytes:", status, ")", "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(sceKernelLseek(fd, 0, 0) == 0, "Seek to start", "Seek failed", "(dump, pre)", "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(int last_reclen = dumpDir(fd); last_reclen == (DIRENT_BUFFER_SIZE - 96), "Read correct amount of data", "Last dirent has incorrect value",
                                           "(dump, pre)", "( last_reclen =", last_reclen, "should be =", DIRENT_BUFFER_SIZE - 96, ")", "( errno =", errno, ")");
  Log("You should see 4 dummy files here");

  //
  // Create more dummy files while dir is open
  //

  errno         = 0;
  tmp_creat_cnt = 0;
  tmp_creat_cnt += touch("/data/therapist/tmp_dirent/dummy_file_5");
  tmp_creat_cnt += touch("/data/therapist/tmp_dirent/dummy_file_6");
  tmp_creat_cnt += touch("/data/therapist/tmp_dirent/dummy_file_7");
  tmp_creat_cnt += touch("/data/therapist/tmp_dirent/dummy_file_8");
  TEST_CASE(tmp_creat_cnt == 0, "Dummy files (5-8) created", "Can't create dummy files in [ /data/therapist/tmp_dirent ]", "( errno =", errno, ")");

  errno = 0;
  TEST_CASE(sceKernelLseek(fd, 0, 0) == 0, "Seek to start", "Seek failed", "(dirents, post)", "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(int status = sceKernelGetdirentries(fd, postbuffer, DIRENT_BUFFER_SIZE, nullptr);
            status == DIRENT_BUFFER_SIZE, "Direntries read correctly", "Can't get direntries", "( read bytes:", status, ")", "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(sceKernelLseek(fd, 0, 0) == 0, "Seek to start", "Seek failed", "(dump, post)", "( errno =", errno, ")");
  errno = 0;
  TEST_CASE(int last_reclen = dumpDir(fd); last_reclen == (DIRENT_BUFFER_SIZE - 192), "Read correct amount of data", "Last dirent has incorrect value",
                                           "(dump, post)", "( last_reclen =", last_reclen, "should be =", DIRENT_BUFFER_SIZE - 192, ")", "( errno =", errno,
                                           ")");
  Log("You should see 8 dummy files here");

  //
  // Cave Johnson. We're done here
  //

  errno = 0;
  TEST_CASE(sceKernelClose(fd) != -1, "Closed", "Can't close", "[ /data/therapist/tmp_dirent ]", "( errno =", errno, ")");

  return GetErrorCounter() == 0;

  // // compare before and after opening

  // LogFilesystem("Creating a new file...\n");
  // sceKernelClose(sceKernelOpen("/data/therapist/tmp/2created_after_opening", (int32_t)OpenFlags::Create, 0644));

  // sceKernelLseek(tmpfd, 0, 0);
  // result = sceKernelGetdirentries(tmpfd, buffer, BUFFER_SIZE, nullptr);
  // LogFilesystemStatus("Printed entries (%lu bytes)\n", ASSERT(printContents(buffer, result)), result);

  // auto q = memcmp(prebuffer, buffer, BUFFER_SIZE);
  // LogFilesystemStatus("Comparing dirent buffers\n", q != 0);

  // sceKernelClose(tmpfd);

  // {
  //     LogFilesystemStatus("Opening directory again after creating a file\n", Test::TEST);
  //     tmpfd = sceKernelOpen("/data/therapist/tmp_dirent", 0, 0777);
  //     LogFilesystemStatus("Printed entries (%lu bytes)\n", ASSERT(printContents(prebuffer, result)), result);
  //     sceKernelClose(tmpfd);
  // }

  // // timing tests

  // char tmpname[24]{0};
  // const char *tftemplate = "/data/therapist/tmp/XD%d";

  // tmpfd = sceKernelOpen("/data/therapist/tmp_dirent", 0, 0644);
  // result = sceKernelGetdirentries(tmpfd, prebuffer, BUFFER_SIZE, nullptr);

  // for (u16 idx = 0; idx < TESTFILES; idx++)
  // {
  //     snprintf(tmpname, 24, tftemplate, idx);
  //     sceKernelClose(sceKernelOpen(tmpname, (int32_t)OpenFlags::Create, 0644));
  // }

  // for (u16 midx = 0; midx < 10; midx++)
  // {
  //     auto bench_start = std::chrono::high_resolution_clock::now();
  //     for (u16 idx = 0; idx < TIMING_ITERATIONS; idx++)
  //     {
  //         sceKernelLseek(tmpfd, 0, 0);
  //         sceKernelGetdirentries(tmpfd, buffer, BUFFER_SIZE, &dirents_index);
  //     }
  //     auto bench_end = std::chrono::high_resolution_clock::now();
  //     auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(bench_end - bench_start).count();
  //     unsigned long long avgtime = duration / TIMING_ITERATIONS / 1000;
  //     LogFilesystem("Refreshing %d files %d times (no changes): avg %llu us/it\n", TESTFILES, TIMING_ITERATIONS, avgtime);
  // }

  // // test removing files sequentially

  // {
  //     result = sceKernelGetdirentries(tmpfd, buffer, BUFFER_SIZE, nullptr);

  //     auto bench_start = std::chrono::high_resolution_clock::now();
  //     for (u16 idx = 0; idx < TESTFILES - 1; idx++)
  //     {
  //         snprintf(tmpname, 24, tftemplate, idx);
  //         sceKernelUnlink(tmpname);

  //         sceKernelLseek(tmpfd, 0, 0);
  //         sceKernelGetdirentries(tmpfd, buffer, BUFFER_SIZE, &dirents_index);
  //     }
  //     auto bench_end = std::chrono::high_resolution_clock::now();
  //     unsigned long long duration = std::chrono::duration_cast<std::chrono::nanoseconds>(bench_end - bench_start).count();
  //     LogFilesystem("Removed %d files in %llu us\n", TESTFILES - 1, duration / 1000);
  // }

  // sceKernelClose(tmpfd);
}

bool TestRelatives(fs::path path, bool expected_mountpoint) {
  LogTest("Testing relative fileno perception of [", path.string(), "]");

  const std::string path_str = path.string();
  std::string       leafstr  = path.filename().string();
  oi::FolderDirent  leaf {};

  s64 leaf_to_self {-2};
  s64 parent_to_leaf {-2};
  s64 leaf_to_parent {-2};

  u8 ret {};

  ResetErrorCounter();

  //
  // test self (2 if mountpoint root)
  ret = GetDir(path, ".", &leaf);
  if (ret <= 0) {
    LogError("Cannot get [ . ] from", path_str);
  } else
    leaf_to_self = leaf.d_fileno;

  //
  // test self-parent path (2 if mountpoint root)
  ret = GetDir(path, "..", &leaf);
  if (ret <= 0) {
    LogError("Cannot get [ .. ] from", path_str);
  } else
    leaf_to_parent = leaf.d_fileno;

  //
  // test how parent sees the child
  if (!leafstr.empty()) {
    ret = GetDir(path.parent_path(), (path == "/" ? "/" : leafstr), &leaf);
    if (ret <= 0) {
      LogError("Cannot get [", leafstr, "] from", path_str);
    } else
      parent_to_leaf = leaf.d_fileno;
  }

  Log(path_str, "sees itself with fileno=", leaf_to_self);
  Log(path_str, "sees its parent with fileno=", leaf_to_parent);
  Log(path_str, "is seen with fileno=", parent_to_leaf);

  bool is_mountpoint = leaf_to_self != parent_to_leaf && leaf_to_self != -2 && leaf_to_parent != -2 && parent_to_leaf != -2;
  if (expected_mountpoint) {
    TEST_CASE(is_mountpoint == expected_mountpoint, "It's a mountpoint", "It's not a mountpoint");
  } else {
    Log(is_mountpoint ? "It's a mountpoint" : "It's not a mountpoint");
  }

  return GetErrorCounter() == 0;
}

bool CompareNormalVsPFS(fs::path path, fs::path leaf, s32 expected_normal_reclen, s32 expected_pfs_reclen) {
  oi::PfsDirent    pfs_dir {};
  oi::FolderDirent normal_dir {};
  const char*      path_str      = path.c_str();
  const char*      leaf_str      = leaf.c_str();
  fs::path         q             = path / leaf;
  const char*      path_full_str = q.c_str();

  ResetErrorCounter();

  LogTest("Compare Normal and PFS dirent for", path_full_str);
  if (GetDir(path, leaf, &normal_dir) == -1) {
    LogError("Can't open", path_full_str, "as normal");
  }
  if (GetDir(path, leaf, &pfs_dir) == -1) {
    LogError("Can't open", path_full_str, "as PFS");
  }

  // return early, there's no point in trying if can't open either
  if (GetErrorCounter() != 0) {
    return false;
  }

  bool expr;

  expr = normal_dir.d_fileno == pfs_dir.d_fileno;

  {
    std::stringstream ss;
    ss << path_full_str << " fileno " << normal_dir.d_fileno << (expr ? " == " : " != ") << pfs_dir.d_fileno;
    TEST_CASE(expr, ss.str(), ss.str());
  }

  expr = normal_dir.d_namlen == pfs_dir.d_namlen;

  {
    std::stringstream ss;
    ss << path_full_str << " namlen " << STR(normal_dir.d_namlen) << (expr ? " == " : " != ") << pfs_dir.d_namlen;
    TEST_CASE(expr, ss.str(), ss.str());
  }

  if (expected_normal_reclen == -1 || expected_pfs_reclen == -1)
    expr = normal_dir.d_reclen == pfs_dir.d_reclen;
  else
    expr = (normal_dir.d_reclen == expected_normal_reclen) && (pfs_dir.d_reclen == expected_pfs_reclen);

  // may not be equal, hence expected values
  {
    std::stringstream ss;
    ss << path_full_str << " reclen " << normal_dir.d_reclen << (expr ? " == " : " != ") << pfs_dir.d_reclen;
    TEST_CASE(expr, ss.str(), ss.str());
  }

  expr = memcmp(normal_dir.d_name, pfs_dir.d_name, normal_dir.d_namlen) == 0;
  TEST_CASE(expr, "Names are equal", "Names are not equal", "(memcmp)")

  if (!expr) {
    // just in case
    normal_dir.d_name[255] = 0;
    pfs_dir.d_name[255]    = 0;
    Log("Normal:\t", normal_dir.d_name);
    Log("PFS:\t", pfs_dir.d_name);
  }

  return GetErrorCounter() == 0;
}
} // namespace FS_Test