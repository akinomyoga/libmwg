// -*- mode: c++; coding: utf-8 -*-
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

#include <mwg/bits/cxx.inttype.h>

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
#ifdef _MSC_VER
  // VC では wchar_t, char16_t, char32_t は符号無し整数の typedef
  typedef char           cAt;
  typedef __wchar_t      cWt;
  typedef unsigned short _cWt;
  typedef mwg::cxx::inttype<u1t, struct c1t_tag> c1t;
  typedef mwg::cxx::inttype<u2t, struct c2t_tag> c2t;
  typedef mwg::cxx::inttype<u4t, struct c4t_tag> c4t;
#elif defined(MWGCONF_STD_CHAR16_T)
  typedef char    cAt; // implementation native
  typedef wchar_t cWt; // implementation native wide
  typedef mwg::cxx::inttype<u1t, struct c1t_tag> c1t; // utf-8
  typedef char16_t c2t; // utf-16
  typedef char32_t c4t; // utf-32
#else
  typedef char    cAt;
  typedef wchar_t cWt;
  typedef mwg::cxx::inttype<u1t, struct c1t_tag> c1t;
  typedef mwg::cxx::inttype<u2t, struct c2t_tag> c2t;
  typedef mwg::cxx::inttype<u4t, struct c4t_tag> c4t;
#endif

}

//=============================================================================
// forward declaration
//-----------------------------------------------------------------------------
namespace mwg {
  // defined in <mwg/char.h>
  template<typename T, int CP = 0> struct char_data;
  template<typename T, int CP = 0> class char_t;

  // defined in <mwg/except.h>
  class except;
}

#endif
