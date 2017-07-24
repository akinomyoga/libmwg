// -*- mode: c++; coding: utf-8 -*-
#pragma%include "../impl/va_args.pp"
#ifndef MWG_STD_DEF_H
#define MWG_STD_DEF_H
#include <mwg/config.h>
#include <cstddef>
// std::size_t std::ptrdiff_t std::nullptr_t

namespace std {namespace tr1 {}}

namespace mwg {
  namespace stdm {
    using namespace ::std;

    // ※以下は元々 C++03 with TR1 の環境のための物だが問題がある。
    //   処理系によって std, std::tr1 の両方で同名の異なるメンバが定義されている。
    //   その場合、以下をすると名前解決ができなくなってしまう。
    // using namespace ::std::tr1;
  }
}

#if __cplusplus >= 201103L
# define MWG_STD_CXX11
#endif
#if __cplusplus >= 201402L
# define MWG_STD_CXX14
#endif
#undef MWG_STD_CXX17

#ifndef MWG_ATTRIBUTE_UNUSED
# ifdef __GNUC__
#  define MWG_ATTRIBUTE_UNUSED __attribute__((unused))
# else
#  define MWG_ATTRIBUTE_UNUSED
# endif
#endif

// c.f. Q_UNUSED
#ifndef mwg_unused
# define mwg_unused(param) (void) param
#endif

//==============================================================================
//  C++03 Features
//------------------------------------------------------------------------------
// #if defined(_MSC_VER) || defined(__WIN32)
// # define MWG_STD_WCHAR_UTF16
// #else
// # define MWG_STD_WCHAR_UTF32
// #endif
//
//==============================================================================
//  C++11 Features
//------------------------------------------------------------------------------
//  nullptr
//------------------------------------------------------------------------------
//?mconf X -t'std::nullptr_t' -oMWGCONF_STD_NULLPTR_T cstddef 'std::nullptr_t* value=0'
#if !mwg_has_feature(cxx_nullptr) && !defined(nullptr)
namespace mwg {
namespace stdm {
  static const class nullptr_t {
  public:
    template<class T>
    operator T*() const {return 0;}
    template<class C, class T>
    operator T C::*() const {return 0;}
  private:
    void operator&() const;
    void operator*() const;
  } nullptr_instance = {};

# define MWG_TEMP_OP(O)                                               \
  template<typename T> bool operator O(T* p, const nullptr_t&) {return p O 0;} \
  template<typename T> bool operator O(const nullptr_t&, T* p) {return 0 O p;} /**/
  MWG_TEMP_OP(==)
  MWG_TEMP_OP(!=)
  MWG_TEMP_OP(<)
  MWG_TEMP_OP(>)
  MWG_TEMP_OP(<=)
  MWG_TEMP_OP(>=)
# undef MWG_TEMP_OP
}
}
# define nullptr ::mwg::stdm::nullptr_instance
#elif !defined(MWGCONF_STD_NULLPTR_T) && mwg_has_feature(cxx_decltype)
namespace mwg {
namespace stdm {
  typedef decltype(nullptr) nullptr_t;
}
}
#endif
//------------------------------------------------------------------------------
//  Defaulted/deleted member functions
//------------------------------------------------------------------------------
#if mwg_has_feature(cxx_defaulted_functions)
# define mwg_std_defaulted = default
#else
# define mwg_std_defaulted
#endif
#if mwg_has_feature(cxx_deleted_functions)
# define mwg_std_deleted = delete
#else
# define mwg_std_deleted
#endif
//------------------------------------------------------------------------------
//  explicit conversion operators
//------------------------------------------------------------------------------
#if mwg_has_feature(cxx_explicit_conversions)
# define mwg_explicit_operator explicit operator
#else
# define mwg_explicit_operator operator
#endif
//------------------------------------------------------------------------------
//  constexpr
//------------------------------------------------------------------------------
#if mwg_has_feature(cxx_constexpr)
//
// Note: constexpr の振る舞いは度々変わっている。
//
// - C++11 では constexpr 関数の中身に強い制限があったが、C++14 では可也自由になった。
// - C++11 では constexpr のついている非静的メンバ関数は自動的に const qualified になったが、
//   C++14 以降では constexpr がついていても勝手に const になったりはしない。
//
# define mwg_constexpr constexpr
# define mwg_constexpr_const constexpr
# ifdef MWG_STD_CXX14
#  define mwg_constexpr14 constexpr
#  define MWGCONF_STD_CONSTEXPR14
# else
#  define mwg_constexpr14
# endif
#else
# define mwg_constexpr
# define mwg_constexpr_const const
# define mwg_constexpr14
#endif
//------------------------------------------------------------------------------
//  noexcept
//------------------------------------------------------------------------------
#if mwg_has_feature(cxx_noexcept)
# define mwg_noexcept         noexcept
# define mwg_noexcept_when(A) noexcept(A)
#else
# define mwg_noexcept throw()
# define mwg_noexcept_when(A)
#endif
//------------------------------------------------------------------------------
//  override / final
//------------------------------------------------------------------------------
#if mwg_has_feature(cxx_override_control)
# define mwg_override override
# define mwg_final    final
#else
# define mwg_override
# define mwg_final
#endif
//------------------------------------------------------------------------------
//  auto / -> decltype()
//------------------------------------------------------------------------------
#if mwg_has_feature(cxx_auto_type) && mwg_has_feature(cxx_decltype)
# define mwg_auto(T) auto
# define mwg_decltyped(EXPR) -> decltype(EXPR)
#else
# define mwg_auto(T) T
# define mwg_decltyped(EXPR)
#endif
//------------------------------------------------------------------------------
//  __VA_ARGS__
//------------------------------------------------------------------------------
#if MWGCONF_MSC_VER? (MWGCONF_MSC_VER >= 1400): (MWGCONF_GCC_VER? (MWGCONF_GCC_VER >= 30000): 1)
# define MWG_STD_VA_ARGS
#endif
//------------------------------------------------------------------------------
//  mwg_static_assert
//------------------------------------------------------------------------------
#if defined(MWG_STD_CXX17)
# define mwg_static_assert(...) static_assert(__VA_ARGS__)
#elif mwg_has_feature(cxx_static_assert)
# pragma%m 1
#  define mwg_static_assert(C, ...) static_assert(C, __VA_ARGS__ "")
# pragma%end
# pragma%x mwg::va_args::declare_variadic_macro
#else
# define MWG_PREPROC_ADDLINE_2(H, L) H##L
# define MWG_PREPROC_ADDLINE_(H, L)  MWG_PREPROC_ADDLINE_2(H, L)
# define MWG_PREPROC_ADDLINE(H)      MWG_PREPROC_ADDLINE_(H, __LINE__)
# define MWG_PREPROC_COMMA           , /* unused */
namespace mwg {
  namespace detail {
    template<bool B, int LINE>
    struct static_assert_tester {};
    template<int LINE>
    struct static_assert_tester<true, LINE> {
      static_assert_tester(...) {} /* to suppress unused warnings */
      typedef int type;
      static const int value = LINE;
    };
  }

// 以下は、C++03 において依存型・非依存型で
// typename が必要だったりそうでなかったりするので駄目
/*
# define mwg_static_assert(C,...) \
    struct static_assert_line##__LINE__ { typename mwg::detail::static_assert_tester<C>::type value; }
*/

/* 実装のメモ
 *
 * 1 static tester<C, __LINE__> dummy; とすると
 *   クラス内で使った時に実体の定義を要求されてしまう。
 *   といって static を外すとクラスのサイズが無駄に大きくなってしまう。
 *   また static const int dummy = hello; という形にする訳にも行かない。
 *
 * 2 tester<C, __LINE__>::type dummy; とすると
 *   C が template type parameter に依存している時に typename が必要になる。
 *
# define mwg_static_assert(C, Message)                                       \
  static MWG_ATTRIBUTE_UNUSED const mwg::detail::static_assert_tester<C, __LINE__>::type      \
    MWG_PREPROC_ADDLINE(static_assert_at_line_) = (0);
 */
# pragma%m 1
#  define mwg_static_assert(C, ...) \
  enum{ MWG_PREPROC_ADDLINE(static_assert_at_line_) = mwg::detail::static_assert_tester<C, __LINE__>::value }
# pragma%end
# pragma%x mwg::va_args::declare_variadic_macro
}
#endif
//------------------------------------------------------------------------------
//  mwg_has_feature(cxx_inline_variables)
//------------------------------------------------------------------------------
#if mwg_has_feature(cxx_inline_variables)
# define mwg_inline_variable inline
# define mwg_inline_variable_static inline
# define mwg_inline_variable_constexpr inline
#else
# define mwg_inline_variable
# define mwg_inline_variable_static static
# define mwg_inline_variable_constexpr constexpr
#endif
//==============================================================================
//  Compiler Intrinsic Features
//------------------------------------------------------------------------------
#ifdef _MSC_VER
# // [VCBUG] typename を使えない筈なのに必要とする場合がある
# define mwg_vc_typename typename
#else
# define mwg_vc_typename
#endif
#if (MWGCONF_GCC_VER >= 40200) || MWGCONF_CLANG_VER || MWGCONF_ICC_VER
# define MWG_STD_QUALIFIED_FUNCTION_TYPES
#endif
#if (MWGCONF_GCC_VER && MWGCONF_GCC_VER < 30400)
# define mwg_gcc336bug20160326_template template
#else
# define mwg_gcc336bug20160326_template
#endif
#if (MWGCONF_GCC_VER && MWGCONF_GCC_VER < 30000)
# define MWGCONF_GCC295BUG_USING_NAMESPACE_STD
#endif
//==============================================================================
//  Utility
//------------------------------------------------------------------------------
namespace mwg {
  template<typename T> struct identity {typedef T type;};

  namespace detail {
#if mwg_has_feature(cxx_rvalue_references)
    template<typename T>
    struct declval_type: mwg::identity<T&&> {typedef T&& reference_type;};
    template<typename T, unsigned N>
    struct declval_type<T[N]>: mwg::identity<T (&&)[N]> {typedef T (&&reference_type)[N];};
    template<typename T>
    struct declval_type<T[]>: mwg::identity<T (&&)[1]> {typedef T (&&reference_type)[1];};
#else
    template<typename T>
    struct declval_type: mwg::identity<T> {typedef T& reference_type;};
    template<typename T>
    struct declval_type<T&>: mwg::identity<T&> {typedef T& reference_type;};
    template<typename T, unsigned N>
    struct declval_type<T[N]>: mwg::identity<T (&)[N]> {typedef T (&reference_type)[N];};
    template<typename T>
    struct declval_type<T[]>: mwg::identity<T (&)[1]> {typedef T (&reference_type)[1];};
#endif

    template<> struct declval_type<void> {};
    template<> struct declval_type<void const> {};
    template<> struct declval_type<void volatile> {};
    template<> struct declval_type<void const volatile> {};
  }

  //template<typename T> T&& declval();
  template<typename T> typename detail::declval_type<T>::type declval() {
    static struct {char m[sizeof(T)];} dummy;
    return reinterpret_cast<typename detail::declval_type<T>::reference_type>(dummy);
  }
}

#endif
