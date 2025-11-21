#include "CppUTest/CommandLineTestRunner.h"
#include "fs_test.h"
#include "log.h"

#include <cstdint>
#include <orbis/SystemService.h>
#include <sstream>

int main(int ac, char** av) {
  // No buffering
  setvbuf(stdout, NULL, _IONBF, 0);

  // Log tests start
  Log();
  Log("<<<< TESTS START >>>>");
  Log();

  // Run file system tests
  FS_Test::RunTests();

  // Log tests end
  Log();
  Log("<<<< TESTS END >>>>");
  Log();

  return 0;

  // int result = RUN_ALL_TESTS(ac, av);
  // sceSystemServiceLoadExec("EXIT", nullptr);
  // return result;
}
