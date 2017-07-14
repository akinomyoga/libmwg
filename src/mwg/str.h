// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_STR_H
#define MWG_STR_H
//HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
#include <cstddef>
#include <iterator>
#include <mwg/std/utility>
#include <mwg/std/type_traits>
#include <mwg/std/memory>
#include <mwg/std/limits>
#include <mwg/range.h>
#include <mwg/functor.h>
#include <mwg/concept.h>
#pragma%include "impl/ManagedTest.pp"
#pragma%x begin_check
#include <mwg/str.h>
#define _a mwg::str

template<typename T>
void check_iterator(T const& str) {
  typedef typename T::const_iterator Iter;
  typedef typename T::char_type char_type;
  Iter b = str.begin(), e = str.end();
  std::size_t l = str.length();

  Iter i; // should be default constructible
  i = b;    // should be copy assignable

  // check relational operators, add/sub operators, and increments
  for (std::size_t index = 0; index < l; index++, ++i) {
    char_type ch = i[index]; // check if it is indexable
    mwg_unused(ch);
    mwg_check(b <= i && i < e && (i <= e && i != e), "bad relational operators");
    mwg_check(i == b + index && i == e - (l - index) && i - b == (std::ptrdiff_t) index, "bad add/subtraction operators");
  }
  mwg_check(i == e && !(i != e));
  for (std::size_t index = 0; index < l; index++, i--) {
    mwg_check(e >= i && i > b && (i >= b && i != b), "bad relational operators");
    mwg_check(i == e - index && i == b + (l - index) && e - i == (std::ptrdiff_t) index, "bad add/subtraction operators");
  }
  mwg_check(i == b && !(i != b));
  mwg_check((i += l) == e);
  mwg_check((i -= l) == b);
}

#pragma%x end_check

namespace mwg {
namespace str_detail {
  template<typename XCH>
  struct char_traits;

  // ※Dummy は struct strbase_tag<XCH> : strbase_tag<> {}; とするため。
  template<typename XCH = void, int Dummy = 0>
  struct strbase_tag;
  template<typename Policy>
  class strbase;

  template<typename T, typename XCH = void>
  struct adapter_traits;
  template<typename T, typename XCH = void>
  struct as_str;

  /*?lwiki
   * :@fn String mwg::==str==(obj);
   *  指定したオブジェクトに対する mwg/str インターフェイスを取得します。\
   *  mwg/str 文字列の操作に関しては後述の節『[[文字列基本機能>#strbase_interface]]』を参照して下さい。
   *  ※`str` はローカル変数として一般的な識別子です。混乱を避けるために、\
   *  `mwg::str` を利用する時は省略せずに `mwg::str(...)` の形で使用することを推奨します。
   */
  template<typename T>
  typename adapter_traits<T>::adapter_type
  str(T const& value) {return typename adapter_traits<T>::adapter_type(value);}

  /*?lwiki
   * :@class class mwg::==strfix==<XCH> : str_detail::strbase<...>;
   *  mwg/str における標準の文字列型です。
   *  `std::shared_ptr` による参照管理の対象です。
   * :@class class mwg::==strsub==<XCH> : str_detail::strbase<...>;
   *  他の文字列の部分文字列を保持します。
   * :@namespace mwg::==str_detail==;
   *  実装用の名前空間です。
   *  mwg/str 文字列 (`strbase<...>`) に対して ADL を介して呼び出す関数はこの名前空間内に定義して下さい。
   * :@class class mwg::str_detail::==strbase==<...>;
   *  mwg/str における文字列型は全てこの型から派生します。
   *  文字列に対する操作を提供する基底クラスです。
   *  また、部分式の評価結果として `strbase` の様々な特殊化が使用されます。
   */
  template<typename XCH>
  class strsub;
  template<typename XCH>
  class strfix;

  template<typename XCH>
  class _stradp_array;

  template<typename StrP1>
  struct _strtmp_sub_policy;
  template<typename StrP, typename Filter>
  struct _strtmp_map_policy;
  template<typename StrP, typename Filter>
  struct _strtmp_ranged_map_policy;
  template<typename XCH>
  struct _filt_tolower;
  template<typename XCH>
  struct _filt_toupper;
  template<typename XCH>
  struct _filt_replace_char;
  template<typename Str1, typename Str2, typename Str3 = void>
  struct _strtmp_cat_policy;
  template<typename Str>
  struct _strtmp_pad_policy;
  template<typename Str>
  struct _strtmp_reverse_policy;
  template<typename Str>
  struct _strtmp_repeat_policy;
}
  static const mwg_constexpr std::ptrdiff_t npos
    =mwg::stdm::numeric_limits<std::ptrdiff_t>::lowest();

  using str_detail::as_str;
  using str_detail::str;
  using str_detail::strfix;
  using str_detail::strsub;
}
namespace mwg {
namespace str_detail {
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  inline std::size_t canonicalize_index(std::ptrdiff_t const& index, std::size_t const& len) {
    if (index == mwg::npos) {
      return len;
    } else if (index < 0) {
      std::ptrdiff_t const _index = len + index;
      return _index >= 0? _index: 0;
    } else
      return (std::size_t) index < len? index: len;
  }

  template<typename T>
  mwg_concept_has_member(has_memfn_index, T, X, index, std::ptrdiff_t (X::*)() const);

#pragma%m mwg_str::policy_requirements
/*?lwiki
&pre(!cpp){
struct StringPolicy {
  typedef char          char_type;
  typedef const char&   char_reference; // can be char, const char&
  typedef StringPolicy  policy_type;

  static const bool has_get_ptr;

  struct const_iterator: public mwg::str_detail::const_iterator_base<Policy> {
    // 以下の関数は index を自明に取得可能な場合に定義する。
    std::ptrdiff_t index() const;

    typedef char_reference reference;
    typedef typename const_iterator_base<Policy>::pointer pointer;
    typedef typename const_iterator_base<Policy>::difference_type difference_type;

    // BidirectionalIterator 要件
    reference operator*() const;
    pointer operator*() const;
    const_iterator& operator++();
    const_iterator  operator++(int);
    const_iterator& operator--();
    const_iterator  operator--(int);
    bool operator==(const_iterator const&) const;
    bool operator!=(const_iterator const&) const;

    // 追加要件
    difference_type operator-(const_iterator const&) const;

    // RandomAccessIterator のインターフェイスも要求する。
    // 但し、実装が非効率な場合には category は random_access_iterator_tag にしない。
    char_reference  operator[](difference_type);
    bool operator< (const_iterator const&) const;
    bool operator<=(const_iterator const&) const;
    bool operator> (const_iterator const&) const;
    bool operator>=(const_iterator const&) const;
    const_iterator  operator+ (difference_type) const;
    const_iterator  operator- (difference_type) const;
    const_iterator& operator+=(difference_type);
    const_iterator& operator-=(difference_type);
    friend inline const_iterator operator+(difference_type, const_iterator const&);
  };

  struct buffer_type {
    char_reference operator[](std::ptrdiff_t) const;

    std::size_t length() const;

    const_iterator begin() const;
    const_iterator end() const;
    const_iterator begin_at(std::ptrdiff_t) const;

    // 以下の関数は StringPolicy::has_get_ptr==true の時にだけ定義される
    const char_type* get_ptr() const;
  };
};
}
*/
#pragma%end

template<typename Char>
class _tmpobj_arrow_operator {
  Char value;
public:
  mwg_constexpr explicit _tmpobj_arrow_operator(Char value): value(value) {}
  mwg_constexpr Char const* operator->() const {return &this->value;}
};
template<typename Char>
class _tmpobj_arrow_operator<Char&> {
  Char& value;
public:
  explicit _tmpobj_arrow_operator(Char& value): value(value) {}
  Char* operator->() const {return &this->value;}
};

#pragma%x begin_test
class Obj {
  int value;
public:
  Obj(int value): value(value) {}
  int getHalf() const {return this->value / 2;}
};
class FakeStorage {
  int value;
public:
  FakeStorage(int value): value(value) {}

  typedef mwg::str_detail::_tmpobj_arrow_operator<Obj> pointer;
  pointer operator->() const {return pointer(Obj(this->value));}
};

void test() {
  FakeStorage fake(123);
  mwg_check((fake->getHalf() == 61));
}
#pragma%x end_test

template<typename Policy, typename IteratorCategory = std::bidirectional_iterator_tag>
struct const_iterator_base: std::iterator<
  IteratorCategory,
  typename Policy::char_type,
  std::ptrdiff_t,
  // operator->() の戻り値の型
  _tmpobj_arrow_operator<typename Policy::char_reference>,
  // operator*() の戻り値の型
  typename Policy::char_reference
> {};

// pointer_const_iterator 用
template<typename XCH>
struct const_iterator_base<XCH*>: std::iterator<
  std::bidirectional_iterator_tag, XCH,
  std::ptrdiff_t,
  /* @var typename pointer;
   *   `operator->()` の戻り値の型として使用する。
   *   @remarks {
   *   他の `const_iterator::pointer` と同様に文字の参照から初期化できるようにする為、
   *   `_tmp_arrow_operator` でくるむ必要がある。}
   */
  _tmpobj_arrow_operator<XCH*>,
  XCH const&
> {};

//-----------------------------------------------------------------------------
// char_traits

// Tr 要件
template<typename XCH>
struct char_traits {
  typedef XCH char_type;

  static std::size_t strlen(const char_type* s);
  static mwg_constexpr char_type null();
};

//-----------------------------------------------------------------------------
// adapter_traits

/*?lwiki
 * :@class struct mwg::str_detail::==adapter_traits==<T>;
 * :@class struct mwg::str_detail::==adapter_traits==<T, XCH>;
 *  任意に与えられた型に対して mwg/str インターフェイスを提供する方法を定義します。
 *  既定で以下の型に対して mwg/str インターフェイスが定義されています。
 *  -`XCH[N]`
 *  -`const XCH[N]`
 *  -`XCH*`
 *  -`const XCH*`
 *  -`std::basic_string<XCH>`
 *  `adapter_traits<T>` の特殊化を定義する事によって、\
 *  新しい型に対して mwg/str インターフェイスを提供できます。
 *  `adapter_traits<T>` の特殊化では以下のメンバを定義します。
 *  &pre(!cpp){
 * struct adapter_traits<T> {
 *   static const bool available = true;
 *   typedef something char_type;
 *   class adapter_type {
 *     adapter_type(T const&);
 *   };
 * };
 * }
 *  :@const static const bool ==available==;
 *   mwg/str インターフェイスを提供できる場合に `true` を設定します。
 *  :@typedef[opt] typename ==adapter_type==;
 *   `available == true` の時に定義します。
 *   具体的に mwg/str を提供する型を指定します。
 *   目的の型 `T` から直接構築できる必要があります。
 *  :@typedef[opt] typename ==char_type==;
 *   `available == true` の時に定義します。
 *   提供される文字列インターフェイスの文字型を指定します。
 *  更に `adapter_traits<T, XCH>` を特殊化する事によって、\
 *  明示的に文字型を指定した時の文字列インターフェイスを提供する方法を定義できます。
 *  上記 `adapter_traits<T>` の時と同じメンバを定義します。\
 *  但し、`char_type` は `XCH` に一致している必要があります。
 */

struct adapter_traits_empty {static const bool available = false;};

template<typename T, bool IsStr>
struct _adapter_traits_1: adapter_traits_empty {};
template<typename T, typename XCH, bool IsString>
struct _adapter_traits_2: adapter_traits_empty {};

template<typename T, typename XCH>
struct adapter_traits: _adapter_traits_2<T, XCH, adapter_traits<T>::available> {};
template<typename T>
struct adapter_traits<T>: _adapter_traits_1<T, stdm::is_base_of<strbase_tag<>, T>::value> {};

template<typename T>
struct _adapter_traits_1<T, true> {
  static const bool available = true;
  typedef T const& adapter_type;
  typedef typename T::char_type char_type;
};
template<typename T, typename XCH>
struct _adapter_traits_2<T, XCH, true>: stdm::conditional<
  mwg::stdm::is_same<typename adapter_traits<T>::char_type, XCH>::value,
  adapter_traits<T>, adapter_traits_empty
  >::type {};

/*?lwiki
 * :@class mwg::==as_str==<T, XCH>;
 *  :@const static const bool ==value==;
 *  :@const static const bool ==available==;
 *   adapter が利用可能かどうかを保持します。
 *  :@typedef[opt] typename ==enable==<R>::type;  // value == true のとき
 *  :@typedef[opt] typename ==disable==<R>::type; // value == false のとき
 *   SFINAE 用に型 R を返します。
 *  :@typedef[opt] typename ==adapter==;
 *  :@typedef[opt] typename ==adapter_type==;
 *   adapter 型を提供します。
 *  :@typedef[opt] typename ==policy_type==;
 *  :@typedef[opt] typename ==char_type==;
 *  :@typedef[opt] typename ==char_reference==;
 *  :@typedef[opt] typename ==const_iterator==;
 *   adapter に関連する様々な型を提供します。
 */

template<typename T, typename XCH, bool = adapter_traits<T, XCH>::available>
struct _as_str: adapter_traits<T, XCH>, stdm::false_type {
  template<typename R = void> struct disable: mwg::identity<R> {};
};
template<typename T, typename XCH>
struct _as_str<T, XCH, true>: adapter_traits<T, XCH>, stdm::true_type {
  template<typename R = void> struct enable: mwg::identity<R> {};

  typedef typename adapter_traits<T, XCH>::adapter_type adapter;

  typedef typename stdm::remove_cv<
    typename stdm::remove_reference<adapter>::type>::type::policy_type policy_type;

  typedef typename policy_type::const_iterator const_iterator;
  typedef typename policy_type::char_reference char_reference;
};

template<typename T, typename XCH>
struct as_str: _as_str<typename stdm::remove_cv<T>::type, XCH> {};

//-----------------------------------------------------------------------------
// adapter_traits<S> default specializations

/*?lwiki
 * :@class struct ==adapter_traits_array==<XCH>;
 *  `adapter_traits` 実装の補助クラスです。\
 *  指定した型から `XCH*` ポインタと長さのペアを取得して、\
 *  それを元にして mwg/str の文字列型を作成します。
 *  :@tparam[in] typename XCH;
 *   文字要素の型を指定します。
 *  派生クラスで以下のメンバ関数を定義する必要があります。
 *  -@fn static const char_type* pointer(C const& s);
 *  -@fn static std::size_t length(C const& s);
 */
template<typename XCH>
struct adapter_traits_array {
  static const bool available = true;
  typedef XCH char_type;
  typedef _stradp_array<XCH> adapter_type;
};

template<typename XCH, std::size_t N>
struct adapter_traits<XCH[N]>: adapter_traits_array<XCH> {
  static const XCH* pointer(const XCH (&value)[N]) {
    return value;
  }
  static std::size_t length(const XCH (&value)[N]) {
    return char_traits<XCH>::strlen(value);
    //return value[N - 1] != char_traits<XCH>::null()? N: N - 1;
  }
};

#ifdef _MSC_VER
/*
 * 何故か vc では const char[N] を
 * template<typename T> void f(T const&); で受け取ると、
 * T = char[N] ではなくて T = const char[N] になる。
 */
template<typename XCH, std::size_t N>
struct adapter_traits<const XCH[N]>: adapter_traits_array<XCH> {
  static const XCH* pointer(const XCH (&value)[N]) {
    return value;
  }
  static std::size_t length(const XCH (&value)[N]) {
    return char_traits<XCH>::strlen(value);
    //return value[N - 1] != char_traits<XCH>::null()? N: N - 1;
  }
};
#endif

template<typename XCH>
struct adapter_traits<XCH*>: adapter_traits_array<XCH> {
  static const XCH* pointer(XCH* value) {
    return value;
  }
  static std::size_t length(XCH* value) {
    return char_traits<XCH>::strlen(value);
  }
};

template<typename XCH>
struct adapter_traits<const XCH*>: adapter_traits_array<XCH> {
  static const XCH* pointer(const XCH* value) {
    return value;
  }
  static std::size_t length(const XCH* value) {
    return char_traits<XCH>::strlen(value);
  }
};

//-----------------------------------------------------------------------------
// predicates

template<typename XCH>
struct isspace_predicator {
  typedef XCH char_type;
  bool operator()(char_type c) const {
    return char_traits<XCH>::isspace(c);
  }
};
template<typename XCH, typename Str1>
struct _pred_any_of_str {
  typedef XCH char_type;
  typename stdx::add_const_reference<Str1>::type s;
  _pred_any_of_str(typename stdx::add_const_reference<Str1>::type s)
    :s(s) {}

  bool operator()(char_type c) const {
    typename stdm::remove_reference<Str1>::type::const_iterator i = s.begin(), iN = s.end();
    for (; i != iN; ++i)
      if (c==*i) return true;
    return false;
  }
};
template<typename XCH, typename Str1>
struct _pred_not_of_str {
  typedef XCH char_type;
  typename stdx::add_const_reference<Str1>::type s;
  _pred_not_of_str(typename stdx::add_const_reference<Str1>::type s)
    :s(s) {}

  bool operator()(char_type c) const {
    typename stdm::remove_reference<Str1>::type::const_iterator i = s.begin(), iN = s.end();
    for (; i != iN; ++i)
      if (c==*i) return false;
    return true;
  }
};

//-----------------------------------------------------------------------------
// default_const_iterator
// pointer_const_iterator
// indexible_const_iterator

template<typename Policy>
class default_const_iterator: public const_iterator_base<Policy> {
  typedef const_iterator_base<Policy>  base;
  typedef default_const_iterator        this_type;
  typedef typename Policy::buffer_type  buffer_type;

  buffer_type const* m_pbuff;
  std::ptrdiff_t     m_index;
public:
  std::ptrdiff_t index() const {return this->m_index;}

public:
  default_const_iterator(buffer_type const& data, std::ptrdiff_t index)
    :m_pbuff(&data), m_index(index) {}
  default_const_iterator(): m_pbuff(nullptr), m_index(0) {}

  typedef typename base::reference       reference;
  typedef typename base::pointer         pointer;
  typedef typename base::difference_type difference_type;

  reference operator* () const {return (*m_pbuff)[m_index];}
  pointer   operator->() const {return pointer(this->operator*());}

  this_type& operator++()    {++this->m_index; return *this;}
  this_type& operator--()    {--this->m_index; return *this;}
  this_type  operator++(int) {this_type ret(*this); ++this->m_index; return ret;}
  this_type  operator--(int) {this_type ret(*this); --this->m_index; return ret;}

  bool operator==(this_type const& rhs) const {return this->m_index == rhs.m_index;}
  bool operator!=(this_type const& rhs) const {return this->m_index != rhs.m_index;}

  difference_type operator- (this_type const& rhs) const {return this->m_index - rhs.m_index;}

  // RandomAccessIterator
  reference  operator[](difference_type offset) const {return (*m_pbuff)[m_index + offset];}
  bool       operator< (this_type const& rhs) const {return this->m_index < rhs.m_index;}
  bool       operator<=(this_type const& rhs) const {return this->m_index <= rhs.m_index;}
  bool       operator> (this_type const& rhs) const {return this->m_index > rhs.m_index;}
  bool       operator>=(this_type const& rhs) const {return this->m_index >= rhs.m_index;}
  this_type  operator+ (difference_type offset) const {return this_type(*this->m_pbuff, this->m_index + offset);}
  this_type  operator- (difference_type offset) const {return this_type(*this->m_pbuff, this->m_index - offset);}
  this_type& operator+=(difference_type offset) {this->m_index += offset; return *this;}
  this_type& operator-=(difference_type offset) {this->m_index -= offset; return *this;}
  friend inline this_type operator+(difference_type offset, this_type const& iter) {return iter + offset;}
};

template<typename Type>
class pointer_const_iterator: public const_iterator_base<Type*> {
  typedef const_iterator_base<Type*> base;
  typedef pointer_const_iterator this_type;
  Type const* data;
public:
  mwg_constexpr pointer_const_iterator(Type const* pointer): data(pointer) {}
  mwg_constexpr pointer_const_iterator(): data(nullptr) {}

  typedef typename base::reference       reference;
  typedef typename base::pointer         pointer;
  typedef typename base::difference_type difference_type;

  reference operator* () const {return *data;}
  pointer   operator->() const {return pointer(this->operator*());}

  this_type& operator++()    {++this->data; return *this;}
  this_type  operator++(int) {return this_type(this->data++);}
  this_type& operator--()    {--this->data; return *this;}
  this_type  operator--(int) {return this_type(this->data--);}

  mwg_constexpr difference_type operator- (this_type const& rhs) const {return this->data - rhs.data;}
  mwg_constexpr bool            operator==(this_type const& rhs) const {return this->data == rhs.data;}
  mwg_constexpr bool            operator!=(this_type const& rhs) const {return this->data != rhs.data;}

  // RandomAccessIterator
  mwg_constexpr reference  operator[](difference_type offset) const {return data[offset];}
  mwg_constexpr bool       operator< (this_type const& rhs) const {return this->data <  rhs.data;}
  mwg_constexpr bool       operator<=(this_type const& rhs) const {return this->data <= rhs.data;}
  mwg_constexpr bool       operator> (this_type const& rhs) const {return this->data >  rhs.data;}
  mwg_constexpr bool       operator>=(this_type const& rhs) const {return this->data >= rhs.data;}
  mwg_constexpr this_type  operator+ (difference_type offset) const {return this_type(this->data + offset);}
  mwg_constexpr this_type  operator- (difference_type offset) const {return this_type(this->data - offset);}
  this_type&               operator+=(difference_type offset) {this->data += offset; return *this;}
  this_type&               operator-=(difference_type offset) {this->data -= offset; return *this;}
  friend inline mwg_constexpr this_type operator+(difference_type offset, this_type const& iter) {return iter + offset;}
};

template<typename Iter>
struct wrap_iterator
  : stdm::conditional<
      stdm::is_pointer<Iter>::value,
      pointer_const_iterator<typename stdm::remove_cv<typename stdm::remove_pointer<Iter>::type>::type>,
      Iter
  > {};

template<typename Iter, bool IterHasIndex = has_memfn_index<Iter>::value>
class indexible_const_iterator: public wrap_iterator<Iter>::type {
  typedef typename wrap_iterator<Iter>::type base;
  typedef indexible_const_iterator           this_type;

  std::ptrdiff_t m_index;
public:
  std::ptrdiff_t index() const {return this->m_index;}

public:
  indexible_const_iterator(base const& iter, std::size_t index)
    :base(iter), m_index(index) {}
  indexible_const_iterator(): m_index(0) {}

  typedef typename base::difference_type difference_type;

  this_type& operator++()    {this->base::operator++(); ++this->m_index; return *this;}
  this_type& operator--()    {this->base::operator--(); --this->m_index; return *this;}
  this_type  operator++(int) {this_type ret(*this); this->base::operator++(); ++this->m_index; return ret;}
  this_type  operator--(int) {this_type ret(*this); this->base::operator--(); --this->m_index; return ret;}

  // base::operator-() よりもこちらの方が効率的の筈。
  difference_type operator- (this_type const& rhs) const {return this->m_index -  rhs.m_index;}
  bool            operator==(this_type const& rhs) const {return this->m_index == rhs.m_index;}
  bool            operator!=(this_type const& rhs) const {return this->m_index != rhs.m_index;}

  // RandomAccessIterator (上書き)
  bool       operator< (this_type const& rhs) const {return this->m_index <  rhs.m_index;}
  bool       operator<=(this_type const& rhs) const {return this->m_index <= rhs.m_index;}
  bool       operator> (this_type const& rhs) const {return this->m_index >  rhs.m_index;}
  bool       operator>=(this_type const& rhs) const {return this->m_index >= rhs.m_index;}
  this_type& operator+=(difference_type offset) {this->base::operator+=(offset); this->m_index += offset; return *this;}
  this_type& operator-=(difference_type offset) {this->base::operator-=(offset); this->m_index -= offset; return *this;}
  this_type  operator+ (difference_type offset) const {return this_type(this->base::operator+(offset), this->m_index + offset);}
  this_type  operator- (difference_type offset) const {return this_type(this->base::operator-(offset), this->m_index - offset);}
  friend inline this_type operator+(difference_type offset, this_type const& iter) {return iter + offset;}
};

template<typename Iter>
class indexible_const_iterator<Iter, true>: public wrap_iterator<Iter>::type {
  typedef typename wrap_iterator<Iter>::type base;
  typedef indexible_const_iterator           this_type;

public:
  indexible_const_iterator(base const& copye, std::ptrdiff_t): base(copye) {}
  indexible_const_iterator(base const& copye): base(copye) {}
  indexible_const_iterator() {}

  typedef typename base::difference_type difference_type;

  this_type& operator++()    {this->base::operator++(); return *this;}
  this_type& operator--()    {this->base::operator--(); return *this;}
  this_type  operator++(int) {this_type ret(*this); this->base::operator++(); return ret;}
  this_type  operator--(int) {this_type ret(*this); this->base::operator--(); return ret;}

  // RandomAccessIterator (上書き)
  this_type& operator+=(difference_type offset) {this->base::operator+=(offset); return *this;}
  this_type& operator-=(difference_type offset) {this->base::operator-=(offset); return *this;}
  this_type  operator+ (difference_type offset) const {return this_type(this->base::operator+(offset));}
  this_type  operator- (difference_type offset) const {return this_type(this->base::operator-(offset));}
  using base::operator-;
  friend inline this_type operator+(difference_type offset, this_type const& iter) {return iter + offset;}
};

//-----------------------------------------------------------------------------
// strbase

#if mwg_has_feature(cxx_initializer_lists)
// C++11 では auto tmp = tmpobj; が可能になるのでコピーコンストラクタを封じる。
// 同時にインスタンスの作成には list-initialization を利用してコピーコンストラクタ呼び出しを回避する。
# define MWG_STR_H_hidden_copy_constructors_of_temporaries
#endif

template<>
struct strbase_tag<> {};
template<typename XCH>
struct strbase_tag<XCH>: strbase_tag<> {};

template<typename Policy>
class strbase: public strbase_tag<typename Policy::char_type> {
public:
  typedef Policy                               policy_type;
  typedef typename policy_type::char_type      char_type;
  typedef char_traits<char_type>               char_traits_type;
  typedef typename policy_type::buffer_type    buffer_type;

  typedef typename policy_type::const_iterator const_iterator;

protected:
  buffer_type data;

public:
  strbase() {}
#if mwg_has_feature(cxx_variadic_templates)
  template<typename... As>
  strbase(As mwg_forward_rvalue... args)
    :data(mwg::stdm::forward<As>(args)...) {}
#else
#pragma%[AN=5]
#pragma%x (
  template<$".for/@/0/An/typename A@/,">
  strbase($".for/@/0/An/A@ mwg_forward_rvalue arg@/,")
    :data($".for/@/0/An/mwg::stdm::forward<A@>(arg@)/,") {}
#pragma%).f/An/1/AN+1/.i
#endif

#ifdef MWG_STR_H_hidden_copy_constructors_of_temporaries
protected:
  template<typename P2>
  friend class strbase;

  // copy/move constructor is protected
  strbase(strbase const& source)
# if mwg_has_feature(cxx_defaulted_functions)
    = default;
# else
    : data(source.data) {}
# endif
# if mwg_has_feature(cxx_rvalue_references)
  strbase(strbase&& source)
#  if mwg_has_feature(cxx_defaulted_functions)
    = default;
#  else
    : data(stdm::move(source.data)) {}
#  endif
# endif
#endif

#pragma%m mwg_str::strbase::doc
  /*?lwiki
   * :@op s==[==index==]==;
   *  指定した位置にある文字を返します。
   * :@fn s.==length==();
   *  文字列中の文字数を取得します。
   * :@fn s.==empty==();
   *  空文字列かどうかを取得します。
   *  c.f. `empty` (C++), <?rb empty??> (Ruby)
   * :@fn s.==begin==();
   * :@fn s.==end==();
   *  文字データの先頭と末端を指すイテレータを返します。
   * :@fn s.==front==();
   *  最初の文字を取得します。
   *  c.f. `front` (C++11), <?pl chr?>/<?pl ord?> (Perl), <?rb chr?>/<?rb ord?> (Ruby)
   * :@fn s.==back==();
   *  最後の文字を取得します。
   *  c.f. `back` (C++11)
   * :@fn s.==fix==();
   * :@op strfix strbase::==operator->==() const;
   *  メソッドチェーンの途中で文字列を実体化したい場合に使用します。\
   *  例えば `const char*` を受け取る関数に文字列を渡したい場合は以下のようにします。
   *  &pre(!cpp){
   * std::FILE* file = std::fopen(mwg::str("HELLO.TXT").tolower().fix().c_str(), "r");
   * std::FILE* file = std::fopen(mwg::str("HELLO.TXT").tolower()->c_str(), "r");
   * }
   *  但し `fix` 及び `->` を用いて得られるインスタンスの寿命は、\
   *  その呼び出しを含む完全式である事に注意して下さい。
   *  &pre(!cpp){
   * const char* filename = mwg::str("HELLO.TXT").tolower()->c_str();
   * // filename は初期化後に dangling pointer になります。
   * // 以下のようにした場合と同様の問題です。
   * const char* filename = (std::string("hello") + ".txt").c_str();
   * }
   *
   */
#pragma%end
public:
  typedef typename policy_type::char_reference char_reference;
  char_reference operator[](std::size_t index) const {
    return data[index];
  }

  std::size_t length() const {
    return data.length();
  }
  bool empty() const {
    return data.length() == 0;
  }

  char_type front() const {
    std::size_t const _len = this->length();
    return _len == 0? char_traits_type::null(): data[0];
  }
  char_type back() const {
    std::size_t const _len = this->length();
    return _len == 0? char_traits_type::null(): data[_len - 1];
  }

  const_iterator begin() const {return data.begin();}
  const_iterator end()   const {return data.end();}
private:
  const_iterator _beginAt(std::ptrdiff_t offset) const {return data.begin_at(offset);}

public:
  strfix<char_type> fix() const {return *this;}
  strfix<char_type> operator->() const {return *this;}

#pragma%x begin_test
#if MWGCONF_CLANG_VER
# pragma clang diagnostic ignored "-Wformat-security"
#endif
  void test() {
    char buff[100];
    std::sprintf(buff, (mwg::str("file") + ": " + "message")->c_str());
    mwg_check(mwg::str(buff) == "file: message", "buff=%s", buff);
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg_str::slice
  //
  //---------------------------------------------------------------------------
#pragma%m mwg_str::slice::doc
  /*?lwiki
   * :@fn s1.==slice==('''range-spec''');
   * :@fn s1.==substr==(i, len);
   *  文字列の指定した範囲を部分文字列として返します。
   *  c.f. `substr` (C++), `Mid` (ATL/MFC), <?cs Substring?> (CLR), `Slice`/`Substr` (mwg-string), \
   *  <?java subSequence?>/<?java substring?> (Java), <?js slice?>/<?js substr?>/<?js substring?> (JavaScript), \
   *  <?awk substr?> (awk, Perl), <?rb slice?> (Ruby)
   * :@fn s1.==head==(n);
   *  文字列の先頭 n 文字を部分文字列として取得します。
   *  c.f. `find_head` (Boost), `Left` (ATL/MFC), `Head` (mwg-string)
   * :@fn s1.==tail==(n);
   *  文字列の末尾 n 文字を部分文字列として取得します。
   *  c.f. `find_tail` (Boost), `Right` (ATL/MFC), `Tail` (mwg-string)
   * :@fn s.==remove==('''range-spec''');
   *  文字列の指定した範囲を取り除いて得られる文字列を返します。
   *  c.f. `erase` (C++), `Delete` (ATL/MFC), `Remove` (CLR)
   */
#pragma%end
public:
  typedef typename mwg::stdm::conditional<
    policy_type::has_get_ptr, strsub<char_type>, strbase<_strtmp_sub_policy<policy_type> > >::type slice_return_type;
  slice_return_type slice(std::ptrdiff_t start, std::ptrdiff_t end = mwg::npos) const {
    std::size_t const len = this->length();
    std::size_t const _start = canonicalize_index(start, len);
    std::size_t _end = canonicalize_index(end, len);
    if (_start > _end) _end = _start;
    return slice_return_type(this->data, _start, _end - _start);
  }
  slice_return_type slice(mwg::irange const& r) const {
    return slice(r.begin(), r.end());
  }
  slice_return_type substr(std::ptrdiff_t start, std::size_t length) const {
    std::size_t const len = this->length();
    std::size_t const _start = canonicalize_index(start, len);
    std::size_t _end = canonicalize_index(start + length, len);
    if (_start > _end) _end = _start;
    return slice_return_type(this->data, _start, _end - _start);
  }
  slice_return_type head(std::size_t len) const {
    std::size_t const _len = this->length();
    if (len > _len) len = _len;
    return slice_return_type(this->data, 0, len);
  }
  slice_return_type tail(std::size_t len) const {
    std::size_t const _len = this->length();
    if (len > _len) len = _len;
    return slice_return_type(this->data, _len - len, len);
  }
  typedef strbase<_strtmp_cat_policy<slice_return_type, slice_return_type> > remove_return_type;
  slice_return_type remove(std::ptrdiff_t start) const {
    std::size_t const _len = this->length();
    std::size_t const _start = canonicalize_index(start, _len);
    return slice_return_type(this->data, 0, _start);
  }
  remove_return_type remove(std::ptrdiff_t start, std::ptrdiff_t end) const {
    std::size_t const _len = this->length();
    std::size_t _start = canonicalize_index(start, _len);
    std::size_t _end = canonicalize_index(end, _len);
    if (_start >= _end)
      _start = _end = _len;

    return remove_return_type(
      slice_return_type(this->data, 0, _start),
      slice_return_type(this->data, _end, _len - _end)
    );
  }
  remove_return_type remove(mwg::irange const& r) const {
    return this->remove(r.begin(), r.end());
  }
#pragma%x begin_test
  void test() {
    mwg_check((_a("hello").slice(2, 4)  == "ll"));
    mwg_check((_a("hello").slice(1, -2) == "el"));
    mwg_check((_a("hello").slice(3)    == "lo"));

    // slice of strtmp with `!has_get_ptr`
    mwg_check((_a("hello").toupper().slice(2, 4)  == "LL"));
    mwg_check((_a("hello").toupper().slice(1, -2) == "EL"));
    mwg_check((_a("hello").toupper().slice(3)    == "LO"));

    mwg_check((_a("0123456789").slice(-6, -3) == "456"));
    mwg_check((_a("0123456789").slice(-3) == "789"));
    mwg_check((_a("0123456789").slice(6, 4) == ""));
    mwg_check((_a("0123456789").slice(6, -6) == ""));

    mwg_check((_a("hello").remove(3) == "hel"));
    mwg_check((_a("hello").remove(1, -2) == "hlo"));
    mwg_check((_a("hello").remove(mwg::make_range(-4, -1)) == "ho"));

    check_iterator(_a("hello").slice(2, 4)); // strsub_policy::const_iterator
    check_iterator((_a("hel") + "lo").slice(2, 4)); // _strtmp_sub_policy::const_iterator
    check_iterator(_a("hello").remove(1, -2)); // _strtmp_cat_policy<2>::const_iterator
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg_str::insert
  //
  //---------------------------------------------------------------------------
#pragma%m mwg_str::insert::doc
  /*?lwiki
   * :@fn s.==replace==('''range-spec''', s2);
   *  指定した範囲を別の文字列に置換します。
   *  c.f. `substr(s,i,len,s2)` (Perl)
   * :@fn s1.==insert==(i,s);
   *  指定した位置に文字列を挿入します。
   *  c.f. `insert` (C++), `Insert` (ATL/MFC), <?cs Insert?> (CLR), `Insert` (mwg-string), <?rb insert?> (Ruby)
   */
#pragma%end
private:

  template<
    typename A,
    bool = as_str<A, char_type>::value
  > struct enable_insert {};

  template<typename A>
  struct enable_insert<A, true>: mwg::identity<
    strbase<_strtmp_cat_policy<
      slice_return_type,
      typename as_str<A, char_type>::adapter,
      slice_return_type>
    >
  > {};

  template<typename T>
  typename enable_insert<T>::type
  _replace_impl(std::size_t _start, std::size_t _end, T const& s) const {
    std::size_t const _len = this->length();
    return typename enable_insert<T>::type(
      slice_return_type(this->data, 0, _start),
      s,
      slice_return_type(this->data, _end, _len - _end)
    );
  }

public:
  template<typename T>
  typename enable_insert<T>::type
  replace(std::ptrdiff_t start, std::ptrdiff_t end, T const& s) const {
    std::size_t const _len = this->length();
    std::size_t _start = canonicalize_index(start, _len);
    std::size_t _end = canonicalize_index(end, _len);
    if (_end < _start) std::swap(_start, _end);
    return _replace_impl<T>(_start, _end, s);
  }
  template<typename T>
  typename enable_insert<T>::type
  replace(mwg::irange const& r, T const& s) const {
    return this->replace(r.begin(), r.end(), s);
  }
  template<typename T>
  typename enable_insert<T>::type
  insert(std::ptrdiff_t index, T const& s) const {
    std::size_t const _len = this->length();
    std::size_t const _index = canonicalize_index(index, _len);
    return _replace_impl<T>(_index, _index, s);
  }

#pragma%x begin_test
  void test() {
    mwg_check((_a("hello").replace(1, -3, "icon") == "hiconllo"));
    mwg_check((_a("hello").replace(1, -3, _a("icon")) == "hiconllo"));
    mwg_check((_a("hello").replace(mwg::make_range(1, -3), "icon") == "hiconllo"));
    mwg_check((_a("hello").insert(1, "icon") == "hiconello"));
    mwg_check((_a("hello").insert(1, _a("icon")) == "hiconello"));
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg_str::map
  //   characterwise operations
  //   tolower, toupper
  //
  //---------------------------------------------------------------------------
#pragma%m mwg_str::map::doc
  /*?lwiki
   * :@fn s.==tolower==(&color(red){[}'''range-spec'''&color(red){]});
   *  文字列内の英大文字を英小文字に変換します。
   *  c.f. `tolower` (C), `to_lower` (Boost), `MakeLower` (ATL/MFC), <?cs ToLower?> (CLR), <?java toLowerCase?> (Java), \
   *  `ToLower` (mwg-string), <?js toLowerCase?> (JavaScript), \
   *  <?pl lc?> (Perl), <?rb downcase?> (Ruby, CLX), <?awk tolower?> (awk), <?php strtolower?> (PHP)
   * :@fn s.==toupper==(&color(red){[}'''range-spec'''&color(red){]});
   *  文字列内の英小文字を英大文字に変換します。
   *  c.f. `toupper` (C), `to_upper` (Boost), `MakeUpper` (ATL/MFC), <?cs ToUpper?> (CLR, mwg-string), \
   *  <?java toUpperCase?> (Java, JavaScript), <?pl uc?> (Perl), \
   *  <?rb upcase?> (Ruby, CLX), <?awk toupper?> (awk), <?php strtoupper?> (PHP)
   * :@fn s.==map==(filter, &color(red){[}'''range-spec'''&color(red){]});
   *  c.f. `std::transform` (C++), <?cs System.Array.ConvertAll?> (CLR), `Map` (mwg-string)
   * :@fn s.==replace==(c1, c2, &color(red){[}'''range-spec'''&color(red){]});
   *  指定した範囲の文字を全て置換します。
   *  c.f. `Replace(c1,c2)` (ATL/MFC)
   * :参考
   *  c.f. <?cs ToLowerInvariant?>/<?cs ToUpperInvariant?> (CLR), \
   *  <?pl ucfirst?>/<?pl lcfirst?> (Perl), \
   *  <?rb capitalize?>/<?rb swapcase?> (Ruby, CLX), <?rb tr?> (Ruby), \
   *  `upcase_if`/`downcase_if`/`swapcase_if`/`capitalize_if` (CLX), \
   *  <?php lcfirst?>/<?php ucfirst?>/<?php ucwords?> (PHP)
   */
#pragma%end
public:
  typedef strbase<_strtmp_map_policy<policy_type, _filt_tolower<char_type> > >        tolower_return_type;
  typedef strbase<_strtmp_ranged_map_policy<policy_type, _filt_tolower<char_type> > > ranged_tolower_return_type;
  typedef strbase<_strtmp_map_policy<policy_type, _filt_toupper<char_type> > >        toupper_return_type;
  typedef strbase<_strtmp_ranged_map_policy<policy_type, _filt_toupper<char_type> > > ranged_toupper_return_type;
  tolower_return_type tolower() const {
    return tolower_return_type(this->data, _filt_tolower<char_type>());
  }
  ranged_tolower_return_type tolower(std::ptrdiff_t start, std::ptrdiff_t end = mwg::npos) const {
    std::size_t const _len = this->length();
    std::size_t const _start = canonicalize_index(start, _len);
    std::size_t _end = canonicalize_index(end, _len);
    if (_end < _start) _end = _start;
    return ranged_tolower_return_type(this->data, _filt_tolower<char_type>(), _start, _end);
  }
  ranged_tolower_return_type tolower(mwg::irange const& r) const {
    return tolower(r.begin(), r.end());
  }
  toupper_return_type toupper() const {
    return toupper_return_type(this->data, _filt_toupper<char_type>());
  }
  ranged_toupper_return_type toupper(std::ptrdiff_t start, std::ptrdiff_t end = mwg::npos) const {
    std::size_t const _len = this->length();
    std::size_t const _start = canonicalize_index(start, _len);
    std::size_t _end = canonicalize_index(end, _len);
    if (_end < _start) _end = _start;
    return ranged_toupper_return_type(this->data, _filt_toupper<char_type>(), _start, _end);
  }
  ranged_toupper_return_type toupper(mwg::irange const& r) const {
    return toupper(r.begin(), r.end());
  }
private:
  template<typename Filter>
  struct enable_map
    :mwg::stdm::enable_if<mwg::be_functor<Filter, char_type(char_type)>::value,
                          strbase<_strtmp_map_policy<policy_type, Filter const&> > > {};
  template<typename Filter>
  struct enable_ranged_map
    :mwg::stdm::enable_if<mwg::be_functor<Filter, char_type(char_type)>::value,
                          strbase<_strtmp_ranged_map_policy<policy_type, Filter const&> > > {};
public:
  template<typename F>
  typename enable_map<F>::type map(F const& filter) const {
    return typename enable_map<F>::type(this->data, filter);
  }
  template<typename F>
  typename enable_ranged_map<F>::type map(F const& filter, std::ptrdiff_t start, std::ptrdiff_t end = mwg::npos) const {
    std::size_t const _len = this->length();
    std::size_t const _start = canonicalize_index(start, _len);
    std::size_t _end = canonicalize_index(end, _len);
    if (_end < _start) _end = _start;
    return typename enable_ranged_map<F>::type(this->data, filter, _start, _end);
  }
  template<typename F>
  typename enable_ranged_map<F>::type map(F const& filter, mwg::irange const& r) const {
    return map(filter, r.begin(), r.end());
  }

private:
  typedef strbase<_strtmp_map_policy<policy_type, _filt_replace_char<char_type> > >        char_replace_return_type;
  typedef strbase<_strtmp_ranged_map_policy<policy_type, _filt_replace_char<char_type> > > ranged_char_replace_return_type;
public:
  char_replace_return_type replace(char_type const& before, char_type const& after) const {
    return char_replace_return_type(this->data, _filt_replace_char<char_type>(before, after));
  }
  ranged_char_replace_return_type replace(char_type const& before, char_type const& after, std::ptrdiff_t start, std::ptrdiff_t end = mwg::npos) const {
    std::size_t const _len = this->length();
    std::size_t const _start = canonicalize_index(start, _len);
    std::size_t _end = canonicalize_index(end, _len);
    if (_end < _start) _end = _start;
    return ranged_char_replace_return_type(this->data, _filt_replace_char<char_type>(before, after), _start, _end);
  }
  ranged_char_replace_return_type replace(char_type const& before, char_type const& after, mwg::irange const& r) const {
    return this->replace(before, after, r.begin(), r.end());
  }

#pragma%x begin_test
  void test() {
    mwg_check( (_a("hello").toupper() == "HELLO"));
    mwg_check( (_a("hello").toupper(2) == "heLLO"));
    mwg_check( (_a("hello").toupper(1, -1) == "hELLo"));
    mwg_check( (_a("hello").toupper(mwg::make_range(1, -2)) == "hELlo"));
    mwg_check( (_a("HELLO").tolower() == "hello"));
    mwg_check( (_a("HELLO").tolower(2) == "HEllo"));
    mwg_check( (_a("HELLO").tolower(1, -1) == "HellO"));
    mwg_check( (_a("HELLO").tolower(mwg::make_range(1, -2)) == "HelLO"));
    mwg_check( (_a("hello").replace('l', 'c') == "hecco"));
    mwg_check( (_a("hello").replace('l', 'p').replace('e', 'i') == "hippo"));
    mwg_check( (_a("hello").replace('l', 'p', -2) == "helpo"));
    mwg_check( (_a("hello").replace('l', 'r', 0, 3) == "herlo"));

    check_iterator(_a("hello").toupper()); // _strtmp_map_policy<>::const_iterator
    check_iterator(_a("hello").toupper(1, -1)); // _strtmp_map_policy<>::const_iterator
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg_str::trim
  //
  //---------------------------------------------------------------------------
#pragma%m mwg_str::trim::doc
  /*?lwiki
   * :@fn s1.==trim==();
   * :@fn s1.==trim==(s2);   // s2   削除文字集合
   * :@fn s1.==trim==(pred); // pred 削除文字を判定する関数
   *  文字列の両端にある連続する空白を除去します。
   *  c.f. `trim` (Boost), `Trim` (ATL/MFC, CLR, mwg-string), `TrimAny` (mwg-string), <?java trim?> (Java, JavaScript), \
   *  <?rb strip?> (Ruby, CLX), `strip_if` (CLX), <?mk strip?> (Makefile)
   * :@fn s1.==ltrim==();
   * :@fn s1.==ltrim==(s2);   // s2   削除文字集合
   * :@fn s1.==ltrim==(pred); // pred 削除文字を判定する関数
   *  文字列の先頭についている連続する空白を除去します。
   *  c.f. `trim_left`/`trim_left_if` (Boost), `TrimLeft` (ATL/MFC), <?cs TrimStart?> (CLR), `TrimL`/`TrimAnyL` (mwg-string), \
   *  <?rb lstrip?> (Ruby, CLX), `lstrip_if` (CLX)
   * :@fn s1.==rtrim==();
   * :@fn s1.==rtrim==(s2);   // s2   削除文字集合
   * :@fn s1.==rtrim==(pred); // pred 削除文字を判定する関数
   *  文字列の末端についている連続する空白を除去します。
   *  c.f. `trim_right`/`trim_right_if` (Boost), `TrimRight` (ATL/MFC), <?cs TrimEnd?> (CLR), `TrimR`/`TrimAnyR` (mwg-string), \
   *  <?rb rstrip?> (Ruby, CLX), `rstrip_if` (CLX)
   */
#pragma%end
public:
  slice_return_type trim () const {return this->trim (isspace_predicator<char_type>());}
  slice_return_type ltrim() const {return this->ltrim(isspace_predicator<char_type>());}
  slice_return_type rtrim() const {return this->rtrim(isspace_predicator<char_type>());}

  template<typename Predicate>
  typename stdm::enable_if<mwg::be_functor<Predicate, bool(char_type)>::value, slice_return_type>::type
  trim(Predicate const& pred) const {
    typedef mwg::functor_traits<Predicate, bool(char_type)> _f;
    const_iterator i = this->begin(), j = this->end();
    while (i != j && _f::invoke(pred, *i)) ++i;
    while (j != i) if (!_f::invoke(pred, *--j)) {++j; break;}
    return slice_return_type(this->data, i - this->begin(), j - i);
  }
  template<typename Predicate>
  typename stdm::enable_if<mwg::be_functor<Predicate, bool(char_type)>::value, slice_return_type>::type
  ltrim(Predicate const& pred) const {
    typedef mwg::functor_traits<Predicate, bool(char_type)> _f;
    const_iterator i = this->begin(), j = this->end();
    while (i != j && _f::invoke(pred, *i)) ++i;
    return slice_return_type(this->data, i - this->begin(), j - i);
  }
  template<typename Predicate>
  typename stdm::enable_if<mwg::be_functor<Predicate, bool(char_type)>::value, slice_return_type>::type
  rtrim(Predicate const& pred) const {
    typedef mwg::functor_traits<Predicate, bool(char_type)> _f;
    const_iterator i = this->begin(), j = this->end();
    while (j != i) if (!_f::invoke(pred, *--j)) {++j; break;}
    return slice_return_type(this->data, 0, j - i);
  }

  template<typename Str>
  typename as_str<Str, char_type>::template enable<slice_return_type>::type
  trim(Str const& set) const {
    typedef typename as_str<Str, char_type>::adapter adapter;
    return this->trim(_pred_any_of_str<char_type, adapter>(set));
  }
  template<typename Str>
  typename as_str<Str, char_type>::template enable<slice_return_type>::type
  ltrim(Str const& set) const {
    typedef typename as_str<Str, char_type>::adapter adapter;
    return this->ltrim(_pred_any_of_str<char_type, adapter>(set));
  }
  template<typename Str>
  typename as_str<Str, char_type>::template enable<slice_return_type>::type
  rtrim(Str const& set) const {
    typedef typename as_str<Str, char_type>::adapter adapter;
    return this->rtrim(_pred_any_of_str<char_type, adapter>(set));
  }

#pragma%x begin_test
  void test() {
    mwg_check((_a("  hello   ").trim () == "hello"));
    mwg_check((_a("  hello   ").ltrim() == "hello   "));
    mwg_check((_a("  hello   ").rtrim() == "  hello"));
    mwg_check((_a("012343210").trim ("012") == "343"));
    mwg_check((_a("012343210").ltrim("012") == "343210"));
    mwg_check((_a("012343210").rtrim("012") == "012343"));
    mwg_check((_a("012343210").trim (_a("012")) == "343"));
    mwg_check((_a("012343210").ltrim(_a("012")) == "343210"));
    mwg_check((_a("012343210").rtrim(_a("012")) == "012343"));
#if mwg_has_feature(cxx_lambdas)
    mwg_check((_a("012343210").trim ([](char c) {return '0' <= c && c <= '2';}) == "343"));
    mwg_check((_a("012343210").ltrim([](char c) {return '0' <= c && c <= '2';}) == "343210"));
    mwg_check((_a("012343210").rtrim([](char c) {return '0' <= c && c <= '2';}) == "012343"));
#endif
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg_str::pad
  //
  //---------------------------------------------------------------------------
#pragma%m mwg_str::pad::doc
  /*?lwiki
   * :@fn s.==pad==(len);
   * :@fn s.==pad==(len, c);
   *  c.f. <?rb center?> (Ruby, CLX)
   * :@fn s.==lpad==(len);
   * :@fn s.==lpad==(len, c);
   *  c.f. <?cs PadLeft?> (CLR), <?rb ljust?> (Ruby, CLX)
   * :@fn s.==rpad==(len);
   * :@fn s.==rpad==(len, c);
   *  c.f. <?cs PadRight?> (CLR), <?rb rjust?> (Ruby, CLX)
   */
#pragma%end
public:
  typedef strbase<_strtmp_pad_policy<strbase<policy_type> > > pad_return_type;
  pad_return_type pad(std::size_t width, char_type const& ch) const {
    std::ptrdiff_t room = (std::ptrdiff_t) width - this->length();
    if (room <= 0)
      return pad_return_type(this->data, 0, this->length(), ch);
    else
      return pad_return_type(this->data, room / 2, width, ch);
  }
  pad_return_type lpad(std::size_t width, char_type const& ch) const {
    std::ptrdiff_t room = (std::ptrdiff_t) width - this->length();
    if (room <= 0)
      return pad_return_type(this->data, 0, this->length(), ch);
    else
      return pad_return_type(this->data, room, width, ch);
  }
  pad_return_type rpad(std::size_t width, char_type const& ch) const {
    std::ptrdiff_t room = (std::ptrdiff_t) width - this->length();
    if (room <= 0)
      return pad_return_type(this->data, 0, this->length(), ch);
    else
      return pad_return_type(this->data, 0, width, ch);
  }
  pad_return_type pad(std::size_t width) const {
    return this->pad(width, char_traits_type::space());
  }
  pad_return_type lpad(std::size_t width) const {
    return this->lpad(width, char_traits_type::space());
  }
  pad_return_type rpad(std::size_t width) const {
    return this->rpad(width, char_traits_type::space());
  }
#pragma%x begin_test
  void test() {
    mwg_check((_a("hello").pad(1) == "hello"));
    mwg_check((_a("hello").lpad(1) == "hello"));
    mwg_check((_a("hello").rpad(1) == "hello"));
    mwg_check((_a("hello").pad(10) == "  hello   "));
    mwg_check((_a("hello").lpad(10) == "     hello"));
    mwg_check((_a("hello").rpad(10) == "hello     "));
    mwg_check((_a("hello").pad(10, '-') == "--hello---"));
    mwg_check((_a("hello").lpad(10, '-') == "-----hello"));
    mwg_check((_a("hello").rpad(10, '-') == "hello-----"));

    // checks for const_iterator begin_at
    mwg_check((_a("hello").pad(10, '-').slice(1, 8) == "-hello-"));
    mwg_check((_a("hello").pad(10, '-').slice(2, 8) == "hello-"));
    mwg_check((_a("hello").pad(10, '-').slice(3, 8) == "ello-"));
    mwg_check((_a("hello").pad(10, '-').slice(3, 5) == "el"));
    mwg_check((_a("hello").pad(10, '-').slice(8) != "-"));
    mwg_check((_a("hello").pad(10, '-').slice(8) == "--"));
    mwg_check((_a("hello").pad(10, '-').slice(8) != "----"));

    check_iterator(_a("hello").pad(10, '-')); // _strtmp_pad_policy<>::const_iterator
    check_iterator(_a("hello").pad(10, '-').slice(1, 8));
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg_str::starts
  //
  //---------------------------------------------------------------------------
#pragma%m mwg_str::starts::doc
  /*?lwiki
   * :@fn s.==starts==(s1); // s1 文字列
   *  文字列が指定された文字列で始まっているかを判定します。
   *  c.f. `starts_with` (Boost), <?cs StartsWith?> (CLR, mwg-string), <?java startsWith?> (Java), <?rb start_with??> (Ruby)
   * :@fn s.==ends==(s1); // s1 文字列
   *  文字列が指定された文字列で終わっているかを判定します。
   *  c.f. `ends_with` (Boost), <?cs EndsWith?> (CLR, mwg-string), <?java endsWith?> (Java), <?rb end_with??> (Ruby)
   * :参考
   *  all/istarts_with/iends_with (Boost)
   */
#pragma%end
public:
  template<typename XStr>
  typename as_str<XStr, char_type>::template enable<bool>::type
  starts(XStr const& _s) const {
    typename as_str<XStr, char_type>::adapter s(_s);
    if (this->length() < s.length()) return false;
    const_iterator i = this->begin();
    typename as_str<XStr, char_type>::const_iterator j = s.begin(), jN = s.end();
    for (; j != jN; ++i, ++j)
      if (*i != *j) return false;
    return true;
  }
  template<typename XStr>
  typename as_str<XStr, char_type>::template enable<bool>::type
  ends(XStr const& _s) const {
    typename as_str<XStr, char_type>::adapter s(_s);
    std::ptrdiff_t offset = this->length() - s.length();
    if (offset < 0) return false;
    const_iterator i = this->_beginAt(offset);
    typename as_str<XStr, char_type>::const_iterator j = s.begin(), jN = s.end();
    for (; j != jN; ++i, ++j)
      if (*i != *j) return false;
    return true;
  }
#pragma%x begin_test
  void test() {
    mwg_check( (_a("hello world").starts("hel")));
    mwg_check( (_a("hello world").starts(_a("hel"))));
    mwg_check(!(_a("hello world").starts("hal")));
    mwg_check(!(_a("hello world").starts(_a("hal"))));
    mwg_check(!(_a("hello world").starts("hello world!")));
    mwg_check(!(_a("hello world").starts(_a("hello world!"))));
    mwg_check( (_a("hello world").ends("orld")));
    mwg_check( (_a("hello world").ends(_a("orld"))));
    mwg_check(!(_a("hello world").ends("olrd")));
    mwg_check(!(_a("hello world").ends(_a("olrd"))));
    mwg_check(!(_a("hello world").ends("+hello world")));
    mwg_check(!(_a("hello world").ends(_a("+hello world"))));
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg_str::find
  //
  //---------------------------------------------------------------------------
#pragma%m mwg_str::find::doc
  /*?lwiki
   * :@fn s.==find==(s&color(red){|}ch&color(red){|}pred&color(red){|}reg, &color(red){[}'''range-spec'''&color(red){]});
   * :@fn s.==rfind==(s&color(red){|}ch&color(red){|}pred&color(red){|}reg, &color(red){[}'''range-spec'''&color(red){]});
   *  指定した文字列を検索して最初に見付かった開始位置を返します。
   *  `find` は範囲先頭から検索を開始し、`rfind` は範囲末端から検索を開始します。
   *  第一引数に検索対象を指定します。第二引数に検索範囲を指定します。検索範囲の指定を省略した場合、文字列全体が検索範囲になります。
   *  :@param[in] String s;
   *   検索対象の文字列を指定します
   *  :@param[in] XCH ch;
   *   検索対象の文字を指定します
   *  :@param[in] bool pred(XCH);
   *   文字判定関数を指定します。条件に合致する文字を検索します。
   *  :@param[in] //TODO: '''regex''' reg;
   *   検索パターンを正規表現で指定します
   *  -`find` -> c.f. `strstr`/`strchr` (C), `find` (C++), `find_first`/`find_regex` (Boost), \
   *   `Find` (ATL/MFC), <?cs IndexOf?> (CLR, mwg-string), <?java indexOf?> (Java),           \
   *   <?js indexOf?>/<?js search?> (JavaScript), <?awk index?> (awk, Perl, Ruby), <?awk match?> (awk)
   *  -`rfind` -> c.f. `strrstr` (C), `rfind` (C++), `find_last` (Boost), \
   *   `ReverseFind` (ATL/MFC), <?cs LastIndexOf?> (CLR), `IndexOfR` (mwg-string), \
   *   <?java lastIndexOf?> (Java, JavaScript), <?pl rindex?> (Perl, Ruby)
   * :@fn s.==find_any==(s2, &color(red){[}'''range-spec'''&color(red){]});
   * :@fn s.==rfind_any==(s2, &color(red){[}'''range-spec'''&color(red){]});
   *  文字集合の何れかの文字の位置を返します。
   *  :@param[in] s2
   *   文字集合を指定します。
   *  `find_any` -> c.f. `strpbrk`/`strcspn` (C), `find_first_of` (C++), `FindOneOf` (ATL/MFC), <?cs IndexOfAny?> (CLR, mwg-string)
   *  `rfind_any` -> c.f. `find_last_of` (C++), <?cs LastIndexOfAny?> (CLR), `IndexOfAnyR` (mwg-string)
   * :@fn s.==find_not==(s2, &color(red){[}'''range-spec'''&color(red){]});
   * :@fn s.==rfind_not==(s2, &color(red){[}'''range-spec'''&color(red){]});
   *  最初に見付かった、文字集合に含まれない文字の位置を返します。
   *  -`find_not` -> c.f. `strspnP` (C), `find_first_not_of` (C++), `IndexOfNot` (mwg-string)
   *  -`rfind_not` -> c.f. `find_last_not_of` (C++), `IndexOfNotR` (mwg-string)
   * :@op s1.find(...) >= 0;
   *  文字列が他方の文字列に含まれているかどうかを判定する時。
   *  c.f. `contains` (Boost), <?cs Contains?> (CLR), <?java contains?>/<?java matches?> (Java), <?rb include??> (Ruby), <?mk findstring?> (Makefile).
   * :参考
   *  c.f. Boost find_nth/ifind_first/ifind_last/ifind_nth
   */
#pragma%end
private:
  template<bool A1, bool A2, bool A3 = false>
  struct _xor: stdm::bool_constant<((A1? 1: 0) + (A2? 1: 0) + (A3? 1: 0) == 1)> {};

  template<typename T, bool HasChar, bool HasPredicate, bool HasString>
  struct enable_find: stdm::enable_if<
    _xor<
      HasChar && stdm::is_same<T, char_type>::value,
      HasPredicate && mwg::be_functor<T, bool(char_type)>::value,
      HasString && as_str<T, char_type>::value
    >::value,
    std::ptrdiff_t
  > {};

private:
  std::ptrdiff_t _find_impl(char_type const& ch, std::ptrdiff_t i, std::ptrdiff_t j) const {
    const_iterator p = this->_beginAt(i);
    for (; i < j; ++i)
      if (*p++ == ch) return i;
    return -1;
  }
  std::ptrdiff_t _rfind_impl(char_type const& ch, std::ptrdiff_t i, std::ptrdiff_t j) const {
    const_iterator p = this->_beginAt(j);
    for (; --j >= i; )
      if (*--p == ch) return j;
    return -1;
  }

private:
  template<typename Predicate>
  typename enable_find<Predicate, false, true, false>::type
  _find_impl(Predicate const& pred, std::ptrdiff_t i, std::ptrdiff_t j) const {
    typedef mwg::functor_traits<Predicate, bool(char_type)> _f;
    const_iterator p = this->_beginAt(i);
    for (; i < j; ++i)
      if (_f::invoke(pred, *p++)) return i;
    return -1;
  }
  template<typename Predicate>
  typename enable_find<Predicate, false, true, false>::type
  _rfind_impl(Predicate const& pred, std::ptrdiff_t i, std::ptrdiff_t j) const {
    typedef mwg::functor_traits<Predicate, bool(char_type)> _f;
    const_iterator p = this->_beginAt(j);
    for (; --j >= i; )
      if (_f::invoke(pred, *--p)) return j;
    return -1;
  }

private:
  template<typename YPolicy>
  bool _find_match_at(std::size_t index, strbase<YPolicy> const& s) const {
    const_iterator p = this->_beginAt(index);
    typename strbase<YPolicy>::const_iterator q = s.begin();
    for (std::size_t end = index + s.length(); index < end; index++)
      if (*p++!=*q++) return false;
    return true;
  }
  template<typename Str>
  typename enable_find<Str, false, false, true>::type
  _find_impl(Str const& _s, std::ptrdiff_t _i0, std::ptrdiff_t _iM) const {
    typename as_str<Str, char_type>::adapter s(_s);
    _iM -= s.length();
    for (std::ptrdiff_t i = _i0; i <= _iM; ++i)
      if (_find_match_at(i, s)) return i;
    return -1;
  }
  template<typename Str>
  typename enable_find<Str, false, false, true>::type
  _rfind_impl(Str const& _s, std::ptrdiff_t _i0, std::ptrdiff_t _iM) const {
    typename as_str<Str, char_type>::adapter s(_s);
    _iM -= s.length();
    for (std::ptrdiff_t i = _iM; i >= _i0; --i)
      if (_find_match_at(i, s)) return i;
    return -1;
  }
  template<typename Str>
  typename enable_find<Str, false, false, true>::type
  _find_any_impl(Str const& s, std::ptrdiff_t i, std::ptrdiff_t iN) const {
    typedef typename as_str<Str, char_type>::adapter adapter;
    return this->_find_impl(_pred_any_of_str<char_type, adapter>(s), i, iN);
  }
  template<typename Str>
  typename enable_find<Str, false, false, true>::type
  _rfind_any_impl(Str const& s, std::ptrdiff_t i, std::ptrdiff_t iN) const {
    typedef typename as_str<Str, char_type>::adapter adapter;
    return this->_rfind_impl(_pred_any_of_str<char_type, adapter>(s), i, iN);
  }
  template<typename Str>
  typename enable_find<Str, false, false, true>::type
  _find_not_impl(Str const& s, std::ptrdiff_t i, std::ptrdiff_t iN) const {
    typedef typename as_str<Str, char_type>::adapter adapter;
    return this->_find_impl(_pred_not_of_str<char_type, adapter>(s), i, iN);
  }
  template<typename Str>
  typename enable_find<Str, false, false, true>::type
  _rfind_not_impl(Str const& s, std::ptrdiff_t i, std::ptrdiff_t iN) const {
    typedef typename as_str<Str, char_type>::adapter adapter;
    return this->_rfind_impl(_pred_not_of_str<char_type, adapter>(s), i, iN);
  }

public:
#define MWG_STRING3_STRING_H_define_find_overloads(FIND, hC, hP, hS) \
  template<typename T> \
  typename enable_find<T, hC, hP, hS>::type \
  FIND(T const& pred) const { \
    return this->_##FIND##_impl(pred, 0, this->length()); \
  } \
  template<typename T> \
  typename enable_find<T, hC, hP, hS>::type \
  FIND(T const& pred, std::ptrdiff_t start, std::ptrdiff_t end = mwg::npos) const { \
    std::size_t const _len = this->length(); \
    return this->_##FIND##_impl(pred, canonicalize_index(start, _len), canonicalize_index(end, _len)); \
  } \
  template<typename T> \
  typename enable_find<T, hC, hP, hS>::type \
  FIND(T const& pred, mwg::irange const& r) const { \
    return this->FIND(pred, r.begin(), r.end()); \
  }

  MWG_STRING3_STRING_H_define_find_overloads(find     , true , true , true);
  MWG_STRING3_STRING_H_define_find_overloads(rfind    , true , true , true);
  MWG_STRING3_STRING_H_define_find_overloads(find_any , false, false, true);
  MWG_STRING3_STRING_H_define_find_overloads(rfind_any, false, false, true);
  MWG_STRING3_STRING_H_define_find_overloads(find_not , false, false, true);
  MWG_STRING3_STRING_H_define_find_overloads(rfind_not, false, false, true);
#undef MWG_STRING3_STRING_H_define_find_overloads

#pragma%x begin_test
  void test() {
    mwg_check((_a("0123401234").find("012") == 0));
    mwg_check((_a("0123401234").find("234") == 2));
    mwg_check((_a("0123401234").find("021") < 0));
    mwg_check((_a("0123401234").find("012", 1) == 5));
    mwg_check((_a("0123401234").find("012", 1, 8) == 5));
    mwg_check((_a("0123401234").find("012", 1, 7) < 0));
    mwg_check((_a("0123401234").rfind("012") == 5));
    mwg_check((_a("0123401234").rfind("234") == 7));
    mwg_check((_a("0123401234").rfind("021") < 0));
    mwg_check((_a("0123401234").rfind("012", 1) == 5));
    mwg_check((_a("0123401234").rfind("012", 1, 8) == 5));
    mwg_check((_a("0123401234").rfind("012", 1, 7) < 0));
    mwg_check((_a("0123401234").rfind("012", 6) < 0));
    mwg_check((_a("0123401234").find('2') == 2));
    mwg_check((_a("0123401234").rfind('2') == 7));
    mwg_check((_a("0123401234").find_any("012") == 0));
    mwg_check((_a("0123401234").find_any("234") == 2));
    mwg_check((_a("0123401234").find_not("012") == 3));
    mwg_check((_a("0123401234").find_not("234") == 0));
    mwg_check((_a("0123401234").rfind_any("012") == 7));
    mwg_check((_a("0123401234").rfind_any("234") == 9));
    mwg_check((_a("0123401234").rfind_not("012") == 9));
    mwg_check((_a("0123401234").rfind_not("234") == 6));
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg_str::misc
  //
  //---------------------------------------------------------------------------
#pragma%m mwg_str::misc::doc
  /*?lwiki
   * :@fn s.==reverse==();
   *  c.f. `Reverse` (mwg-string), <?rb reverse?> (Ruby), <?php strrev?> (PHP)
   * :@fn s.==repeat==(n);
   *  c.f. `Repeat` (mwg-string), <?rb operator*?> (Ruby)
   */
#pragma%end
private:
  typedef strbase<_strtmp_reverse_policy<strbase<policy_type> > > reverse_return_type;
public:
  reverse_return_type reverse() const {
    return reverse_return_type(*this);
  }
private:
  typedef strbase<_strtmp_repeat_policy<strbase<policy_type> > > repeat_return_type;
public:
  repeat_return_type repeat(std::size_t count) const {
    return repeat_return_type(*this, count);
  }
#pragma%x begin_test
  void test() {
    mwg_check( (_a("HELLO").reverse() == "OLLEH"));
    mwg_check( (_a("HELLO").repeat(3) == "HELLOHELLOHELLO"));
  }
#pragma%x end_test
};

//-----------------------------------------------------------------------------
// strsub, _stradp_array

template<typename XCH>
struct strsub_policy {
  typedef XCH              char_type;
  typedef const char_type& char_reference;
  typedef strsub_policy    policy_type;
  static const bool has_get_ptr = true;
  typedef char_type const* const_iterator;

  struct buffer_type {
    const char_type* ptr;
    std::size_t len;
  public:
    buffer_type(const char_type* ptr, std::size_t length)
      :ptr(ptr), len(length) {}
  public:
    char_reference operator[](std::size_t index) const {
      return ptr[index];
    }
    std::size_t length() const {
      return this->len;
    }
    const_iterator begin() const {
      return this->ptr;
    }
    const_iterator end() const {
      return this->ptr + this->len;
    }
    const_iterator begin_at(std::ptrdiff_t index) const {return this->ptr + index;}

    const char_type* get_ptr() const {
      return this->ptr;
    }
  };
};

template<typename XCH>
class strsub: public strbase<strsub_policy<XCH> > {
  typedef strbase<strsub_policy<XCH> > base;

public:
  typedef XCH char_type;

private:
  template<typename StrP>
  friend class strbase;

  template<typename BufferType>
  strsub(BufferType const& data, std::size_t start, std::size_t length)
    :base(data.get_ptr() + start, length) {}

public:
  strsub(const char_type* ptr, std::size_t length)
    :base(ptr, length) {}
  strsub(const char_type* beg, const char* end)
    :base(beg, end - beg) {}
};

template<typename XCH>
class _stradp_array: public strbase<strsub_policy<XCH> > {
  typedef strbase<strsub_policy<XCH> > base;

public:
  typedef typename base::char_type char_type;

public:
  _stradp_array(const char_type* ptr, std::size_t length)
    :base(ptr, length) {}

  template<typename T>
  _stradp_array(T const& value)
    :base(adapter_traits<T, char_type>::pointer(value), adapter_traits<T, char_type>::length(value)) {}
};

//-----------------------------------------------------------------------------
// strfix

template<typename XCH>
struct strfix_policy {
  typedef strfix_policy    policy_type;
  typedef XCH              char_type;
  typedef const char_type& char_reference;
  static const bool has_get_ptr = true;

  typedef char_type const* const_iterator;

  typedef char_traits<char_type> char_traits_type;

  class buffer_type {
  private:
    friend class strfix<char_type>;

    struct bucket {
      char_type* data;
      std::size_t len;
    public:
      bucket(): data(nullptr), len(-1) {
        this->allocate(0);
      }
      bucket(std::size_t len): data(nullptr), len(-1) {
        this->allocate(len);
      }
      ~bucket() {
        this->free();
      }
      void reset(std::size_t len) {
        this->allocate(len);
      }
    private:
      void free() {
        delete[] this->data;
        this->data = nullptr;
      }
      void allocate(std::size_t length) {
        /* 同じ長さでも必ず再確保する。
         * というのも、自分自身を参照しながら自分自身を更新する事があるから:
         * | s = s.reverse()
         */
        // if (this->len == length) return;

        this->free();
        this->len = length;
        this->data = new char_type[length + 1];
      }
    };
    mwg::stdm::shared_ptr<bucket> ptr;

  public:
    buffer_type(): ptr(new bucket) {
      ptr->data[0] = char_traits_type::null();
    }

  public:
    template<typename StrP>
    buffer_type(strbase<StrP> const& s)
      :ptr(new bucket(s.length()))
    {
      this->copy_content(s);
    }
    template<typename StrP>
    void reset(strbase<StrP> const& s) {
      this->ptr->reset(s.length());
      this->copy_content(s);
    }
  private:
    template<typename StrP>
    void copy_content(strbase<StrP> const& s) {
      typename StrP::const_iterator j = s.begin();
      std::size_t const iN = s.length();
      char_type* const data = ptr->data;
      for (std::size_t i = 0; i < iN; ++i, ++j)
        data[i] = *j;
      data[iN] = char_traits_type::null();
    }

  public:
    char_reference operator[](std::size_t index) const {
      return ptr->data[index];
    }
    std::size_t length() const {
      return ptr->len;
    }
    const_iterator begin() const {
      return ptr->data;
    }
    const_iterator end() const {
      return ptr->data + ptr->len;
    }
    const_iterator begin_at(std::ptrdiff_t index) const {return ptr->data + index;}

    const char_type* get_ptr() const {
      return ptr->data;
    }
  };
};

template<typename XCH>
class strfix: public strbase<strfix_policy<XCH> > {
  typedef strbase<strfix_policy<XCH> > base;

public:
  typedef typename base::char_type char_type;

public:
  strfix() {}

  strfix(strfix const& s): base(s.data) {}
  strfix& operator=(strfix const& rhs) {
    this->data = rhs.data;
    return *this;
  }
#if mwg_has_feature(cxx_rvalue_references)
  strfix(strfix&& s): base(mwg::stdm::move(s.data)) {}
  strfix& operator=(strfix&& rhs) {
    this->data = mwg::stdm::move(rhs.data);
    return *this;
  }
#endif

  template<typename YStr>
  strfix(YStr const& src, typename as_str<YStr, char_type>::template enable<mwg::invalid_type*>::type = 0)
    :base(mwg::str(src)) {}
  template<typename YStr>
  typename as_str<YStr, char_type>::template enable<strfix&>::type
  operator=(YStr const& rhs) {
    this->data.reset(mwg::str(rhs));
    return *this;
  }

#if mwg_has_feature(cxx_ref_qualifiers)
  strfix      &  fix()      &  {return *this;}
  strfix const&  fix() const&  {return *this;}
  strfix      && fix()      && {return stdm::move(*this);}
  strfix const&& fix() const&& {return stdm::move(*this);}
#else
  strfix      &  fix()        {return *this;}
  strfix const&  fix() const  {return *this;}
#endif
  strfix      * operator->()       {return this;}
  strfix const* operator->() const {return this;}

  char_type const* c_str() const {return this->data.get_ptr();}
};

#pragma%x begin_check

//-----------------------------------------------------------------------------
// _strtest_repeated_chars_policy

namespace mwg {
namespace str_detail {

  /// @class _strtest_repeated_chars_policy
  /// 同じ文字が指定した回数だけ繰り返される文字列。
  /// これはデバグ用の StringPolicy である。
  /// has_memfn_index に対する処理をテストする為に、
  /// has_memfn_index な const_iterator の例として実装された。
  template<typename XCH>
  struct _strtest_repeated_chars_policy {
    typedef _strtest_repeated_chars_policy policy_type;
    typedef XCH char_type;
    typedef XCH char_reference;
    typedef char_traits<char_type> char_traits_type;
    static const bool has_get_ptr = false;

    class const_iterator: public const_iterator_base<policy_type> {
      typedef const_iterator_base<policy_type> base;
      typedef const_iterator this_type;
      char_type value;
      std::ptrdiff_t m_index;
    public:
      std::ptrdiff_t index() const {return this->m_index;}

      mwg_constexpr const_iterator(char_type value, std::ptrdiff_t index): value(value), m_index(index) {}
      mwg_constexpr const_iterator(): value(char_traits_type::null()), m_index(0) {}

      typedef typename base::reference       reference;
      typedef typename base::pointer         pointer;
      typedef typename base::difference_type difference_type;

      mwg_constexpr reference operator*() const {return value;}
      mwg_constexpr pointer operator->() const {return pointer(this->operator*());}

      this_type& operator++() {++this->m_index; return *this;}
      this_type& operator--() {--this->m_index; return *this;}
      this_type  operator++(int) {return this_type(this->value, this->m_index++);}
      this_type  operator--(int) {return this_type(this->value, this->m_index--);}

      mwg_constexpr difference_type operator- (this_type const& rhs) const {return this->m_index -  rhs.m_index;}
      mwg_constexpr bool            operator==(this_type const& rhs) const {return this->m_index == rhs.m_index;}
      mwg_constexpr bool            operator!=(this_type const& rhs) const {return this->m_index != rhs.m_index;}
      mwg_constexpr bool            operator< (this_type const& rhs) const {return this->m_index <  rhs.m_index;}
      mwg_constexpr bool            operator<=(this_type const& rhs) const {return this->m_index <= rhs.m_index;}
      mwg_constexpr bool            operator> (this_type const& rhs) const {return this->m_index >  rhs.m_index;}
      mwg_constexpr bool            operator>=(this_type const& rhs) const {return this->m_index >= rhs.m_index;}

      mwg_constexpr reference operator[](difference_type) const {return value;}
      this_type& operator+=(difference_type offset) {this->m_index += offset; return *this;}
      this_type& operator-=(difference_type offset) {this->m_index -= offset; return *this;}
      mwg_constexpr this_type  operator+(difference_type offset) const {return this_type(this->value, this->m_index + offset);}
      mwg_constexpr this_type  operator-(difference_type offset) const {return this_type(this->value, this->m_index - offset);}
      friend inline mwg_constexpr this_type operator+(difference_type offset, this_type const& iter) {return iter + offset;}
    };

    class buffer_type {
    private:
      char_type value;
      std::size_t m_length;
    public:
      buffer_type(char_type value, std::size_t length)
        :value(value), m_length(length) {}

      char_reference operator[](std::ptrdiff_t) const {return this->value;}
      std::size_t length() const {return this->m_length;}

      const_iterator begin() const {return const_iterator(this->value, 0);}
      const_iterator end() const {return const_iterator(this->value, this->m_length);}
      const_iterator begin_at(std::ptrdiff_t offset) const {return const_iterator(this->value, offset);}
    };
  };
}
}

#pragma%x end_check
//-----------------------------------------------------------------------------
// _strtmp_sub_policy                                                  @tmp.sub

template<typename Iter, bool IterHasIndex = has_memfn_index<Iter>::value>
class index_displaced_iterator { /* not supported */ };

template<typename Iter>
class index_displaced_iterator<Iter, true>: public wrap_iterator<Iter>::type {
  typedef typename wrap_iterator<Iter>::type base;
  std::ptrdiff_t offset;

public:
  index_displaced_iterator(base const& iter, std::ptrdiff_t offset)
    :base(iter), offset(offset) {}

  index_displaced_iterator(): offset(0) {}

  std::ptrdiff_t index() const {
    return this->base::index() - offset;
  }
};

template<typename Policy>
struct _strtmp_sub_policy {
public:
  typedef _strtmp_sub_policy             policy_type;
  typedef typename Policy::char_type     char_type;
  typedef typename Policy::char_reference char_reference;
  static const bool has_get_ptr = false;

private:
  typedef typename Policy::const_iterator original_iterator;

public:
  // has_memfn_index<original_iterator>::value 分岐 (1/2)
  typedef typename stdm::conditional<
    has_memfn_index<original_iterator>::value, index_displaced_iterator<original_iterator>,
    original_iterator>::type const_iterator;

  class buffer_type {
    const typename Policy::buffer_type& buff;
    std::size_t m_start;
    std::size_t m_length;

  public:
    buffer_type(const typename Policy::buffer_type& buff, std::size_t start, std::size_t length)
      :buff(buff), m_start(start), m_length(length) {}

  public:
    char_reference operator[](std::size_t index) const {
      return buff[m_start + index];
    }
    std::size_t length() const {
      return this->m_length;
    }

  private:
    // has_memfn_index<original_iterator>::value 分岐 (2/2)
    template<typename Iter>
    typename stdm::enable_if<has_memfn_index<Iter>::value, const_iterator>::type
    static modify_iterator(Iter const& iter, std::ptrdiff_t offset) {
      return const_iterator(iter, offset);
    }
    template<typename Iter>
    typename stdm::enable_if<!has_memfn_index<Iter>::value, Iter const&>::type
    static modify_iterator(Iter const& iter, std::ptrdiff_t) {return iter;}

  public:
    const_iterator begin() const {
      return modify_iterator(buff.begin_at(m_start), m_start);
    }
    const_iterator end() const {
      return modify_iterator(buff.begin_at(m_start + m_length), m_start);
    }
    const_iterator begin_at(std::ptrdiff_t index) const {
      return modify_iterator(buff.begin_at(m_start + index), m_start);
    }
  };
};

#pragma%x begin_test
  void test() {
    using namespace mwg::str_detail;

    // !has_memfn_index な基底 const_iterator から、const_iterator を初期化
    typedef _stradp_array<char> a;
    mwg_check((strbase<_strtmp_sub_policy<a::policy_type> >(a::buffer_type("hello", 5), 1, 3) == "ell"));

    // has_memfn_index な基底 const_iterator から、const_iterator を初期化
    typedef _strtest_repeated_chars_policy<char> b;
    mwg_check((strbase<_strtmp_sub_policy<b::policy_type> >(b::buffer_type('A', 5), 1, 3) == "AAA"));

    check_iterator(strbase<_strtest_repeated_chars_policy<char> >('A', 5));
  }
#pragma%x end_test

//-----------------------------------------------------------------------------
// _strtmp_map_policy, _strtmp_ranged_map_policy                       @tmp.map

/* :@tp Filter
 *  Filter には filter を格納する形式を指定する。
 *  filter と結果文字列が同じ完全式の部分式である場合には、寿命が一致しているので参照を指定する。
 *  そうでない場合、例えば tolower/toupper など内部関数で filter が生成される場合は、
 *  参照で記録すると dangling になるので Filter の型そのものを指定する必要がある。
 */
template<typename Policy, typename Filter>
struct _strtmp_map_policy {
  typedef _strtmp_map_policy         policy_type;
  typedef typename Policy::char_type char_type;
  typedef char_type                  char_reference;
  static const bool has_get_ptr = false;

  typedef typename Policy::const_iterator original_iterator;
  typedef typename mwg::stdm::remove_reference<Filter>::type filter_type;
  class const_iterator: public wrap_iterator<original_iterator>::type {
    typedef typename wrap_iterator<original_iterator>::type base;
    filter_type const* m_filter;
  public:
    const_iterator(base const& iter, filter_type const& filter)
      :base(iter), m_filter(&filter) {}
    const_iterator(base const& iter, const_iterator const& origin)
      :base(iter), m_filter(origin.m_filter) {}
    const_iterator(): m_filter(nullptr) {}

    typedef char_reference reference;
    typedef typename base::pointer         pointer;
    typedef typename base::difference_type difference_type;

    reference operator*() const {return (*this->m_filter)(this->base::operator*());}
    pointer operator->() const {return pointer(this->operator*());}

    const_iterator& operator++()    {this->base::operator++(); return *this;}
    const_iterator& operator--()    {this->base::operator--(); return *this;}
    const_iterator  operator++(int) {return const_iterator(this->base::operator++(0), *this);}
    const_iterator  operator--(int) {return const_iterator(this->base::operator--(0), *this);}

    // RandomAccessIterator
    using base::operator-;
    const_iterator& operator+=(difference_type offset) {this->base::operator+=(offset); return *this;}
    const_iterator& operator-=(difference_type offset) {this->base::operator-=(offset); return *this;}
    const_iterator  operator+ (difference_type offset) const {return const_iterator(this->base::operator+(offset), *this);}
    const_iterator  operator- (difference_type offset) const {return const_iterator(this->base::operator-(offset), *this);}
    friend inline const_iterator operator+(difference_type offset, const_iterator const& iter) {return iter + offset;}
  };

  class buffer_type {
    const typename Policy::buffer_type& buff;
    Filter filter;
  public:
    buffer_type(const typename Policy::buffer_type& buff, filter_type const& filter)
      :buff(buff), filter(filter) {}
  public:
    char_reference operator[](std::size_t index) const {
      return this->filter(this->buff[index]);
    }
    std::size_t length() const {
      return this->buff.length();
    }
    const_iterator begin() const {
      return const_iterator(this->buff.begin(), this->filter);
    }
    const_iterator end() const {
      return const_iterator(this->buff.end(), this->filter);
    }
    const_iterator begin_at(std::ptrdiff_t index) const {
      return const_iterator(this->buff.begin_at(index), this->filter);
    }
  };
};

template<typename Policy, typename Filter>
struct _strtmp_ranged_map_policy {
  typedef _strtmp_ranged_map_policy   policy_type;
  typedef typename Policy::char_type  char_type;
  typedef char_type                   char_reference;
  static const bool has_get_ptr = false;

private:
  typedef typename mwg::stdm::remove_reference<Filter>::type filter_type;
  class ranged_filter {
    Filter                  m_filter;
    mwg::range<std::size_t> m_range;
  public:
    ranged_filter(filter_type const& filter, std::size_t start, std::size_t end)
      :m_filter(filter), m_range(start, end) {}
    char_reference operator()(char_type const& value, std::ptrdiff_t index) const {
      if (this->m_range.contains(index))
        return this->m_filter(value);
      else
        return value;
    }
  };

private:
  typedef typename Policy::const_iterator original_iterator;
  typedef typename stdm::conditional<has_memfn_index<original_iterator>::value, original_iterator,
    indexible_const_iterator<original_iterator> >::type indexed_iterator;
public:
  class const_iterator: public indexed_iterator {
    typedef indexed_iterator base;
    ranged_filter const* m_filter;
  public:
    const_iterator(indexed_iterator const& iter, ranged_filter const& filter)
      :base(iter), m_filter(&filter) {}
    const_iterator(indexed_iterator const& iter, const_iterator const& origin)
      :base(iter), m_filter(origin.m_filter) {}
    const_iterator(): m_filter(nullptr) {}

    typedef char_reference                 reference;
    typedef typename base::pointer         pointer;
    typedef typename base::difference_type difference_type;

    reference operator* () const {return (*this->m_filter)(this->base::operator*(), this->base::index());}
    pointer   operator->() const {return pointer(this->operator*());}

    const_iterator& operator++()    {this->base::operator++(); return *this;}
    const_iterator& operator--()    {this->base::operator--(); return *this;}
    const_iterator  operator++(int) {return const_iterator(this->base::operator++(0), *this);}
    const_iterator  operator--(int) {return const_iterator(this->base::operator--(0), *this);}

    // RandomAccessIterator
    using base::operator-;
    const_iterator& operator+=(difference_type offset) {this->base::operator+=(offset); return *this;}
    const_iterator& operator-=(difference_type offset) {this->base::operator-=(offset); return *this;}
    const_iterator  operator+ (difference_type offset) const {return const_iterator(this->base::operator+(offset), *this);}
    const_iterator  operator- (difference_type offset) const {return const_iterator(this->base::operator-(offset), *this);}
    friend inline const_iterator operator+(difference_type offset, const_iterator const& iter) {return iter + offset;}
  };

  class buffer_type {
    const typename Policy::buffer_type& buff;
    ranged_filter m_filter;
  public:
    buffer_type(const typename Policy::buffer_type& buff, filter_type const& filter, std::size_t start, std::size_t end)
      :buff(buff), m_filter(filter, start, end) {}
  public:
    char_reference operator[](std::size_t index) const {
      return this->m_filter(this->buff[index], index);
    }
    std::size_t length() const {
      return this->buff.length();
    }

  private:
    template<typename Iter>
    typename stdm::enable_if<has_memfn_index<Iter>::value, Iter const&>::type
    static create_indexed(Iter const& iter, std::ptrdiff_t) {
      return iter;
    }

    template<typename Iter>
    typename stdm::enable_if<!has_memfn_index<Iter>::value, indexed_iterator>::type
    static create_indexed(Iter const& iter, std::ptrdiff_t index) {
      return indexed_iterator(iter, index);
    }

  public:
    const_iterator begin() const {
      return const_iterator(create_indexed(buff.begin(), 0), this->m_filter);
    }
    const_iterator end() const {
      return const_iterator(create_indexed(buff.end(), buff.length()), this->m_filter);
    }
    const_iterator begin_at(std::ptrdiff_t index) const {
      return const_iterator(create_indexed(buff.begin_at(index), index), this->m_filter);
    }
  };
};

template<typename XCH>
struct _filt_tolower {
  typedef XCH char_type;
  char_type operator()(const char_type& c) const {
    return char_traits<char_type>::tolower(c);
  }
};
template<typename XCH>
struct _filt_toupper {
  typedef XCH char_type;
  char_type operator()(const char_type& c) const {
    return char_traits<char_type>::toupper(c);
  }
};
template<typename XCH>
struct _filt_replace_char {
  typedef XCH char_type;
  char_type cS;
  char_type cD;
public:
  _filt_replace_char(char_type const& before, char_type const& after)
    :cS(before), cD(after) {}
  char_type operator()(const char_type& c) const {
    return c == cS? cD: c;
  }
};

//-----------------------------------------------------------------------------
// _strtmp_pad_policy                                                  @tmp.pad

template<typename Str>
struct _strtmp_pad_policy {
  typedef _strtmp_pad_policy                        policy_type;
  typedef typename Str::policy_type::char_type      char_type;
  typedef typename Str::policy_type::char_reference char_reference;
  typedef char_traits<char_type>                    char_traits_type;
  static const bool has_get_ptr = false;

private:
  typedef typename Str::policy_type::const_iterator original_iterator;
public:
  class const_iterator: public wrap_iterator<original_iterator>::type {
    typedef typename wrap_iterator<original_iterator>::type base;
    typedef const_iterator this_type;
    std::ptrdiff_t             m_index;
    char_type                  m_pad;
    mwg::range<std::ptrdiff_t> m_range;
  public:
    const_iterator(original_iterator const& iter, std::ptrdiff_t index, char_type paddingCharacter, mwg::range<std::ptrdiff_t> r)
      :base(iter), m_index(index), m_pad(paddingCharacter), m_range(r) {}
    const_iterator(original_iterator const& iter, std::ptrdiff_t index, const_iterator const& origin)
      :base(iter), m_index(index), m_pad(origin.m_pad), m_range(origin.m_range) {}
    const_iterator(): m_pad(char_traits_type::space()), m_range(0, 0) {}

    typedef char_reference                 reference;
    typedef typename base::pointer         pointer;
    typedef typename base::difference_type difference_type;

  public:
    std::ptrdiff_t index() const {return this->m_index;}

    reference operator* () const {return this->m_range.contains(this->m_index)? this->base::operator*(): this->m_pad;}
    pointer   operator->() const {return pointer(this->operator*());}

    this_type& operator++() {
      if (this->m_range.contains(this->m_index))
        this->base::operator++();
      this->m_index++;
      return *this;
    }
    this_type& operator--() {
      this->m_index--;
      if (this->m_range.contains(this->m_index))
        this->base::operator--();
      return *this;
    }
    this_type operator++(int) {this_type ret(*this); this->operator++(); return ret;}
    this_type operator--(int) {this_type ret(*this); this->operator--(); return ret;}

    difference_type operator- (this_type const& rhs) const {return this->m_index -  rhs.m_index;}
    bool            operator==(this_type const& rhs) const {return this->m_index == rhs.m_index;}
    bool            operator!=(this_type const& rhs) const {return this->m_index != rhs.m_index;}

    // RandomAccessIterator (上書き)
    bool       operator< (this_type const& rhs) const {return this->m_index <  rhs.m_index;}
    bool       operator<=(this_type const& rhs) const {return this->m_index <= rhs.m_index;}
    bool       operator> (this_type const& rhs) const {return this->m_index >  rhs.m_index;}
    bool       operator>=(this_type const& rhs) const {return this->m_index >= rhs.m_index;}
    this_type& operator+=(difference_type offset) {
      std::ptrdiff_t const& indexOld = this->m_index;
      std::ptrdiff_t const  indexNew = indexOld + offset;
      std::ptrdiff_t const  baseIndexOld = mwg::clamp(indexOld, this->m_range);
      std::ptrdiff_t const  baseIndexNew = mwg::clamp(indexNew, this->m_range);
      this->m_index = indexNew;
      this->base::operator+=(baseIndexNew - baseIndexOld);
      return *this;
    }
    this_type& operator-=(difference_type offset) {return operator+=(-offset);}
    this_type  operator+ (difference_type offset) const {return this_type(*this) += offset;}
    this_type  operator- (difference_type offset) const {return this_type(*this) -= offset;}
    friend inline this_type operator+(difference_type offset, this_type const& iter) {return iter + offset;}
  };

  class buffer_type {
    typename Str::buffer_type const& s;
    std::size_t lpad_len;
    std::size_t m_length;
    char_type c;
  public:
    buffer_type(typename Str::buffer_type const& s, std::size_t lpad_len, std::size_t len, char_type c)
      :s(s), lpad_len(lpad_len), m_length(len), c(c) {}
  public:
    char_reference operator[](std::size_t index) const {
      std::ptrdiff_t index1 = std::ptrdiff_t(index) - std::ptrdiff_t(this->lpad_len);
      if (0 <= index1 && (std::size_t) index1 < this->s.length())
        return this->s[index1];
      else
        return this->c;
    }
    std::size_t length() const {
      return this->m_length;
    }
    const_iterator begin() const {return const_iterator(s.begin(), 0, c, mwg::range<std::ptrdiff_t>(lpad_len, lpad_len + s.length()));}
    const_iterator end()   const {return const_iterator(s.end(), this->length(), c, mwg::range<std::ptrdiff_t>(lpad_len, lpad_len + s.length()));}
    const_iterator begin_at(std::ptrdiff_t index) const {
      mwg::range<std::ptrdiff_t> const r(lpad_len, lpad_len + s.length());
      std::ptrdiff_t const baseIndex = mwg::clamp(index, r) - this->lpad_len;
      return const_iterator(s.begin_at(baseIndex), index, c, r);
    }
  };

};

//-----------------------------------------------------------------------------
// _strtmp_cat_policy, operator+                                       @tmp.cat

template<typename Str1, typename Str2, typename Str3>
struct _strtmp_cat_policy {
  typedef _strtmp_cat_policy policy_type;
  typedef typename mwg::stdm::remove_reference<Str1>::type string_type1;
  typedef typename mwg::stdm::remove_reference<Str2>::type string_type2;
  typedef typename mwg::stdm::remove_reference<Str3>::type string_type3;
  typedef typename string_type1::policy_type policy_type1;
  typedef typename string_type2::policy_type policy_type2;
  typedef typename string_type2::policy_type policy_type3;

  typedef typename policy_type1::char_type char_type;
  typedef typename mwg::stdm::conditional<
    mwg::stdm::is_same<
      typename policy_type1::char_reference,
      typename policy_type2::char_reference
    >::value && mwg::stdm::is_same<
      typename policy_type1::char_reference,
      typename policy_type3::char_reference
    >::value,
    typename policy_type1::char_reference,
    typename policy_type1::char_type
  >::type char_reference;

  static const bool has_get_ptr = false;
  typedef default_const_iterator<policy_type> const_iterator;

  class buffer_type {
    Str1 str1;
    Str2 str2;
    Str3 str3;
    std::size_t len1;
    std::size_t len2;
  public:
    buffer_type(string_type1 const& str1, string_type2 const& str2, string_type3 const& str3)
      :str1(str1), str2(str2), str3(str3), len1(str1.length()), len2(str2.length()) {}
  public:
    char_reference operator[](std::size_t index) const {
      std::ptrdiff_t const index2 = std::ptrdiff_t(index) - len1;
      if (index2 < 0)
        return this->str1[index];
      std::ptrdiff_t const index3 = std::ptrdiff_t(index2) - len2;
      if (index3 < 0)
        return this->str2[index2];
      return this->str3[index3];
    }
    std::size_t length() const {
      return len1 + len2 + this->str3.length();
    }
    const_iterator begin() const {return const_iterator(*this, 0);}
    const_iterator end()   const {return const_iterator(*this, this->length());}
    const_iterator begin_at(std::ptrdiff_t index) const {return const_iterator(*this, index);}
  };
};

template<typename Str1, typename Str2>
struct _strtmp_cat_policy<Str1, Str2> {
  typedef _strtmp_cat_policy         policy_type;
  typedef typename mwg::stdm::remove_reference<Str1>::type string_type1;
  typedef typename mwg::stdm::remove_reference<Str2>::type string_type2;
  typedef typename string_type1::policy_type policy_type1;
  typedef typename string_type2::policy_type policy_type2;

  typedef typename policy_type1::char_type char_type;
  typedef typename mwg::stdm::conditional<
    mwg::stdm::is_same<
      typename policy_type1::char_reference,
      typename policy_type2::char_reference>::value,
    typename policy_type1::char_reference,
    typename policy_type1::char_type >::type char_reference;

  static const bool has_get_ptr = false;

  typedef default_const_iterator<policy_type> const_iterator;

  class buffer_type {
    Str1 str1;
    Str2 str2;
    std::size_t length1;
  public:
    buffer_type(string_type1 const& str1, string_type2 const& str2)
      :str1(str1), str2(str2), length1(str1.length()) {}
  public:
    char_reference operator[](std::size_t index) const {
      std::ptrdiff_t const index2 = std::ptrdiff_t(index) - length1;
      if (index2 < 0)
        return this->str1[index];
      else
        return this->str2[index2];
    }
    std::size_t length() const {
      return length1 + this->str2.length();
    }
    const_iterator begin() const {return const_iterator(*this, 0);}
    const_iterator end()   const {return const_iterator(*this, this->length());}
    const_iterator begin_at(std::ptrdiff_t index) const {return const_iterator(*this, index);}
  };
};

template<
  typename X, typename Y,
  bool = as_str<X, typename as_str<Y>::char_type>::value
> struct enable_concat {};

template<typename X, typename Y>
struct enable_concat<X, Y, true>: mwg::identity<
  strbase<_strtmp_cat_policy<
    typename as_str<X>::adapter,
    typename as_str<Y>::adapter
  > >
> {};

template<typename X, typename Y>
typename enable_concat<X, Y>::type
operator+(X const& lhs, Y const& rhs) {
#ifdef MWG_STR_H_hidden_copy_constructors_of_temporaries
  return {lhs, rhs};
#else
  typedef typename enable_concat<X, Y>::type return_type;
  return return_type(lhs, rhs);
#endif
}

#pragma%x begin_test
void test() {
  mwg_check((_a("hello") + _a(" world") == "hello world"));
  mwg_check((_a("hello") + " world" == "hello world"));
  mwg_check(("hello" + _a(" world") + "!" == "hello world!"));
}
#pragma%x end_test

//-----------------------------------------------------------------------------
// reverse, repeat                                         @tmp.rev @tmp.repeat

template<typename Str>
struct _strtmp_reverse_policy {
  typedef _strtmp_reverse_policy                    policy_type;
  typedef typename Str::policy_type::char_type      char_type;
  typedef typename Str::policy_type::char_reference char_reference;
  static const bool has_get_ptr = false;

  typedef default_const_iterator<policy_type> const_iterator;

  class buffer_type {
    Str const& s;
  public:
    buffer_type(Str const& s): s(s) {}
  public:
    char_reference operator[](std::size_t index) const {
      return this->s[this->s.length() - 1 - index];
    }
    std::size_t    length() const {return this->s.length();}
    const_iterator begin()  const {return const_iterator(*this, 0);}
    const_iterator end()    const {return const_iterator(*this, this->length());}
    const_iterator begin_at(std::ptrdiff_t index) const {return const_iterator(*this, index);}
  };
};

template<typename Str>
struct _strtmp_repeat_policy {
  typedef _strtmp_repeat_policy                     policy_type;
  typedef typename Str::policy_type::char_type      char_type;
  typedef typename Str::policy_type::char_reference char_reference;
  static const bool has_get_ptr = false;

  typedef default_const_iterator<policy_type> const_iterator;

  class buffer_type {
    Str const& m_str;
    std::size_t m_repeatCount;
  public:
    buffer_type(Str const& s, std::size_t repeatCount)
      :m_str(s), m_repeatCount(repeatCount) {}
  public:
    char_reference operator[](std::size_t index) const {
      return this->m_str[index % this->m_str.length()];
    }
    std::size_t    length() const {return this->m_str.length() * this->m_repeatCount;}
    const_iterator begin()  const {return const_iterator(*this, 0);}
    const_iterator end()    const {return const_iterator(*this, this->length());}
    const_iterator begin_at(std::ptrdiff_t index) const {return const_iterator(*this, index);}
  };
};

//-----------------------------------------------------------------------------
// 関係演算子                                                           @op.rel

#pragma%m mwg_str::compare::doc
/*?lwiki
 * :@op s1=={==}==s2;
 * :@op s1=={!=}==s2;
 * :@op s1=={<=}==s2;
 * :@op s1=={>=}==s2;
 * :@op s1==<==s2;
 * :@op s1==>==s2;
 *  文字列を比較します。
 * :@fn ==compare==(s1, s2);
 *  二つの文字列を比較します。`s1>s2` の時 `1`, `s1==s2` の時 `0`, `s1<s2` の時 `-1` を返します。
 *  c.f. `strcmp`/`strncmp` (C), `lexicographical_compare` (Boost), \
 *  `Compare` (ATL/MFC), <?cs CompareOriginal?> (CLR), <?java compareTo?> (Java), <?rb operator<=>?> (Ruby)
 * :@fn ==icompare==(s1, s2);
 *  ASCII 大文字・小文字を区別せずに、二つの文字列を比較します。
 *  c.f. `stricmp`/`strcasecmp`/`strnicmp`/`strncasecmp` (C), `ilexicographical_compare` (Boost), \
 *  `CompareNoCase` (ATL/MFC), <?cs Compare?> (CLR), <?java compareToIgnoreCase?> (Java), <?rb casecmp?> (Ruby)
 * :他
 *  c.f. `strcoll`/`wcscoll` (C), `Collate`/`CollateNoCase` (ATL/MFC)
 */
#pragma%end

template<
  typename X, typename Y, typename R,
  bool=as_str<X, typename as_str<Y>::char_type>::value
> struct enable_compare: mwg::identity<R> {};

template<typename X, typename Y>
typename enable_compare<X, Y, int>::type
compare(X const& _lhs, Y const& _rhs) {
  typename as_str<X>::adapter lhs(_lhs);
  typename as_str<Y>::adapter rhs(_rhs);
  typename as_str<X>::const_iterator i = lhs.begin(), iN = lhs.end();
  typename as_str<Y>::const_iterator j = rhs.begin(), jN = rhs.end();
  for (; i != iN && j != jN; ++i, ++j)
    if (*i != *j) return *i > *j? 1: -1;
  return i != iN? 1: j != jN? -1: 0;
}

template<typename X, typename Y>
typename enable_compare<X, Y, int>::type
icompare(X const& _lhs, Y const& _rhs) {
  typename as_str<X>::adapter lhs(_lhs);
  typename as_str<Y>::adapter rhs(_rhs);
  return compare(lhs.tolower(), rhs.tolower());
}

template<typename X, typename Y>
typename enable_compare<X, Y, bool>::type
operator==(X const& _lhs, Y const& _rhs) {
  typename as_str<X>::adapter lhs(_lhs);
  typename as_str<Y>::adapter rhs(_rhs);
  if (lhs.length() != rhs.length()) return false;
  typename as_str<X>::const_iterator i = lhs.begin(), iN = lhs.end();
  typename as_str<Y>::const_iterator j = rhs.begin();
  for (; i != iN; ++i, ++j)
    if (*i != *j) return false;
  return true;
}

template<typename X, typename Y>
typename enable_compare<X, Y, bool>::type
operator<(X const& _lhs, Y const& _rhs) {
  typename as_str<X>::adapter lhs(_lhs);
  typename as_str<Y>::adapter rhs(_rhs);
  typename as_str<X>::const_iterator i = lhs.begin(), iN = lhs.end();
  typename as_str<Y>::const_iterator j = rhs.begin(), jN = rhs.end();
  for (; i != iN && j != jN; ++i, ++j)
    if (*i != *j) return *i < *j;
  return i == iN && j != jN;
}

template<typename X, typename Y>
typename enable_compare<X, Y, bool>::type
operator!=(X const& lhs, Y const& rhs) {return !(lhs == rhs);}
template<typename X, typename Y>
typename enable_compare<X, Y, bool>::type
operator> (X const& lhs, Y const& rhs) {return rhs < lhs;}
template<typename X, typename Y>
typename enable_compare<X, Y, bool>::type
operator<=(X const& lhs, Y const& rhs) {return !(rhs < lhs);}
template<typename X, typename Y>
typename enable_compare<X, Y, bool>::type
operator>=(X const& lhs, Y const& rhs) {return !(lhs < rhs);}

#pragma%x begin_test
void test() {
  mwg_check(compare(_a("hello"), _a("hello")) ==  0);
  mwg_check(compare(_a("hello"),   "hello" )  ==  0);
  mwg_check(compare(   "hello" ,_a("hello"))  ==  0);
  mwg_check(compare(_a("hello"), _a("world")) == -1);
  mwg_check(compare(_a("hello"),   "world" )  == -1);
  mwg_check(compare(   "hello" ,_a("world"))  == -1);
  mwg_check(compare(_a("world"), _a("hello")) ==  1);
  mwg_check(compare(   "world" ,_a("hello"))  ==  1);
  mwg_check(compare(_a("world"),   "hello" )  ==  1);
  mwg_check(compare(_a("hello"),   "hell"  )  ==  1);
  mwg_check(compare(_a("hell" ),   "hello ")  == -1);

  // assume ASCII codes
  mwg_check( compare(_a("hello"), _a("HELLO")) ==  1);
  mwg_check( compare(_a("hello"), _a("WORLD")) ==  1);
  mwg_check( compare(_a("WORLD"), _a("hello")) == -1);
  mwg_check(icompare(_a("hello"), _a("HELLO")) ==  0);
  mwg_check(icompare(_a("hello"), _a("WORLD")) == -1);
  mwg_check(icompare(_a("WORLD"), _a("hello")) ==  1);

  mwg_check( (_a("hello") == "hello"));
  mwg_check(!(_a("hello") != "hello"));
  mwg_check(!(_a("hello") == "world"));
  mwg_check( (_a("hello") != "world"));
  mwg_check(!(_a("hello") == "hell"));
  mwg_check( (_a("hello") != "hell"));

  mwg_check(!(_a("hello") < "hello"));
  mwg_check(!(_a("hello") > "hello"));
  mwg_check( (_a("hello") < "world"));
  mwg_check(!(_a("hello") > "world"));
  mwg_check(!(_a("hello") < "hell"));
  mwg_check( (_a("hello") > "hell"));

  mwg_check( (_a("hello") <= "hello"));
  mwg_check( (_a("hello") >= "hello"));
  mwg_check( (_a("hello") <= "world"));
  mwg_check(!(_a("hello") >= "world"));
  mwg_check(!(_a("hello") <= "hell"));
  mwg_check( (_a("hello") >= "hell"));

  mwg::strfix<char> s1;
  mwg_check( (s1 == ""));
  mwg::strfix<char> s2 = "012345";
  mwg_check( (s2 == "012345"));
  s1 = "21345";
  mwg_check( (s1 == "21345"));
}
#pragma%x end_test

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
} /* end of namespace str_detail */
} /* end of namespace mwg */

#include "bits/str.strbuf.h"

//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
//
// char support
//
#include <cstring>
#include <cctype>

namespace mwg {
namespace str_detail {

template<>
struct char_traits<char> {
  typedef char char_type;
  static std::size_t strlen(const char_type* s) {
    return std::strlen(s);
  }
  static mwg_constexpr char_type null() {return '\0';}
  static mwg_constexpr char_type space() {return ' ';}
  static mwg_constexpr char_type tolower(char_type c) {
    return 'A' <= c && c <= 'Z'? char_type(c + ('a' - 'A')): c;
  }
  static mwg_constexpr char_type toupper(char_type c) {
    return 'a' <= c && c <= 'z'? char_type(c + ('A' - 'a')): c;
  }
  static bool isspace(char_type c) {
    // return '\t' <= c && c <= '\r' || c == ' '; for ASCII
    return std::isspace(c);
  }
};

}
}

//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
//
// wchar_t support
//
#include <cwchar>
#include <cwctype>

namespace mwg {
namespace str_detail {

template<>
struct char_traits<wchar_t> {
  typedef wchar_t char_type;
  static std::size_t strlen(const char_type* s) {
    return std::wcslen(s);
  }
  static mwg_constexpr char_type null() {return L'\0';}
  static mwg_constexpr char_type space() {return L' ';}
  static mwg_constexpr char_type tolower(char_type c) {
    return L'A' <= c && c <= L'Z'? char_type(c + (L'a' - L'A')): c;
  }
  static mwg_constexpr char_type toupper(char_type c) {
    return L'a' <= c && c <= L'z'? char_type(c + (L'A' - L'a')): c;
  }
  static bool isspace(char_type c) {
    return std::iswspace(c);
  }
};

}
}

#pragma%x begin_test
void test() {
  mwg_check((_a(L"AbCdE").toupper(1, -1) == L"ABCDE"));
  mwg_check((_a(L"aBcDe").tolower(1, -1) == L"abcde"));
  mwg_check((_a(L"  hello  ").trim() == L"hello"));
  mwg_check((_a(L"  hello  ").ltrim() == L"hello  "));
  mwg_check((_a(L"  hello  ").rtrim() == L"  hello"));
  mwg_check((_a(L"world").pad(7) == L" world "));
}
#pragma%x end_test

//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
//
// std::basic_string support
//
#include <string>

namespace mwg {
namespace str_detail {

template<typename XCH, typename Tr, typename Alloc>
struct adapter_traits<std::basic_string<XCH, Tr, Alloc> >: adapter_traits_array<XCH> {
  static const XCH* pointer(std::basic_string<XCH, Tr, Alloc> const& s) {
    return s.c_str();
  }
  static std::size_t length(std::basic_string<XCH, Tr, Alloc> const& s) {
    return s.length();
  }
};

}
}
#pragma%x begin_test
void test() {
  std::string s1("hello");
  mwg_check((mwg::str(s1).toupper(1, 4) == "hELLo"));
}
#pragma%x end_test

//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
/*?lwiki
 * *文字列基本機能 (`mwg::str_detail::strbase`) #strbase_interface
 */
#pragma%x mwg_str::strbase::doc
/*?lwiki
 * ***注意点: `auto` による変数宣言
 * 多くの演算で式テンプレートを使用している為、式の結果は一時オブジェクトの型になります。
 * 一時オブジェクトの寿命を延長すると問題が生じるのでコピーコンストラクタを隠蔽しています。
 * つまり、以下のように `auto` を用いて変数を作成することができません。一般に `mwg::strfix<CHAR>` を使用して下さい。
 * もしくは `fix()` メンバ関数を呼び出して明示的に `mwg::strfix<CHAR>` を構築して下さい。
 * &pre(!cpp){
 * // ERROR
 * auto a = mwg::str("hello").toupper();
 *
 * // OK
 * mwg::strfix<char> a = mwg::str("hello").toupper();
 * auto a = mwg::str("hello").toupper().fix();
 * }
 *
 * ***定義: <?cpp* '''range-spec'''?>
 * 以下に繰り返し現れる仮引数として <?cpp* '''range-spec'''?> を定義します。
 * <?cpp* function('''range-spec''')?> は、関数が以下の3つの多重定義を持つことを表します。
 * -`function(i)`
 * -`function(i, j)`
 * -`function(r)`
 * 仮引数 <?cpp* '''range-spec'''?> は、文字列内部の範囲を指定するために用います。
 * それぞれの仮引数は以下の様に指定します。
 * :@param[in] std::ptrdiff_t i, j;
 *  それぞれ範囲の開始位置と終端位置を指定します。
 *  負の数が指定された場合は文字列末端からの相対位置と解釈します。
 *  `mwg::npos` が指定された場合は文字列末端と解釈します。
 *  終端位置が省略された場合は、文字列末端を意味します。
 * :@param[in] mwg::irange r;
 *  範囲を指定します。開始位置、終端位置は上記 `i`, `j` と同様に解釈されます。
 *
 * **連結・切り出し・挿入・削除
 * :@op s1==+==s2
 *  文字列を連結します。
 */
#pragma%x mwg_str::slice::doc
#pragma%x mwg_str::insert::doc
/*?lwiki
 *
 * **分割・結合
 * :@fn [TODO] s.split(i=0, opt=0);
 * :@fn [TODO] s.split(s1|reg|ch|pred, i=0, opt=0);
 * :@fn [TODO] s.rsplit(i=0, opt=0);
 * :@fn [TODO] s.rsplit(s1|reg|ch|pred, i=-1, opt=0);
 *  空白または指定された分割子で文字列を分割します。
 *  :@param[in] i
 *   最大分割数を指定します。0 以下の場合には無制限であることを表します。
 *  :@param[in] opt
 *   オプション trim/remove_empty_string
 *  :@param[in] s1
 *   分割文字列
 *  :@param[in] reg
 *   分割子の正規表現
 *  :@param[in] ch
 *   分割文字
 *  :@param[in] pred
 *   文字判定子
 *  c.f. `split`/`iter_split` (Boost), <?cs Split?> (CLR), <?java split?> (Java, JavaScript), \
 *  <?rb partition?>/<?rb rpartition?> (Ruby), <?awk split?> (awk. Ruby, CLX), `split_if` (CLX), `Tokenize(cset, int&)` (ATL/MFC)
 * :@fn [TODO] arr.==join==();
 * :@fn [TODO] arr.==join==(s1); // s1 分割文字列
 *  文字列の集合を連結します。
 *  c.f. `join`/`join_if` (Boost), <?rb join?> (Ruby, CLX)
 *
 * **文字の変換
 */
#pragma%x mwg_str::map::doc
/*?lwiki
 *
 * **空白・幅調節
 */
#pragma%x mwg_str::trim::doc
#pragma%x mwg_str::pad::doc
/*?lwiki
 *
 * **判定
 */
#pragma%x mwg_str::compare::doc
#pragma%x mwg_str::starts::doc
/*?lwiki
 *
 * **検索・置換
 */
#pragma%x mwg_str::find::doc
/*?lwiki
 * :@fn [TODO] s.==replace==(s1, s2, n=mwg::npos); // 文字列を置換
 * :@fn [TODO] s.==replace1==(s1, s2, n=0); // 文字列を置換
 *  c.f. `Replace(s1, s2)` (ATL/MFC), `replace`/`replace_all` (CLX), <?js replace?> (JavaScript)
 *
 * **正規表現
 * :@fn [TODO] s.==replace==(reg, s2); // reg 正規表現
 * :@fn [TODO] s.==replace==(reg, fun); // fun 置換後の文字列を決める関数
 *  c.f. `replace_all`/`replace_regex`/`replace_first`/`replace_last` (Boost), <?cs Relace?> (CLR, mwg-string), \
 *    <?java replace?> (Java, JavaScript), <?java replaceAll?>/<?java replaceFirst?> (Java), \
 *    <?awk sub?>/<?awk gsub?> (awk, Ruby), <?mk subst?>/<?mk patsubst?> (Makefile).
 *  -Boost の `replace_nth` に対応する関数は、それ程有用とは思われないので提供しない。
 *  -Boost の `replace_regex_all`, `replace_head`, `replace_tail`, \
 *   `ireplace_first`, `ireplace_last`, `ireplace_nth`, `ireplace_all` \
 *   に対応する関数は正規表現及びそのフラグを用いて表現できるので提供しない。
 *   ■→最適化の観点から行くと `head`, `tail` は有用かも知れない。
 *  -Boost の `erase_all`, `erase_regex`, `erase_regex_all`, `erase_head`, \
 *   `erase_tail`, `erase_first`, `erase_last`, `erase_nth`, \
 *   `ierase_first`, `ierase_last`, `ierase_nth`, `ierase_all` \
 *   に対応する関数は置換後の文字列に `""` を指定すれば良いだけなので提供しない。
 * :@fn [TODO] s1.==match==(reg, &color(red){[}'''range-spec'''&color(red){]}); // reg 正規表現
 * :@fn [TODO] s1.==rmatch==(reg, &color(red){[}'''range-spec'''&color(red){]});
 * :@fn [TODO] s1.==match_at==(reg, &color(red){[}'''range-spec'''&color(red){]});
 * :@fn [TODO] s1.==rmatch_at==(reg, &color(red){[}'''range-spec'''&color(red){]});
 *  正規表現に対する一致を試す。
 *  -`match`: 先頭から順に一致を試す。
 *  -`rmatch`: 末尾から順に一致を試す。
 *  -`match_at`: 先頭を含む部分列に対してだけ、一致を試す。
 *  -`rmatch_at`: 末端を含む部分列に対してだけ、一致を試す。
 *  c.f. `Search`/`SearchR`/`Match`/`MatchAt` (mwg-string), <?js match?> (JavaScript), <?awk match?> (awk)
 * :@fn [TODO] match_iterator
 *  c.f. `find_all`/`iter_find` (Boost), <?rb scan?> (Ruby), <?js match?> (JavaScript)
 *
 * **他
 */
#pragma%x mwg_str::misc::doc
/*?lwiki
 * :format, operator%
 *  c.f. `sprintf` (C), `AppendFormat, Format, FormatV, FormatMessage, FormatMessageV` (ATL/MFC), `Format` (CLR), <?java format?> (Java), <?awk sprintf?> (awk, Perl), `operator%` (Ruby)
 * :参考
 *  -<?pl chop?>/<?pl chomp?> (Perl, Ruby, CLX)
 *  -mwg-string: ReverseMap
 *  -Perl: hex oct,
 *  -Ruby: count, crypt, delete, hash sum, \
 *   hex oct to_i to_f to_c to_r to_s to_str, succ next, squeeze tr_s
 *  -CLX: `unique(s)`/`squeeze(s,c)`/`squeeze_if(s,pred)`
 *  -ATL/MFC: `GetEnvironmentVariable`, `LoadString`, `BSTR AllocSysString() const, BSTR SetSysString(BSTR*) const`, `AnsiToOem, OemToAnsi`,
 *  -`splice` (JavaScript)
 *  -`Remove(s,c)` (ATL/MFC), `remove(s,c), remove_if(s,pred)` (CLX)
 *  -`SpanIncluding, SpanExcluding` (ATL/MFC)
 *
 * *新しい文字列型を定義する方法
mwg/str では、文字列の内部形式と文字列に対する操作を分離して実装しています。\
文字列の内部形式は `StringPolicy` を用いて定義されます。\
文字列に対する操作は `strbase<StringPolicy>` によって提供されます。\
ここでは、`StringPolicy` を定義して、新しい文字列の内部形式を追加する方法を説明します。

`StringPolicy` は以下の様なメンバを持つクラスとして定義します。
*/
#pragma%x mwg_str::policy_requirements
/*?lwiki
:@class class StringPolicy;
 :@typedef buffer_type;
  文字列の内部表現を格納する型です。
 :@typedef const_iterator;
  文字列に含まれる文字を列挙する反復子です。
 :@typedef char_type;
  単一の文字を表現する型です。
 :@typedef char_reference;
  文字列の文字を `buffer_type::operator[]` や `const_iterator::operator*` を通して取得する際の型です。\
  内部表現に対応するデータが存在する場合には、そのデータへの参照 (`char_type const&`) になります。\
  それ以外の場合は、単に `char_type` になります。
 :@const static const bool has_get_ptr;
  文字データが連続した領域に格納され、その先頭へのポインタが得られる場合に true を指定します。
*/
//HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
#include <mwg/bits/autoload.inl>
#endif
#pragma%x begin_test
void test() {
  mwg_check( (_a("HELLO").repeat(3).tolower(5, -3).reverse() == "OLLehollehOLLEH"));

  // OK
  // auto a = _a("hello world!").tolower(2, -3);
  // auto const& a = _a("hello world!").tolower(2, -3); // 危険・コンパイルが通る
  // mwg_unused(a);
}
#pragma%x end_test
#pragma%x begin_check
// namespace string_bench {
//   int test_compare1();
//   void test() {
//     for (mwg::i8t i = 0; i < 10000000LL; i++)
//       string_bench::test_compare1();
//   }
// }

int main() {
  managed_test::run_tests();
  return 0;
}
#pragma%x end_check
