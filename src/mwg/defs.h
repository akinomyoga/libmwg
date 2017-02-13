// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_DEFS_H
#define MWG_DEFS_H

#include <cstddef>
// std::size_t std::ptrdiff_t std::nullptr_t

#include <mwg/config.h>

//------------------------------------------------------------------------------
//N 名前空間定義
//T 型定義
//F 関数定義・フィールド定義
//M マクロ定義
//C コメント
//E 例
//X 試験
//------------------------------------------------------------------------------

namespace std {namespace tr1 {}}

namespace mwg {
  namespace stdm {
    using namespace ::std;

    // ※以下は元々 C++03 with TR1 の環境のための物だが問題がある。
    //   処理系によって std, std::tr1 の両方で同名の異なるメンバが定義されている。
    //   その場合、以下をすると名前解決ができなくなってしまう。
    // using namespace ::std::tr1;
  }

#   define MWG_PREPROC_ADDLINE__(H, L) H##L
#   define MWG_PREPROC_ADDLINE_(H, L)  MWG_PREPROC_ADDLINE__(H, L)
#   define MWG_PREPROC_ADDLINE(H)      MWG_PREPROC_ADDLINE_(H, __LINE__)
#   define MWG_PREPROC_COMMA           ,

#ifndef MWG_ATTRIBUTE_UNUSED
# ifdef __GNUC__
#  define MWG_ATTRIBUTE_UNUSED __attribute__((unused))
# else
#  define MWG_ATTRIBUTE_UNUSED
# endif
#endif

// c.f. Q_UNUSED
#ifndef mwg_unused
# define mwg_unused(param) (void)param
#endif
}

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//  C++03 Features
//------------------------------------------------------------------------------
#if defined(_MSC_VER) || defined(__WIN32)
# define MWG_STD_WCHAR_UTF16
#else
# define MWG_STD_WCHAR_UTF32
#endif
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//  C++0x Features
//------------------------------------------------------------------------------
//  nullptr
//------------------------------------------------------------------------------
//?mconf X -t'std::nullptr_t' -oMWGCONF_STD_NULLPTR_T cstddef 'std::nullptr_t* value=0'
#if !defined(MWGCONF_STD_NULLPTR) && !defined(nullptr)
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

#   define MWG_TEMP_OP(O)                                               \
  template<typename T> bool operator O(T* p, const nullptr_t&) {return p O 0;} \
  template<typename T> bool operator O(const nullptr_t&, T* p) {return 0 O p;} /**/
  MWG_TEMP_OP(==)
  MWG_TEMP_OP(!=)
  MWG_TEMP_OP(<)
  MWG_TEMP_OP(>)
  MWG_TEMP_OP(<=)
  MWG_TEMP_OP(>=)
#   undef MWG_TEMP_OP
}
}
#   define nullptr ::mwg::stdm::nullptr_instance
#elif !defined(MWGCONF_STD_NULLPTR_T) && defined(MWGCONF_STD_DECLTYPE)
namespace mwg {
namespace stdm {
  typedef decltype(nullptr) nullptr_t;
}
}
#endif
//------------------------------------------------------------------------------
//  Defaulted/deleted member functions
//------------------------------------------------------------------------------
#ifdef MWGCONF_STD_DEFAULTED_FUNCTIONS
# define mwg_std_defaulted = default
#else
# define mwg_std_defaulted
#endif
#ifdef MWGCONF_STD_DELETED_FUNCTIONS
# define mwg_std_deleted = delete
#else
# define mwg_std_deleted
#endif
//------------------------------------------------------------------------------
//  explicit conversion operators
//------------------------------------------------------------------------------
#ifdef MWGCONF_STD_EXPLICIT_CONVERSIONS
# define mwg_explicit_operator explicit operator
#else
# define mwg_explicit_operator operator
#endif
//------------------------------------------------------------------------------
//  constexpr
//------------------------------------------------------------------------------
#ifdef MWGCONF_STD_CONSTEXPR
//
// Note: constexpr の振る舞いは度々変わっている。
//
// - C++11 では constexpr 関数の中身に強い制限があったが、C++14 では可也自由になった。
// - C++11 では constexpr のついている非静的メンバ関数は自動的に const qualified になったが、
//   C++14 以降では constexpr がついていても勝手に const になったりはしない。
//
# define mwg_constexpr constexpr
# define mwg_constexpr_const constexpr
# define mwg_constexpr14
#else
# define mwg_constexpr
# define mwg_constexpr_const const
# define mwg_constexpr14
#endif
//------------------------------------------------------------------------------
//  static_assert
//------------------------------------------------------------------------------
#ifndef MWGCONF_STD_STATIC_ASSERT
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
# define static_assert(C,...)                                     \
    struct static_assert_line##__LINE__{                          \
      typename mwg::detail::static_assert_tester<C>::type value; \
    }
*/

/* 実装のメモ
 *
 * 1 static tester<C, __LINE__> dummy; とすると
 *   クラス内で使った時に実体の定義を要求されてしまう。
 *   といって static を外すとクラスのサイズが無駄に大きくなってしまう。
 *   また static const int dummy=hello; という形にする訳にも行かない。
 *
 * 2 tester<C, __LINE__>::type dummy; とすると
 *   C が template type parameter に依存している時に typename が必要になる。
 *
# define static_assert(C, Message)                                       \
  static MWG_ATTRIBUTE_UNUSED const mwg::detail::static_assert_tester<C, __LINE__>::type      \
    MWG_PREPROC_ADDLINE(static_assert_at_line_) = (0);
 */
# define static_assert(C, Message)                                       \
  enum{ MWG_PREPROC_ADDLINE(static_assert_at_line_) = mwg::detail::static_assert_tester<C, __LINE__>::value }
}
#endif
//------------------------------------------------------------------------------
//  __VA_ARGS__
//------------------------------------------------------------------------------
#if defined(_MSC_VER)? (_MSC_VER >= 1400): (defined(__GNUC__)? (__GNUC__ >= 3): 1)
# define MWG_STD_VA_ARGS
#endif
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//  Compiler Intrinsic Features
//------------------------------------------------------------------------------
#ifdef _MSC_VER
# // [VCBUG] typename を使えない筈なのに必要とする場合がある
# define mwg_vc_typename typename
#else
# define mwg_vc_typename
#endif
#if defined(MWGCONF_GCC_VER) && (MWGCONF_GCC_VER >= 40200) || defined(MWGCONF_CLANG_VER) || defined(MWGCONF_ICC_VER)
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
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  Utility
//------------------------------------------------------------------------------
namespace mwg {
  template<typename T> struct identity {typedef T type;};

#ifdef MWGCONF_STD_RVALUE_REFERENCES
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

  //template<typename T> T&& declval();
  template<typename T> typename declval_type<T>::type declval() {
    static struct {char m[sizeof(T)];} dummy;
    return reinterpret_cast<typename declval_type<T>::reference_type>(dummy);
  }
}
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  整数型の定義
//------------------------------------------------------------------------------
#include <mwg/std/cstdint> /* requires static_assert */
namespace mwg {
  typedef mwg::stdm::int8_t   i1t;
  typedef mwg::stdm::int16_t  i2t;
  typedef mwg::stdm::int32_t  i4t;
  typedef mwg::stdm::int64_t  i8t;
  typedef mwg::stdm::uint8_t  u1t;
  typedef mwg::stdm::uint16_t u2t;
  typedef mwg::stdm::uint32_t u4t;
  typedef mwg::stdm::uint64_t u8t;

  typedef mwg::stdm::intptr_t  iPt;
  typedef mwg::stdm::uintptr_t uPt;

  typedef u1t size1t;
  typedef u2t size2t;
  typedef u4t size4t;
  typedef u8t size8t;
  typedef i1t ptrdiff1t;
  typedef i2t ptrdiff2t;
  typedef i4t ptrdiff4t;
  typedef i8t ptrdiff8t;

  typedef std::ptrdiff_t itt; // c.f. "t" = ptrdiff_t in printf (C99)
  typedef std::size_t    uzt; // c.f. "z" = size_t    in printf (C99)

  typedef u1t byte ;
  typedef u2t word ;
  typedef u4t dword;
  typedef u8t qword;

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  文字型の定義

  // defined in <mwg/char.h>
  template<typename T, int CP = 0>
  struct char_data;

  template<typename T, int CP = 0>
  class char_t;

#ifdef _MSC_VER
  // VC では wchar_t, char16_t, char32_t は符号無し整数の typedef
  typedef char           cAt;
  typedef __wchar_t      cWt;
  typedef unsigned short _cWt;
  typedef char_t<u1t>    c1t;
  typedef char_t<u2t>    c2t;
  typedef char_t<u4t>    c4t;
# define MWG_TA(x) x
# define MWG_TW(x) L##x
#elif defined(MWGCONF_STD_CHAR16_T)
  typedef char           cAt; // machine native
  typedef wchar_t        cWt; // machine native wide
  typedef char_t<u1t>    c1t; // utf-8
  typedef char16_t       c2t; // utf-16
  typedef char32_t       c4t; // utf-32
# define MWG_TA(x) x
# define MWG_TW(x) L##x
# define MWG_T1(x) u8##x
# define MWG_T2(x) u##x
# define MWG_T4(x) U##x
#else
  typedef char           cAt;
  typedef wchar_t        cWt;
  typedef char_t<u1t>    c1t;
  typedef char_t<u2t>    c2t;
  typedef char_t<u4t>    c4t;
# define MWG_TA(x) x
# define MWG_TW(x) L##x
#endif

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT

  // defined in <mwg/except.h>
  class except;
}

#endif
