// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BITS_TYPE_TRAITS_MEMBER_POINTER_H
#define MWG_BITS_TYPE_TRAITS_MEMBER_POINTER_H
#pragma%include "../impl/VariadicMacros.pp"
#include <mwg/defs.h>
namespace mwg {
namespace type_traits {

  /*?lwiki
   * :@class class mwg::type_traits::==is_member_pointer==<MP>;
   *  :@var static const bool ==value==;
   *   `MP` がメンバポインタかどうかを判定します。
   *  :@typedef typedef '''object/function-type''' ==member_type==;
   *   `MP` がメンバポインタの時、メンバのオブジェクト型または関数型を取得します。
   *  :@typedef typedef '''class-type''' ==object_type==;
   *   `MP` がメンバポインタの時、メンバが定義されるクラスを取得します。
   */
  namespace detail {
    template<class MP>
    struct is_member_pointer {
      static mwg_constexpr_const bool value = false;
      typedef void member_type;
      typedef void object_type;
    };
    template<class C, class T>
    struct is_member_pointer_def {
      static mwg_constexpr_const bool value = true;
      typedef C object_type;
      typedef T member_type;
    };
    template<class T, class C>
    struct is_member_pointer<T C::*>: is_member_pointer_def<C, T> {};
#pragma%m 2
    template<class R, class C, class... A>
    struct is_member_pointer<R (C::*)(A...) QUALIFIER>:
      is_member_pointer_def<C QUALIFIER, R (A...)> {};
    template<class R, class C, class... A>
    struct is_member_pointer<R (C::*)(A..., ...) QUALIFIER>:
      is_member_pointer_def<C QUALIFIER, R (A..., ...)> {};
#pragma%end

    // unqualified member function pointers:
    //   一部の古いコンパイラでメンバ関数ポインタは T C::* に一致しない。
#if defined(MWGCONF_GCC_VER) && (MWGCONF_GCC_VER < 40200) || defined(MWGCONF_MSC_VER)
#pragma%m 1
#pragma%x 2.r/QUALIFIER//
#pragma%end
#pragma%x variadic_expand_0toArNm1
#endif

    // qualified member function pointers:
    //   qualifier を object_type の側に付け替える。
#pragma%m 1
#pragma%x 2.r/QUALIFIER/const/
#pragma%x 2.r/QUALIFIER/volatile/
#pragma%x 2.r/QUALIFIER/const volatile/
#pragma%end
#pragma%x variadic_expand_0toArNm1
  }
  template<typename MemPtr>
  struct is_member_pointer: detail::is_member_pointer<MemPtr> {};

  /*?lwiki
   * :@class class mwg::type_traits::==create_member_pointer==<C, T>;
   *  クラスの型とメンバの型からメンバポインタ型を生成します。
   *  :@tparam[in] typename C;
   *   メンバを保持するクラスの型を指定します。
   *  :@tparam[in] typename T;
   *   メンバを保持するクラスの型を指定します。
   *  :@typedef typedef '''member-pointer-type''' ==type==;
   *   メンバポインタ型を提供します。
   */
  template<typename C, typename T>
  struct create_member_pointer: mwg::identity<T C::*> {};
#pragma%m 2
  template<class R, class C, class... A>
  struct create_member_pointer<C QUALIFIER, R (A...)>: mwg::identity<R (C::*)(A...) QUALIFIER> {};
  template<class R, class C, class... A>
  struct create_member_pointer<C QUALIFIER, R (A..., ...)>: mwg::identity<R (C::*)(A..., ...) QUALIFIER> {};
#pragma%end
#pragma%m 1
#pragma%x 2.r/QUALIFIER//
#pragma%x 2.r/QUALIFIER/const/
#pragma%x 2.r/QUALIFIER/volatile/
#pragma%x 2.r/QUALIFIER/const volatile/
#pragma%end
#pragma%x variadic_expand_0toArNm1
#if defined(MWGCONF_STD_VARIADIC_TEMPLATES) && defined(MWGCONF_STD_REF_QUALIFIERS)
# pragma%x 2.r/QUALIFIER/\&/
# pragma%x 2.r/QUALIFIER/const\&/
# pragma%x 2.r/QUALIFIER/volatile\&/
# pragma%x 2.r/QUALIFIER/const volatile\&/
# if defined(MWGCONF_STD_RVALUE_REFERENCES)
#  pragma%x 2.r/QUALIFIER/\&\&/
#  pragma%x 2.r/QUALIFIER/const\&\&/
#  pragma%x 2.r/QUALIFIER/volatile\&\&/
#  pragma%x 2.r/QUALIFIER/const volatile\&\&/
# endif
#endif

}
}
#endif
