// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BITS_MPL_UTIL_H
#define MWG_BITS_MPL_UTIL_H
#include <cstddef>
#include <mwg/std/type_traits>
namespace mwg {
namespace mpl {

  // deprecated (originally located in mwg/mpl.h)
  //============================================================================
  //  type_for_arg<T>::type : 関数の引数として適当な物を選択
  //    (値渡し or const 参照渡し)
  //----------------------------------------------------------------------------
  template<typename T, bool B = mwg::stdm::is_scalar<T>::value> struct type_for_arg;
  template<typename T> struct type_for_arg<T, true>: identity<T> {};
  template<typename T> struct type_for_arg<T, false>: identity<const T&> {};
  // long double → サイズが大きいので参照で渡す様にする?
  // 参照 → 参照の参照は参照なので OK
  // 関数ポインタ → 自動的にポインタとして扱われる? ので大丈夫?
  // メンバへのポインタ → is_scalar が対応しているので OK
  // 配列 → 配列は要素数情報の保存も考えて参照で受け取れた方が良いと思う。
  template<typename T, std::size_t N, bool B>
  struct type_for_arg<T[N], B>: identity<T (&)[N]> {};
  template<typename T, bool B>
  struct type_for_arg<T[], B>: identity<T*> {};

}
}
#endif
