// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BIO_DEFS_H
#define MWG_BIO_DEFS_H
#include <cstdio>
#include <istream>
#include <mwg/defs.h>
#include <mwg/except.h>
#include <mwg/std/type_traits>
namespace mwg {
namespace bio {
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

class readonly_error;

mwg_define_class_error_ex(io_error, mwg::except, mwg::ecode::io);
mwg_define_class_error_ex(nosupport_error, io_error, mwg::ecode::ESupport);
mwg_define_class_error_ex(file_open_error, io_error, 0);
mwg_define_class_error_ex(ill_format_error, io_error, 0);

// header files:
//  three models to access binary data
//    <mwg/bio/tape.h>
//    <mwg/bio/view.h>
//    <mwg/bio/disk.h>
//  experimental headers
//    <mwg/bio/mwt-experimental2.h>
//    <mwg/bio/xml3.h>

class itape;
class idisk;
class iview;

#ifndef MWG_SYS_BIGENDIAN
// ※ちゃんと括弧で括らないと誤解釈するコンパイラが存在するようだ。
# if ( \
    defined(__BYTE_ORDER)&&defined(__BIG_ENDIAN)&&defined(__LITTLE_ENDIAN)? \
    defined(__BYTE_ORDER)&&(__BYTE_ORDER==__BIG_ENDIAN):(               \
      defined(__BYTE_ORDER__)&&defined(__ORDER_BIG_ENDIAN__)&&defined(__ORDER_LITTLE_ENDIAN__)? \
      defined(__BYTE_ORDER__)&&(__BYTE_ORDER__==__ORDER_BIG_ENDIAN__):( \
        defined(_BIG_ENDIAN)!=defined(_LITTLE_ENDIAN)?defined(_BIG_ENDIAN):( \
          defined(__BIG_ENDIAN)!=defined(__LITTLE_ENDIAN)?defined(__BIG_ENDIAN):( \
            defined(__BIG_ENDIAN__)!=defined(__LITTLE_ENDIAN__)?defined(__BIG_ENDIAN__): \
            0 /* defaultly little endian */)))))
#   define MWG_SYS_BIGENDIAN
# endif
#endif

namespace detail {

  static const int adapter_cast_from_other   = -1;
  static const int adapter_cast_from_tape    = 1;
  static const int adapter_cast_from_disk    = 2;
  static const int adapter_cast_from_view    = 3;
  static const int adapter_cast_from_stream  = 10;
  static const int adapter_cast_from_istream = 11;
  static const int adapter_cast_from_ostream = 12;
  static const int adapter_cast_from_file    = 13;
  template<typename R, typename A>
  struct adapter_cast_switch {
    static const int value =
      stdm::is_base_of<itape, A>::value ? adapter_cast_from_tape :
      stdm::is_base_of<idisk, A>::value ? adapter_cast_from_disk :
      stdm::is_base_of<iview, A>::value ? adapter_cast_from_view :
      stdm::is_base_of<std::iostream, A>::value ? adapter_cast_from_stream :
      stdm::is_base_of<std::istream, A>::value  ? adapter_cast_from_istream :
      stdm::is_base_of<std::ostream, A>::value  ? adapter_cast_from_ostream :
      stdm::is_same<FILE*, A>::value            ? adapter_cast_from_file :
      adapter_cast_from_other;
  };

  template<typename R, typename A, int I = 0>
  struct adapter_cast_impl {
    //typedef return_type;
    //static return_type create_adapter(...);
  };

  template<typename R, typename A>
  struct adapter_cast_impl<R, A>:
    adapter_cast_impl<R, A, adapter_cast_switch<R, A>::value> {};

  template<typename R, typename A>
  struct adapter_cast_simple_construct {
    typedef R return_type;
    static return_type create_adapter(const A& tape) { return return_type(tape); }
  };
}

template<typename R, typename A>
typename mwg::bio::detail::adapter_cast_impl<R, A>::return_type
  adapter_cast(const A& value)
{
  return mwg::bio::detail::adapter_cast_impl<R, A>::create_adapter(value);
}

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
