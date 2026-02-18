#include "fs_test.h"

#include <cmath>
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

#define ITER_ACTION 2500
#define ITER_READS  5
#define ITER_WRITES 5

// chunk = 8, order = 1 -> 8b, multiples - how many chunks to write for testing
// =4, =3 -> 4MB
void testWrite(u64 base, u64 order, u64 multiples, const char* path) {
  u64 test_start        = 0;
  u64 test_end          = 0;
  u64 write_test_chunk  = base * (std::pow(1024, order));
  u64 write_test_bytes  = multiples * write_test_chunk;
  u64 write_test_start  = 0;
  u64 write_test_total  = 0;
  u8* write_test_buffer = new u8[write_test_chunk] {255};

  char* size_str = "xB";
  switch (order) {
    case 0: size_str[0] = 'b'; break;
    case 1: size_str[0] = 'k'; break;
    case 2: size_str[0] = 'M'; break;
    case 3: size_str[0] = 'G'; break;
  }

  test_start = tick();
  for (auto i = ITER_WRITES; i; i--) {
    sceKernelUnlink(path);
    int fd           = sceKernelOpen(path, O_WRONLY | O_CREAT, 0777);
    write_test_start = tick();
    for (u32 i = 0; i < write_test_bytes; i += write_test_chunk) {
      sceKernelWrite(fd, write_test_buffer, write_test_chunk);
    }
    sceKernelFsync(fd);
    write_test_total += tick() - write_test_start;
  }
  test_end = tick();

  double write_test_throughput = ITER_WRITES * double(write_test_bytes) / double(write_test_total);
  Log("write() in /data:", write_test_total / (ITER_WRITES * 1000), "us/it");
  Log("write+sync speed:", write_test_throughput * 1000000000.0 / (1024 * 1024), "MB/s");
  Log("write+sync duration:", write_test_total / 1000000000.0, "s");
  Log("Test duration:", (test_end - test_start) / 1000000000.0, "s");
  delete[] write_test_buffer;
}

void RunTests() {
  RegenerateDir("/data/amphitheathre");

  Log();
  Log("<<<< TEST SUITE STARTING >>>>");
  Log("Function iterations:", ITER_ACTION);
  Log("Read iterations:", ITER_READS);
  Log("Write iterations:", ITER_WRITES);

  u64    test_start    = 0;
  u64    test_end      = 0;
  double test_duration = 0;
  int    fd            = 0;

  Log();
  Log("\t<<<< open() >>>>");
  Log();

  u64 open_start = 0;
  u64 open_total = 0;
  test_start     = tick();
  touch("/data/amphitheathre/open_file");
  for (auto i = ITER_ACTION; i; i--) {
    open_start = tick();
    fd         = sceKernelOpen("/data/amphitheathre/open_file", O_RDONLY, 0777);
    open_total += tick() - open_start;
    sceKernelClose(fd);
  }
  test_end      = tick();
  test_duration = (test_end - test_start) / 1000;
  Log("open() in /data, one file:", open_total / ITER_ACTION, "ns/it");
  Log("Action duration:", open_total / 1000, "us");
  Log("Test duration:", test_duration, "us");

  Log();
  Log("\t<<<< write() 4KB speed test >>>>");
  Log("\tNormalized to 1GB") Log();
  testWrite(4, 1, 262144, "/data/amphitheathre/write_benchmark_4kB");
  Log();
  Log("\t<<<< write() 8KB speed test >>>>");
  Log("\tNormalized to 1GB") Log();
  testWrite(8, 1, 131072, "/data/amphitheathre/write_benchmark_8kB");
  Log();
  Log("\t<<<< write() 64kB speed test >>>>");
  Log("\tNormalized to 1GB") Log();
  testWrite(64, 1, 16384, "/data/amphitheathre/write_benchmark_64kB");
  Log();
  Log("\t<<<< write() 1MB speed test >>>>");
  Log("\tNormalized to 1GB") Log();
  testWrite(1, 2, 1024, "/data/amphitheathre/write_benchmark_1MB");
  Log();
  Log("\t<<<< write() 4MB speed test >>>>");
  Log("\tNormalized to 1GB") Log();
  testWrite(4, 2, 256, "/data/amphitheathre/write_benchmark_4MB");
  Log();
  Log("\t<<<< write() 8MB speed test >>>>");
  Log("\tNormalized to 1GB") Log();
  testWrite(8, 2, 128, "/data/amphitheathre/write_benchmark_8MB");
}
} // namespace FS_Test