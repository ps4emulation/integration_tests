#include <CppUTest/CommandLineTestRunner.h>
#include <orbis/SystemService.h>

IMPORT_TEST_GROUP(MemoryTests);

int main(int ac, char** av) {
  // No buffering
  setvbuf(stdout, NULL, _IONBF, 0);
  int result = RUN_ALL_TESTS(ac, av);
  sceSystemServiceLoadExec("EXIT", nullptr);
  return result;
}
