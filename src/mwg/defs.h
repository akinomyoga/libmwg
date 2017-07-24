// -*- mode: c++; coding: utf-8 -*-
#pragma%include "impl/ManagedTest.pp"
#pragma%x begin_check
#include <mwg/defs.h>
#pragma%x end_check
#ifndef MWG_DEFS_H
#define MWG_DEFS_H

#include <mwg/config.h>

/* mwg/std/def.h
 *
 *   mwg_unused
 *   namespace mwg::stdm
 *   C++03/11/14 language features
 *   mwg::identity
 *   mwg::declval
 */
#include <mwg/std/def.h>

#include <mwg/std/cstdint> /* requires mwg_static_assert */

//#include <mwg/bits/cxx.char_t.h>

/* 以下は元々コメントの区切りに使っていた物の一覧だが現在は使用に消極的である。
 *
 * - N 名前空間定義
 * - T 型定義
 * - F 関数定義・フィールド定義
 * - M マクロ定義
 * - C コメント
 * - E 例
 * - X 試験
 */

//=============================================================================
//  integral types
//-----------------------------------------------------------------------------
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
}

//=============================================================================
//  character types
//-----------------------------------------------------------------------------
namespace mwg {
  // defined in <mwg/char.h>
  template<typename T, int CP = 0>
  struct char_data;

  template<typename T, int CP = 0>
  class char_t;

#pragma%x begin_test
  void test() {
    typedef mwg::cxx::char_detail::char_t<mwg::stdm::uint16_t> c2t;

    c2t value = '0';
    mwg_check(value == value);
    mwg_check(value == (int) '0');
    mwg_check((int) '0' == value);
    mwg_check(value == '0');
    mwg_check('0' == value);

    c2t v2 = value;
    v2 += value;
    v2 += '0';
    mwg_check(value + value == 2 * value);
    mwg_check(value + 1 == '1');
    mwg_check(1 + value == '1');

    mwg_check(0);
  }
#pragma%x end_test

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

}

//=============================================================================
// forward declaration
//-----------------------------------------------------------------------------
namespace mwg {
  // defined in <mwg/except.h>
  class except;
}

#endif
#pragma%x begin_check
int main() {
  managed_test::run_tests();
  return 0;
}
#pragma%x end_check
