// -*- mode:C++;coding:utf-8 -*-
#ifndef MWG_FUNCSIG_H
#define MWG_FUNCSIG_H
namespace mwg{
namespace funcsig{
#pragma%include "bits/functor/functor.variadic.pp"
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  // decrease_arity
  template<typename S> struct decrease_arity{};
#pragma%define 1 (
  template<typename R %s_typenames% ,typename AA>
  struct decrease_arity<R(%types% ${.eval#%AR%!="0"?",":""} AA)>{
    typedef R(type)(%types%);
  };
#pragma%)
#pragma%expand mwg::functor::arities

  // get_arity
  template<typename S> struct get_arity{static const int value=-1;};
#pragma%define 1 (
  template<typename R %s_typenames%>
  struct get_arity<R(%types%)>{static const int value=%AR%;};
#pragma%)
#pragma%expand mwg::functor::arities

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
