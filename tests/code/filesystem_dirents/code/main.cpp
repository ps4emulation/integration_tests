#include "fs_test.h"
#include "log.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <orbis/SystemService.h>

IMPORT_TEST_GROUP(DirentTests);

int main(int ac, char** av) {
  // No buffering
  setvbuf(stdout, NULL, _IONBF, 0);

  // Log tests start
  Log();
  Log("<<<< TESTS START >>>>");
  Log();

  // prepare files
  RegenerateDir("/data/enderman");
  sceKernelMkdir("/data/enderman/dumps", 0777);
  std::string nf_path        = "/data/enderman/filewithaverylongname";
  std::string nf_path_longer = "/data/enderman/filewithunnecesarilylongnamejusttomesswitheveryone";
  char        nf_num[4] {0};
  for (u8 idx = 1; idx <= 50; idx++) {
    snprintf(nf_num, 4, "%02d", idx);
    touch(nf_path + std::string(nf_num));
    touch(nf_path_longer + std::string(nf_num));
  }

  // Run file system tests
  int result = RUN_ALL_TESTS(ac, av);
  RunTests();

  // Log tests end
  Log();
  Log("<<<< TESTS END >>>>");
  Log();

  sceSystemServiceLoadExec("EXIT", nullptr);
  return result;
}
