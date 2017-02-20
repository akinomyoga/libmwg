// -*- coding: cp932 -*-
#include <cstdio>
#include <type_traits>

// (0) original
// template<typename F>
// struct functor_traits_1: std::integral_constant<bool, std::is_function<typename std::remove_pointer<F>::type>::value> {};
// template<typename F, typename S>
// struct isF: std::integral_constant<bool, functor_traits_1<F>::value> {};

// (1) これを使ってもならない。
// template<typename F, typename S>
// struct isF: std::true_type {};

// (2) これもならない
// template<typename F> struct always_true: std::true_type {};
// template<typename F, typename S>
// struct isF: std::integral_constant<bool, always_true<F>::value> {};

// (3) これがなる!
template<typename F>
struct isF: std::is_function<typename std::remove_pointer<F>::type> {};

struct C {
  C() {}
  //template<typename F> explicit C(const F& f) {} // これだとならない。

  // explicit は再現に必要。isF のダミー引数に同じ型を指定する必要がある。
  template<typename F> explicit C(const F& f, typename std::enable_if<isF<F>::value>::type* = nullptr) {}

  template<typename F> void operator=(const F&) {} // 戻り値は void でも再現する。
};

void f() {}
int func1() {return 123;}
char func2() {return 1;}

int main() {
  //C<int(int, int)> g1(&func1);
  //C<int(int, char)> g1(&func1); // なんと int(int, char) だと再現しない → 今試したら再現する
  //C<void()> g1; // これだとならない。
  //C g1(&func1);
  C g1; // 何故かこれだとなる
  std::printf("isF<ptr, fun> = %d\n", isF<void (*)()>::value? 1: 0);
  g1 = f;
  //g1 = func1;
  // g1.operator=(func1); // メンバ関数としての呼び出しだと再現しない。
  // g1 = 0; // 違う型だと再現しない。
  // g1 = func2; // 違う関数型でも再現しない。
  // tf(func1); // テンプレート関数に渡すだけではならない。
  // a = func1; // 別のクラスに代入してもならない。
  std::printf("isF<ptr, fun> = %d\n", isF<void (*)()>::value? 1: 0);

  return 0;
}
