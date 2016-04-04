// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef MWG_FUNCSIG_H
#define MWG_FUNCSIG_H
#include <cstddef>
#include <mwg/defs.h>
namespace mwg{
namespace funcsig{
#pragma%include "impl/VariadicMacros.pp"
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  // arity
  template<typename S> struct arity{static const int value=-1;};
#pragma%m 1
  template<typename R,typename... A>
  struct arity<R(A...)>{static const int value=sizeof...(A);};
  template<typename R,typename... A>
  struct arity<R(A...,...)>{static const int value=sizeof...(A);};
#pragma%end
#pragma%x variadic_expand_0toArN

  // arity_pop
  template<typename S> struct arity_pop{};
#pragma%m 1
  template<typename R,typename ATail,typename... A>
  struct arity_pop<R(A...,ATail)>:mwg::identity<R(A...)>{};
#pragma%end
#pragma%x variadic_expand_0toArNm1

  // arity_push
  template<typename S,typename ATail>
  struct arity_push{};
#pragma%m 1
  template<typename R,typename ATail,typename... A>
  struct arity_push<R(A...),ATail>:mwg::identity<R(A...,ATail)>{};
#pragma%end
#pragma%x variadic_expand_0toArNm1

  template<std::size_t I,typename F>
  struct parameter{};
  template<std::size_t I,typename R>
  struct parameter<I,R()>:mwg::identity<void>{};
  template<std::size_t I,typename R>
  struct parameter<I,R(...)>:mwg::identity<void>{};
#pragma%m 1
  template<typename R,typename AHead,typename... A>
  struct parameter<0,R(AHead,A...)>:mwg::identity<AHead>{};
  template<std::size_t I,typename R,typename AHead,typename... A>
  struct parameter<I,R(AHead,A...)>:parameter<I-1,R(A...)>{};
  template<typename R,typename AHead,typename... A>
  struct parameter<0,R(AHead,A...,...)>:mwg::identity<AHead>{};
  template<std::size_t I,typename R,typename AHead,typename... A>
  struct parameter<I,R(AHead,A...,...)>:parameter<I-1,R(A...,...)>{};
#pragma%end
#pragma%x variadic_expand_0toArNm1

  template<typename F>
  struct returns{};
#pragma%m 1
  template<typename R,typename... A>
  struct returns<R(A...)>:mwg::identity<R>{};
  template<typename R,typename... A>
  struct returns<R(A...,...)>:mwg::identity<R>{};
#pragma%end
#pragma%x variadic_expand_0toArN

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
