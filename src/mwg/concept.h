// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef MWG_CONCEPT_H
#define MWG_CONCEPT_H
#include "mwg/defs.h"
namespace mwg{
  struct unknown_type{int d;};
  struct invalid_type{};

namespace concept_detail{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

  template<class T,T mem>
  struct mustbe_type{};

//-----------------------------------------------------------------------------
// SFINAE eval 用の型
  struct true_t{int x[1];};
  struct false_t{int x[2];};
#define mwg_concept_bool_eval(expr) (sizeof(expr)==sizeof(mwg::concept_detail::true_t))

//-----------------------------------------------------------------------------
// void 回避
  struct void_t{};
  template<typename T>
  struct enable_unless_void{
    typedef T& type;
    typedef const T& const_type;
  };
  template<>
  struct enable_unless_void<void>{};
  template<typename T>
  typename enable_unless_void<T>::type operator,(T& left,const void_t& right);
  template<typename T>
  typename enable_unless_void<T>::const_type operator,(const T& left,const void_t& right);
#define mwg_concept_void          mwg::concept_detail::void_t
#define mwg_concept_void2t(expr)  (expr,mwg_concept_void())

//-----------------------------------------------------------------------------
//  macro: mwg_concept_is_assignable(T,expr)
//-----------------------------------------------------------------------------
  template<typename T>
  struct is_assignable_impl{
    static mwg::concept_detail::true_t eval(T v);
    static mwg::concept_detail::false_t eval(...);
  };
#define mwg_concept_is_assignable(T,expr) \
  mwg_concept_bool_eval(mwg::concept_detail::is_assignable_impl<T>::eval(mwg_concept_void2t(expr)))
//-----------------------------------------------------------------------------
//  macro: mwg_requires          (boolean_expr,T)
//  macro: mwg_concept_condition (boolean_expr)
//  macro: mwg_concept_nest      (name,T,X,boolean_expr,declarations)
//-----------------------------------------------------------------------------
#ifdef MWG_STD_VA_ARGS
# define mwg_requires(CONDITION,...) typename mwg::stdm::enable_if<CONDITION,__VA_ARGS__>::type
# define mwg_concept_condition(...) static const bool value=(__VA_ARGS__)
# define mwg_concept_nest(name,T,X,cond,...)                                  \
  template<typename X,bool B> struct name##_mwg_1{static const bool value=B;};\
  template<typename X>        struct name##_mwg_1<X,true>{__VA_ARGS__};       \
  struct name{mwg_concept_condition(name##_impl_<T,cond>::value);}         /**/
#else
# define mwg_requires(CONDITION,EXPR) typename mwg::stdm::enable_if<CONDITION,EXPR>::type
# define mwg_concept_condition(EXPR) static const bool value=(EXPR)
# define mwg_concept_nest(name,T,X,cond,EXPR)                                 \
  template<typename X,bool B> struct name##_mwg_1{static const bool value=B;};\
  template<typename X>        struct name##_mwg_1<X,true>{EXPR};              \
  struct name{mwg_concept_condition(name##_impl_<T,cond>::value);}         /**/
#endif
#if 0<MWG_GNUC_VER&&MWG_GNUC_VER<40000
# define MWG_CONCEPT_OVERLOAD_FAIL
# define mwg_gcc3_concept_overload(I) ,int(*)[I]=0
#else
# define mwg_gcc3_concept_overload(I)
#endif
//-----------------------------------------------------------------------------
//  macro: mwg_concept_sfinae_param
//  macro: mwg_concept_sfinae_param_true(X,PARAMS)
//  macro: mwg_concept_sfinae_param_false(X,PARAMS)
//  macro: mwg_concept_sfinae_param_check(T,ARGS)
//-----------------------------------------------------------------------------
#define mwg_concept_sfinae_param struct sfinae_checker
#define mwg_concept_sfinae_param_true(X,PARAMS)                               \
  template<typename X> static mwg::concept_detail::true_t  eval PARAMS
#define mwg_concept_sfinae_param_false(X,PARAMS)                              \
  template<typename X> static mwg::concept_detail::false_t eval PARAMS
#define mwg_concept_sfinae_param_check(T,ARGS)                                \
  mwg_concept_condition(mwg_concept_bool_eval(sfinae_checker::template eval<T> ARGS))
//-----------------------------------------------------------------------------
//  mwcro: mwg_concept_sfinae_typeOverload(name,T,X,(X*,...),(declval<T>(),...))
//  mwcro: mwg_concept_sfinae_typeDecltypeOverload(name,T,X,(EXPR))
//    条件 MWGCONF_STD_DECLTYPE
//-----------------------------------------------------------------------------
#define mwg_concept_sfinae_typeOverload(NAME,T,X,PARAMS,ARGS)                 \
  struct NAME{                                                                \
    mwg_concept_sfinae_param{                                                 \
      mwg_concept_sfinae_param_true(X,PARAMS);                                \
      mwg_concept_sfinae_param_false(X,(...));                                \
    };                                                                        \
    mwg_concept_sfinae_param_check(T,ARGS);                                   \
  }                                                                        /**/
#if defined(MWGCONF_STD_DECLTYPE)&&defined(MWGCONF_STD_AUTO_TYPE)
# define mwg_concept_sfinae_typeDecltypeOverload(NAME,T,X,EXPR)               \
  struct NAME{                                                                \
    mwg_concept_sfinae_param{                                                 \
      template<class X> static auto eval(int)                                 \
        -> decltype(EXPR,mwg::concept_detail::true_t());                      \
      mwg_concept_sfinae_param_false(X,(...));                                \
    };                                                                        \
    mwg_concept_sfinae_param_check(T,(0));                                    \
  }                                                                        /**/
#endif
//-----------------------------------------------------------------------------
//  mwg_concept_is_valid_expression(name,T,X,検証対象の式)
//-----------------------------------------------------------------------------
  /**
   * @def mwg_concept_is_valid_expression(name,T,X,...)
   * \a __VA_ARGS__ で指定した式が型的に有効な式かどうかを判定するクラスを定義します。
   * 有効かどうかを判定する為に SFINAE を使用します。
   * その為に、\a __VA_ARGS__ の (有効性に関わる) 部分式の一つ (E1 とする) を選んで、
   * それを mwg::declval\<X\>() に置き換えます。
   * \a X には使用されていない識別子を指定します。
   * そして、部分式 E1 の (置き換える前の) 型を \a T に指定します。
   * マクロを使用する事によって \a name という名前のクラスが定義され、
   * 式が有効かどうかの判定結果は name::value によって提供されます。
   *
   * 例えば、式 1->*1 が有効な式かどうかを判定するには以下の様にします。
   * \code
   *   mwg_concept_is_valid_expression(int_is_int_member_pointer_accessible,int,X,mwg::declval\<X\>()->*1);
   *   bool value=int_is_int_member_pointer_accessible::value;
   * \endcode
   *
   * より実際的には、式の一部の型が事前に分からない場合に用いられます。
   * 不確定の型 T の変数 t1 について t1->*1 が有効な式がどうかを判定する場合は以下の様にします。
   * \code
   *   template\<typename T\>
   *   mwg_concept_is_valid_expression(is_int_member_pointer_accessible,T,X,mwg::declval\<X\>()->*123);
   *
   *   template\<typename T\>
   *   typename std::enable_if\<is_int_member_pointer_accessible\<int\>::value,void\>::type
   *   my_super_function(T const\& lhs){lhs-\>*123;}
   * \endcode
   */
#if !defined(_MSC_VER)&&defined(mwg_concept_sfinae_typeDecltypeOverload)
# ifdef MWG_STD_VA_ARGS
#   define mwg_concept_is_valid_expression(name,T,X,...)                      \
      mwg_concept_sfinae_typeDecltypeOverload(name,T,X,(__VA_ARGS__))
# else
#   define mwg_concept_is_valid_expression(name,T,X,EXPR)                     \
      mwg_concept_sfinae_typeDecltypeOverload(name,T,X,EXPR)
# endif
#endif

  /**
   * @def mwg_concept_is_valid_expression_vc2010A(name,T,X,...)
   * vc2010 用の mwg_concept_is_valid_expression(name,T,X,...) の代替実装です。
   * 一部の形式の式に対してのみ正しく動作する事が確認されています。
   *
   * 正しく判定できる式とできない式の一覧は以下になります。
   * - OK T() + U(),            vc2010
   * - OK T().operator()(A1()), vc2010
   * - OK T().get(A1()),        vc2010
   * - OK T().operator&(),      vc2010
   * - OK + T(),                vc2010
   * - NG - T(),                vc2010 when T = int*, int[1] etc
   */
#if defined(_MSC_VER)&&defined(MWGCONF_STD_DECLTYPE)
# define mwg_concept_is_valid_expression_vc2010A(name,T,X,...)                \
  struct name{                                                                \
    struct fromN{};                                                           \
    struct fromA{template<typename U> fromA(U const volatile&){}};            \
    template<typename U> static int known2int(const U&);                      \
    static fromN known2int(fromA);                                            \
    mwg_concept_sfinae_param_true(X,                                          \
      (decltype(known2int(mwg_concept_void2t(__VA_ARGS__)))) ) ;              \
    mwg_concept_sfinae_param_false(X,(...));                                  \
    mwg_concept_condition(mwg_concept_bool_eval(eval<T>(0)));                 \
  }                                                                        /**/
#endif

  /**
   * @def mwg_concept_is_valid_expression_vc2008s(name,T,X,...)
   * vc2008 用の mwg_concept_is_valid_expression(name,T,X,...) の代替実装です。
   * 一部の形式の式に対してのみ正しく動作する事が確認されています。
   *
   * 正しく判定できる式とできない式の一覧は以下になります。
   * - OK T() + U(),            vc2008-2010
   * - OK T().operator()(A1()), vc2008-2010
   * - OK T().get(A1()),        vc2008-2010
   * - OK T().operator&(),      vc2008-2010
   * - OK T().operator+(),      vc2008-2010
   * - NG + T(),                vc2008-2010
   *   次の様なコンパイルエラーを生じます。
   *   error C2675: 単項演算子 '-' : 'T_' は、この演算子または定義済の演算子に
   *   適切な型への変換の定義を行いません。(新しい動作; ヘルプを参照)
   * - OK & T(),                vc2008-2010
   *
   * ※ 何故か choker を中で定義しないと正しく動作しない。
   */
#if defined(_MSC_VER)&&(_MSC_VER>=1400)
# define mwg_concept_is_valid_expression_vc2008s(name,T,X,...)                \
  struct name{                                                                \
    mwg_concept_sfinae_param{                                                 \
      template<std::size_t I,typename TT> struct choker:mwg::identity<TT>{};  \
      template<typename TT>               struct choker<0,TT>{};              \
      mwg_concept_sfinae_param_true(X,                                        \
        (typename choker<sizeof((__VA_ARGS__,0)),int>::type));                \
      mwg_concept_sfinae_param_false(X,(...));                                \
    };                                                                        \
    mwg_concept_sfinae_param_check(T,(0));                                    \
  };                                                                       /**/
#endif

//-----------------------------------------------------------------------------
//  mwg_concept_is_valid_type(name,T,X,Xを含む型名)::value
//-----------------------------------------------------------------------------
# define mwg_concept_is_valid_type(name,T,X,typeexpr)                         \
  struct name{                                                                \
    mwg_concept_sfinae_param{                                                 \
      mwg_concept_sfinae_param_true(X,                                        \
        (X*,typename mwg::identity<typeexpr>* x=0));                          \
      mwg_concept_sfinae_param_false(X,(...));                                \
    };                                                                        \
    mwg_concept_sfinae_param_check(T,(mwg::declval<T*>()));                   \
  }                                                                        /**/
//-----------------------------------------------------------------------------
//  mwg_concept_has_member(name,T,X,メンバ名,メンバポインタ型)::value
//-----------------------------------------------------------------------------
#define mwg_concept_has_member(name,T,X,MemberName,MemberType)                \
  struct name{                                                                \
    mwg_concept_sfinae_param{                                                 \
      mwg_concept_sfinae_param_true(X,                                        \
        (mwg::concept_detail::mustbe_type<MemberType,&X::MemberName>* d));    \
      mwg_concept_sfinae_param_false(X,(...));                                \
    };                                                                        \
    mwg_concept_sfinae_param_check(T,(0));                                    \
  }                                                                        /**/
//-----------------------------------------------------------------------------
//  mwg_concept_has_type_member(name,T,メンバー型名)::value
//  mwg_concept_using_type_member(name,T,メンバー型名)::value
//-----------------------------------------------------------------------------
/* #define mwg_concept_has_type_member(name,T,member_type)                       \
 *   mwg_concept_is_valid_type(name,T,T_mwg__,typename T_mwg__::member_type)
 */
#define mwg_concept_has_type_member(name,T,member_type)                       \
  struct name{                                                                \
    mwg_concept_is_valid_type(_mwg_1,T,T_mwg__,typename T_mwg__::member_type);\
    mwg_concept_condition(_mwg_1::value);                                     \
    template<typename X,bool B> struct _mwg_2:                                \
      mwg::identity<mwg::unknown_type>{};                                     \
    template<typename X>        struct _mwg_2<X,true>:                        \
      mwg::identity<typename X::member_type>{};                               \
    typedef typename _mwg_2<T,_mwg_1::value>::type type;                      \
  }                                                                        /**/
#define mwg_concept_using_type_member(name,T,member_type)                     \
  mwg_concept_has_type_member(name,T,member_type);                            \
  template<typename X,bool B> struct name##_mwg_1{                            \
    typedef mwg::unknown_type type; };                                        \
  template<typename X>        struct name##_mwg_1<X,true>{                    \
    typedef X::member_type type; };                                           \
  typedef name##_mwg_1<T,name::value>::type member_type                    /**/
//-----------------------------------------------------------------------------
//  mwg_concept_is_variant_functor(name,T,X,共変戻り型,反変引数)::value
//-----------------------------------------------------------------------------
#ifdef mwg_concept_is_valid_expression
# define mwg_concept_param(T) (*reinterpret_cast<typename mwg::identity<T>::type*>(0))
  //mwg::declval<typename std::add_lvalue_reference<T>::type>
# define mwg_concept_is_contrav_functor(name,T,X,ContravArgs)                 \
  mwg_concept_is_valid_expression(name,T,X,mwg::declval<X>()ContravArgs)      /**/
# define mwg_concept_is_variant_functor(name,T,X,CovRet,ContravArgs)          \
  struct name{                                                                \
    mwg_concept_is_contrav_functor(name##_1_,T,X,ContravArgs);                \
    template<typename X,bool B> struct check_rettype{mwg_concept_condition(B);};\
    template<typename X>        struct check_rettype<X,true>{                 \
      mwg_concept_condition(mwg_concept_is_assignable(CovRet,mwg::declval<X>()ContravArgs)); \
    };                                                                        \
    mwg_concept_condition(check_rettype<T,name##_1_::value>::value);          \
  }                                                                        /**/
#else
# define mwg_concept_param(type) type
//# define mwg_concept_void        void
# define mwg_concept_is_variant_functor(name,T,X,CovRet,ContravArgs)          \
  struct name{                                                                \
    mwg_concept_has_member(name##_1_,T,X,                                     \
      operator(),CovRet (X::*)ContravArgs const);                             \
    mwg_concept_condition(name##_1_::value||                                  \
      mwg::stdm::is_same<T,CovRet ContravArgs>::value||                       \
      mwg::stdm::is_same<T,CovRet (*)ContravArgs>::value);                    \
  }                                                                        /**/
#endif
//EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
//  USAGE (使い方の例)
#if 0
  template<typename XCH,typename T>
  struct TIndexOfMatch{
    mwg_concept_has_member(c1,T,X,operator(),int (X::*)(const XCH*,int));
    mwg_concept_has_member(c2,T,X,operator(),int (X::*)(const XCH*,int) const);
    mwg_concept_condition(c1::value||c2::value);
  };
  template<typename DMatch> mwg_requires((TIndexOfMatch<XCH,DMatch>::value),
  int) IndexOf (const DMatch& matcher) const{
    return Impl::IndexOf(EX_STR(*this),matcher);
  }
#endif
//EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
#pragma%x begin_check
// -*- coding:sjis -*-
#include <mwg/defs.h>
#include <mwg/except.h>
#include <mwg/concept.h>
#include <mwg/std/type_traits>

struct A{
  typedef int content;
};
struct B{
  static const int content=0;
};
struct C{
};

void test_is_assignable(){
  mwg_assert(( mwg_concept_is_assignable(double,0)));
  mwg_assert((!mwg_concept_is_assignable(double,A())));
}

namespace sfinae{
  template<typename T>
  mwg_concept_sfinae_typeOverload(is_class_or_union_impl,T,X,(void(X::*)(void)),(0));

  void test_typeOverload(){
    mwg_assert(( is_class_or_union_impl<A>::value));
    mwg_assert((!is_class_or_union_impl<A*>::value));
    mwg_assert((!is_class_or_union_impl<int>::value));
    mwg_assert((!is_class_or_union_impl<int&>::value));
  }

#ifdef mwg_concept_is_valid_expression
  template<typename T>
  mwg_concept_is_valid_expression(can_be_added_int,T,X,(mwg::declval<X>()+1));
  void func1(int){}
  template<typename T>
  mwg_concept_is_valid_expression(canbe_arg_of_func1,T,X,func1(mwg::declval<X>()));
#endif
  void test_is_valid_expression(){
#ifdef mwg_concept_is_valid_expression
    mwg_assert(( can_be_added_int<int>::value));
    mwg_assert(( can_be_added_int<int&>::value));
    mwg_assert(( can_be_added_int<double>::value));
    mwg_assert((!can_be_added_int<A>::value));
    mwg_assert((!can_be_added_int<void>::value));

    mwg_assert(( canbe_arg_of_func1<int>::value));
    mwg_assert((!canbe_arg_of_func1<int*>::value));
#endif
  }

  template<typename T>
  mwg_concept_is_valid_type(can_add_pointer,T,X,X*);
  template<typename T>
  mwg_concept_is_valid_type(have_valid_type_member,T,X,typename X::content);
  struct have_valid_type_member_1{typedef int& content;};
  void test_is_valid_type(){
    mwg_assert(( have_valid_type_member<A>::value));
    mwg_assert((!have_valid_type_member<B>::value));
    mwg_assert((!have_valid_type_member<C>::value));
    mwg_assert((!have_valid_type_member<int>::value));
    mwg_assert(( have_valid_type_member<have_valid_type_member_1>::value));
  }

  template<typename T>
  mwg_concept_has_member(has_member_print,T,X,print,void(X::*)(const char*));
  struct has_member_print_1{void print(const char*){}};
  struct has_member_print_2{void print(const char*) const{}};
  struct has_member_print_3{int print(const char*){return 0;}};
  struct has_member_print_4{void print(){}};
  void test_has_member(){
    mwg_assert(( has_member_print<has_member_print_1>::value));
    mwg_assert((!has_member_print<has_member_print_2>::value));
    mwg_assert((!has_member_print<has_member_print_3>::value));
    mwg_assert((!has_member_print<has_member_print_4>::value));
    mwg_assert((!has_member_print<A>::value));
    mwg_assert((!has_member_print<int>::value));
  }

  template<typename T>
  mwg_concept_has_type_member(have_content_type,T,content);
  template<typename T>
  struct have_contente_type2{
    mwg_concept_has_type_member(cond,T,content);
  };
  void test_has_type_member(){
    mwg_assert(( have_valid_type_member<A>::value));
    mwg_assert((!have_valid_type_member<B>::value));
    mwg_assert((!have_valid_type_member<C>::value));
    mwg_assert((!have_valid_type_member<int>::value));
    mwg_assert(( have_valid_type_member<have_valid_type_member_1>::value));
  }

  template<typename T>
  mwg_concept_is_variant_functor(is_xy_functor,T,X,int*,(mwg_concept_param(int),mwg_concept_param(int)));
  struct F1{ int operator()(int x) const{return x*x;} };
  struct F2{ int* operator()(int) const{return 0;} };
  struct F3{ int operator()(int x,int y) const{return x*y;} };
  struct F4{ int* operator()(int,int) const{return 0;} };
  void test_is_variant_functor(){
#ifdef mwg_concept_is_valid_expression
    mwg_assert((!is_xy_functor<F1>::value));
    mwg_assert((!is_xy_functor<F2>::value));
    mwg_assert((!is_xy_functor<F3>::value));
    mwg_assert(( is_xy_functor<F4>::value));
#else
    mwg_assert((!is_xy_functor<F1>::value));
    mwg_assert((!is_xy_functor<F2>::value));
    mwg_assert((!is_xy_functor<F3>::value));
    mwg_assert(( is_xy_functor<F4>::value));
#endif

  }
}

//-----------------------------------------------------------------------------
namespace apply{
  struct A1{
    int operator+(int){return 10;}

    void get(int) const{}
    int get(int*) const{return 0;}
    void get(double*)  {}

    void operator()(int) const{}
    int operator()(int*) const{return 0;}
    void operator()(double*)  {}

    void operator&(){}
    int operator+(){return 0;}
    int operator-(){return 0;}
  };

//-----------------------------------------------------------------------------
// is_method_available

#ifdef mwg_concept_is_valid_expression
  template<typename T,typename A>
  mwg_concept_is_valid_expression(is_method_available,T,T_,mwg::declval<T_>().get(mwg::declval<A>()));
  template<typename T> struct is_method_available<T,void>:mwg::stdm::false_type{};
  template<typename T,typename A>
  mwg_concept_is_valid_expression(is_opmethod_available,T,T_,mwg::declval<T_>().operator()(mwg::declval<A>()));
  template<typename T> struct is_opmethod_available<T,void>:mwg::stdm::false_type{};
  template<typename T>
  mwg_concept_is_valid_expression(is_opaddr_available,T,T_,mwg::declval<T_>().operator&());
#elif defined(mwg_concept_is_valid_expression_vc2010A)
  template<typename T,typename A>
  mwg_concept_is_valid_expression_vc2010A(is_method_available,T,T_,mwg::declval<T_>().get(mwg::declval<A>()));
  template<typename T> struct is_method_available<T,void>:mwg::stdm::false_type{};
  template<typename T,typename A>
  mwg_concept_is_valid_expression_vc2010A(is_opmethod_available,T,T_,mwg::declval<T_>().operator()(mwg::declval<A>()));
  template<typename T> struct is_opmethod_available<T,void>:mwg::stdm::false_type{};
  template<typename T>
  mwg_concept_is_valid_expression_vc2010A(is_opaddr_available,T,T_,mwg::declval<T_>().operator&());
#elif defined(mwg_concept_is_valid_expression_vc2008s)
  template<typename T,typename A>
  mwg_concept_is_valid_expression_vc2008s(is_method_available,T,T_,mwg::declval<T_>().get(mwg::declval<A>()));
  template<typename T> struct is_method_available<T,void>:mwg::stdm::false_type{};
  template<typename T,typename A>
  mwg_concept_is_valid_expression_vc2008s(is_opmethod_available,T,T_,mwg::declval<T_>().operator()(mwg::declval<A>()));
  template<typename T> struct is_opmethod_available<T,void>:mwg::stdm::false_type{};
  template<typename T>
  mwg_concept_is_valid_expression_vc2008s(is_opaddr_available,T,T_,mwg::declval<T_>().operator&());
#else
# define test_is_method_available_skip
#endif
  void test_is_method_available(){
#ifdef test_is_method_available_skip
    std::puts("test is_method_available: skipped");
#else
    // t.get(u)
    mwg_assert(( is_method_available<A1,int>::value));
    mwg_assert(( is_method_available<A1,int*>::value));
    mwg_assert(( is_method_available<A1,double*>::value));
    mwg_assert(( is_method_available<A1 const,int>::value));
    mwg_assert(( is_method_available<A1 const,int*>::value));
    mwg_assert((!is_method_available<A1 const,double*>::value));

    mwg_assert((!is_method_available<int,int>::value));
    mwg_assert((!is_method_available<int,float>::value));
    mwg_assert((!is_method_available<int,A>::value));
    mwg_assert((!is_method_available<int*,int>::value));
    mwg_assert((!is_method_available<int*,std::size_t>::value));
    mwg_assert((!is_method_available<int*,int*>::value));
    mwg_assert((!is_method_available<A,int>::value));
    mwg_assert((!is_method_available<A,int*>::value));
    mwg_assert((!is_method_available<A,A>::value));
    mwg_assert((!is_method_available<A*,int>::value));
    mwg_assert((!is_method_available<A*,int*>::value));
    mwg_assert((!is_method_available<A1,A1>::value));
    mwg_assert((!is_method_available<A1,A1*>::value));
    mwg_assert((!is_method_available<A1*,int>::value));
    mwg_assert((!is_method_available<A1*,int*>::value));

    mwg_assert((!is_method_available<A1,void>::value));
    mwg_assert((!is_method_available<A1*,void>::value));
    mwg_assert((!is_method_available<int,void>::value));
    mwg_assert((!is_method_available<void,int>::value));
    mwg_assert((!is_method_available<void,void>::value));

    // t.operator()(u)
    mwg_assert(( is_opmethod_available<A1,int>::value));
    mwg_assert(( is_opmethod_available<A1,int*>::value));
    mwg_assert(( is_opmethod_available<A1,double*>::value));
    mwg_assert(( is_opmethod_available<A1 const,int>::value));
    mwg_assert(( is_opmethod_available<A1 const,int*>::value));
    mwg_assert((!is_opmethod_available<A1 const,double*>::value));

    mwg_assert((!is_opmethod_available<int,int>::value));
    mwg_assert((!is_opmethod_available<int,float>::value));
    mwg_assert((!is_opmethod_available<int,A>::value));
    mwg_assert((!is_opmethod_available<int*,int>::value));
    mwg_assert((!is_opmethod_available<int*,std::size_t>::value));
    mwg_assert((!is_opmethod_available<int*,int*>::value));
    mwg_assert((!is_opmethod_available<A,int>::value));
    mwg_assert((!is_opmethod_available<A,int*>::value));
    mwg_assert((!is_opmethod_available<A,A>::value));
    mwg_assert((!is_opmethod_available<A*,int>::value));
    mwg_assert((!is_opmethod_available<A*,int*>::value));
    mwg_assert((!is_opmethod_available<A1,A1>::value));
    mwg_assert((!is_opmethod_available<A1,A1*>::value));
    mwg_assert((!is_opmethod_available<A1*,int>::value));
    mwg_assert((!is_opmethod_available<A1*,int*>::value));

    mwg_assert((!is_opmethod_available<A1,void>::value));
    mwg_assert((!is_opmethod_available<A1*,void>::value));
    mwg_assert((!is_opmethod_available<int,void>::value));
    mwg_assert((!is_opmethod_available<void,int>::value));
    mwg_assert((!is_opmethod_available<void,void>::value));

    // t.operator&()
    mwg_assert(( is_opaddr_available<A1>::value));
    mwg_assert((!is_opaddr_available<A1 const>::value));

    mwg_assert((!is_opaddr_available<int>::value));
    mwg_assert((!is_opaddr_available<int*>::value));
    mwg_assert((!is_opaddr_available<A>::value));
    mwg_assert((!is_opaddr_available<A*>::value));
    mwg_assert((!is_opaddr_available<A1*>::value));
    mwg_assert((!is_opaddr_available<void>::value));
#endif
  }

//-----------------------------------------------------------------------------
// is_operator_available

#ifdef mwg_concept_is_valid_expression
  template<typename T,typename U>
  mwg_concept_is_valid_expression(is_add_operator_available,T,T_,mwg::declval<T_>()+mwg::declval<U>());
  template<typename T> struct is_add_operator_available<T,void>:mwg::stdm::false_type{};

  template<typename T> mwg_concept_is_valid_expression(is_nsign_available,T,T_,-mwg::declval<T_>());
  template<typename T> mwg_concept_is_valid_expression(is_psign_available,T,T_,+mwg::declval<T_>());
#elif defined(mwg_concept_is_valid_expression_vc2010A)
  template<typename T,typename U>
  mwg_concept_is_valid_expression_vc2010A(is_add_operator_available,T,T_,mwg::declval<T_>()+mwg::declval<U>());
  template<typename T> struct is_add_operator_available<T,void>:mwg::stdm::false_type{};

  template<typename T> mwg_concept_is_valid_expression_vc2010A(is_nsign_available,T,T_,-mwg::declval<T_>());
  template<typename T> mwg_concept_is_valid_expression_vc2010A(is_psign_available,T,T_,+mwg::declval<T_>());
#elif defined(mwg_concept_is_valid_expression_vc2008s)
  // ■以下の物で正しい結果になると思いきや、<A1,int> の判定で失敗する。
  template<typename T,typename U>
  mwg_concept_is_valid_expression_vc2008s(is_add_operator_available,T,T_,mwg::declval<T_>()+mwg::declval<U>());
  template<typename T> struct is_add_operator_available<T,void>:mwg::stdm::false_type{};
  template<typename T> struct is_add_operator_available<void,T>:mwg::stdm::false_type{};
  template<>           struct is_add_operator_available<void,void>:mwg::stdm::false_type{};

  // ERR: C2675: '-' : 'T_'
  // template<typename T,bool B>
  // mwg_concept_is_valid_expression_vc2008s(is_psign_available,T,T_,-mwg::declval<T_>());

  // 以下の様にするとコンパイルエラーにはならないが、
  // クラスに対して常に false を返す。
  // クラス以外に対しては正しい結果を出している様に見える。
  // →と思ったら配列型に対する結果が誤っている。
  template<typename T,bool B=true>
  mwg_concept_is_valid_expression_vc2008s(is_nsign_available,T,T_,-mwg::declval<typename mwg::stdm::enable_if<B,T_>::type>());
  template<typename T,bool B=true>
  mwg_concept_is_valid_expression_vc2008s(is_psign_available,T,T_,+mwg::declval<typename mwg::stdm::enable_if<B,T_>::type>());
  template<> struct is_psign_available<void>:mwg::stdm::false_type{};
  template<> struct is_nsign_available<void>:mwg::stdm::false_type{};
#else
# define test_is_operator_available_skip
#endif
  void test_is_operator_available(){
#ifdef test_is_operator_available_skip
    std::puts("test is_add_operator_available: skipped");
#else
    mwg_assert(( is_add_operator_available<int,int>::value));
    mwg_assert(( is_add_operator_available<int,float>::value));
    mwg_assert(( is_add_operator_available<double,float>::value));
    mwg_assert(( is_add_operator_available<char,int>::value));
    mwg_assert(( is_add_operator_available<int*,int>::value));
    mwg_assert(( is_add_operator_available<int*,std::size_t>::value));

    mwg_assert((!is_add_operator_available<int*,int*>::value));
    mwg_assert((!is_add_operator_available<A,int>::value));
    mwg_assert((!is_add_operator_available<int,A>::value));
    mwg_assert((!is_add_operator_available<A,A>::value));
    mwg_assert(( is_add_operator_available<A1,int>::value));
    mwg_assert(( is_add_operator_available<A1,char>::value));
    mwg_assert(( is_add_operator_available<A1,double>::value));
    mwg_assert((!is_add_operator_available<int,A1>::value));
    mwg_assert((!is_add_operator_available<double,A1>::value));

    mwg_assert((!is_add_operator_available<int,void>::value));
    mwg_assert((!is_add_operator_available<void,int>::value));
    mwg_assert((!is_add_operator_available<void,void>::value));

    // +hoge
    mwg_assert(( is_psign_available<int>::value));
    mwg_assert(( is_nsign_available<int>::value));
    mwg_assert(( is_psign_available<double>::value));
    mwg_assert(( is_nsign_available<double>::value));
    mwg_assert(( is_psign_available<char>::value));
    mwg_assert(( is_nsign_available<char>::value));
#ifndef _MSC_VER
    mwg_assert(( is_psign_available<int*>::value)); // vc2008s error
    mwg_assert((!is_nsign_available<int*>::value)); // vc2010A error, vc2008s error
#endif
    mwg_assert(( is_psign_available<bool>::value));
    mwg_assert(( is_nsign_available<bool>::value));
#ifndef _MSC_VER
    mwg_assert(( is_psign_available<int[1]>::value)); // vc2008s error
    mwg_assert((!is_nsign_available<int[1]>::value)); // vc2010A error
#endif
    mwg_assert((!is_psign_available<A>::value));
    mwg_assert((!is_nsign_available<A>::value));
#ifndef _MSC_VER
    mwg_assert(( is_psign_available<A1>::value)); // vc2008s error
    mwg_assert(( is_nsign_available<A1>::value)); // vc2008s error
#endif
    mwg_assert((!is_psign_available<A1 const>::value));
    mwg_assert((!is_nsign_available<A1 const>::value));
    mwg_assert((!is_psign_available<void>::value));
    mwg_assert((!is_nsign_available<void>::value));
#endif
  }
}

int main(){
  test_is_assignable();
  sfinae::test_typeOverload();
  sfinae::test_is_valid_expression();
  sfinae::test_is_valid_type();
  sfinae::test_has_member();
  sfinae::test_has_type_member();
  sfinae::test_is_variant_functor();

  apply::test_is_method_available();
  apply::test_is_operator_available();
  return 0;
}
#pragma%x end_check
