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

  float logb(float value) {return logb_impl(value);}
  double logb(double value) {return logb_impl(value);}
  long double logb(long double value) {return logb_impl(value);}

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
      man += std::numeric_limits<T>::epsilon();
    else
      man -= std::numeric_limits<T>::epsilon();
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

#define MWG_STDM_CMATH_Defines_nbint
#ifdef MWG_STDM_CMATH_Defines_nbint
# include <mwg/std/cfenv>
namespace mwg {
namespace stdm {
namespace cmath_detail {

  template<typename T>
  static T nbint_impl(T value) {
    switch (fegetround()) {
    case FE_DOWNWARD:
      return std::floor(value);
    case FE_UPWARD:
      return std::ceil(value);
    case FE_TOWARDZERO:
      return mwg::stdm::trunc(value);
    case FE_TONEAREST:
    default:
      if (mwg::stdm::isnan(value)) return value;
      T const ret = std::floor(value);
      T const frac = value - ret;
      return (frac == (T) 0.5? mwg::stdm::trunc(ret * (T) 0.5) == ret * (T) 0.5: frac < (T) 0.5)? ret: ret + T(1.0);
    }
  }

  float nbint(float value) {return nbint_impl(value);}
  double nbint(double value) {return nbint_impl(value);}
  long double nbint(long double value) {return nbint_impl(value);}

}
}
}
#endif

#if defined(MWG_STDM_CMATH_Defines_lint) || defined(MWG_STDM_CMATH_Defines_llint)
# include <mwg/std/cfenv>
# include <mwg/std/limits>
namespace mwg {
namespace stdm {
namespace cmath_detail {

  template<typename R, typename T>
  static R lint_impl(T value) {
    if (mwg::stdm::isnan(value)) {
      feraiseexcept(FE_INVALID);
      return (R) 0;
    }

    if (value < mwg::stdm::numeric_limits<R>::lowest()) {
      feraiseexcept(FE_INVALID);
      return mwg::stdm::numeric_limits<R>::lowest();
    }

    if (mwg::stdm::numeric_limits<R>::max() < value) {
      feraiseexcept(FE_INVALID);
      return mwg::stdm::numeric_limits<R>::max();
    }

    return (R) value;
  }

# ifdef MWG_STDM_CMATH_Defines_lint
  long lint(float value) {return lint_impl<long>(value);}
  long lint(double value) {return lint_impl<long>(value);}
  long lint(long double value) {return lint_impl<long>(value);}
# endif
# ifdef MWG_STDM_CMATH_Defines_llint
  long long llint(float value) {return lint_impl<long long>(value);}
  long long llint(double value) {return lint_impl<long long>(value);}
  long long llint(long double value) {return lint_impl<long long>(value);}
# endif

}
}
}
#endif

#if !defined(MWGCONF_HAS_STD_REMQUO) && !defined(MWGCONF_HAS_REMQUO)
namespace mwg {
namespace stdm {
namespace cmath_detail {

  template<typename T>
  static T remquo_impl(T lhs, T rhs, int* quo) {
    if (mwg::stdm::isnan(lhs) || mwg::stdm::isnan(rhs)) return lhs;
    if (mwg::stdm::isinf(lhs) || rhs == (T) 0.0) {
      feraiseexcept(FE_INVALID);
      return NAN;
    }

    T const div = lhs / rhs;
    T const flor = std::floor(div);
    T const frac = div - flor;
    T const n = (frac == (T) 0.5? mwg::stdm::trunc(flor * (T) 0.5) == flor * (T) 0.5: frac < (T) 0.5)? flor: flor + T(1.0);
    *quo = (int) n;
    return lhs - n * rhs;
  }

  float remquo(float lhs, float rhs, int* quo) {return remquo_impl(lhs, rhs, quo);}
  double remquo(double lhs, double rhs, int* quo) {return remquo_impl(lhs, rhs, quo);}
  long double remquo(long double lhs, long double rhs, int* quo) {return remquo_impl(lhs, rhs, quo);}

}
}
}
#endif

#if !defined(MWGCONF_HAS_STD_NAN) && !defined(MWGCONF_HAS_NAN)
#include <cstdlib>
#include <cstring>
namespace mwg {
namespace stdm {
namespace cmath_detail {

  static inline const char* nan_strtod_arg(const char* arg) {
    static std::size_t size = 0;
    static char* data = nullptr;

    std::size_t const len = arg? 6 + std::strlen(arg): 4;
    if (size < len) {
      if (data) std::free(data);
      size = len + 10;
      data = (char*) std::malloc(size);
      std::strcpy(data, "NAN");
    }

    if (arg) {
      data[3] = '\0';
    } else {
      data[3] = '(';
      std::strcpy(data + 4, arg);
      data[len - 2] = ')';
      data[len - 1] = '\0';
    }

    return data;
  }
  double nan(const char* arg) {
    return std::strtod(cmath_detail::nan_strtod_arg(arg), (char**) nullptr);
  }
  float nanf(const char* arg) {
#ifdef MWGCONF_HAS_STD_STRTOF
    return mwg::stdm::strtof(cmath_detail::nan_strtod_arg(arg), (char**) nullptr);
#elif defined(MWGCONF_HAS_STRTOF)
    return ::stdm::strtof(cmath_detail::nan_strtod_arg(arg), (char**) nullptr);
#else
    return (float) nan(arg);
#endif
  }
  long double nanl(const char* arg) {
#ifdef MWGCONF_HAS_STD_STRTOLD
    return mwg::stdm::strtold(cmath_detail::nan_strtod_arg(arg), (char**) nullptr);
#elif defined(MWGCONF_HAS_STRTOLD)
    return ::stdm::strtold(cmath_detail::nan_strtod_arg(arg), (char**) nullptr);
#else
    return (long double) nan(arg);
#endif
  }
}
}
}
#endif
