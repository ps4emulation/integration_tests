#include "CppUTest/TestHarness.h"

#pragma clang diagnostic ignored "-Wformat"

TEST_GROUP(InsertqTests) {void setup() {

} void teardown() {

}};

extern "C" {
void longs_equal(long a, long b) { // export
  LONGS_EQUAL(a, b);
}

void insertq_const_test(); // import

extern const long table_value[256][256]; // import
}

TEST(InsertqTests, Const) {

  printf("--TEST:[insertq xmm0,xmm1,0xXX,0xXX]--\n");

  insertq_const_test();

  printf("--STOP--\n");
}

// Insert field starting at bit 0 of xmm1 with the length
// specified by xmm1[69:64]. This field is inserted into
// xmm0 starting at the bit position specified by
// xmm1[77:72].

//[0..63,(64..69),70..71,(72..77)]

#define INSERTQ_XMM(len, pos)                                                                                                                                  \
  {                                                                                                                                                            \
    int64_t value[2] = {-1, -1};                                                                                                                               \
    int64_t masks[2] = {0, (len) | (pos << 8)};                                                                                                                \
    asm("movups %1, %%xmm0;"                                                                                                                                   \
        "movups %2, %%xmm1;"                                                                                                                                   \
        ".byte 0x66;"                                                                                                                                          \
        "insertq %%xmm1, %%xmm0;"                                                                                                                              \
        "movups %%xmm0, %0;"                                                                                                                                   \
        : "=m"(value)            /* output */                                                                                                                  \
        : "m"(value), "m"(masks) /* input */                                                                                                                   \
        : "%xmm0", "%xmm1"       /* clobbered register */                                                                                                      \
    );                                                                                                                                                         \
    LONGS_EQUAL(table_value[pos][len], value[0]);                                                                                                              \
    LONGS_EQUAL(0, value[1]);                                                                                                                                  \
    if ((table_value[pos][len] != value[0]) || (value[1] != 0)) {                                                                                              \
      printf("FAILED: len,pos: 0x%02x:0x%02x"                                                                                                                  \
             " out: 0x%016llX:0x%016llX != 0x%016llX:0x%016llX \n",                                                                                            \
             (len), (pos), (value[0]), (value[1]), (table_value[pos][len]), 0);                                                                                \
    }                                                                                                                                                          \
  }

TEST(InsertqTests, Xmm) {

  printf("--TEST:[insertq xmm0,xmm1]--\n");

  for (int len = 0; len <= 255; len++)
    for (int pos = 0; pos <= 255; pos++) {
      INSERTQ_XMM(len, pos);
    }

  printf("--STOP--\n");
}
