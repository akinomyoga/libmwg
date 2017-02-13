// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_MPL_H
#define MWG_MPL_H
#include <cstddef>
#include <climits>
#include <mwg/std/type_traits>
namespace mwg {
namespace mpl {
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  // class any_type{
  //   void* d;
  // public:
  //   any_type(): d(0){}
  //   template<typename T>
  //   operator T&(){return *reinterpret_cast<T*>(d);}
  // };

//-----------------------------------------------------------------------------
// macro mwg_mpl_nullref
//
// #define mwg_mpl_nullref(T) (*reinterpret_cast<typename mwg::identity<T>::type*>(0))
//   mwg_mpl_nullref(const T) -> mwg::declval<T>()
//   mwg_mpl_nullref(T)       -> mwg::declval<T&>()

//*****************************************************************************
//  整数テンプレート
//-----------------------------------------------------------------------------

  // template<unsigned I> struct impl_add_bytes: mwg::std::integral_constant<int, (I & 0xff)+(I >> 8 & 0xff)+(I >> 16 & 0xff)+(I >> 24 & 0xff)> {};
  // template<unsigned I> struct impl_add_nibbles: impl_add_bytes<(I & 0x0f0f0f0f)+(I >> 4 & 0x0f0f0f0f)> {};
  // template<unsigned I> struct count_bits: impl_add_nibbles<(I & 0x11111111)+(I >> 1 & 0x11111111)+(I >> 2 & 0x11111111)+(I >> 3 & 0x11111111)> {};

  template<int I> struct is_power_of_2;
  template<>      struct is_power_of_2<0>  : mwg::stdm::false_type {};
  template<>      struct is_power_of_2<1>  : mwg::stdm::true_type {};
  template<>      struct is_power_of_2<2>  : mwg::stdm::true_type {};
  template<>      struct is_power_of_2<4>  : mwg::stdm::true_type {};
  template<>      struct is_power_of_2<8>  : mwg::stdm::true_type {};
  template<>      struct is_power_of_2<16> : mwg::stdm::true_type {};
  template<>      struct is_power_of_2<32> : mwg::stdm::true_type {};
  template<>      struct is_power_of_2<64> : mwg::stdm::true_type {};
  template<>      struct is_power_of_2<128>: mwg::stdm::true_type {};
  template<int I> struct is_power_of_2: mwg::stdm::integral_constant<bool,
    (I % 256 == 0 && is_power_of_2<I / 256>::value)
    > {};

#pragma%x begin_check
#include <cstdio>
#include <mwg/mpl.h>
#include <mwg/except.h>

void test_is_power_of_2() {
  mwg_check(( mwg::mpl::is_power_of_2<1>::value));
  mwg_check(( mwg::mpl::is_power_of_2<2>::value));
  mwg_check(( mwg::mpl::is_power_of_2<4>::value));
  mwg_check(( mwg::mpl::is_power_of_2<8>::value));
  mwg_check((!mwg::mpl::is_power_of_2<0>::value));
  mwg_check((!mwg::mpl::is_power_of_2<3>::value));
  mwg_check((!mwg::mpl::is_power_of_2<5>::value));
  mwg_check((!mwg::mpl::is_power_of_2<6>::value));
  mwg_check((!mwg::mpl::is_power_of_2<7>::value));
  mwg_check((!mwg::mpl::is_power_of_2<9>::value));

  mwg_check(( mwg::mpl::is_power_of_2<16>::value));
  mwg_check(( mwg::mpl::is_power_of_2<32>::value));
  mwg_check(( mwg::mpl::is_power_of_2<64>::value));
  mwg_check((!mwg::mpl::is_power_of_2<23>::value));
  mwg_check((!mwg::mpl::is_power_of_2<43>::value));
  mwg_check((!mwg::mpl::is_power_of_2<61>::value));
  mwg_check((!mwg::mpl::is_power_of_2<23>::value));
  mwg_check((!mwg::mpl::is_power_of_2<98>::value));

  mwg_check(( mwg::mpl::is_power_of_2<128>::value));
  mwg_check(( mwg::mpl::is_power_of_2<256>::value));
  mwg_check(( mwg::mpl::is_power_of_2<512>::value));
  mwg_check((!mwg::mpl::is_power_of_2<122>::value));
  mwg_check((!mwg::mpl::is_power_of_2<132>::value));
  mwg_check((!mwg::mpl::is_power_of_2<432>::value));
  mwg_check((!mwg::mpl::is_power_of_2<327>::value));
  mwg_check((!mwg::mpl::is_power_of_2<243>::value));

  mwg_check(( mwg::mpl::is_power_of_2<1024>::value));
  mwg_check(( mwg::mpl::is_power_of_2<2048>::value));
  mwg_check(( mwg::mpl::is_power_of_2<4096>::value));
  mwg_check(( mwg::mpl::is_power_of_2<8192>::value));
  mwg_check((!mwg::mpl::is_power_of_2<1536>::value));
  mwg_check((!mwg::mpl::is_power_of_2<9999>::value));
  mwg_check((!mwg::mpl::is_power_of_2<2134>::value));
  mwg_check((!mwg::mpl::is_power_of_2<4321>::value));
  mwg_check((!mwg::mpl::is_power_of_2<3192>::value));
  mwg_check((!mwg::mpl::is_power_of_2<3928>::value));
  mwg_check((!mwg::mpl::is_power_of_2<4329>::value));
  mwg_check((!mwg::mpl::is_power_of_2<3412>::value));
  mwg_check((!mwg::mpl::is_power_of_2<8888>::value));
}
#pragma%x end_check

  namespace integral_detail {
#ifdef _MSC_VER
    template<typename I, I Z1>
    struct integral_abs: mwg::stdm::integral_constant<I, (Z1 < 0? -1 * Z1: Z1)> {};
#else
    template<typename I, I Z1>
    struct integral_abs: mwg::stdm::integral_constant<I, (Z1 < 0? -Z1: Z1)> {}; // TODO: when Z1 == INTMAX_MIN
#endif
    template<typename I, I Z1>
    struct integral_sgn: mwg::stdm::integral_constant<I, (Z1 < 0? -1: 1)> {};

    template<typename I, I ZMin, I ZMax>
    struct integral_limits_impl {
      static const I min_value=ZMin;
      static const I max_value=ZMax;
    };

    template<typename I> struct integral_limits {};
    template<> struct integral_limits<char>:
      integral_limits_impl<char, CHAR_MIN, CHAR_MAX> {};
    template<> struct integral_limits<signed char>:
      integral_limits_impl<signed char, SCHAR_MIN, SCHAR_MAX> {};
    template<> struct integral_limits<unsigned char>:
      integral_limits_impl<unsigned char, 0, UCHAR_MAX> {};
    template<> struct integral_limits<short>:
      integral_limits_impl<short, SHRT_MIN, SHRT_MAX> {};
    template<> struct integral_limits<unsigned short>:
      integral_limits_impl<unsigned short, 0, USHRT_MAX> {};
    template<> struct integral_limits<int>:
      integral_limits_impl<int, INT_MIN, INT_MAX> {};
    template<> struct integral_limits<unsigned int>:
      integral_limits_impl<unsigned int, 0, UINT_MAX> {};
    template<> struct integral_limits<long>:
      integral_limits_impl<long, LONG_MIN, LONG_MAX> {};
    template<> struct integral_limits<unsigned long>:
      integral_limits_impl<unsigned long, 0, ULONG_MAX> {};
#if defined(MWGCONF_HAS_LONGLONG)&&!defined(_MSC_VER)
    template<> struct integral_limits<long long>:
      integral_limits_impl<long long, LLONG_MIN, LLONG_MAX> {};
    template<> struct integral_limits<unsigned long long>:
      integral_limits_impl<unsigned long long, 0, ULLONG_MAX> {};
#endif
#if defined(MWGCONF_HAS_DISTINCT_INT64)||defined(_MSC_VER)
    template<> struct integral_limits<__int64>:
      integral_limits_impl<__int64, _I64_MIN, _I64_MAX> {};
    template<> struct integral_limits<unsigned __int64>:
      integral_limits_impl<unsigned __int64, 0, _UI64_MAX> {};
#endif
    template<> struct integral_limits<bool>:
      integral_limits_impl<bool, false, true> {};


    template<typename I, I Z1, I Z2>
    struct integral_gcd_impl;
    template<typename I, I Z1, I Z2, int S>
    struct integral_gcd_impl2;

    template<typename I, I Z1, I Z2, int S>
    struct integral_gcd_impl2: integral_gcd_impl<I, Z2, Z1 - (Z1 / Z2) * Z2> {};
    template<typename I, I Z1, I Z2>
    struct integral_gcd_impl2<I, Z1, Z2, 1>: integral_gcd_impl<I, Z2, Z1> {};
    template<typename I, I Z1, I Z2>
    struct integral_gcd_impl2<I, Z1, Z2, 2>: mwg::stdm::integral_constant<I, Z1> {};
    template<typename I, I Z1, I Z2>
    struct integral_gcd_impl: integral_gcd_impl2<I, Z1, Z2, (Z1 < Z2? 1: Z2 == 0? 2: 0)> {};

#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
    template<typename I, I... Zs>
    struct integral_gcd {};
    template<typename I>
    struct integral_gcd<I>
      :mwg::stdm::integral_constant<I, 1> {};
    template<typename I, I Z1>
    struct integral_gcd<I, Z1>
      :mwg::stdm::integral_constant<I, Z1> {};
    template<typename I, I Z0, I Z1>
    struct integral_gcd<I, Z0, Z1>
      :integral_gcd_impl<I, integral_abs<I, Z0>::value, integral_abs<I, Z1>::value> {};
    template<typename I, I Z0, I Z1, I... Zs>
    struct integral_gcd<I, Z0, Z1, Zs...>
      :integral_gcd<I, integral_gcd<I, Z0, Z1>::value, Zs...> {};

    template<typename I, I... Zs>
    struct integral_lcm {};
    template<typename I>
    struct integral_lcm<I>
      :mwg::stdm::integral_constant<I, 1> {};
    template<typename I, I Z1>
    struct integral_lcm<I, Z1>
      :mwg::stdm::integral_constant<I, Z1> {};
    template<typename I, I Z0, I Z1>
    struct integral_lcm<I, Z0, Z1>
      :mwg::stdm::integral_constant<I, (Z0 / integral_gcd<I, Z0, Z1>::value) * Z1> {};
    template<typename I, I Z0, I Z1, I... Zs>
    struct integral_lcm<I, Z0, Z1, Zs...>
      :integral_lcm<I, integral_lcm<I, Z0, Z1>::value, Zs...> {};
#else
    template<typename I, I Z1, I Z2>
    struct integral_gcd: integral_gcd_impl<I, integral_abs<I, Z1>::value, integral_abs<I, Z2>::value> {};
    template<typename I, I Z1, I Z2>
    struct integral_lcm: mwg::stdm::integral_constant<I, (Z1 / integral_gcd<I, Z1, Z2>::value) * Z2> {};
#endif

#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
    template<typename I, I... Zs>
    struct integral_max {};
    template<typename I>
    struct integral_max<I>
      :mwg::stdm::integral_constant<I, integral_limits<I>::min_value> {};
    template<typename I, I Z1>
    struct integral_max<I, Z1>
      :mwg::stdm::integral_constant<I, Z1> {};
    template<typename I, I Z0, I Z1>
    struct integral_max<I, Z0, Z1>
      :mwg::stdm::integral_constant<I, (Z0 >= Z1? Z0: Z1)> {};
    template<typename I, I Z0, I Z1, I... Zs>
    struct integral_max<I, Z0, Z1, Zs...>
      :integral_max<I, integral_max<I, Z0, Z1>::value, Zs...> {};

    template<typename I, I... Zs>
    struct integral_min {};
    template<typename I>
    struct integral_min<I>
      :mwg::stdm::integral_constant<I, integral_limits<I>::min_value> {};
    template<typename I, I Z0, I Z1>
    struct integral_min<I, Z0, Z1>
      :mwg::stdm::integral_constant<I, (Z0 <= Z1? Z0: Z1)> {};
    template<typename I, I Z0, I Z1, I... Zs>
    struct integral_min<I, Z0, Z1, Zs...>
      :integral_min<I, integral_min<I, Z0, Z1>::value, Zs...> {};
#else
    template<
      typename I,
      I Z0=integral_limits<I>::min_value,
      I Z1=integral_limits<I>::min_value,
      I Z2=integral_limits<I>::min_value,
      I Z3=integral_limits<I>::min_value,
      I Z4=integral_limits<I>::min_value,
      I Z5=integral_limits<I>::min_value,
      I Z6=integral_limits<I>::min_value,
      I Z7=integral_limits<I>::min_value,
      I Z8=integral_limits<I>::min_value,
      I Z9=integral_limits<I>::min_value >
    struct integral_max
      :integral_max<I, (Z0 > Z1? Z0: Z1), Z2, Z3, Z4, Z5, Z6, Z7, Z8, Z9> {};
    template<
      typename I,
      I Z0=integral_limits<I>::max_value,
      I Z1=integral_limits<I>::max_value,
      I Z2=integral_limits<I>::max_value,
      I Z3=integral_limits<I>::max_value,
      I Z4=integral_limits<I>::max_value,
      I Z5=integral_limits<I>::max_value,
      I Z6=integral_limits<I>::max_value,
      I Z7=integral_limits<I>::max_value,
      I Z8=integral_limits<I>::max_value,
      I Z9=integral_limits<I>::max_value >
    struct integral_min
      :integral_min<I, (Z0 < Z1? Z0: Z1), Z2, Z3, Z4, Z5, Z6, Z7, Z8, Z9> {};

/*---- WORKAROUND -------------------------------------------------------------
 * 特殊化の際、既定テンプレート引数が他のテンプレートパラメータ I を含む事はできない
 *| template<typename I, I Z0>
 *| struct integral_max<I, Z0> // ERR: 既定引数 integral_limits<I>::max_value でエラー
 *|   :mwg::stdm::integral_constant<I, Z0> {};
 *| template<typename I, I Z0>
 *| struct integral_min<I, Z0> // ERR
 *|   :mwg::stdm::integral_constant<I, Z0> {};
 */
# define MWG_MPL_H__define_integral_minmax_result(type) \
template<type Z0> struct integral_max<type, Z0>: mwg::stdm::integral_constant<type, Z0> {}; \
template<type Z0> struct integral_min<type, Z0>: mwg::stdm::integral_constant<type, Z0> {}
    MWG_MPL_H__define_integral_minmax_result(char);
    MWG_MPL_H__define_integral_minmax_result(signed char);
    MWG_MPL_H__define_integral_minmax_result(unsigned char);
    MWG_MPL_H__define_integral_minmax_result(short);
    MWG_MPL_H__define_integral_minmax_result(unsigned short);
    MWG_MPL_H__define_integral_minmax_result(int);
    MWG_MPL_H__define_integral_minmax_result(unsigned int);
    MWG_MPL_H__define_integral_minmax_result(long);
    MWG_MPL_H__define_integral_minmax_result(unsigned long);
#  ifdef MWGCONF_HAS_LONGLONG
    MWG_MPL_H__define_integral_minmax_result(long long);
    MWG_MPL_H__define_integral_minmax_result(unsigned long long);
#  endif
#  ifdef MWGCONF_HAS_DISTINCT_INT64
    MWG_MPL_H__define_integral_minmax_result(__int64);
    MWG_MPL_H__define_integral_minmax_result(unsigned __int64);
#  endif
    MWG_MPL_H__define_integral_minmax_result(bool);
# undef MWG_MPL_H__define_integral_minmax_result
/*- end of WORKAROUND -------------------------------------------------------*/
#endif

  }

  using integral_detail::integral_abs;
  using integral_detail::integral_sgn;
  using integral_detail::integral_gcd;
  using integral_detail::integral_lcm;

  using integral_detail::integral_max;
  using integral_detail::integral_min;
  using integral_detail::integral_limits;

#pragma%x begin_check
void test_integral_minmax() {
  mwg_check(( mwg::mpl::integral_min<int, -1,  22,  3,  34,  15>::value ==  -1));
  mwg_check(( mwg::mpl::integral_max<int, -1,  22,  3,  34,  15>::value ==  34));
  mwg_check(( mwg::mpl::integral_min<int,  1,  22,  3,  34,  15>::value ==   1));
  mwg_check(( mwg::mpl::integral_max<int,  1,  22,  3,  34,  15>::value ==  34));
  mwg_check(( mwg::mpl::integral_min<int, -1, -22, -3, -34, -15>::value == -34));
  mwg_check(( mwg::mpl::integral_max<int, -1, -22, -3, -34, -15>::value ==  -1));
}
#pragma%x end_check

//*****************************************************************************
//  型に関する判定
/*
 * //-----------------------------------------------------------------------------
 * // mustbe_type<T, mem> : mem の型が T でなかったら適用失敗
 * //-----------------------------------------------------------------------------
 *   template<class T, T mem>
 *   struct mustbe_type {typedef mwg::stdm::true_type type;};
 * //----------------------------------------------------------------------------
 * // mwg_mpl_is_assignable(T, expr) : expr を T 型の変数に代入可能か否かの判定
 * //----------------------------------------------------------------------------
 *   template<typename T>
 *   struct is_assignable_impl {
 *     static mwg::mpl::true_t eval(T v);
 *     static mwg::mpl::false_t eval(...);
 *   };
 * #define mwg_mpl_is_assignable(T, expr) \
 *   (sizeof(mwg::mpl::true_t) == sizeof(mwg::mpl::is_assignable_impl<T>::eval(mwg_mpl_void2iarr(expr))))
 */

  //============================================================================
  //  type_for_arg<T>::type : 関数の引数として適当な物を選択
  //    (値渡し or const 参照渡し)
  //----------------------------------------------------------------------------
  template<typename T, bool B=mwg::stdm::is_scalar<T>::value> struct type_for_arg;
  template<typename T> struct type_for_arg<T, true>: identity<T> {};
  template<typename T> struct type_for_arg<T, false>: identity<const T&> {};
  // long double → サイズが大きいので参照で渡す様にする?
  // 参照 → 参照の参照は参照なので OK
  // 関数ポインタ → 自動的にポインタとして扱われる? ので大丈夫?
  // メンバへのポインタ → is_scalar が対応しているので OK
  // 配列 → 配列は要素数情報の保存も考えて参照で受け取れた方が良いと思う。
  template<typename T, std::size_t N, bool B>
  struct type_for_arg<T[N], B>: identity<T(&)[N]> {};
  template<typename T, bool B>
  struct type_for_arg<T[], B>: identity<T*> {};

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
} /* endof namespace mpl */
} /* endof namespace mwg */
#endif
#pragma%x begin_check
int main() {
  test_is_power_of_2();
  test_integral_minmax();
  return 0;
}
#pragma%x end_check
