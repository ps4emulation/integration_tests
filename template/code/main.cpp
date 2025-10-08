#include "CppUTest/CommandLineTestRunner.h"

#include <cstdint>
#include <sstream>

int main(int ac, char** av) {
  // No buffering
  setvbuf(stdout, NULL, _IONBF, 0);
  return RUN_ALL_TESTS(ac, av);
}
