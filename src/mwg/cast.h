// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_CAST_H
#define MWG_CAST_H
#include <cstdlib>
#include <sstream>
#include <string>
#include <complex>
#include <mwg_config.h>
namespace mwg {
namespace detail {
  // TODO: struct lexical_bad_cast:bad_cat{};

  template<typename T, typename S, typename F = void>
  struct lexical_cast_impl {};

  template<typename T, typename C>
  struct lexical_cast_impl<T, std::basic_string<C> > {
    typedef lexical_cast_impl<T, const C*> base;
    typedef typename base::return_type return_type;
    static return_type cast(const std::basic_string<C>& value) {
      return base::cast(value.c_str());
    }
  };

  template<typename S>
  struct lexical_cast_impl<std::string, S> {
    typedef std::string return_type;
    static return_type cast(const S& value) {
      std::ostringstream buff;
      buff << value;
      return buff.str();
    }
  };

  template<typename T>
  struct lexical_cast_impl<T, const char*> {
    typedef T return_type;
    static return_type cast(const char* value) {
      std::istringstream buff(value);
      return_type ret;
      buff >> ret;
      return ret;
    }
  };

  //?mconf X ::atoll     'stdlib.h' '::atoll("0")'
  //?mconf X ::strtoll   'stdlib.h' '::strtoll("0",NULL,10)'
  //?mconf X ::_atoi64   'stdlib.h' '::_atoi64("0")'
  //?mconf X ::_strtoi64 'stdlib.h' '::_strtoi64("0",NULL,10)'

#define mwg_tmp_define_lexical_cast_by_expr(T,S,EXPR) \
  template<>                                          \
  struct lexical_cast_impl<T, S> {                    \
    typedef T return_type;                            \
    static return_type cast(S const& value) {         \
      return EXPR;                                    \
    }                                                 \
  }

  mwg_tmp_define_lexical_cast_by_expr(double, const char*, std::atof(value));
  mwg_tmp_define_lexical_cast_by_expr(float, const char*, float(std::atof(value)));
  mwg_tmp_define_lexical_cast_by_expr(int, const char*, std::atoi(value));
  mwg_tmp_define_lexical_cast_by_expr(long, const char*, std::atol(value));
#if defined(MWGCONF_HAS_LONGLONG)&&defined(MWGCONF_HAS_ATOLL)
  mwg_tmp_define_lexical_cast_by_expr(long long, const char*, ::atoll(value));
#elif defined(MWGCONF_HAS_LONGLONG)&&defined(MWGCONF_HAS_STRTOLL)
  mwg_tmp_define_lexical_cast_by_expr(long long, const char*, ::strtoll(value, NULL, 10));
#elif defined(MWGCONF_HAS_INT64)&&defined(MWGCONF_HAS__ATOI64)
  mwg_tmp_define_lexical_cast_by_expr(__int64, const char*, ::_atoi64(value));
#elif defined(MWGCONF_HAS_INT64)&&defined(MWGCONF_HAS__STRTOI64)
  mwg_tmp_define_lexical_cast_by_expr(__int64, const char*, ::_strtoi64(value, NULL, 10));
#endif

  template<typename T, typename S, typename F = void>
  struct adapter_cast_impl {};
}

  template<typename T,typename S>
  typename detail::lexical_cast_impl<T, S>::return_type
  lexical_cast(const S& value) {
    return detail::lexical_cast_impl<T, S>::cast(value);
  }

  template<typename T, typename S>
  typename detail::adapter_cast_impl<T, S>::return_type
  adapter_cast(const S& value) {
    return detail::adapter_cast_impl<T, S>::cast(value);
  }
}
#undef mwg_tmp_define_lexical_cast_by_expr
#endif
