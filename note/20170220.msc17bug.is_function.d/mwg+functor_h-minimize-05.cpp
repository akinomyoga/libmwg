// -*- coding: cp932 -*-
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <new>
#include <mwg/except.h>
#include <type_traits>
#include <utility>

namespace mwg{
namespace functor_detail{

  struct functor_traits_empty: std::false_type {
    typedef void sgn_t;
  };

  template<typename S>
  struct functor_traits_signature: std::true_type {
    typedef S sgn_t;
  };

  template<typename F, typename = void>
  struct functor_traits_1: functor_traits_empty {};
  template<typename F>
  struct functor_traits_1<F*, typename std::enable_if<std::is_function<F>::value>::type>: functor_traits_signature<F> {};
  template<typename F>
  struct functor_traits_1<F , typename std::enable_if<std::is_function<F>::value>::type>: functor_traits_signature<F> {};

  // template<typename F, typename S>
  // struct is_functor: std::is_same<S, typename functor_traits_1<F>::sgn_t> {};

  template<typename F, typename S>
  struct be_functor: std::integral_constant<bool, functor_traits_1<F>::value> {};

  // template<typename F> struct always_true: std::true_type {};
  // template<typename F, typename S>
  // struct be_functor: std::integral_constant<bool, always_true<F>::value> {}; // これもならない。
  // template<typename F, typename S>
  // struct be_functor: std::true_type {}; // これを使ってもならない

  template<typename S>
  struct functor {
    //template<typename F> explicit functor(const F& f) {} // これだとならない。

    template<typename F> explicit functor(const F& f, typename std::enable_if<be_functor<F, S>::value, void*>::type = nullptr) {}

    functor(functor const&) {}

    //template<typename F>
    //typename std::enable_if<be_functor<F, S>::value, functor&>::type
    //typename std::enable_if<is_functor<F, S>::value, functor&>::type
    //functor& operator=(const F& f) {return *this;}

    template<typename F> void operator=(const F&) {} // 戻り値は void でも再現する。
  };

} // end of functor_detail
  using functor_detail::functor;
} // end of mwg

int func1() {
  return 123;
}

char func2() {return 1;}

int main() {
  mwg::functor<int(int, int)> g1(&func1);
  //mwg::functor<int(int, char)> g1(&func1); // なんと int(int, char) だと再現しない。
  mwg_printd("be_functor<ptr, fun> = %d", mwg::functor_detail::be_functor<int (*)(), int(int, int)>::value? 1: 0);
  mwg_printd("be_functor<ptr, fun> = %d", mwg::functor_detail::be_functor<int (*)(), int(int, int)>::value? 1: 0);
  g1 = func1;
  // g1.operator=(func1); // メンバ関数としての呼び出しだと再現しない。
  // g1 = 0; // 違う型だと再現しない。
  // g1 = func2; // 違う関数型でも再現しない。
  // tf(func1); // テンプレート関数に渡すだけではならない。
  // a = func1; // 別のクラスに代入してもならない。
  mwg_printd("be_functor<ptr, fun> = %d", mwg::functor_detail::be_functor<int (*)(), int(int, int)>::value? 1: 0);
  mwg_printd("be_functor<ptr, fun> = %d", mwg::functor_detail::be_functor<int (*)(), int(int, int)>::value? 1: 0);

  return 0;
}
