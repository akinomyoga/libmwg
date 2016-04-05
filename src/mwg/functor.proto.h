// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef MWG_FUNCTOR_PROTO_H
#define MWG_FUNCTOR_PROTO_H
namespace mwg{
namespace functor_detail{
  template<typename F>
  struct functor_case_traits;
    // Tr::fct_t
    // Tr::fct_tr functor_traits<F>
    // Tr::case_data
    // Tr::endata
    // Tr::dedata
  template<typename F,typename S=void>
  struct functor_traits;

  struct functor_traits_empty;
  template<typename S>
  struct functor_traits_signature;

  template<typename S>
  struct functor_case;
  template<typename S,typename T,bool INTERIOR>
  class functor_case_data;
  template<typename S,typename Tr>
  class functor_case_impl;

  template<typename S>
  struct functor_base;
  template<typename S> class functor;
  template<typename S> class functor_ref;
  template<typename S> class vfunctor;
  template<typename S> class vfunctor_ref;

  template<typename F,typename S> struct is_functor;
  template<typename F,typename S> struct be_functor;
}

  using functor_detail::functor_traits;

  using functor_detail::functor;
  using functor_detail::functor_ref;
  using functor_detail::vfunctor;
  using functor_detail::vfunctor_ref;

  using functor_detail::is_functor;
  using functor_detail::be_functor;
}
#endif

