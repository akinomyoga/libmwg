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
// msc18 (Visual Studio 2013) は defaulted move constructor を定義できない。
//
//   error C2610: 'W::W(W &&)' : は既定値にできる特殊メンバー関数ではありません
//
// というエラーメッセージになる。
//
struct W {W(W&&) = default;};

// int main() {
//   Hello h1(12);
//   Hello h3(h1);

//   return 0;
// }
