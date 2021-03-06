// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BITS_MPL_INTEGER_H
#define MWG_BITS_MPL_INTEGER_H
#include <cstddef>
#include <climits>
#include <limits>
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

//*****************************************************************************
//  整数テンプレート
//-----------------------------------------------------------------------------

  // template<unsigned I> struct impl_add_bytes: mwg::std::integral_constant<int, (I & 0xff)+(I >> 8 & 0xff)+(I >> 16 & 0xff)+(I >> 24 & 0xff)> {};
  // template<unsigned I> struct impl_add_nibbles: impl_add_bytes<(I & 0x0f0f0f0f)+(I >> 4 & 0x0f0f0f0f)> {};
  // template<unsigned I> struct count_bits: impl_add_nibbles<(I & 0x11111111)+(I >> 1 & 0x11111111)+(I >> 2 & 0x11111111)+(I >> 3 & 0x11111111)> {};

  namespace is_pow2_detail {

    template<typename UIntType, UIntType value, std::size_t width = std::numeric_limits<UIntType>::digits, std::size_t shift = (width + 1) / 2>
    struct for_unsigned: for_unsigned<
      UIntType,
      (value % ((UIntType) 1 << shift) == 0? value >> shift: value),
      width - shift> {};
    template<typename UIntType, UIntType value, std::size_t shift>
    struct for_unsigned<UIntType, value, 0, shift>: stdm::bool_constant<value == 1>::type {};

    template<typename IntType, IntType value, typename UIntType = typename stdm::make_unsigned<IntType>::type, bool = (value >= 0)>
    struct for_signed: for_unsigned<UIntType, (UIntType) value> {};
    template<typename IntType, IntType value, typename UIntType>
    struct for_signed<IntType, value, UIntType, false>: stdm::false_type {};

    template<typename Integer, Integer value, typename = void> struct impl: stdm::false_type {};
    template<typename IntType, IntType value>
    struct impl<IntType, value, typename stdm::enable_if<stdm::is_signed<IntType>::value>::type>: for_signed<IntType, value> {};
    template<typename UIntType, UIntType value>
    struct impl<UIntType, value, typename stdm::enable_if<stdm::is_unsigned<UIntType>::value>::type>: for_unsigned<UIntType, value> {};
  }

  template<typename Integer, Integer value> struct is_pow2: is_pow2_detail::impl<Integer, value> {};


#pragma%x begin_check
#include <cstdio>
#include <mwg/bits/mpl.integer.h>
#include <mwg/except.h>

void test_is_pow2() {
  mwg_check(( mwg::mpl::is_pow2<int, 1>::value));
  mwg_check(( mwg::mpl::is_pow2<int, 2>::value));
  mwg_check(( mwg::mpl::is_pow2<int, 4>::value));
  mwg_check(( mwg::mpl::is_pow2<int, 8>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 0>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 3>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 5>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 6>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 7>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 9>::value));

  mwg_check(( mwg::mpl::is_pow2<int, 16>::value));
  mwg_check(( mwg::mpl::is_pow2<int, 32>::value));
  mwg_check(( mwg::mpl::is_pow2<int, 64>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 23>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 43>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 61>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 23>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 98>::value));

  mwg_check(( mwg::mpl::is_pow2<int, 128>::value));
  mwg_check(( mwg::mpl::is_pow2<int, 256>::value));
  mwg_check(( mwg::mpl::is_pow2<int, 512>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 122>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 132>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 432>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 327>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 243>::value));

  mwg_check(( mwg::mpl::is_pow2<int, 1024>::value));
  mwg_check(( mwg::mpl::is_pow2<int, 2048>::value));
  mwg_check(( mwg::mpl::is_pow2<int, 4096>::value));
  mwg_check(( mwg::mpl::is_pow2<int, 8192>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 1536>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 9999>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 2134>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 4321>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 3192>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 3928>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 4329>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 3412>::value));
  mwg_check((!mwg::mpl::is_pow2<int, 8888>::value));
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

    /* template<typename IntType> struct integral_limits;
     *
     * std::numeric_limits<I>::min(), max() は C++03 では constant expression ではないので、
     * integral_limits<I>::min_value, integral_limits<I>::max_value として最小値・最大値を提供する。
     */

    template<typename I, I ZMin, I ZMax>
    struct integral_limits_impl {
      static mwg_constexpr_const I min_value = ZMin;
      static mwg_constexpr_const I max_value = ZMax;
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

#if mwg_has_feature(cxx_variadic_templates)
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

#if mwg_has_feature(cxx_variadic_templates)
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
      I Z0 = integral_limits<I>::min_value,
      I Z1 = integral_limits<I>::min_value,
      I Z2 = integral_limits<I>::min_value,
      I Z3 = integral_limits<I>::min_value,
      I Z4 = integral_limits<I>::min_value,
      I Z5 = integral_limits<I>::min_value,
      I Z6 = integral_limits<I>::min_value,
      I Z7 = integral_limits<I>::min_value,
      I Z8 = integral_limits<I>::min_value,
      I Z9 = integral_limits<I>::min_value >
    struct integral_max:
      integral_max<I, (Z0 > Z1? Z0: Z1), Z2, Z3, Z4, Z5, Z6, Z7, Z8, Z9> {};
    template<
      typename I,
      I Z0 = integral_limits<I>::max_value,
      I Z1 = integral_limits<I>::max_value,
      I Z2 = integral_limits<I>::max_value,
      I Z3 = integral_limits<I>::max_value,
      I Z4 = integral_limits<I>::max_value,
      I Z5 = integral_limits<I>::max_value,
      I Z6 = integral_limits<I>::max_value,
      I Z7 = integral_limits<I>::max_value,
      I Z8 = integral_limits<I>::max_value,
      I Z9 = integral_limits<I>::max_value >
    struct integral_min:
      integral_min<I, (Z0 < Z1? Z0: Z1), Z2, Z3, Z4, Z5, Z6, Z7, Z8, Z9> {};

/*---- WORKAROUND -------------------------------------------------------------
 * 特殊化の際、既定テンプレート引数が他のテンプレートパラメータ I を含む事はできない
 *| template<typename I, I Z0>
 *| struct integral_max<I, Z0> // ERR: 既定引数 integral_limits<I>::max_value でエラー
 *|   :mwg::stdm::integral_constant<I, Z0> {};
 *| template<typename I, I Z0>
 *| struct integral_min<I, Z0> // ERR
 *|   :mwg::stdm::integral_constant<I, Z0> {};
 */
# define MWG_MPL_H_define_integral_minmax_result(type) \
template<type Z0> struct integral_max<type, Z0>: mwg::stdm::integral_constant<type, Z0> {}; \
template<type Z0> struct integral_min<type, Z0>: mwg::stdm::integral_constant<type, Z0> {}
    MWG_MPL_H_define_integral_minmax_result(char);
    MWG_MPL_H_define_integral_minmax_result(signed char);
    MWG_MPL_H_define_integral_minmax_result(unsigned char);
    MWG_MPL_H_define_integral_minmax_result(short);
    MWG_MPL_H_define_integral_minmax_result(unsigned short);
    MWG_MPL_H_define_integral_minmax_result(int);
    MWG_MPL_H_define_integral_minmax_result(unsigned int);
    MWG_MPL_H_define_integral_minmax_result(long);
    MWG_MPL_H_define_integral_minmax_result(unsigned long);
#  ifdef MWGCONF_HAS_LONGLONG
    MWG_MPL_H_define_integral_minmax_result(long long);
    MWG_MPL_H_define_integral_minmax_result(unsigned long long);
#  endif
#  ifdef MWGCONF_HAS_DISTINCT_INT64
    MWG_MPL_H_define_integral_minmax_result(__int64);
    MWG_MPL_H_define_integral_minmax_result(unsigned __int64);
#  endif
    MWG_MPL_H_define_integral_minmax_result(bool);
# undef MWG_MPL_H_define_integral_minmax_result
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

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
} /* endof namespace mpl */
} /* endof namespace mwg */
#endif
#pragma%x begin_check
int main() {
  test_is_pow2();
  test_integral_minmax();
  return 0;
}
#pragma%x end_check
