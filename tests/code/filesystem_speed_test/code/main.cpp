#include "fs_test.h"
#include "log.h"

#include <orbis/SystemService.h>

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

  sceSystemServiceLoadExec("EXIT", nullptr);
  return 0;
}
