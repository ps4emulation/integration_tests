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

  // Run file system tests
  RegenerateDir("/data/enderman");
  sceKernelMkdir("/data/enderman/dumps", 0777);
  int result = RUN_ALL_TESTS(ac, av);
  RunTests();

  // Log tests end
  Log();
  Log("<<<< TESTS END >>>>");
  Log();

  sceSystemServiceLoadExec("EXIT", nullptr);
  return result;
}
