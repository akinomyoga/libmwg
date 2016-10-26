// -*- mode: c++; coding: utf-8 -*-
#include <mwg/std/cfenv>
#include <mwg/std/cmath>

#if !defined(MWGCONF_HAS_STD_ILOGB) && !defined(MWGCONF_HAS_ILOGB)
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
