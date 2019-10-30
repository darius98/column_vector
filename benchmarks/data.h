#pragma once

struct Aligned {
  int x;
  int y;
  int z;
  int t;

  Aligned(int x, int y, int z, int t) : x(x), y(y), z(z), t(t) {}
};

struct Unaligned {
  int x;
  double y;
  char z;

  Unaligned(int x, double y, char z) : x(x), y(y), z(z) {}
};

struct Large {
  long long x00;
  long long x01;
  long long x02;
  long long x03;
  long long x04;
  long long x05;
  long long x06;
  long long x07;
  long long x08;
  long long x09;
  long long x10;
  long long x11;
  long long x12;
  long long x13;
  long long x14;
  long long x15;
  long long x16;
  long long x17;
  long long x18;
  long long x19;
  long long x20;
  long long x21;
  long long x22;
  long long x23;
  long long x24;
  long long x25;
  long long x26;
  long long x27;
  long long x28;
  long long x29;
  long long x30;
  long long x31;

  Large(long long x00, long long x01, long long x02, long long x03,
        long long x04, long long x05, long long x06, long long x07,
        long long x08, long long x09, long long x10, long long x11,
        long long x12, long long x13, long long x14, long long x15,
        long long x16, long long x17, long long x18, long long x19,
        long long x20, long long x21, long long x22, long long x23,
        long long x24, long long x25, long long x26, long long x27,
        long long x28, long long x29, long long x30, long long x31)
      : x00(x00), x01(x01), x02(x02), x03(x03), x04(x04), x05(x05), x06(x06),
        x07(x07), x08(x08), x09(x09), x10(x10), x11(x11), x12(x12), x13(x13),
        x14(x14), x15(x15), x16(x16), x17(x17), x18(x18), x19(x19), x20(x20),
        x21(x21), x22(x22), x23(x23), x24(x24), x25(x25), x26(x26), x27(x27),
        x28(x28), x29(x29), x30(x30), x31(x31) {}
};