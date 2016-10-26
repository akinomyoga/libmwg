// -*- mode: c++; coding: utf-8 -*-
#include <mwg/std/cmath>

#if !defined(MWGCONF_HAS_STD_ILOGB) && !defined(MWGCONF_HAS_ILOGB)
# include <mwg/std/cfenv>
namespace mwg {
namespace stdm {
namespace cmath_detail {

  template<typename T>
  static int ilogb_impl(T value) {
    T const _value = std::abs((T) value);
    if (mwg::stdm::isfinite(_value)) {
      if (_value == (T) 0.0) {
        mwg::stdm::feraiseexcept(FE_INVALID);
        return FP_ILOGB0;
      }

      if (FLT_RADIX == 2) {
        int ret;
        std::frexp(_value, &ret);
        return ret + 1;
      } else {
        int const ret = (int) (std::log(_value) / std::log((T) FLT_RADIX));
        return scalbn(_value, -ret) < 1.0? ret - 1: ret;
      }
    } else if (mwg::stdm::isinf(_value)) {
      mwg::stdm::feraiseexcept(FE_INVALID);
      return INT_MAX;
    } else {
      mwg::stdm::feraiseexcept(FE_INVALID);
      return FP_ILOGBNAN;
    }
  }

  int ilogb(float value) {return ilogb_impl(value);}
  int ilogb(double value) {return ilogb_impl(value);}
  int ilogb(long double value) {return ilogb_impl(value);}

}
}
}
#endif

#if !defined(MWGCONF_HAS_STD_LOGB) && !defined(MWGCONF_HAS_LOGB)
# include <mwg/std/cfenv>
namespace mwg {
namespace stdm {
namespace cmath_detail {

  template<typename T>
  static T logb_impl(T value) {
    T const _value = std::abs((T) value);
    if (mwg::stdm::isfinite(_value)) {
      if (_value == (T) 0.0) {
        mwg::stdm::feraiseexcept(FE_DIVBYZERO);
        return (T) -INFINITY;
      }

      if (FLT_RADIX == 2) {
        int ret;
        std::frexp(_value, &ret);
        return ret + 1;
      } else {
        int const ret = (int) (std::log(_value) / std::log((T) FLT_RADIX));
        return (T) (scalbn(_value, -ret) < 1.0? ret - 1: ret);
      }
    } else if (mwg::stdm::isinf(_value)) {
      return (T) INFINITY;
    } else {
      return (T) NAN;
    }
  }

  int logb(float value) {return logb_impl(value);}
  int logb(double value) {return logb_impl(value);}
  int logb(long double value) {return logb_impl(value);}

}
}
}
#endif

#if !defined(MWGCONF_HAS_STD_NEXTAFTER) && !defined(MWGCONF_HAS_NEXTAFTER)
# include <mwg/std/cfenv>
# include <mwg/std/limits>
namespace mwg {
namespace stdm {
namespace cmath_detail {

  template<typename T>
  static T nextafter_impl(T from, T to) {
    if (mwg::stdm::isnan(from) || mwg::stdm::isnan(to))
      return NAN;
    if (from == to)
      return to;
    if (mwg::stdm::isinf(from))
      return from > T(0.0)? mwg::stdm::numeric_limits<T>::max(): mwg::stdm::numeric_limits<T>::lowest();
    if (from == 0.0)
      return mwg::stdm::copysign(mwg::stdm::numeric_limits<T>::min(), to);

    int const exp = mwg::stdm::logb(from);
    T man = mwg::stdm::scalbn(from, -exp);
    if (from < to)
      man += mwg::stdm::numeric_limits<T>::epsilon();
    else
      man -= mwg::stdm::numeric_limits<T>::epsilon();
    T const next = mwg::stdm::scalbn(man, exp);

    if (mwg::stdm::isinf(next))
      feraiseexcept(FE_INEXACT | FE_OVERFLOW);
    else if (next == 0.0 || !mwg::stdm::isnormal(next))
      feraiseexcept(FE_INEXACT | FE_UNDERFLOW);

    return next;
  }

  float nextafter(float lhs, float rhs) {return nextafter_impl(lhs, rhs);}
  double nextafter(double lhs, double rhs) {return nextafter_impl(lhs, rhs);}
  long double nextafter(long double lhs, long double rhs) {return nextafter_impl(lhs, rhs);}

}
}
}
#endif
