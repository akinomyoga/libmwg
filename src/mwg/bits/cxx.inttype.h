// -*- mode: c++; coding: utf-8 -*-
#pragma%include "../impl/ManagedTest.pp"
#pragma%x begin_check
#include <mwg/std/cstdint>
#include <mwg/bits/cxx.inttype.h>
#pragma%x end_check
#ifndef MWG_BITS_CXX_INTTYPE_H
#define MWG_BITS_CXX_INTTYPE_H
#include <mwg/std/def.h>
#include <mwg/std/type_traits>
namespace mwg {
namespace cxx {

  namespace inttype_detail {

    template<typename UIntType, typename U1, typename U2, typename U3, typename U4, bool Condition>
    struct select_promoted_impl: identity<U1> {};

    template<typename UIntType, typename U1 = void, typename U2 = void, typename U3 = void, typename U4 = void>
    struct select_promoted: select_promoted_impl<UIntType, U1, U2, U3, U4, (sizeof(U1) <= sizeof(UIntType))> {};

    template<typename UIntType, typename U1, typename U2, typename U3, typename U4>
    struct select_promoted_impl<UIntType, U1, U2, U3, U4, false>: select_promoted<UIntType, U2, U3, U4> {};

    template<typename UIntType>
    struct select_promoted<UIntType, void, void, void, void>: identity<UIntType> {};

    template<typename CharClass, typename T, bool = stdm::is_arithmetic<T>::value>
    struct binary_result {};
    template<typename CharClass, typename T>
    struct binary_result<CharClass, T, true>: stdm::common_type<typename CharClass::promoted_type, T>{};

    template<typename UIntType, typename Tag = void>
    class inttype {
    public:
      // Note: unsigned long is guaranteed to be wider or equal to 32 bit.
      typedef UIntType underlying_type;
      typedef typename select_promoted<UIntType, int, unsigned, long, unsigned long>::type promoted_type;

    private:
      underlying_type m_value;

    public:
      mwg_constexpr inttype() {}
      mwg_constexpr inttype(underlying_type value): m_value(value) {}
      mwg_constexpr inttype(inttype const& c): m_value(c.m_value) {}

      mwg_constexpr operator promoted_type() const {return m_value;}
      mwg_constexpr operator bool() const {return m_value != 0;}

      mwg_constexpr bool operator!() const {return m_value == 0;}
      mwg_constexpr promoted_type operator~() const {return ~m_value;}
      mwg_constexpr promoted_type operator+() const {return +m_value;}
      mwg_constexpr promoted_type operator-() const {return -m_value;}
      mwg_constexpr14 inttype& operator++() {m_value++; return *this;}
      mwg_constexpr14 inttype& operator--() {m_value--; return *this;}
      mwg_constexpr14 inttype operator++(int) {inttype ret(*this); m_value++; return ret;}
      mwg_constexpr14 inttype operator--(int) {inttype ret(*this); m_value--; return ret;}

      mwg_constexpr promoted_type operator*(promoted_type value) const {return m_value * value;}
      mwg_constexpr promoted_type operator/(promoted_type value) const {return m_value / value;}
      mwg_constexpr promoted_type operator%(promoted_type value) const {return m_value % value;}
      mwg_constexpr promoted_type operator+(promoted_type value) const {return m_value + value;}
      mwg_constexpr promoted_type operator-(promoted_type value) const {return m_value - value;}
      mwg_constexpr promoted_type operator>>(promoted_type value) const {return m_value >> value;}
      mwg_constexpr promoted_type operator<<(promoted_type value) const {return m_value << value;}
      mwg_constexpr promoted_type operator|(promoted_type value) const {return m_value | value;}
      mwg_constexpr promoted_type operator^(promoted_type value) const {return m_value ^ value;}
      mwg_constexpr promoted_type operator&(promoted_type value) const {return m_value & value;}

      template<typename T> mwg_constexpr typename binary_result<inttype, T>::type operator* (T const& value) const {return (promoted_type) m_value * value;}
      template<typename T> mwg_constexpr typename binary_result<inttype, T>::type operator/ (T const& value) const {return (promoted_type) m_value / value;}
      template<typename T> mwg_constexpr typename binary_result<inttype, T>::type operator% (T const& value) const {return (promoted_type) m_value % value;}
      template<typename T> mwg_constexpr typename binary_result<inttype, T>::type operator+ (T const& value) const {return (promoted_type) m_value + value;}
      template<typename T> mwg_constexpr typename binary_result<inttype, T>::type operator- (T const& value) const {return (promoted_type) m_value - value;}
      template<typename T> mwg_constexpr typename binary_result<inttype, T>::type operator>>(T const& value) const {return (promoted_type) m_value >> value;}
      template<typename T> mwg_constexpr typename binary_result<inttype, T>::type operator<<(T const& value) const {return (promoted_type) m_value << value;}
      template<typename T> mwg_constexpr typename binary_result<inttype, T>::type operator| (T const& value) const {return (promoted_type) m_value | value;}
      template<typename T> mwg_constexpr typename binary_result<inttype, T>::type operator^ (T const& value) const {return (promoted_type) m_value ^ value;}
      template<typename T> mwg_constexpr typename binary_result<inttype, T>::type operator& (T const& value) const {return (promoted_type) m_value & value;}

      template<typename T> friend mwg_constexpr typename binary_result<inttype, T>::type operator* (T const& lhs, inttype const& rhs) {return lhs *  (promoted_type) rhs.m_value;}
      template<typename T> friend mwg_constexpr typename binary_result<inttype, T>::type operator/ (T const& lhs, inttype const& rhs) {return lhs /  (promoted_type) rhs.m_value;}
      template<typename T> friend mwg_constexpr typename binary_result<inttype, T>::type operator% (T const& lhs, inttype const& rhs) {return lhs %  (promoted_type) rhs.m_value;}
      template<typename T> friend mwg_constexpr typename binary_result<inttype, T>::type operator+ (T const& lhs, inttype const& rhs) {return lhs +  (promoted_type) rhs.m_value;}
      template<typename T> friend mwg_constexpr typename binary_result<inttype, T>::type operator- (T const& lhs, inttype const& rhs) {return lhs -  (promoted_type) rhs.m_value;}
      template<typename T> friend mwg_constexpr typename binary_result<inttype, T>::type operator>>(T const& lhs, inttype const& rhs) {return lhs >> (promoted_type) rhs.m_value;}
      template<typename T> friend mwg_constexpr typename binary_result<inttype, T>::type operator<<(T const& lhs, inttype const& rhs) {return lhs << (promoted_type) rhs.m_value;}
      template<typename T> friend mwg_constexpr typename binary_result<inttype, T>::type operator| (T const& lhs, inttype const& rhs) {return lhs |  (promoted_type) rhs.m_value;}
      template<typename T> friend mwg_constexpr typename binary_result<inttype, T>::type operator^ (T const& lhs, inttype const& rhs) {return lhs ^  (promoted_type) rhs.m_value;}
      template<typename T> friend mwg_constexpr typename binary_result<inttype, T>::type operator& (T const& lhs, inttype const& rhs) {return lhs &  (promoted_type) rhs.m_value;}

      mwg_constexpr bool operator==(inttype const& rhs) const {return m_value == rhs.m_value;}
      mwg_constexpr bool operator!=(inttype const& rhs) const {return m_value != rhs.m_value;}
      mwg_constexpr bool operator<(inttype const& rhs) const {return m_value < rhs.m_value;}
      mwg_constexpr bool operator>(inttype const& rhs) const {return m_value > rhs.m_value;}
      mwg_constexpr bool operator<=(inttype const& rhs) const {return m_value <= rhs.m_value;}
      mwg_constexpr bool operator>=(inttype const& rhs) const {return m_value >= rhs.m_value;}

      template<typename T> mwg_constexpr bool operator==(T const& rhs) const {return m_value == rhs;}
      template<typename T> mwg_constexpr bool operator!=(T const& rhs) const {return m_value != rhs;}
      template<typename T> mwg_constexpr bool operator< (T const& rhs) const {return m_value <  rhs;}
      template<typename T> mwg_constexpr bool operator> (T const& rhs) const {return m_value >  rhs;}
      template<typename T> mwg_constexpr bool operator<=(T const& rhs) const {return m_value <= rhs;}
      template<typename T> mwg_constexpr bool operator>=(T const& rhs) const {return m_value >= rhs;}

      template<typename T> friend mwg_constexpr bool operator==(T const& lhs, inttype const& rhs) {return lhs == rhs.m_value;}
      template<typename T> friend mwg_constexpr bool operator!=(T const& lhs, inttype const& rhs) {return lhs != rhs.m_value;}
      template<typename T> friend mwg_constexpr bool operator< (T const& lhs, inttype const& rhs) {return lhs <  rhs.m_value;}
      template<typename T> friend mwg_constexpr bool operator> (T const& lhs, inttype const& rhs) {return lhs >  rhs.m_value;}
      template<typename T> friend mwg_constexpr bool operator<=(T const& lhs, inttype const& rhs) {return lhs <= rhs.m_value;}
      template<typename T> friend mwg_constexpr bool operator>=(T const& lhs, inttype const& rhs) {return lhs >= rhs.m_value;}

      mwg_constexpr14 inttype& operator=(promoted_type value) {m_value = value; return *this;}
      mwg_constexpr14 inttype& operator*=(promoted_type value) {m_value *= value; return *this;}
      mwg_constexpr14 inttype& operator/=(promoted_type value) {m_value /= value; return *this;}
      mwg_constexpr14 inttype& operator%=(promoted_type value) {m_value %= value; return *this;}
      mwg_constexpr14 inttype& operator+=(promoted_type value) {m_value += value; return *this;}
      mwg_constexpr14 inttype& operator-=(promoted_type value) {m_value -= value; return *this;}
      mwg_constexpr14 inttype& operator>>=(promoted_type value) {m_value >>= value; return *this;}
      mwg_constexpr14 inttype& operator<<=(promoted_type value) {m_value <<= value; return *this;}
      mwg_constexpr14 inttype& operator|=(promoted_type value) {m_value |= value; return *this;}
      mwg_constexpr14 inttype& operator^=(promoted_type value) {m_value ^= value; return *this;}
      mwg_constexpr14 inttype& operator&=(promoted_type value) {m_value &= value; return *this;}

    };

  }

  using inttype_detail::inttype;

#pragma%x begin_test
  void test() {
    typedef mwg::cxx::inttype<mwg::stdm::uint16_t> c2t;

    c2t value = '0';
    mwg_check(value == value);
    mwg_check(value == (int) '0');
    mwg_check((int) '0' == value);
    mwg_check(value == '0');
    mwg_check('0' == value);

    c2t v2 = value;
    v2 += value;
    v2 += '0';
    mwg_check(value + value == 2 * value);
    mwg_check(value + 1 == '1');
    mwg_check(1 + value == '1');
  }
#pragma%x end_test

}
}
#endif
#pragma%x begin_check
int main() {
  managed_test::run_tests();
  return 0;
}
#pragma%x end_check
