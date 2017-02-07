// -*- mode: C++; coding: utf-8 -*-
#ifndef MWG_FUNSIG_H
#define MWG_FUNSIG_H
#pragma%include "../../impl/VariadicMacros.pp"
#include <cstddef>
#include <mwg/defs.h>
#include <mwg/concept.h>
namespace mwg {
namespace funsig {

  namespace detail {
    //
    // Note: テンプレート特殊化の選択で R(A..., T) のパターンにマッチさせる事はできない。
    //   従って、最後の引数に対しては Arity - 1 回 rotate してから処理する必要がある。
    //

    template<typename S, std::size_t N, template<typename A> class Filter>
    struct filtered_rotate: filtered_rotate<typename filtered_rotate<S, 1, Filter>::type, N - 1, Filter> {};
    template<template<typename A> class Filter, typename S>
    struct filtered_rotate<S, 0, Filter>: mwg::identity<S> {};
#pragma%m 1
    template<template<class A> class Filter, class R, class Head, class... A>
    struct filtered_rotate<R (Head, A...), 1, Filter>: mwg::identity<R(A..., typename Filter<Head>::type)> {};
#pragma%end
#pragma%x variadic_expand_0toArNm1

    template<std::size_t Arity, typename R, typename A0, typename Unshift>
    struct extract_def {
      static mwg_constexpr_const std::size_t arity = Arity;
      typedef R result_t;
      typedef A0 first_parameter_t;
      typedef Unshift unshift_t;
    };
    template<typename S>
    struct extract {};
    template<typename R>
    struct extract<R()>: extract_def<0, R, void, R()> {};
#pragma%m 1
    template<typename R, typename T, typename... A>
    struct extract<R(T, A...)>: extract_def<1 + sizeof...(A), R, T, R(A...)> {};
#pragma%end
#pragma%x variadic_expand_0toArNm1

    template<typename Push, typename Shift, typename SetResult>
    struct transform_def {
      typedef Push push_t;
      typedef Push shift_t;
      typedef SetResult set_result_t;
    };
    template<typename S, typename T>
    struct transform {};
#pragma%m 1
    template<typename T, typename R, typename... A>
    struct transform<R(A...), T>: transform_def<R(A..., T), R(T, A...), T(A...)> {};
#pragma%end
#pragma%x variadic_expand_0toArN
  }

  template<typename S>
  struct arity: stdm::integral_constant<std::size_t, detail::extract<S>::arity> {};

  template<typename S, std::ptrdiff_t N = 1>
  struct rotate: detail::filtered_rotate<S, (N % arity<S>::value + N) % arity<S>::value, mwg::identity> {};
  template<typename R>
  struct rotate<R()>: mwg::identity<R()> {};
  template<typename S, template<typename A> class Filter>
  struct filter: detail::filtered_rotate<S, arity<S>::value, Filter> {};

  template<typename S, typename A>
  struct shift: mwg::identity<typename detail::transform<S, A>::shift_t> {};
  template<typename S, typename A>
  struct push: mwg::identity<typename detail::transform<S, A>::push_t> {};
  template<typename S>
  struct unshift: mwg::identity<typename detail::extract<S>::unshift_t> {};
  template<typename S>
  struct pop: mwg::identity<typename detail::extract<typename rotate<S, -1>::type>::unshift_t> {};

  template<typename S, typename A = mwg::invalid_type>
  struct result: mwg::identity<typename detail::transform<S, A>::set_result_t> {};
  template<typename S>
  struct result<S>: mwg::identity<typename detail::extract<S>::result_t> {};

  namespace detail {
    template<
      typename S, std::ptrdiff_t I, typename New,

      std::size_t N = arity<S>::value,
      typename S1 = typename rotate<S, I>::type,
      typename S2 = typename unshift<S1>::type,
      typename S3 = typename shift<S2, New>::type,
      typename S4 = typename rotate<S3, N - I>::type>
    struct set_parameter: mwg::identity<S4>::type {};
  }
  template<typename S, std::ptrdiff_t I, typename A = mwg::invalid_type>
  struct parameter: detail::set_parameter<S, I, A> {};
  template<typename S, std::ptrdiff_t I>
  struct parameter<S, I>: mwg::identity<typename detail::extract<typename rotate<S, I>::type>::first_parameter_t> {};
}
}
#endif
