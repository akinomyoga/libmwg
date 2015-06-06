// -*- mode:C++;coding:utf-8 -*-
#ifndef MWG_STDM_TYPE_TRAITS__IS_CONSTRUCTIBLE
#define MWG_STDM_TYPE_TRAITS__IS_CONSTRUCTIBLE
#if MWGCONF_HEADER_STD>=2011
# include <type_traits>
#else
# if MWGCONF_HEADER_STD>=2005
#  include <type_traits>
#  define MWG_STDM_TYPE_TRAITS__IS_CONSTRUCTIBLE__tr1_available
# elif defined(MWGCONF_HEADER_TR1)
#  include <tr1/type_traits>
#  define MWG_STDM_TYPE_TRAITS__IS_CONSTRUCTIBLE__tr1_available
# endif
#endif
#include <mwg/defs.h>
#include <mwg/concept.h>
namespace mwg{
namespace stdm{
//HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH

//-----------------------------------------------------------------------------
//  各実装の手段と対応するクラス名の一覧
//-----------------------------------------------------------------------------

  namespace is_constructible_detail{
#ifdef MWG_STDM_TYPE_TRAITS__IS_CONSTRUCTIBLE__tr1_available
    // TR1 has_*<T>::value による実装。
    // + is_trivially_default_constructible
    // + is_nothrow_default_constructible
    // + is_trivially_copy_constructible
    // + is_nothrow_copy_constructible
    // + is_trivially_copy_assignable
    // + is_nothrow_copy_assignable
    // + is_trivially_destructible
    // + (has_virtual_destructor) // TR1 と同じ名前なので改めて定義する必要はない

    // ※ C++11 ライブラリをロードしている時には tr1 の機能は使えない
#elif MWGCONF_MSC_VER>=140050215
    // MSC __has_*(T) による実装
    // + is_trivially_default_constructible
    // + is_nothrow_default_constructible
    // + is_trivially_copy_constructible
    // + is_nothrow_copy_constructible
    // + is_trivially_copy_assignable
    // + is_nothrow_copy_assignable
    // + is_trivially_destructible
    // + has_virtual_destructor
#elif MWGCONF_GCC_VER>=40300&&!defined(__GCCXML__)
    // GCC __has_*(T) による実装
    // + is_trivially_default_constructible
    // + is_nothrow_default_constructible
    // + is_trivially_copy_constructible
    // + is_nothrow_copy_constructible
    // + is_trivially_copy_assignable
    // + is_nothrow_copy_assignable
#elif defined(MWGCONF_CLANG_VER)
    // CLANG __has_*(T) による実装
    // + is_trivially_default_constructible
    // + is_nothrow_default_constructible
    // + is_trivially_copy_constructible
    // + is_nothrow_copy_constructible
    // + is_trivially_copy_assignable
    // + is_nothrow_copy_assignable
#else
    // is_pod<T>::value による実装
    // INCOMPLETE
    // + is_trivially_default_constructible
    // + is_nothrow_default_constructible
    // + is_trivially_copy_constructible
    // + is_nothrow_copy_constructible
    // + is_trivially_copy_assignable
    // + is_nothrow_copy_assignable
#endif

#ifdef mwg_concept_is_valid_expression
    template<typename T> struct Holder{T m;};

    // mwg_concept_is_valid_expression による実装
    // INCOMPLETE: protected な関数を定義している場合、第一種過誤。
    // + is_default_constructible
    // + is_copy_constructible
    // + is_copy_assignable

    // TODO:
    // mwg_concept_is_valid_expression_vs2010A
    // mwg_concept_is_valid_expression_vs2008s などを使った実装について考える
#else
    // (is_trivially_*<T>::value||is_nothrow_*<T>::value) による実装
    // INCOMPLETE: trivial でも nothrow でもないコンストラクタを持っている場合、第二種過誤。
    // + is_default_constructible
    // + is_copy_constructible
    // + is_copy_assignable
#endif

    // TODO:

    // is_constructible
    // is_trivially_constructible
    // is_nothrow_constructible

    // is_assignable
    // is_trivially_assignable
    // is_nothrow_assignable

    // is_move_constructible
    // is_trivially_move_constructible
    // is_nothrow_move_constructible

    // is_move_assignable
    // is_trivially_move_assignable
    // is_nothrow_move_assignable

    // is_destructible
    // is_nothrow_destructible
  }

//-----------------------------------------------------------------------------
//  is_trivially_default_constructible
//-----------------------------------------------------------------------------
// using is_pod

#ifndef MWGCONF_HAS_IS_TRIVIALLY_DEFAULT_CONSTRUCTIBLE
  namespace is_constructible_detail{
# ifdef MWGCONF_HAS_IS_TRIVIALLY_CONSTRUCTIBLE
    template<typename T> struct is_trivially_default_constructible:is_trivially_constructible<T>{};
# elif defined(MWG_STDM_TYPE_TRAITS__IS_CONSTRUCTIBLE__tr1_available)
    template<typename T> struct is_trivially_default_constructible:has_trivial_constructor<T>{};
# elif MWGCONF_MSC_VER>=140050215
    template<typename T> struct is_trivially_default_constructible:integral_constant<bool,is_pod<T>::value||__has_trivial_constructor(T)>{};
# elif MWGCONF_GCC_VER>=40300&&!defined(__GCCXML__)
    template<typename T> struct is_trivially_default_constructible:integral_constant<bool,is_pod<T>::value||(__has_trivial_constructor(T)&&!is_volatile<T>::value)>{};
# elif defined(MWGCONF_CLANG_VER)
    template<typename T> struct is_trivially_default_constructible:integral_constant<bool,is_pod<T>::value||__has_trivial_constructor(T)>{};
# else
#  define mwg_stdm_is_trivially_default_constructible__incomplete
    /* INCOMPLETE */
    template<typename T> struct is_trivially_default_constructible:is_pod<T>{};
# endif
  }
  using is_constructible_detail::is_trivially_default_constructible;
#endif

//-----------------------------------------------------------------------------
//  is_nothrow_default_constructible
//-----------------------------------------------------------------------------
// using is_pod

#ifndef MWGCONF_HAS_IS_NOTHROW_DEFAULT_CONSTRUCTIBLE
  namespace is_constructible_detail{
# ifdef MWGCONF_HAS_IS_NOTHROW_CONSTRUCTIBLE
    template<typename T> struct is_nothrow_default_constructible:is_nothrow_constructible<T>{};
# elif defined(MWG_STDM_TYPE_TRAITS__IS_CONSTRUCTIBLE__tr1_available)
    template<typename T> struct is_nothrow_default_constructible:has_nothrow_constructor<T>{};
# elif MWGCONF_MSC_VER>=140050215
    template<typename T> struct is_nothrow_default_constructible:integral_constant<bool,is_pod<T>::value||__has_nothrow_constructor(T)>{};
# elif MWGCONF_GCC_VER>=40300&&!defined(__GCCXML__)
    template<typename T> struct is_nothrow_default_constructible:integral_constant<bool,is_pod<T>::value||(__has_nothrow_constructor(T)&&!is_volatile<T>::value)>{};
# elif defined(MWGCONF_CLANG_VER)
    template<typename T> struct is_nothrow_default_constructible:integral_constant<bool,is_pod<T>::value||__has_nothrow_constructor(T)>{};
# else
#  define mwg_stdm_is_nothrow_default_constructible__incomplete
    /* INCOMPLETE */
    template<typename T> struct is_nothrow_default_constructible:is_pod<T>{};
# endif
  }
  using is_constructible_detail::is_nothrow_default_constructible;
#endif

//-----------------------------------------------------------------------------
//  is_default_constructible
//-----------------------------------------------------------------------------
// using is_trivially_default_constructible
// using is_nothrow_default_constructible

#ifndef MWGCONF_HAS_IS_DEFAULT_CONSTRUCTIBLE
  namespace is_constructible_detail{
# ifdef MWGCONF_HAS_IS_CONSTRUCTIBLE
    //template<typename T> struct is_default_constructible:is_constructible<T>{};

    // 配列型・参照型を指定するとエラーになる標準ライブラリがあるので:
    template<typename T>
    struct is_default_constructible:integral_constant<
      bool,!is_reference<T>::value&&is_constructible<typename remove_extent<typename remove_reference<T>::type>::type>::value>{};
# elif defined(mwg_concept_is_valid_expression)
    template<typename T>
    mwg_concept_is_valid_expression(is_default_constructible_impl,Holder<T>,X,(X()));
    template<typename T>
    struct is_default_constructible:integral_constant<
      bool,!is_reference<T>::value&&is_default_constructible_impl<typename remove_reference<T>::type>::value>{};
# else
#  define mwg_stdm_is_default_constructible__incomplete
    template<typename T>
    struct is_default_constructible:integral_constant<
      bool,is_trivially_default_constructible<T>::value||is_nothrow_default_constructible<T>::value>{};
# endif
  }
  using is_constructible_detail::is_default_constructible;
#endif

//-----------------------------------------------------------------------------
//  is_trivially_copy_constructible
//-----------------------------------------------------------------------------
// using is_pod

#ifndef MWGCONF_HAS_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
  namespace is_constructible_detail{
# ifdef MWGCONF_HAS_IS_TRIVIALLY_CONSTRUCTIBLE
    template<typename T> struct is_trivially_copy_constructible:is_trivially_constructible<T,typename add_lvalue_reference<const T>::type>{};
# elif defined(MWG_STDM_TYPE_TRAITS__IS_CONSTRUCTIBLE__tr1_available)
    template<typename T> struct is_trivially_copy_constructible:has_trivial_copy<T>{};
# elif MWGCONF_MSC_VER>=140050215
    template<typename T> struct is_trivially_copy_constructible:integral_constant<bool,is_pod<T>::value||__has_trivial_copy(T)>{};
# elif MWGCONF_GCC_VER>=40300&&!defined(__GCCXML__)
    template<typename T> struct is_trivially_copy_constructible:integral_constant<bool,is_pod<T>::value||(__has_trivial_copy(T)&&!is_reference<T>::value&&!is_volatile<T>::value)>{};
# elif defined(MWGCONF_CLANG_VER)
    template<typename T> struct is_trivially_copy_constructible:integral_constant<bool,is_pod<T>::value||(__has_trivial_copy(T)&&!is_reference<T>::value&&!is_volatile<T>::value)>{};
# else
#  define mwg_stdm_is_trivially_copy_constructible__incomplete
    template<typename T> struct is_trivially_copy_constructible:is_pod<T>{};
# endif
  }
  using is_constructible_detail::is_trivially_copy_constructible;
#endif

//-----------------------------------------------------------------------------
//  is_nothrow_copy_constructible
//-----------------------------------------------------------------------------
// using is_pod

#ifndef MWGCONF_HAS_IS_NOTHROW_COPY_CONSTRUCTIBLE
  namespace is_constructible_detail{
# ifdef MWGCONF_HAS_IS_NOTHROW_CONSTRUCTIBLE
    template<typename T> struct is_nothrow_copy_constructible:is_nothrow_constructible<T,typename add_lvalue_reference<const T>::type>{};
# elif defined(MWG_STDM_TYPE_TRAITS__IS_CONSTRUCTIBLE__tr1_available)
    template<typename T> struct is_nothrow_copy_constructible:has_nothrow_copy<T>{};
# elif MWGCONF_MSC_VER>=140050215
    template<typename T> struct is_nothrow_copy_constructible:integral_constant<bool,is_pod<T>::value||__has_nothrow_copy(T)||__has_trivial_copy(T)>{};
# elif MWGCONF_GCC_VER>=40300&&!defined(__GCCXML__)
    template<typename T> struct is_nothrow_copy_constructible:integral_constant<bool,is_pod<T>::value||(__has_nothrow_copy(T)&&!is_reference<T>::value&&!is_volatile<T>::value)>{};
# elif defined(MWGCONF_CLANG_VER)
    template<typename T> struct is_nothrow_copy_constructible:integral_constant<bool,is_pod<T>::value||(__has_nothrow_copy(T)&&!is_reference<T>::value&&!is_volatile<T>::value)>{};
# else
#  define mwg_stdm_is_nothrow_copy_constructible__incomplete
    template<typename T> struct is_nothrow_copy_constructible:is_pod<T>{};
# endif
  }
  using is_constructible_detail::is_nothrow_copy_constructible;
#endif

//-----------------------------------------------------------------------------
//  is_copy_constructible
//-----------------------------------------------------------------------------
// using is_trivially_copy_constructible
// using is_nothrow_copy_constructible

#ifndef MWGCONF_HAS_IS_COPY_CONSTRUCTIBLE
  namespace is_constructible_detail{
# ifdef MWGCONF_HAS_IS_CONSTRUCTIBLE
    template<typename T> struct is_copy_constructible:is_constructible<T,typename add_lvalue_reference<const T>::type>{};
# elif defined(mwg_concept_is_valid_expression)
    template<typename T>
    mwg_concept_is_valid_expression(is_copy_constructible,Holder<T>,X,(X(mwg::declval<const X&>())));
# else
#  define mwg_stdm_is_copy_constructible__incomplete
    template<typename T>
    struct is_copy_constructible:integral_constant<
      bool,is_trivially_copy_constructible<T>::value||is_nothrow_copy_constructible<T>::value>{};
# endif
  }
  using is_constructible_detail::is_copy_constructible;
#endif

//-----------------------------------------------------------------------------
//  is_trivially_copy_assignable
//-----------------------------------------------------------------------------
// using is_pod

#ifndef MWGCONF_HAS_IS_TRIVIALLY_COPY_ASSIGNABLE
  namespace is_constructible_detail{
# ifdef MWGCONF_HAS_IS_TRIVIALLY_ASSIGNABLE
    template<typename T> struct is_trivially_copy_assignable:is_trivially_assignable<
      typename add_lvalue_reference<T>::type,
      typename add_lvalue_reference<const T>::type>{};
# elif defined(MWG_STDM_TYPE_TRAITS__IS_CONSTRUCTIBLE__tr1_available)
    template<typename T> struct is_trivially_copy_assignable:has_trivial_assign<T>{};
# elif MWGCONF_MSC_VER>=140050215
    template<typename T> struct is_trivially_copy_assignable:integral_constant<bool,is_pod<T>::value||__has_trivial_assign(T)>{};
# elif MWGCONF_GCC_VER>=40300&&!defined(__GCCXML__)
    template<typename T> struct is_trivially_copy_assignable:integral_constant<bool,is_pod<T>::value||(__has_trivial_assign(T)&&!is_const<T>::value&&!is_volatile<T>::value)>{};
# elif defined(MWGCONF_CLANG_VER)
    template<typename T> struct is_trivially_copy_assignable:integral_constant<bool,is_pod<T>::value||(__has_trivial_assign(T)&&!is_volatile<T>::value)>{};
# else
#  define mwg_stdm_is_trivially_copy_assignable__incomplete
    template<typename T> struct is_trivially_copy_assignable:is_pod<T>{};
# endif
  }
  using is_constructible_detail::is_trivially_copy_assignable;
#endif

//-----------------------------------------------------------------------------
//  is_nothrow_copy_assignable
//-----------------------------------------------------------------------------
// using is_pod

#ifndef MWGCONF_HAS_IS_NOTHROW_COPY_ASSIGNABLE
  namespace is_constructible_detail{
# ifdef MWGCONF_HAS_IS_NOTHROW_ASSIGNABLE
    template<typename T> struct is_nothrow_copy_assignable:is_nothrow_assignable<
      typename add_lvalue_reference<T>::type,
      typename add_lvalue_reference<const T>::type>{};
# elif defined(MWG_STDM_TYPE_TRAITS__IS_CONSTRUCTIBLE__tr1_available)
    template<typename T> struct is_nothrow_copy_assignable:has_nothrow_assign<T>{};
# elif MWGCONF_MSC_VER>=140050215
    template<typename T> struct is_nothrow_copy_assignable:integral_constant<bool,is_pod<T>::value||__has_nothrow_assign(T)||__has_trivial_assign(T)>{};
# elif MWGCONF_GCC_VER>=40300&&!defined(__GCCXML__)
    template<typename T> struct is_nothrow_copy_assignable:integral_constant<bool,is_pod<T>::value||(__has_nothrow_assign(T)&&!is_const<T>::value&&!is_volatile<T>::value)>{};
# elif defined(MWGCONF_CLANG_VER)
    template<typename T> struct is_nothrow_copy_assignable:integral_constant<bool,is_pod<T>::value||(__has_nothrow_assign(T)&&!is_volatile<T>::value)>{};
# else
#  define mwg_stdm_is_nothrow_copy_assignable__incomplete
    template<typename T> struct is_nothrow_copy_assignable:is_pod<T>{};
# endif
  }
  using is_constructible_detail::is_nothrow_copy_assignable;
#endif

//-----------------------------------------------------------------------------
//  is_copy_assignable
//-----------------------------------------------------------------------------
// using is_trivially_copy_assignable
// using is_nothrow_copy_assignable

#ifndef MWGCONF_HAS_IS_COPY_ASSIGNABLE
  namespace is_constructible_detail{
# ifdef MWGCONF_HAS_IS_ASSIGNABLE
    template<typename T> struct is_copy_assignable:is_assignable<
      typename add_lvalue_reference<T>::type,
      typename add_lvalue_reference<const T>::type>{};
# elif defined(mwg_concept_is_valid_expression)
    template<typename T>
    mwg_concept_is_valid_expression(is_copy_assignable,Holder<T>,X,(mwg::declval<X&>()=mwg::declval<const X&>()));
# else
#  define mwg_stdm_is_copy_assignable__incomplete
    template<typename T>
    struct is_copy_assignable:integral_constant<
      bool,is_trivially_copy_assignable<T>::value||is_nothrow_copy_assignable<T>::value>{};
# endif
  }
  using is_constructible_detail::is_copy_assignable;
#endif

//-----------------------------------------------------------------------------
// is_trivially_destructible
//-----------------------------------------------------------------------------

#ifndef MWGCONF_HAS_IS_TRIVIALLY_DESTRUCTIBLE
  namespace is_constructible_detail{
# if defined(MWG_STDM_TYPE_TRAITS__IS_CONSTRUCTIBLE__tr1_available)
    template<typename T> struct is_trivially_destructible:has_trivial_assign<T>{};
# elif MWGCONF_MSC_VER>=140050215
    template<typename T> struct is_trivially_destructible:integral_constant<bool,is_pod<T>::value||__has_trivial_destructor(T)>{};
# elif MWGCONF_GCC_VER>=40300&&!defined(__GCCXML__)
    template<typename T> struct is_trivially_destructible:integral_constant<bool,is_pod<T>::value||__has_trivial_destructor(T)>{};
# elif defined(MWGCONF_CLANG_VER)
    template<typename T> struct is_trivially_destructible:integral_constant<bool,is_pod<T>::value||__has_trivial_destructor(T)>{};
# else
#  define mwg_stdm_is_trivially_destructible__incomplete
    template<typename T> struct is_trivially_destructible:is_pod<T>{};
# endif
  }
  using is_constructible_detail::is_trivially_destructible;
#endif

//-----------------------------------------------------------------------------
// has_virtual_destructor
//-----------------------------------------------------------------------------

#ifndef MWGCONF_HAS_HAS_VIRTUAL_DESTRUCTOR
  namespace is_constructible_detail{
# if defined(MWG_STDM_TYPE_TRAITS__IS_CONSTRUCTIBLE__tr1_available)
    // template<typename T> struct has_virtual_destructor:has_virtual_destructor<T>{}; // same as that of the tr1
# elif MWGCONF_MSC_VER>=140050215
    template<typename T> struct has_virtual_destructor:integral_constant<bool,__has_virtual_destructor(T)>{};
# elif MWGCONF_GCC_VER>=40300&&!defined(__GCCXML__)
    template<typename T> struct has_virtual_destructor:integral_constant<bool,__has_virtual_destructor(T)>{};
# elif defined(MWGCONF_CLANG_VER)
    template<typename T> struct has_virtual_destructor:integral_constant<bool,__has_virtual_destructor(T)>{};
# else
#  define mwg_stdm_has_virtual_destructor__incomplete
    template<typename T> struct has_virtual_destructor:false_type{};
# endif
  }
# if !(MWGCONF_HEADER_STD>=2005||defined(MWGCONF_HEADER_TR1))
  using is_constructible_detail::has_virtual_destructor;
# endif
#endif

//-----------------------------------------------------------------------------

//HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
}
}
#endif
