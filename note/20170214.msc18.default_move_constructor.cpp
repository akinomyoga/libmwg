// -*- coding: cp932 -*-
#include <cstdio>

struct Hello {
  int m_value;
  Hello(int value): m_value(value) {}
  Hello(Hello const&) = default;
  //Hello(Hello&&) = default;
};

// struct World {
//   World(World&&) = default;
// };

//
// msc18 (Visual Studio 2013) �� defaulted move constructor ���`�ł��Ȃ��B
//
//   error C2610: 'W::W(W &&)' : �͊���l�ɂł�����ꃁ���o�[�֐��ł͂���܂���
//
// �Ƃ����G���[���b�Z�[�W�ɂȂ�B
//
struct W {W(W&&) = default;};

// int main() {
//   Hello h1(12);
//   Hello h3(h1);

//   return 0;
// }
