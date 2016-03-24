// -*- mode:C++;coding:utf-8 -*-
#ifndef MWG_STDM_TYPE_TRAITS__IS_CONVERTIBLE
#define MWG_STDM_TYPE_TRAITS__IS_CONVERTIBLE
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

#if defined(_MSC_VER)&&_MSC_FULL_VER>=140050215
  template<typename From,typename To>
  struct is_convertible:integral_constant<bool,
    is_void<From>::value&&is_void<To>::value||__is_convertible_to(From,To)
  >{};
#elif defined(mwg_concept_is_valid_expression)
  template<typename From,typename To>
  struct is_convertible{
    template<typename T> static T expr();
    mwg_concept_is_valid_expression(mwg_is_convertible_to,From,F,To(expr<F>())); // ■←これだと explicit にしか変換できない場合も含むのでは?
    mwg_concept_condition(is_void<From>::value&&is_void<To>::value||!is_void<From>::value&&!is_void<To>::value&&mwg_is_convertible_to::value);
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
    struct param_force_conversion{
      template<typename T> param_force_conversion(const volatile T&){}
      template<typename T> param_force_conversion(const T&){}
      template<typename T> param_force_conversion(T&){}
    };

    template<typename From,typename To>
    struct is_convertible_core{
      mwg_concept_sfinae_param{
        mwg_concept_sfinae_param_true(X,(X,int));
        mwg_concept_sfinae_param_false(X,(param_force_conversion,...));
      };
      mwg_concept_sfinae_param_check(To,(mwg::declval<From>(),0));
    };
# else
    template<typename From,typename To>
    struct is_convertible_core:integral_constant<bool,is_void<From>::value&&is_void<To>::value||is_same<From,To>::value||is_base_of<To,From>::value>{};
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
