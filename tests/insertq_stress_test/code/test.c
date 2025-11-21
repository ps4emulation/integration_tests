
#pragma clang diagnostic ignored "-Wformat"

#include "table_value.inc"

#include <stdint.h>
#include <stdio.h>

extern void longs_equal(long a, long b); // import

#define LONGS_EQUAL(a, b) longs_equal(a, b)

// Insert field starting at bit 0 of xmm1 with the length
// specified by [5:0] of the first immediate byte. This
// field is inserted into xmm0 starting at the bit position
// specified by [5:0] of the second immediate byte.

#define INSERTQ_CONST(len, pos)                                                                                                                                \
  if (((len + pos) >= 64) || (len >= 64) || (len == 0) || (pos >= 64) || (pos == 0)) {                                                                         \
    int64_t value[2] = {-1, -1};                                                                                                                               \
    asm("movups %1, %%xmm0;"                                                                                                                                   \
        "xorps %%xmm1, %%xmm1;"                                                                                                                                \
        "insertq %3, %2, %%xmm1, %%xmm0;"                                                                                                                      \
        "movups %%xmm0, %0;"                                                                                                                                   \
        : "=m"(value)                    /* output */                                                                                                          \
        : "m"(value), "g"(len), "g"(pos) /* input */                                                                                                           \
        : "%xmm0", "%xmm1"               /* clobbered register */                                                                                              \
    );                                                                                                                                                         \
    LONGS_EQUAL(table_value[pos][len], value[0]);                                                                                                              \
    LONGS_EQUAL(0, value[1]);                                                                                                                                  \
    if ((table_value[pos][len] != value[0]) || (value[1] != 0)) {                                                                                              \
      printf("FAILED: len,pos: 0x%02x:0x%02x"                                                                                                                  \
             " out: 0x%016llX:0x%016llX != 0x%016llX:0x%016llX \n",                                                                                            \
             (len), (pos), (value[0]), (value[1]), (table_value[pos][len]), 0);                                                                                \
    }                                                                                                                                                          \
  }

#define INSERTQ_CONST_SHIFT_0(mask)                                                                                                                            \
  {                                                                                                                                                            \
    INSERTQ_CONST(0x00, (mask));                                                                                                                               \
    INSERTQ_CONST(0x01, (mask));                                                                                                                               \
    INSERTQ_CONST(0x02, (mask));                                                                                                                               \
    INSERTQ_CONST(0x03, (mask));                                                                                                                               \
    INSERTQ_CONST(0x04, (mask));                                                                                                                               \
    INSERTQ_CONST(0x05, (mask));                                                                                                                               \
    INSERTQ_CONST(0x06, (mask));                                                                                                                               \
    INSERTQ_CONST(0x07, (mask));                                                                                                                               \
    INSERTQ_CONST(0x08, (mask));                                                                                                                               \
    INSERTQ_CONST(0x09, (mask));                                                                                                                               \
    INSERTQ_CONST(0x0A, (mask));                                                                                                                               \
    INSERTQ_CONST(0x0B, (mask));                                                                                                                               \
    INSERTQ_CONST(0x0C, (mask));                                                                                                                               \
    INSERTQ_CONST(0x0D, (mask));                                                                                                                               \
    INSERTQ_CONST(0x0E, (mask));                                                                                                                               \
    INSERTQ_CONST(0x0F, (mask));                                                                                                                               \
    INSERTQ_CONST(0x10, (mask));                                                                                                                               \
    INSERTQ_CONST(0x11, (mask));                                                                                                                               \
    INSERTQ_CONST(0x12, (mask));                                                                                                                               \
    INSERTQ_CONST(0x13, (mask));                                                                                                                               \
    INSERTQ_CONST(0x14, (mask));                                                                                                                               \
    INSERTQ_CONST(0x15, (mask));                                                                                                                               \
    INSERTQ_CONST(0x16, (mask));                                                                                                                               \
    INSERTQ_CONST(0x17, (mask));                                                                                                                               \
    INSERTQ_CONST(0x18, (mask));                                                                                                                               \
    INSERTQ_CONST(0x19, (mask));                                                                                                                               \
    INSERTQ_CONST(0x1A, (mask));                                                                                                                               \
    INSERTQ_CONST(0x1B, (mask));                                                                                                                               \
    INSERTQ_CONST(0x1C, (mask));                                                                                                                               \
    INSERTQ_CONST(0x1D, (mask));                                                                                                                               \
    INSERTQ_CONST(0x1E, (mask));                                                                                                                               \
    INSERTQ_CONST(0x1F, (mask));                                                                                                                               \
    INSERTQ_CONST(0x20, (mask));                                                                                                                               \
    INSERTQ_CONST(0x21, (mask));                                                                                                                               \
    INSERTQ_CONST(0x22, (mask));                                                                                                                               \
    INSERTQ_CONST(0x23, (mask));                                                                                                                               \
    INSERTQ_CONST(0x24, (mask));                                                                                                                               \
    INSERTQ_CONST(0x25, (mask));                                                                                                                               \
    INSERTQ_CONST(0x26, (mask));                                                                                                                               \
    INSERTQ_CONST(0x27, (mask));                                                                                                                               \
    INSERTQ_CONST(0x28, (mask));                                                                                                                               \
    INSERTQ_CONST(0x29, (mask));                                                                                                                               \
    INSERTQ_CONST(0x2A, (mask));                                                                                                                               \
    INSERTQ_CONST(0x2B, (mask));                                                                                                                               \
    INSERTQ_CONST(0x2C, (mask));                                                                                                                               \
    INSERTQ_CONST(0x2D, (mask));                                                                                                                               \
    INSERTQ_CONST(0x2E, (mask));                                                                                                                               \
    INSERTQ_CONST(0x2F, (mask));                                                                                                                               \
    INSERTQ_CONST(0x30, (mask));                                                                                                                               \
    INSERTQ_CONST(0x31, (mask));                                                                                                                               \
    INSERTQ_CONST(0x32, (mask));                                                                                                                               \
    INSERTQ_CONST(0x33, (mask));                                                                                                                               \
    INSERTQ_CONST(0x34, (mask));                                                                                                                               \
    INSERTQ_CONST(0x35, (mask));                                                                                                                               \
    INSERTQ_CONST(0x36, (mask));                                                                                                                               \
    INSERTQ_CONST(0x37, (mask));                                                                                                                               \
    INSERTQ_CONST(0x38, (mask));                                                                                                                               \
    INSERTQ_CONST(0x39, (mask));                                                                                                                               \
    INSERTQ_CONST(0x3A, (mask));                                                                                                                               \
    INSERTQ_CONST(0x3B, (mask));                                                                                                                               \
    INSERTQ_CONST(0x3C, (mask));                                                                                                                               \
    INSERTQ_CONST(0x3D, (mask));                                                                                                                               \
    INSERTQ_CONST(0x3E, (mask));                                                                                                                               \
    INSERTQ_CONST(0x3F, (mask));                                                                                                                               \
  }

#define INSERTQ_CONST_SHIFT_1(mask)                                                                                                                            \
  {                                                                                                                                                            \
    INSERTQ_CONST(0xC0, (mask));                                                                                                                               \
    INSERTQ_CONST(0xC1, (mask));                                                                                                                               \
    INSERTQ_CONST(0xC2, (mask));                                                                                                                               \
    INSERTQ_CONST(0xC3, (mask));                                                                                                                               \
    INSERTQ_CONST(0xC4, (mask));                                                                                                                               \
    INSERTQ_CONST(0xC5, (mask));                                                                                                                               \
    INSERTQ_CONST(0xC6, (mask));                                                                                                                               \
    INSERTQ_CONST(0xC7, (mask));                                                                                                                               \
    INSERTQ_CONST(0xC8, (mask));                                                                                                                               \
    INSERTQ_CONST(0xC9, (mask));                                                                                                                               \
    INSERTQ_CONST(0xCA, (mask));                                                                                                                               \
    INSERTQ_CONST(0xCB, (mask));                                                                                                                               \
    INSERTQ_CONST(0xCC, (mask));                                                                                                                               \
    INSERTQ_CONST(0xCD, (mask));                                                                                                                               \
    INSERTQ_CONST(0xCE, (mask));                                                                                                                               \
    INSERTQ_CONST(0xCF, (mask));                                                                                                                               \
    INSERTQ_CONST(0xD0, (mask));                                                                                                                               \
    INSERTQ_CONST(0xD1, (mask));                                                                                                                               \
    INSERTQ_CONST(0xD2, (mask));                                                                                                                               \
    INSERTQ_CONST(0xD3, (mask));                                                                                                                               \
    INSERTQ_CONST(0xD4, (mask));                                                                                                                               \
    INSERTQ_CONST(0xD5, (mask));                                                                                                                               \
    INSERTQ_CONST(0xD6, (mask));                                                                                                                               \
    INSERTQ_CONST(0xD7, (mask));                                                                                                                               \
    INSERTQ_CONST(0xD8, (mask));                                                                                                                               \
    INSERTQ_CONST(0xD9, (mask));                                                                                                                               \
    INSERTQ_CONST(0xDA, (mask));                                                                                                                               \
    INSERTQ_CONST(0xDB, (mask));                                                                                                                               \
    INSERTQ_CONST(0xDC, (mask));                                                                                                                               \
    INSERTQ_CONST(0xDD, (mask));                                                                                                                               \
    INSERTQ_CONST(0xDE, (mask));                                                                                                                               \
    INSERTQ_CONST(0xDF, (mask));                                                                                                                               \
    INSERTQ_CONST(0xE0, (mask));                                                                                                                               \
    INSERTQ_CONST(0xE1, (mask));                                                                                                                               \
    INSERTQ_CONST(0xE2, (mask));                                                                                                                               \
    INSERTQ_CONST(0xE3, (mask));                                                                                                                               \
    INSERTQ_CONST(0xE4, (mask));                                                                                                                               \
    INSERTQ_CONST(0xE5, (mask));                                                                                                                               \
    INSERTQ_CONST(0xE6, (mask));                                                                                                                               \
    INSERTQ_CONST(0xE7, (mask));                                                                                                                               \
    INSERTQ_CONST(0xE8, (mask));                                                                                                                               \
    INSERTQ_CONST(0xE9, (mask));                                                                                                                               \
    INSERTQ_CONST(0xEA, (mask));                                                                                                                               \
    INSERTQ_CONST(0xEB, (mask));                                                                                                                               \
    INSERTQ_CONST(0xEC, (mask));                                                                                                                               \
    INSERTQ_CONST(0xED, (mask));                                                                                                                               \
    INSERTQ_CONST(0xEE, (mask));                                                                                                                               \
    INSERTQ_CONST(0xEF, (mask));                                                                                                                               \
    INSERTQ_CONST(0xF0, (mask));                                                                                                                               \
    INSERTQ_CONST(0xF1, (mask));                                                                                                                               \
    INSERTQ_CONST(0xF2, (mask));                                                                                                                               \
    INSERTQ_CONST(0xF3, (mask));                                                                                                                               \
    INSERTQ_CONST(0xF4, (mask));                                                                                                                               \
    INSERTQ_CONST(0xF5, (mask));                                                                                                                               \
    INSERTQ_CONST(0xF6, (mask));                                                                                                                               \
    INSERTQ_CONST(0xF7, (mask));                                                                                                                               \
    INSERTQ_CONST(0xF8, (mask));                                                                                                                               \
    INSERTQ_CONST(0xF9, (mask));                                                                                                                               \
    INSERTQ_CONST(0xFA, (mask));                                                                                                                               \
    INSERTQ_CONST(0xFB, (mask));                                                                                                                               \
    INSERTQ_CONST(0xFC, (mask));                                                                                                                               \
    INSERTQ_CONST(0xFD, (mask));                                                                                                                               \
    INSERTQ_CONST(0xFE, (mask));                                                                                                                               \
    INSERTQ_CONST(0xFF, (mask));                                                                                                                               \
  }

void insertq_const_test() { // export
  INSERTQ_CONST_SHIFT_0(0x00);
  INSERTQ_CONST_SHIFT_0(0x01);
  INSERTQ_CONST_SHIFT_0(0x02);
  INSERTQ_CONST_SHIFT_0(0x03);
  INSERTQ_CONST_SHIFT_0(0x04);
  INSERTQ_CONST_SHIFT_0(0x05);
  INSERTQ_CONST_SHIFT_0(0x06);
  INSERTQ_CONST_SHIFT_0(0x07);
  INSERTQ_CONST_SHIFT_0(0x08);
  INSERTQ_CONST_SHIFT_0(0x09);
  INSERTQ_CONST_SHIFT_0(0x0A);
  INSERTQ_CONST_SHIFT_0(0x0B);
  INSERTQ_CONST_SHIFT_0(0x0C);
  INSERTQ_CONST_SHIFT_0(0x0D);
  INSERTQ_CONST_SHIFT_0(0x0E);
  INSERTQ_CONST_SHIFT_0(0x0F);
  INSERTQ_CONST_SHIFT_0(0x10);
  INSERTQ_CONST_SHIFT_0(0x11);
  INSERTQ_CONST_SHIFT_0(0x12);
  INSERTQ_CONST_SHIFT_0(0x13);
  INSERTQ_CONST_SHIFT_0(0x14);
  INSERTQ_CONST_SHIFT_0(0x15);
  INSERTQ_CONST_SHIFT_0(0x16);
  INSERTQ_CONST_SHIFT_0(0x17);
  INSERTQ_CONST_SHIFT_0(0x18);
  INSERTQ_CONST_SHIFT_0(0x19);
  INSERTQ_CONST_SHIFT_0(0x1A);
  INSERTQ_CONST_SHIFT_0(0x1B);
  INSERTQ_CONST_SHIFT_0(0x1C);
  INSERTQ_CONST_SHIFT_0(0x1D);
  INSERTQ_CONST_SHIFT_0(0x1E);
  INSERTQ_CONST_SHIFT_0(0x1F);
  INSERTQ_CONST_SHIFT_0(0x20);
  INSERTQ_CONST_SHIFT_0(0x21);
  INSERTQ_CONST_SHIFT_0(0x22);
  INSERTQ_CONST_SHIFT_0(0x23);
  INSERTQ_CONST_SHIFT_0(0x24);
  INSERTQ_CONST_SHIFT_0(0x25);
  INSERTQ_CONST_SHIFT_0(0x26);
  INSERTQ_CONST_SHIFT_0(0x27);
  INSERTQ_CONST_SHIFT_0(0x28);
  INSERTQ_CONST_SHIFT_0(0x29);
  INSERTQ_CONST_SHIFT_0(0x2A);
  INSERTQ_CONST_SHIFT_0(0x2B);
  INSERTQ_CONST_SHIFT_0(0x2C);
  INSERTQ_CONST_SHIFT_0(0x2D);
  INSERTQ_CONST_SHIFT_0(0x2E);
  INSERTQ_CONST_SHIFT_0(0x2F);
  INSERTQ_CONST_SHIFT_0(0x30);
  INSERTQ_CONST_SHIFT_0(0x31);
  INSERTQ_CONST_SHIFT_0(0x32);
  INSERTQ_CONST_SHIFT_0(0x33);
  INSERTQ_CONST_SHIFT_0(0x34);
  INSERTQ_CONST_SHIFT_0(0x35);
  INSERTQ_CONST_SHIFT_0(0x36);
  INSERTQ_CONST_SHIFT_0(0x37);
  INSERTQ_CONST_SHIFT_0(0x38);
  INSERTQ_CONST_SHIFT_0(0x39);
  INSERTQ_CONST_SHIFT_0(0x3A);
  INSERTQ_CONST_SHIFT_0(0x3B);
  INSERTQ_CONST_SHIFT_0(0x3C);
  INSERTQ_CONST_SHIFT_0(0x3D);
  INSERTQ_CONST_SHIFT_0(0x3E);
  INSERTQ_CONST_SHIFT_0(0x3F);

  INSERTQ_CONST_SHIFT_1(0xC0);
  INSERTQ_CONST_SHIFT_1(0xC1);
  INSERTQ_CONST_SHIFT_1(0xC2);
  INSERTQ_CONST_SHIFT_1(0xC3);
  INSERTQ_CONST_SHIFT_1(0xC4);
  INSERTQ_CONST_SHIFT_1(0xC5);
  INSERTQ_CONST_SHIFT_1(0xC6);
  INSERTQ_CONST_SHIFT_1(0xC7);
  INSERTQ_CONST_SHIFT_1(0xC8);
  INSERTQ_CONST_SHIFT_1(0xC9);
  INSERTQ_CONST_SHIFT_1(0xCA);
  INSERTQ_CONST_SHIFT_1(0xCB);
  INSERTQ_CONST_SHIFT_1(0xCC);
  INSERTQ_CONST_SHIFT_1(0xCD);
  INSERTQ_CONST_SHIFT_1(0xCE);
  INSERTQ_CONST_SHIFT_1(0xCF);
  INSERTQ_CONST_SHIFT_1(0xD0);
  INSERTQ_CONST_SHIFT_1(0xD1);
  INSERTQ_CONST_SHIFT_1(0xD2);
  INSERTQ_CONST_SHIFT_1(0xD3);
  INSERTQ_CONST_SHIFT_1(0xD4);
  INSERTQ_CONST_SHIFT_1(0xD5);
  INSERTQ_CONST_SHIFT_1(0xD6);
  INSERTQ_CONST_SHIFT_1(0xD7);
  INSERTQ_CONST_SHIFT_1(0xD8);
  INSERTQ_CONST_SHIFT_1(0xD9);
  INSERTQ_CONST_SHIFT_1(0xDA);
  INSERTQ_CONST_SHIFT_1(0xDB);
  INSERTQ_CONST_SHIFT_1(0xDC);
  INSERTQ_CONST_SHIFT_1(0xDD);
  INSERTQ_CONST_SHIFT_1(0xDE);
  INSERTQ_CONST_SHIFT_1(0xDF);
  INSERTQ_CONST_SHIFT_1(0xE0);
  INSERTQ_CONST_SHIFT_1(0xE1);
  INSERTQ_CONST_SHIFT_1(0xE2);
  INSERTQ_CONST_SHIFT_1(0xE3);
  INSERTQ_CONST_SHIFT_1(0xE4);
  INSERTQ_CONST_SHIFT_1(0xE5);
  INSERTQ_CONST_SHIFT_1(0xE6);
  INSERTQ_CONST_SHIFT_1(0xE7);
  INSERTQ_CONST_SHIFT_1(0xE8);
  INSERTQ_CONST_SHIFT_1(0xE9);
  INSERTQ_CONST_SHIFT_1(0xEA);
  INSERTQ_CONST_SHIFT_1(0xEB);
  INSERTQ_CONST_SHIFT_1(0xEC);
  INSERTQ_CONST_SHIFT_1(0xED);
  INSERTQ_CONST_SHIFT_1(0xEE);
  INSERTQ_CONST_SHIFT_1(0xEF);
  INSERTQ_CONST_SHIFT_1(0xF0);
  INSERTQ_CONST_SHIFT_1(0xF1);
  INSERTQ_CONST_SHIFT_1(0xF2);
  INSERTQ_CONST_SHIFT_1(0xF3);
  INSERTQ_CONST_SHIFT_1(0xF4);
  INSERTQ_CONST_SHIFT_1(0xF5);
  INSERTQ_CONST_SHIFT_1(0xF6);
  INSERTQ_CONST_SHIFT_1(0xF7);
  INSERTQ_CONST_SHIFT_1(0xF8);
  INSERTQ_CONST_SHIFT_1(0xF9);
  INSERTQ_CONST_SHIFT_1(0xFA);
  INSERTQ_CONST_SHIFT_1(0xFB);
  INSERTQ_CONST_SHIFT_1(0xFC);
  INSERTQ_CONST_SHIFT_1(0xFD);
  INSERTQ_CONST_SHIFT_1(0xFE);
  INSERTQ_CONST_SHIFT_1(0xFF);
}
