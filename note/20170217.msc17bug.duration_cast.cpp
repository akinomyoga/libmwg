// -*- mode: c++; coding: cp932 -*-
#include <cstdio>
#include <chrono>
#include <ratio>

int main() {
  std::chrono::duration<double> a;

  // msc17 �ł͂��ꂪ�R���p�C���ł��Ȃ��B
  (void) std::chrono::duration_cast<std::chrono::milliseconds>(a);

  // msc17 �͂�����R���p�C���ł��Ȃ��B
  (void) std::ratio_equal<std::ratio_add<std::ratio<1, 4>, std::ratio<1, 4> >, std::ratio<1, 2> >::value;

  // ����͑��v
  (void) std::ratio_equal<std::ratio<1, 2>, std::ratio<1, 2> >::value;

  return 0;
}
