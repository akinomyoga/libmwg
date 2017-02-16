// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_STDM_TYPE_TRAITS_IS_CONVERTIBLE
#define MWG_STDM_TYPE_TRAITS_IS_CONVERTIBLE
#include <mwg/concept.h>
#pragma%include ../impl/warning_push.inl
namespace mwg{
namespace stdm{
//HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH

// * 規格 [meta.rel] に従うと、
//   is_convertible の結果は以下が valid か否かと同値である。
//   template<typename From> typename mwg::stdm::add_rvalue_reference<From>::type create();
//   To test(){return create<From>();}
// * 各コンパイラで変換可能性が規格と異なる場合、
//   is_convertible は規格の指定する結果ではなくて、
//   そのコンパイラに於ける変換可能性を返す様に実装する方が良い。

#if defined(_MSC_VER) && (_MSC_FULL_VER >= 140050215)
# if (_MSC_VER == 1900)
  //
  // Note: これは msc19 の work around である。
  //   msc19 では何故か実際の動作と is_convertible の動作が異なる。
  //   実際の動作に合う様に修正を行う。
  //   cf. note/20131211.stdlib11-std_is_convertible-check_def.cpp
  //
  namespace detail {
    template<typename From, typename To>
    struct is_convertible_core: integral_constant<
      bool, is_void<From>::value && is_void<To>::value || __is_convertible_to(From, To)> {};

    template<
      typename From, typename To,

      typename from_cv_t = From,
      typename to_cv_t = To,
      typename from_raw_t = typename stdm::remove_cv<from_cv_t>::type,
      typename to_raw_t = typename stdm::remove_cv<to_cv_t>::type,
      bool value = (is_convertible_core<from_raw_t&, to_raw_t const&>::value &&
        is_const<to_cv_t>::value &&
        (is_volatile<to_cv_t>::value || !is_volatile<from_cv_t>::value))>
    struct is_convertible_rval2lref: integral_constant<bool, value> {};

    template<typename From, typename To>
    struct is_convertible_2: is_convertible_core<From, To> {};
    template<typename From, typename To>
    struct is_convertible_2<From&&, To& >: is_convertible_rval2lref<From, To> {};
    template<typename From, typename To>
    struct is_convertible_2<From& , To& >: is_convertible_core<From& , To& > {};
    template<typename From, typename To>
    struct is_convertible_2<From&&, To&&>: is_convertible_core<From&&, To&&> {};
    template<typename From, typename To>
    struct is_convertible_2<From& , To&&>: is_convertible_core<From& , To&&> {};

    template<typename From, typename To, bool = is_void<From>::value, bool = is_void<To>::value>
    struct is_convertible_1: is_convertible_2<typename add_rvalue_reference<From>::type, To> {};
    template<typename From, typename To>
    struct is_convertible_1<From, To, true, true>: true_type {};
    template<typename From, typename To>
    struct is_convertible_1<From, To, true, false>: false_type {};
    template<typename From, typename To>
    struct is_convertible_1<From, To, false, true>: false_type {};
  }

  template<typename From, typename To>
  struct is_convertible: detail::is_convertible_1<From, To> {};
# else
  template<typename From,typename To>
  struct is_convertible:integral_constant<bool,
    is_void<From>::value&&is_void<To>::value||__is_convertible_to(From,To)>{};
# endif
#elif defined(mwg_concept_is_valid_expression)
  template<typename From,typename To>
  struct is_convertible{
    template<typename T> static T expr();
    mwg_concept_is_valid_expression(mwg_is_convertible_to,From,F,To(expr<F>())); // ■←これだと explicit にしか変換できない場合も含むのでは?
    mwg_concept_condition(is_void<From>::value && is_void<To>::value || !is_void<From>::value && !is_void<To>::value && mwg_is_convertible_to::value);
  };
#else
  namespace detail{
// # if defined(_MSC_VER)&&_MSC_FULL_VER>=140050215
//     template<typename From,typename To>
//     struct is_convertible_core:integral_constant<bool,__is_convertible_to(From,To)>{};
// # elif defined(mwg_concept_is_valid_expression)
//     template<typename From,typename To>
//     mwg_concept_is_valid_expression(is_convertible_core,From,F,To(mwg::declval<F>()));
//# elif defined(__GNUC__)
# if defined(__GNUC__)
    struct param_force_conversion {
      template<typename T> param_force_conversion(const volatile T&) {}
      template<typename T> param_force_conversion(const T&) {}
      template<typename T> param_force_conversion(T&) {}
    };

    template<typename From,typename To,typename=void>
    struct is_convertible_core{
      mwg_concept_sfinae_param{
        mwg_concept_sfinae_param_true(X, (X, int));
        mwg_concept_sfinae_param_false(X, (param_force_conversion, ...));
      };
      mwg_concept_sfinae_param_check(To, (mwg::declval<From>(), 0));
    };

    // g++-3.4.6 が double->int narrowing conversion に対して警告を出すので、
    // narrowing conversion だけ特別扱いする。
    template<typename From,typename To>
    struct is_convertible_core<From,To,typename enable_if<(is_floating_point<From>::value&&is_integral<To>::value)>::type>:true_type{};
# else
    template<typename From,typename To>
    struct is_convertible_core: integral_constant<bool,is_void<From>::value&&is_void<To>::value||is_same<From,To>::value||is_base_of<To,From>::value>{};
# endif

    template<typename F,typename T> struct is_reference_convertible_nocv:is_base_of<T,F>{};
    template<typename F>            struct is_reference_convertible_nocv<F,void>:true_type{};
    template<typename F,typename T> struct is_reference_convertible:integral_constant<
      bool,
      (!((is_const<F>::value&&!is_const<T>::value)||(is_volatile<F>::value&&!is_volatile<T>::value))&&
        is_reference_convertible_nocv<typename remove_cv<F>::type,typename remove_cv<T>::type>::value)
      >{};

    template<typename F,typename T> struct is_convertible_noptr           :is_convertible_core<F,T>{};
    template<typename F           > struct is_convertible_noptr<F   ,void>:false_type{};
    template<           typename T> struct is_convertible_noptr<void,T   >:false_type{};
    template<                     > struct is_convertible_noptr<void,void>:true_type{};

    template<typename F,typename T> struct is_convertible_noref           :is_convertible_noptr<F,T>{};
    template<typename F,typename T> struct is_convertible_noref<F   ,T*>  :false_type{};
    template<typename F,typename T> struct is_convertible_noref<F*  ,T >  :false_type{};
    template<typename F,typename T> struct is_convertible_noref<F*  ,T*>  :is_reference_convertible<F,T>{};

    // TODO: rvalue_reference に対する対応?
    template<typename F,typename T> struct is_convertible_nocv          :is_convertible_noref<
      typename remove_cv<F>::type,
      typename remove_cv<T>::type>{};
    template<typename F,typename T> struct is_convertible_nocv <F   ,T&>:integral_constant<
      bool,(is_const<T>::value&&!is_volatile<F>::value&&!is_volatile<T>::value&&is_convertible_noref<F,T>::value)>{};
    template<typename F,typename T> struct is_convertible_nocv <F&  ,T >:is_convertible_noref<
      typename remove_cv<F>::type,
      typename remove_cv<T>::type>{};
    template<typename F,typename T> struct is_convertible_nocv <F&  ,T&>:is_reference_convertible<F,T>{};
  }

  template<typename From,typename To>
  struct is_convertible
    :detail::is_convertible_nocv<From,To>{};

#endif

//HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
}
}
#pragma%include ../impl/warning_pop.inl
#endif
