#include "CppUTest/CommandLineTestRunner.h"

#include <cstdint>
#include <orbis/SystemService.h>
#include <sstream>

IMPORT_TEST_GROUP(EventTest);

int main(int ac, char** av) {
  // No buffering
  setvbuf(stdout, NULL, _IONBF, 0);
  int result = RUN_ALL_TESTS(ac, av);
  sceSystemServiceLoadExec("EXIT", nullptr);
  return result;
}
