// -*- mode: c++; coding: utf-8 -*-
#include <mwg/std/type_traits>
#include <mwg/concept.h>
#include "funcsig.h"

namespace mwg{
namespace functor_detail{
  namespace sig=mwg::funcsig;

  //?lwiki *bits/functor.type_traits.pp
  //---------------------------------------------------------------------------
  /*?lwiki
   * :@class class mwg::functor_detail::==is_vararg_function==<F>;
   *  :@var static const bool ==value==;
   *   `F` が可変長引数を持つ関数型かどうかを判定します。
   *  :@typedef mwg::functor_detail::is_vararg_function<F>::==without_vararg_sgn==;
   *   `F` が可変長引数を持つ関数型の場合に、可変長部分を除いた関数型を取得します。
   */
  template<typename F>
  struct is_vararg_function:stdm::false_type{typedef void without_vararg_sgn;};
#pragma%m 1
  template<typename R,typename... A>
  struct is_vararg_function<R(A...,...)>:stdm::true_type{
    typedef R (without_vararg_sgn)(A...);
  };
#pragma%end
#pragma%x variadic_expand_0toArN

  //---------------------------------------------------------------------------
  /*?lwiki
   * :@class class mwg::functor_detail::==is_vararg_function_pointer==<F>;
   *  :@var static const bool ==value==;
   *   `F` が可変長引数を持つ関数ポインタ型かどうかを判定します。
   *  :@typedef typedef '''function-type''' ==signature_type==;
   *   `F` が可変長引数を持つ関数ポインタ型の場合に、関数型を取得します。
   *  :@typedef typedef '''function-type''' ==without_vararg_sgn==;
   *   `F` が可変長引数を持つ関数ポインタ型の場合に、可変長部分を除いた関数型を取得します。
   */
  template<typename T>
  struct is_vararg_function_pointer:stdm::false_type{typedef void signature_type;};
  template<typename F>
  struct is_vararg_function_pointer<F*>:is_vararg_function<F>{typedef F signature_type;};

  //---------------------------------------------------------------------------
  /*?lwiki
   * :@class class mwg::functor_detail::==is_function_pointer==<F>;
   *  :@var static const bool ==value==;
   *   `F` が関数ポインタ型かどうかを判定します。
   *  :@typedef typedef '''function-type''' ==signature_type==;
   *   `F` が関数ポインタ型の場合に、関数型を取得します。
   */
  template<typename T>
  struct is_function_pointer:stdm::false_type{typedef void signature_type;};
  template<typename F>
  struct is_function_pointer<F*>:stdm::is_function<F>{typedef F signature_type;};

  //---------------------------------------------------------------------------
  /*?lwiki
   * :@class class mwg::functor_detail::==is_memfun_pointer==<Mfp>;
   *  :@var static const bool ==value==;
   *   `Mfp` がメンバ関数へのポインタかどうかを判定します。
   *  :@typedef typedef '''member-pointer-type''' ==member_type==;
   *   `Mfp` がメンバ関数へのポインタの時、関数型を取得します。
   *  :@typedef typedef '''class-type''' ==object_type==;
   *   `Mfp` がメンバ関数へのポインタの時、メンバが定義されるクラスを取得します。
   *  :@typedef typedef '''function-type''' ==functor_sgn==;
   *   `Mfp` がメンバ関数へのポインタの時、メンバ関数ポインタを関手として解釈する時の関数型を取得します。
   *   第一引数に `object_type` を受け取り、第二引数以降に本来の引数を受け取ります。
   */
  template<typename Mfp>
  struct is_memfun_pointer:stdm::false_type{
    typedef void member_type;
    typedef void object_type;
    typedef void functor_sgn;
  };
#pragma%m 1
  template<typename R,typename C,typename... A>
  struct is_memfun_pointer<R(C::*)(A...) QUALIFIER>:stdm::true_type{
    typedef C QUALIFIER object_type;
    typedef R (member_type)(A...);
    typedef R (functor_sgn)(C QUALIFIER&,A...);
  };
#pragma%end
#pragma%m 1
#pragma%x 1.r/QUALIFIER//
#pragma%x 1.r/QUALIFIER/const/
#pragma%x 1.r/QUALIFIER/volatile/
#pragma%x 1.r/QUALIFIER/const volatile/
#pragma%end
#pragma%x variadic_expand_0toArN

  //---------------------------------------------------------------------------
  template<typename T> T expr();

  //---------------------------------------------------------------------------
  /*?lwiki
   * :@var mwg::functor_detail::detail::==is_covariant==<typename F,typename T>::value;
   *  `F` の実引数を `T` の仮引数に渡す事ができるかどうかを判定します。
   *  `T=void` の場合は仮引数がないことを意味し、呼び出しに実引数を必要としないので常に `true` になります。
   */
  template<typename F,typename T>
  struct is_covariant:stdm::integral_constant<bool,(stdm::is_void<T>::value||stdm::is_convertible<F,T>::value)>{};

  //---------------------------------------------------------------------------
  /*?lwiki
   * :@var mwg::functor_detail::==is_variant_function==<typename F,typename T>::value;
   *  関数型 `F` を関数型 `T` に変換できるかどうかを判定します。
   *  `T` として受け取った引数を `F` の引数に変換でき (引数の反変性)、
   *  かつ、`F` の戻り値を `T` の戻り値に変換できる (戻り値の共変性) 必要があります。
   */
  template<typename FSgn,typename TSgn>
  struct is_variant_function;

  namespace detail{
    template<
      typename FromSignature,typename ToSignature,
      std::size_t K      = 0,
      typename FromParam = typename sig::parameter<K,FromSignature>::type,
      typename ToParam   = typename sig::parameter<K,ToSignature  >::type,
      bool               = is_covariant<ToParam,FromParam>::value>
    struct has_contravariant_parameters:stdm::false_type{};

    template<
      typename FromSignature,typename ToSignature,std::size_t K,
      typename FromParam,typename ToParam>
    struct has_contravariant_parameters<FromSignature,ToSignature,K,FromParam,ToParam,true>
      :has_contravariant_parameters<FromSignature,ToSignature,K+1>{};

    template<typename FromSignature,typename ToSignature,std::size_t K>
    struct has_contravariant_parameters<FromSignature,ToSignature,K,void,void,true>:stdm::true_type{};
  }

  template<typename FSgn,typename TSgn>
  struct is_variant_function:stdm::integral_constant<bool,
    (is_covariant<typename sig::returns<FSgn>::type,typename sig::returns<TSgn>::type>::value&&
      detail::has_contravariant_parameters<FSgn,TSgn>::value)>{};

  //---------------------------------------------------------------------------
  /*?lwiki
   * :@def #define ==MWG_FUNCTOR_H__VariantFunctorEnabled==
   *  関数の共変性・反変性を有効にするかどうかを決定します。
   */
#if (\
    defined(mwg_concept_is_valid_expression)\
    ||defined(_MSC_VER)&&defined(mwg_concept_is_valid_expression_vc2010A)\
  )
#   define MWG_FUNCTOR_H__VariantFunctorEnabled
#endif

  //---------------------------------------------------------------------------
  /*?lwiki
   * :@var mwg::functor_detail::==has_single_operator_functor==<typename F>::value;
   *  `mwg::declval<F>.operator()` が有効な型かどうかを判定します。
   */
#ifdef mwg_concept_is_valid_expression
  template<typename F>
  mwg_concept_is_valid_expression(has_single_operator_functor,F,F_,&F_::operator());
#elif defined(_MSC_VER)&&defined(mwg_concept_is_valid_expression_vc2010A)
  template<typename F>
  mwg_concept_is_valid_expression_vc2010A(has_single_operator_functor,F,F_,((F_*)0)->operator());
#else
  template<typename F>
  struct has_single_operator_functor:mwg::stdm::false_type{};
#endif

  //---------------------------------------------------------------------------
  /*?lwiki
   * :@class class mwg::functor_detail::==is_pointer_to_single_operator_functor==<P>;
   *  :@var static const bool ==value==;
   *   `mwg::declval<P>->operator()` が有効な型かどうかを判定します。
   *   唯一の `operator()` が定義されている場合に有効な型になります。
   *   複数の overload が定義されている場合には無効です。
   *  :@typedef typedef '''function-type''' ==operator_type==;
   *   `operator()` のメンバ関数へのポインタ型を取得します。
   */
  template<typename P>
  struct is_pointer_to_single_operator_functor{
#if defined(MWGCONF_STD_DECLTYPE)

# ifdef mwg_concept_is_valid_expression
    mwg_concept_is_valid_expression(c1,P,P_,*expr<P_>());
# elif defined(_MSC_VER)&&defined(mwg_concept_is_valid_expression_vc2010A)
    mwg_concept_is_valid_expression_vc2010A(c1_1,P,P_,expr<P_>().operator*());
    struct c1:stdm::integral_constant<bool,c1_1::value||stdm::is_pointer<P>::value>{};
# else
    struct c1:stdm::false_type{};
# endif

    template<typename F_,bool B=has_single_operator_functor<F_>::value>
    struct get_operator_functor:stdm::false_type{
      typedef mwg::unknown_type operator_type;
    };
    template<typename F_> struct get_operator_functor<F_,true>:stdm::true_type{
      typedef decltype(&F_::operator()) operator_type;
    };

    template<typename P_,bool B> struct c2:stdm::false_type{
      typedef mwg::unknown_type operator_type;
    };
# ifdef _MSC_VER
    template<typename P_> struct c2<P_,true>{
      typedef typename stdm::remove_reference<decltype(*expr<P_>())>::type functor_type;
      typedef typename get_operator_functor<functor_type>::type operator_type;
      static const bool value=get_operator_functor<functor_type>::value;
    };
# else
    template<typename P_> struct c2<P_,true>
      :get_operator_functor< typename stdm::remove_reference<decltype(*expr<P_>())>::type >
    {};
# endif

    mwg_concept_condition((c2<P,c1::value>::value));
    typedef typename c2<P,c1::value>::operator_type operator_type;
#else
    mwg_concept_condition(false);
    typedef mwg::unknown_type operator_type;
#endif
  };

  //---------------------------------------------------------------------------
  /*?lwiki
   * :@class class mwg::functor_detail::can_be_called_as<typename F,typename S>;
   *  :@var static const bool value;
   *   `declval<F>.operator()(...)` を指定した引数で呼出可能かどうかを判定します。
   *  :@typedef typedef '''function-type''' signature_type;
   *   `value==true` の時、関手 `F` の模倣する関数型を取得します。
   */
  template<typename F,typename S>
  struct can_be_called_as;

  namespace detail{
    template<typename F,typename S>
    struct can_be_called_as_impl1;
    template<typename F,typename S,bool=can_be_called_as_impl1<F,S>::value>
    struct can_be_called_as_impl2;

    template<typename F,typename S>
    struct can_be_called_as_impl1:stdm::false_type{
#ifdef mwg_concept_is_valid_expression
# define MWG_FUNCTOR_H__can_be_called_as__declare_c1(Parameters,Arguments) \
      mwg_concept_is_valid_expression(c1,F,F_,expr<F_>().operator() Arguments);
#elif defined(_MSC_VER)&&defined(mwg_concept_is_valid_expression_vc2010A)
# define MWG_FUNCTOR_H__can_be_called_as__declare_c1(Parameters,Arguments) \
      mwg_concept_is_valid_expression_vc2010A(c1,F,F_,expr<F_>().operator() Arguments);
#else
# define MWG_FUNCTOR_H__can_be_called_as__declare_c1(Parameters,Arguments) \
      mwg_concept_has_member(c1_1,F,X,operator(),R(X::*) Parameters);   \
      mwg_concept_has_member(c1_2,F,X,operator(),R(X::*) Parameters const); \
      struct c1:stdm::integral_constant<bool,(c1_1::value||c1_2::value)>{};
      // // 以下の様に permissive にすると overload 選択などで問題あり。
      // struct c1:stdm::true_type{};
#endif

#if defined(MWGCONF_STD_DECLTYPE)
# define MWG_FUNCTOR_H__can_be_called_as__declare_OpR(Parameters,Arguments) \
      template<typename F_,bool B> struct s1{typedef void type;};       \
      template<typename F_> struct s1<F_,true>{                         \
        typedef decltype(expr<F_>().operator() Arguments) type;         \
      };                                                                \
      typedef typename s1<F,c1::value>::type OpR;
#else
# define MWG_FUNCTOR_H__can_be_called_as__declare_OpR(Parameters,Arguments) \
      typedef R OpR;
#endif

#define MWG_FUNCTOR_H__can_be_called_as__content(Parameters,Arguments)  \
      MWG_FUNCTOR_H__can_be_called_as__declare_c1(Parameters,Arguments) \
      MWG_FUNCTOR_H__can_be_called_as__declare_OpR(Parameters,Arguments) \
      mwg_concept_condition((c1::value&&is_covariant<OpR,R>::value));
    };

#pragma%m 1
    template<typename F,typename R,typename... A>
    struct can_be_called_as_impl1<F,R(A...)>{
      MWG_FUNCTOR_H__can_be_called_as__content((A...),(expr<A>()...));
    };
#pragma%end
#pragma%x variadic_expand_0toArN

#undef MWG_FUNCTOR_H__can_be_called_as__content
#undef MWG_FUNCTOR_H__can_be_called_as__declare_OpR
#undef MWG_FUNCTOR_H__can_be_called_as__declare_c1

    template<typename F,typename S,bool>
    struct can_be_called_as_impl2:stdm::false_type{};
    template<typename F,typename S>
    struct can_be_called_as_impl2<F,S,true>:stdm::true_type{
      typedef S signature_type;
    };
    template<typename F,typename S>
    struct can_be_called_as_impl2<F,S,false>
      :can_be_called_as_impl2<F,typename sig::arity_pop<S>::type>{};
    template<typename F,typename R>
    struct can_be_called_as_impl2<F,R(),false>
      :stdm::false_type{};
  }

  template<typename F,typename S>
  struct can_be_called_as:detail::can_be_called_as_impl2<F,S>{};

  //---------------------------------------------------------------------------

#pragma%x begin_check
void check_can_be_called_as(){
  using namespace mwg::functor_detail;
  mwg_check( (is_variant_function<int (int),int (int)>::value));
  mwg_check( (is_variant_function<void(int),void(int)>::value));
  mwg_check( (is_variant_function<int (int),void(int)>::value));
  mwg_check(!(is_variant_function<void(int),int (int)>::value));
  mwg_check(!(is_variant_function<int(int ),int(void)>::value));
  mwg_check( (is_variant_function<int(void),int(int )>::value));

  mwg_check( (is_covariant<int,int>::value));
  mwg_check( (is_covariant<int,void>::value));
  mwg_check(!(is_covariant<void,int>::value));
}
#pragma%x end_check
}
}
//------------------------------------------------------------------------------
