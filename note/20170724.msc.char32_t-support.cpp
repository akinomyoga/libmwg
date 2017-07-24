// -*- coding: sjis -*-
#include <cstddef>

/*
 * MSVC 19 未満は char32_t/char16_t に対応しない。
 * 一応 cstddef に char32_t/char16_t が typedef されるが、
 * 正体は unsigned, unsigned short なので、多重定義出来ない。
 */
void f(char16_t ch) {}
void f(short ch) {}
void f(unsigned short ch) {}

void f(char32_t ch) {}
void f(int ch) {}
void f(unsigned int ch) {}
void f(long ch) {}
void f(unsigned long ch) {}

int main() {
  return 0;
}
