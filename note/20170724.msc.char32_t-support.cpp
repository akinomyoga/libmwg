// -*- coding: sjis -*-
#include <cstddef>

/*
 * MSVC 19 ������ char32_t/char16_t �ɑΉ����Ȃ��B
 * �ꉞ cstddef �� char32_t/char16_t �� typedef ����邪�A
 * ���̂� unsigned, unsigned short �Ȃ̂ŁA���d��`�o���Ȃ��B
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
