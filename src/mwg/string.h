// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef MWG_STRING_H
#define MWG_STRING_H
//HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
#include <cstddef>
#include <mwg/std/utility>
#include <mwg/std/type_traits>
#include <mwg/std/memory>
#include <mwg/std/limits>
#include <mwg/range.h>
#include <mwg/functor.h>
#pragma%x begin_check
#include <vector>
#include <mwg/except.h>
#include <mwg/string.h>

class managed_test{
public:
  virtual void test()=0;
  managed_test(){testerList.push_back(this);}

private:
  static std::vector<managed_test*> testerList;
public:
  static void run_tests(){
    for(int i=0,iN=testerList.size();i<iN;i++)
      testerList[i]->test();
  }
};
std::vector<managed_test*> managed_test::testerList;

#pragma%[itest=0]
#pragma%m begin_test
#pragma%%[itest++]
#pragma%%x begin_check
#pragma%%x
class _test$"itest":managed_test{
#pragma%%end.i
#pragma%end
#pragma%m end_test
#pragma%%x
} _test$"itest"_instance;
#pragma%%end.i
#pragma%%x end_check
#pragma%end
#pragma%x end_check
namespace mwg{
namespace string3_detail{
  template<typename XCH>
  struct char_traits;
  template<typename Policy>
  class strbase;

  template<typename XCH>
  class strsub;
  template<typename XCH>
  class stradp;
  template<typename XCH>
  class string;
  /*?lwiki
   * :@class class mwg::==string==<XCH> : strbase<...>;
   *  `mwg::string` における標準の文字列型です。
   *  `std::shared_ptr` による参照管理の対象です。
   * :@class class mwg::==stradp==<XCH> : strbase<...>;
   *  他の型の文字列に対する `mwg::string` インターフェイスを提供します。
   *  -`XCH[N]`
   *  -`const XCH[N]`
   *  -`XCH*`
   *  -`const XCH*`
   *  -`std::basic_string<XCH>`
   * :@class class mwg::==strsub==<XCH> : strbase<...>;
   *  他の文字列の部分文字列を保持します。
   * :@class class mwg::string3_detail::==strbase==<...>;
   *  文字列に対する操作を提供する基底クラスです。
   *  また、部分式の評価結果として `strbase` の様々な特殊化が使用されます。
   *
   */

  template<typename XCH,typename C,typename Ret>
  struct adapter_enabler;

  template<typename StrP1>
  struct _strtmp_sub_policy;
  template<typename StrP,typename Filter>
  struct _strtmp_map_policy;
  template<typename StrP,typename Filter>
  struct _strtmp_ranged_map_policy;
  template<typename XCH>
  struct _filt_tolower;
  template<typename XCH>
  struct _filt_toupper;
  template<typename XCH>
  struct _filt_replace_char;
  template<typename Str1,typename Str2,typename Str3=void>
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
}
namespace mwg{
namespace string3_detail{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  inline std::size_t canonicalize_index(std::ptrdiff_t const& index,std::size_t const& len){
    if(index==mwg::npos){
      return len;
    }else if(index<0){
      std::ptrdiff_t const _index=len+index;
      return _index>=0?_index:0;
    }else
      return (std::size_t)index<len?index:len;
  }

#pragma%m mwg::string::policy_requirements
  struct StringPolicy{
    typedef char          char_type;
    typedef const char&   char_at_type; // e.g. char, const char&
    typedef StringPolicy  policy_type;

    static const bool has_get_ptr;

    struct const_iterator{
      static const bool has_index;
      // 以下2関数は has_index==true の時にだけ定義される。
      std::ptrdiff_t index() const;

      char_at_type operator*() const;

      const_iterator& operator++();
      const_iterator  operator++(int);
      const_iterator& operator--();
      const_iterator  operator--(int);

      bool operator==(const_iterator const&) const;
      bool operator!=(const_iterator const&) const;

      const_iterator  operator+(std::ptrdiff_t) const;
      const_iterator  operator-(std::ptrdiff_t) const;
      std::ptrdiff_t  operator-(const_iterator const&) const;
    };

    struct buffer_type{
      char_at_type operator[](std::ptrdiff_t) const;

      std::size_t length() const;

      const_iterator begin() const;
      const_iterator end() const;
      const_iterator begin_at(std::ptrdiff_t) const;

      // 以下の関数は StringPolicy::has_get_ptr==true の時にだけ定義される
      const char_type* get_ptr() const;
    };
  };
#pragma%end

//-----------------------------------------------------------------------------
// char_traits

// Tr 要件
template<typename XCH>
struct char_traits{
  typedef XCH char_type;

  static std::size_t strlen(const char_type* str);
  static mwg_constexpr char_type null();
};

//-----------------------------------------------------------------------------
// adapter_traits

template<typename C,typename XCH=void>
struct adapter_traits:adapter_traits<C>{
  static const bool adaptable
    =mwg::stdm::is_same<typename adapter_traits<C>::char_type,XCH>::value;
};
template<typename T>
struct adapter_traits<T>{
  typedef void char_type;

  // typedef XCH char_type;
  // static const XCH* pointer(C const& str);
  // static std::size_t length(C const& str);
};

template<typename XCH,typename C,typename Ret>
struct adapter_enabler
  :mwg::stdm::enable_if<adapter_traits<C,XCH>::adaptable,Ret>{};

// default specializations

template<typename XCH,std::size_t N>
struct adapter_traits<XCH[N]>{
  typedef XCH char_type;
  static const XCH* pointer(const XCH (&value)[N]){
    return value;
  }
  static std::size_t length(const XCH (&value)[N]){
    return value[N-1]!=char_traits<XCH>::null()?N:N-1;
  }
};

#ifdef _MSC_VER
/*
 * 何故か vc では const char[N] を
 * template<typename T> void f(T const&); で受け取ると、
 * T = char[N] ではなくて T = const char[N] になる。
 */
template<typename XCH,std::size_t N>
struct adapter_traits<const XCH[N]>{
  typedef XCH char_type;
  static const XCH* pointer(const XCH (&value)[N]){
    return value;
  }
  static std::size_t length(const XCH (&value)[N]){
    return value[N-1]!=char_traits<XCH>::null()?N:N-1;
  }
};
#endif

template<typename XCH>
struct adapter_traits<XCH*>{
  typedef XCH char_type;
  static const XCH* pointer(XCH* value){
    return value;
  }
  static std::size_t length(XCH* value){
    return char_traits<XCH>::strlen(value);
  }
};

template<typename XCH>
struct adapter_traits<const XCH*>{
  typedef XCH char_type;
  static const XCH* pointer(const XCH* value){
    return value;
  }
  static std::size_t length(const XCH* value){
    return char_traits<XCH>::strlen(value);
  }
};

//-----------------------------------------------------------------------------
// predicaters

template<typename XCH>
struct isspace_predicator{
  typedef XCH char_type;
  bool operator()(char_type c) const{
    return char_traits<XCH>::isspace(c);
  }
};
template<typename XCH,typename Str1>
struct _pred_any_of_str{
  typedef XCH char_type;
  Str1 const& str;
  _pred_any_of_str(Str1 const& str)
    :str(str){}

  bool operator()(char_type c) const{
    typename Str1::const_iterator i=str.begin(),iN=str.end();
    for(;i!=iN;++i)
      if(c==*i)return true;
    return false;
  }
};
template<typename XCH,typename Str1>
struct _pred_not_of_str{
  typedef XCH char_type;
  Str1 const& str;
  _pred_not_of_str(Str1 const& str)
    :str(str){}

  bool operator()(char_type c) const{
    typename Str1::const_iterator i=str.begin(),iN=str.end();
    for(;i!=iN;++i)
      if(c==*i)return false;
    return true;
  }
};

//-----------------------------------------------------------------------------
// default_const_iterator
// pointer_const_iterator
// indexible_const_iterator

template<typename Policy>
class default_const_iterator{
  typedef default_const_iterator this_type;
  typedef typename Policy::char_at_type char_at_type;
  typedef typename Policy::buffer_type  buffer_type;

  buffer_type const& data;
  std::ptrdiff_t m_index;
public:
  static const bool has_index=true;
  std::ptrdiff_t index() const{return this->m_index;}
  std::ptrdiff_t operator-(this_type const& rhs) const{return this->m_index-rhs.m_index;}

public:
  default_const_iterator(buffer_type const& data,std::ptrdiff_t index)
    :data(data),m_index(index){}
  char_at_type operator*() const{return data[m_index];}

  this_type& operator++()   {++this->m_index;return *this;}
  this_type& operator--()   {--this->m_index;return *this;}
  this_type  operator++(int){this_type ret(*this);++this->m_index;return ret;}
  this_type  operator--(int){this_type ret(*this);--this->m_index;return ret;}

  this_type operator+(std::ptrdiff_t offset) const{return this_type(this->data,this->m_index+offset);}
  this_type operator-(std::ptrdiff_t offset) const{return this_type(this->data,this->m_index-offset);}

  bool operator==(this_type const& rhs) const{return this->m_index==rhs.m_index;}
  bool operator!=(this_type const& rhs) const{return this->m_index!=rhs.m_index;}
  bool operator< (this_type const& rhs) const{return this->m_index< rhs.m_index;}
  bool operator<=(this_type const& rhs) const{return this->m_index<=rhs.m_index;}
  bool operator> (this_type const& rhs) const{return this->m_index> rhs.m_index;}
  bool operator>=(this_type const& rhs) const{return this->m_index>=rhs.m_index;}
};

template<typename Type>
class pointer_const_iterator{
  typedef pointer_const_iterator this_type;
  typedef Type                   char_type;
  typedef Type const&            char_at_type;

  Type const* data;

public:
  static const bool has_index=false;

public:
  pointer_const_iterator(Type const* pointer):data(pointer){}

  char_at_type operator*() const{return *data;}

  this_type& operator++()   {++this->data;return *this;}
  this_type  operator++(int){return this_type(this->data++);}
  this_type& operator--()   {--this->data;return *this;}
  this_type  operator--(int){return this_type(this->data--);}

  bool operator==(this_type const& rhs) const{return this->data==rhs.data;}
  bool operator!=(this_type const& rhs) const{return this->data!=rhs.data;}
  bool operator< (this_type const& rhs) const{return this->data< rhs.data;}
  bool operator<=(this_type const& rhs) const{return this->data<=rhs.data;}
  bool operator> (this_type const& rhs) const{return this->data> rhs.data;}
  bool operator>=(this_type const& rhs) const{return this->data>=rhs.data;}

  std::ptrdiff_t operator-(this_type const& rhs) const{return this->data-rhs.data;}
  this_type operator+(std::ptrdiff_t offset) const{return this_type(this->data+offset);}
  this_type operator-(std::ptrdiff_t offset) const{return this_type(this->data-offset);}
};

template<typename Iter,bool IterHasIndex=Iter::has_index>
class indexible_const_iterator:public Iter{
  typedef Iter                     base;
  typedef indexible_const_iterator this_type;

  std::ptrdiff_t m_index;
public:
  static const bool has_index=true;
  std::ptrdiff_t index() const{return this->m_index;}
  std::ptrdiff_t operator-(this_type const& rhs) const{return this->m_index-rhs.m_index;}

public:
  indexible_const_iterator(base const& iter,std::size_t index)
    :base(iter),m_index(index){}

  this_type& operator++()   {this->base::operator++();++this->m_index;return *this;}
  this_type& operator--()   {this->base::operator--();--this->m_index;return *this;}
  this_type  operator++(int){this_type ret(*this);this->base::operator++();++this->m_index;return ret;}
  this_type  operator--(int){this_type ret(*this);this->base::operator--();--this->m_index;return ret;}
  this_type  operator+(std::ptrdiff_t offset) const{return this_type(this->base::operator+(offset),this->m_index+offset);}
  this_type  operator-(std::ptrdiff_t offset) const{return this_type(this->base::operator-(offset),this->m_index-offset);}
};

template<typename Iter>
class indexible_const_iterator<Iter,true>:public Iter{
  typedef Iter                     base;
  typedef indexible_const_iterator this_type;

public:
  indexible_const_iterator(base const& copye,std::ptrdiff_t):base(copye){}
  indexible_const_iterator(base const& copye):base(copye){}

  this_type& operator++()   {this->base::operator++();return *this;}
  this_type& operator--()   {this->base::operator--();return *this;}
  this_type  operator++(int){this_type ret(*this);this->base::operator++());return ret;}
  this_type  operator--(int){this_type ret(*this);this->base::operator--());return ret;}
  this_type  operator+(std::ptrdiff_t offset) const{return this_type(this->base::operator+(offset));}
  this_type  operator-(std::ptrdiff_t offset) const{return this_type(this->base::operator-(offset));}
};

//-----------------------------------------------------------------------------
// strbase

template<typename XCH>
struct strbase_tag{};

template<typename Policy>
class strbase:public strbase_tag<typename Policy::char_type>{
public:
  typedef Policy                               policy_type;
  typedef typename policy_type::char_type      char_type;
  typedef char_traits<char_type>               char_traits_type;
  typedef typename policy_type::buffer_type    buffer_type;

  typedef typename policy_type::const_iterator const_iterator;

protected:
  buffer_type data;

public:
  strbase(){}
#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
  template<typename... As>
  strbase(As mwg_forward_rvalue... args)
    :data(mwg::stdm::forward<As>(args)...){}
#else
#pragma%[AN=5]
#pragma%x (
  template<$".for/@/0/An/typename A@/,">
  strbase($".for/@/0/An/A@ mwg_forward_rvalue arg@/,")
    :data($".for/@/0/An/mwg::stdm::forward<A@>(arg@)/,"){}
#pragma%).f/An/1/AN+1/.i
#endif

#pragma%m mwg::string::strbase::doc
  /*?lwiki
   * :@op s==[==index==]==;
   *  指定した位置にある文字を返します。
   * :@fn s.==length==();
   *  文字列の長さを取得します。
   * :@fn s.==empty==();
   *  空文字列かどうかを取得します。
   *  c.f. `empty` (C++), <?rb empty??> (Ruby)
   * :@fn s.==begin==();
   * :@fn s.==end==();
   *  文字データの先頭と末端を指すイテレータを返します。
   * :@fn s.==front==();
   *  最初の文字を取得します。
   *  c.f. <?pl chr?>/<?pl ord?> (Perl), <?rb chr?>/<?rb ord?> (Ruby)
   * :@fn s.==back==();
   *  最後の文字を取得します。
   */
#pragma%end
public:
  typedef typename policy_type::char_at_type char_at_type;
  char_at_type operator[](std::size_t index) const{
    return data[index];
  }

  std::size_t length() const{
    return data.length();
  }
  bool empty() const{
    return data.length()==0;
  }

  char_type front() const{
    std::size_t const _len=this->length();
    return _len==0?char_traits_type::null():data[0];
  }
  char_type back() const{
    std::size_t const _len=this->length();
    return _len==0?char_traits_type::null():data[_len-1];
  }

  const_iterator begin() const{return data.begin();}
  const_iterator end()   const{return data.end();}
private:
  const_iterator _beginAt(std::ptrdiff_t offset) const{return data.begin_at(offset);}

  //---------------------------------------------------------------------------
  //
  // mwg::string::slice
  //
  //---------------------------------------------------------------------------
#pragma%m mwg::string::slice::doc
  /*?lwiki
   * :@fn s1.==slice==('''range-spec''');
   * :@fn s1.==substr==(i,len);
   *  文字列の指定した範囲を部分文字列として返します。
   *  c.f. <`substr` (C++), `Mid` (ATL/MFC), <?cs Substring?> (CLR), `Slice`/`Substr` (mwg-string), \
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
    policy_type::has_get_ptr,strsub<char_type>,strbase<_strtmp_sub_policy<policy_type> > >::type slice_return_type;
  slice_return_type slice(std::ptrdiff_t start,std::ptrdiff_t end=mwg::npos) const{
    std::size_t const len=this->length();
    std::size_t const _start=canonicalize_index(start,len);
    std::size_t _end=canonicalize_index(end,len);
    if(_start>_end)_end=_start;
    return slice_return_type(this->data,_start,_end-_start);
  }
  slice_return_type slice(mwg::range_i const& r) const{
    return slice(r.begin(),r.end());
  }
  slice_return_type substr(std::ptrdiff_t start,std::size_t length) const{
    std::size_t const len=this->length();
    std::size_t const _start=canonicalize_index(start,len);
    std::size_t _end=canonicalize_index(start+length,len);
    if(_start>_end)_end=_start;
    return slice_return_type(this->data,_start,_end-_start);
  }
  slice_return_type head(std::size_t len) const{
    std::size_t const _len=this->length();
    if(len>_len)len=_len;
    return slice_return_type(this->data,0,len);
  }
  slice_return_type tail(std::size_t len) const{
    std::size_t const _len=this->length();
    if(len>_len)len=_len;
    return slice_return_type(this->data,_len-len,len);
  }
  typedef strbase<_strtmp_cat_policy<slice_return_type,slice_return_type> > remove_return_type;
  slice_return_type remove(std::ptrdiff_t start) const{
    std::size_t const _len=this->length();
    std::size_t const _start=canonicalize_index(start,_len);
    return slice_return_type(this->data,0,_start);
  }
  remove_return_type remove(std::ptrdiff_t start,std::ptrdiff_t end) const{
    std::size_t const _len=this->length();
    std::size_t _start=canonicalize_index(start,_len);
    std::size_t _end=canonicalize_index(end,_len);
    if(_start>=_end)
      _start=_end=_len;

    return remove_return_type(
      slice_return_type(this->data,0,_start),
      slice_return_type(this->data,_end,_len-_end)
    );
  }
  remove_return_type remove(mwg::range_i const& r) const{
    return this->remove(r.begin(),r.end());
  }
#pragma%x begin_test
  void test(){
    typedef mwg::stradp<char> _a;
    mwg_assert((_a("hello").slice(2,4) =="ll"));
    mwg_assert((_a("hello").slice(1,-2)=="el"));
    mwg_assert((_a("hello").slice(3)   =="lo"));

    // slice of strtmp with `!has_get_ptr`
    mwg_assert((_a("hello").toupper().slice(2,4) =="LL"));
    mwg_assert((_a("hello").toupper().slice(1,-2)=="EL"));
    mwg_assert((_a("hello").toupper().slice(3)   =="LO"));

    mwg_assert((_a("0123456789").slice(-6,-3)=="456"));
    mwg_assert((_a("0123456789").slice(-3)=="789"));
    mwg_assert((_a("0123456789").slice(6,4)==""));
    mwg_assert((_a("0123456789").slice(6,-6)==""));

    mwg_assert((_a("hello").remove(3)=="hel"));
    mwg_assert((_a("hello").remove(1,-2)=="hlo"));
    mwg_assert((_a("hello").remove(mwg::make_range(-4,-1))=="ho"));
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg::string::insert
  //
  //---------------------------------------------------------------------------
#pragma%m mwg::string::insert::doc
  /*?lwiki
   * :@fn s.==replace==('''range-spec''',s2);
   *  指定した範囲を別の文字列に置換します。
   *  c.f. `substr(s,i,len,s2)` (Perl)
   * :@fn s1.==insert==(i,str);
   *  指定した位置に文字列を挿入します。
   *  c.f. `insert` (C++), `Insert` (ATL/MFC), <?cs Insert?> (CLR), `Insert` (mwg-string), <?rb insert?> (Ruby)
   */
#pragma%end
private:
  template<typename A1>
  struct range_replace_switch
    :mwg::stdm::integral_constant<int,
      mwg::stdm::is_base_of<strbase_tag<char_type>,A1>::value?1:
      adapter_traits<A1,char_type>::adaptable?2:
      0>{};
  template<typename A1,int S>
  struct range_replace_enabler
    :mwg::stdm::enable_if<
      S==range_replace_switch<A1>::value,
      typename mwg::stdm::conditional<
        S==1,strbase<_strtmp_cat_policy<slice_return_type,A1 const&,slice_return_type> >,
        typename mwg::stdm::conditional<
          S==2,strbase<_strtmp_cat_policy<slice_return_type,stradp<char_type>,slice_return_type> >,
          void
        >::type
      >::type
    >{};

  template<typename T,int S,typename T_>
  typename range_replace_enabler<T,S>::type
  _replace_impl(std::size_t _start,std::size_t _end,T_ const& str) const{
    std::size_t const _len=this->length();
    return typename range_replace_enabler<T,S>::type(
      slice_return_type(this->data,0,_start),
      str,
      slice_return_type(this->data,_end,_len-_end)
    );
  }
public:
  template<typename T>
  typename range_replace_enabler<T,1>::type
  replace(std::ptrdiff_t start,std::ptrdiff_t end,T const& str) const{
    std::size_t const _len=this->length();
    std::size_t _start=canonicalize_index(start,_len);
    std::size_t _end=canonicalize_index(end,_len);
    if(_end<_start)std::swap(_start,_end);
    return _replace_impl<T,1>(_start,_end,str);
  }
  template<typename T>
  typename range_replace_enabler<T,2>::type
  replace(std::ptrdiff_t start,std::ptrdiff_t end,T const& str) const{
    std::size_t const _len=this->length();
    std::size_t _start=canonicalize_index(start,_len);
    std::size_t _end=canonicalize_index(end,_len);
    if(_end<_start)std::swap(_start,_end);
    return _replace_impl<T,2>(_start,_end,str);
  }
  template<typename T>
  typename range_replace_enabler<T,1>::type
  replace(mwg::range_i const& r,T const& str) const{
    return this->replace(r.begin(),r.end(),str);
  }
  template<typename T>
  typename range_replace_enabler<T,2>::type
  replace(mwg::range_i const& r,T const& str) const{
    return this->replace(r.begin(),r.end(),str);
  }
  template<typename T>
  typename range_replace_enabler<T,1>::type
  insert(std::ptrdiff_t index,T const& str) const{
    std::size_t const _len=this->length();
    std::size_t const _index=canonicalize_index(index,_len);
    return _replace_impl<T,1>(_index,_index,str);
  }
  template<typename T>
  typename range_replace_enabler<T,2>::type
  insert(std::ptrdiff_t index,T const& str) const{
    std::size_t const _len=this->length();
    std::size_t const _index=canonicalize_index(index,_len);
    return _replace_impl<T,2>(_index,_index,str);
  }
#pragma%x begin_test
  void test(){
    typedef mwg::stradp<char> _a;
    mwg_assert((_a("hello").replace(1,-3,"icon")=="hiconllo"));
    mwg_assert((_a("hello").replace(1,-3,_a("icon"))=="hiconllo"));
    mwg_assert((_a("hello").replace(mwg::make_range(1,-3),"icon")=="hiconllo"));
    mwg_assert((_a("hello").insert(1,"icon")=="hiconello"));
    mwg_assert((_a("hello").insert(1,_a("icon"))=="hiconello"));
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg::string::map
  //   characterwise operations
  //   tolower, toupper
  //
  //---------------------------------------------------------------------------
#pragma%m mwg::string::map::doc
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
   * :@fn s.==map==(filter,&color(red){[}'''range-spec'''&color(red){]});
   *  c.f. `std::transform` (C++), <?cs System.Array.ConvertAll?> (CLR), `Map` (mwg-string)
   * :@fn s.==replace==(c1,c2,&color(red){[}'''range-spec'''&color(red){]});
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
  typedef strbase<_strtmp_map_policy<policy_type,_filt_tolower<char_type> > >        tolower_return_type;
  typedef strbase<_strtmp_ranged_map_policy<policy_type,_filt_tolower<char_type> > > ranged_tolower_return_type;
  typedef strbase<_strtmp_map_policy<policy_type,_filt_toupper<char_type> > >        toupper_return_type;
  typedef strbase<_strtmp_ranged_map_policy<policy_type,_filt_toupper<char_type> > > ranged_toupper_return_type;
  tolower_return_type tolower() const{
    return tolower_return_type(this->data,_filt_tolower<char_type>());
  }
  ranged_tolower_return_type tolower(std::ptrdiff_t start,std::ptrdiff_t end=mwg::npos) const{
    std::size_t const _len=this->length();
    std::size_t const _start=canonicalize_index(start,_len);
    std::size_t _end=canonicalize_index(end,_len);
    if(_end<_start)_end=_start;
    return ranged_tolower_return_type(this->data,_filt_tolower<char_type>(),_start,_end);
  }
  ranged_tolower_return_type tolower(mwg::range_i const& r) const{
    return tolower(r.begin(),r.end());
  }
  toupper_return_type toupper() const{
    return toupper_return_type(this->data,_filt_toupper<char_type>());
  }
  ranged_toupper_return_type toupper(std::ptrdiff_t start,std::ptrdiff_t end=mwg::npos) const{
    std::size_t const _len=this->length();
    std::size_t const _start=canonicalize_index(start,_len);
    std::size_t _end=canonicalize_index(end,_len);
    if(_end<_start)_end=_start;
    return ranged_toupper_return_type(this->data,_filt_toupper<char_type>(),_start,_end);
  }
  ranged_toupper_return_type toupper(mwg::range_i const& r) const{
    return toupper(r.begin(),r.end());
  }
private:
  template<typename Filter>
  struct map_enabler
    :mwg::stdm::enable_if<mwg::be_functor<Filter,char_type(char_type)>::value,
                          strbase<_strtmp_map_policy<policy_type,Filter const&> > >{};
  template<typename Filter>
  struct ranged_map_enabler
    :mwg::stdm::enable_if<mwg::be_functor<Filter,char_type(char_type)>::value,
                          strbase<_strtmp_ranged_map_policy<policy_type,Filter const&> > >{};
public:
  template<typename F>
  typename map_enabler<F>::type map(F const& filter) const{
    return typename map_enabler<F>::type(this->data,filter);
  }
  template<typename F>
  typename ranged_map_enabler<F>::type map(F const& filter,std::ptrdiff_t start,std::ptrdiff_t end=mwg::npos) const{
    std::size_t const _len=this->length();
    std::size_t const _start=canonicalize_index(start,_len);
    std::size_t _end=canonicalize_index(end,_len);
    if(_end<_start)_end=_start;
    return typename ranged_map_enabler<F>::type(this->data,filter,_start,_end);
  }
  template<typename F>
  typename ranged_map_enabler<F>::type map(F const& filter,mwg::range_i const& r) const{
    return map(filter,r.begin(),r.end());
  }

private:
  typedef strbase<_strtmp_map_policy<policy_type,_filt_replace_char<char_type> > >        char_replace_return_type;
  typedef strbase<_strtmp_ranged_map_policy<policy_type,_filt_replace_char<char_type> > > ranged_char_replace_return_type;
public:
  char_replace_return_type replace(char_type const& before,char_type const& after) const{
    return char_replace_return_type(this->data,_filt_replace_char<char_type>(before,after));
  }
  ranged_char_replace_return_type replace(char_type const& before,char_type const& after,std::ptrdiff_t start,std::ptrdiff_t end=mwg::npos) const{
    std::size_t const _len=this->length();
    std::size_t const _start=canonicalize_index(start,_len);
    std::size_t _end=canonicalize_index(end,_len);
    if(_end<_start)_end=_start;
    return ranged_char_replace_return_type(this->data,_filt_replace_char<char_type>(before,after),_start,_end);
  }
  ranged_char_replace_return_type replace(char_type const& before,char_type const& after,mwg::range_i const& r) const{
    return this->replace(before,after,r.begin(),r.end());
  }

#pragma%x begin_test
  void test(){
    typedef mwg::stradp<char> _a;
    mwg_assert( (_a("hello").toupper()=="HELLO"));
    mwg_assert( (_a("hello").toupper(2)=="heLLO"));
    mwg_assert( (_a("hello").toupper(1,-1)=="hELLo"));
    mwg_assert( (_a("hello").toupper(mwg::make_range(1,-2))=="hELlo"));
    mwg_assert( (_a("HELLO").tolower()=="hello"));
    mwg_assert( (_a("HELLO").tolower(2)=="HEllo"));
    mwg_assert( (_a("HELLO").tolower(1,-1)=="HellO"));
    mwg_assert( (_a("HELLO").tolower(mwg::make_range(1,-2))=="HelLO"));
    mwg_assert( (_a("hello").replace('l','c')=="hecco"));
    mwg_assert( (_a("hello").replace('l','p').replace('e','i')=="hippo"));
    mwg_assert( (_a("hello").replace('l','p',-2)=="helpo"));
    mwg_assert( (_a("hello").replace('l','r',0,3)=="herlo"));
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg::string::trim
  //
  //---------------------------------------------------------------------------
#pragma%m mwg::string::trim::doc
  /*?lwiki
   * :@fn s1.==trim==();
   * :@fn s1.==trim==(s2);   // s2   削除文字集合
   * :@fn s1.==trim==(pred); // pred 削除文字を判定する関数
   *  文字列の両端にある連続する空白を除去します。
   *  c.f. `trim` (Boost), `Trim` (ATL/MFC, CLR, mwg-string), `TrimAny` (mwg-string), <?java trim?> (Java, JavaScript), \
   *  `strip` (Ruby, CLX), `strip_if` (CLX), strip (Makefile)
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
private:
  template<typename A1,int Swch>
  struct trim_enabler
    :mwg::stdm::enable_if<
    Swch==(
      mwg::stdm::is_base_of<strbase_tag<char_type>,A1>::value?1:
      adapter_traits<A1,char_type>::adaptable?2:
      mwg::be_functor<A1,bool(char_type)>::value?3:0
    ),slice_return_type >{};
public:
  slice_return_type trim() const{
    return this->trim(isspace_predicator<char_type>());
  }
  template<typename StrP>
  typename trim_enabler<strbase<StrP>,1>::type
  trim(strbase<StrP> const& set) const{
    return this->trim(_pred_any_of_str<char_type,strbase<StrP> >(set));
  }
  template<typename XStr>
  typename trim_enabler<XStr,2>::type
  trim(XStr const& set) const{
    return this->trim(_pred_any_of_str<char_type,stradp<char_type> >(set));
  }
  template<typename FPred>
  typename trim_enabler<FPred,3>::type
  trim(FPred const& pred) const{
    typedef mwg::functor_traits<FPred,bool(char_type)> _f;
    const_iterator i=this->begin(),j=this->end();
    while(i!=j&&_f::invoke(pred,*i))++i;
    while(j!=i)if(!_f::invoke(pred,*--j)){++j;break;}
    return slice_return_type(this->data,i-this->begin(),j-i);
  }
  slice_return_type ltrim() const{
    return this->ltrim(isspace_predicator<char_type>());
  }
  template<typename StrP>
  typename trim_enabler<strbase<StrP>,1>::type
  ltrim(strbase<StrP> const& set) const{
    return this->ltrim(_pred_any_of_str<char_type,strbase<StrP> >(set));
  }
  template<typename XStr>
  typename trim_enabler<XStr,2>::type
  ltrim(XStr const& set) const{
    return this->ltrim(_pred_any_of_str<char_type,stradp<char_type> >(set));
  }
  template<typename FPred>
  typename trim_enabler<FPred,3>::type
  ltrim(FPred const& pred) const{
    typedef mwg::functor_traits<FPred,bool(char_type)> _f;
    const_iterator i=this->begin(),j=this->end();
    while(i!=j&&_f::invoke(pred,*i))++i;
    return slice_return_type(this->data,i-this->begin(),j-i);
  }
  slice_return_type rtrim() const{
    return this->rtrim(isspace_predicator<char_type>());
  }
  template<typename StrP>
  typename trim_enabler<strbase<StrP>,1>::type
  rtrim(strbase<StrP> const& set) const{
    return this->rtrim(_pred_any_of_str<char_type,strbase<StrP> >(set));
  }
  template<typename XStr>
  typename trim_enabler<XStr,2>::type
  rtrim(XStr const& set) const{
    return this->rtrim(_pred_any_of_str<char_type,stradp<char_type> >(set));
  }
  template<typename FPred>
  typename trim_enabler<FPred,3>::type
  rtrim(FPred const& pred) const{
    typedef mwg::functor_traits<FPred,bool(char_type)> _f;
    const_iterator i=this->begin(),j=this->end();
    while(j!=i)if(!_f::invoke(pred,*--j)){++j;break;}
    return slice_return_type(this->data,0,j-i);
  }
#pragma%x begin_test
  void test(){
    typedef mwg::stradp<char> _a;
    mwg_assert((_a("  hello   ").trim ()=="hello"));
    mwg_assert((_a("  hello   ").ltrim()=="hello   "));
    mwg_assert((_a("  hello   ").rtrim()=="  hello"));
    mwg_assert((_a("012343210").trim ("012")=="343"));
    mwg_assert((_a("012343210").ltrim("012")=="343210"));
    mwg_assert((_a("012343210").rtrim("012")=="012343"));
    mwg_assert((_a("012343210").trim (_a("012"))=="343"));
    mwg_assert((_a("012343210").ltrim(_a("012"))=="343210"));
    mwg_assert((_a("012343210").rtrim(_a("012"))=="012343"));
#ifdef MWGCONF_STD_LAMBDAS
    mwg_assert((_a("012343210").trim ([](char c){return '0'<=c&&c<='2';})=="343"));
    mwg_assert((_a("012343210").ltrim([](char c){return '0'<=c&&c<='2';})=="343210"));
    mwg_assert((_a("012343210").rtrim([](char c){return '0'<=c&&c<='2';})=="012343"));
#endif
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg::string::pad
  //
  //---------------------------------------------------------------------------
#pragma%m mwg::string::pad::doc
  /*?lwiki
   * :@fn s.==pad==(len);
   * :@fn s.==pad==(len,c);
   *  c.f. <?rb center?> (Ruby, CLX)
   * :@fn s.==lpad==(len);
   * :@fn s.==lpad==(len,c);
   *  c.f. <?cs PadLeft?> (CLR), <?rb ljust?> (Ruby, CLX)
   * :@fn s.==rpad==(len);
   * :@fn s.==rpad==(len,c);
   *  c.f. <?cs PadRight?> (CLR), <?rb rjust?> (Ruby, CLX)
   */
#pragma%end
public:
  typedef strbase<_strtmp_pad_policy<strbase<policy_type> > > pad_return_type;
  pad_return_type pad(std::size_t width,char_type const& ch) const{
    std::ptrdiff_t room=(std::ptrdiff_t)width-this->length();
    if(room<=0)
      return pad_return_type(*this,0,this->length(),ch);
    else
      return pad_return_type(*this,room/2,width,ch);
  }
  pad_return_type lpad(std::size_t width,char_type const& ch) const{
    std::ptrdiff_t room=(std::ptrdiff_t)width-this->length();
    if(room<=0)
      return pad_return_type(*this,0,this->length(),ch);
    else
      return pad_return_type(*this,room,width,ch);
  }
  pad_return_type rpad(std::size_t width,char_type const& ch) const{
    std::ptrdiff_t room=(std::ptrdiff_t)width-this->length();
    if(room<=0)
      return pad_return_type(*this,0,this->length(),ch);
    else
      return pad_return_type(*this,0,width,ch);
  }
  pad_return_type pad(std::size_t width) const{
    return this->pad(width,char_traits_type::space());
  }
  pad_return_type lpad(std::size_t width) const{
    return this->lpad(width,char_traits_type::space());
  }
  pad_return_type rpad(std::size_t width) const{
    return this->rpad(width,char_traits_type::space());
  }
#pragma%x begin_test
  void test(){
    typedef mwg::stradp<char> _a;
    mwg_assert((_a("hello").pad(1)=="hello"));
    mwg_assert((_a("hello").lpad(1)=="hello"));
    mwg_assert((_a("hello").rpad(1)=="hello"));
    mwg_assert((_a("hello").pad(10)=="  hello   "));
    mwg_assert((_a("hello").lpad(10)=="     hello"));
    mwg_assert((_a("hello").rpad(10)=="hello     "));
    mwg_assert((_a("hello").pad(10,'-')=="--hello---"));
    mwg_assert((_a("hello").lpad(10,'-')=="-----hello"));
    mwg_assert((_a("hello").rpad(10,'-')=="hello-----"));
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg::string::starts
  //
  //---------------------------------------------------------------------------
#pragma%m mwg::string::starts::doc
  /*?lwiki
   * :@fn s.==starts==(s1); // s1 文字列
   *  文字列が指定された文字列で始まっているかを判定します。
   *  c.f. starts_with (Boost), StartsWith (CLR), startsWith (Java), StartsWith (mwg-string), start_with? (Ruby)
   * :@fn s.==ends==(s1); // s1 文字列
   *  文字列が指定された文字列で終わっているかを判定します。
   *  c.f. ends_with (Boost), EndsWith (CLR), endsWith (Java), EndsWith (mwg-string), end_with? (Ruby)
   * :参考
   *  all/istarts_with/iends_with (Boost)
   */
#pragma%end
public:
  template<typename StrP>
  bool starts(strbase<StrP> const& str) const{
    if(this->length()<str.length())return false;
    const_iterator i=this->begin();
    typename strbase<StrP>::const_iterator j=str.begin(),jN=str.end();
    for(;j!=jN;++i,++j)
      if(*i!=*j)return false;
    return true;
  }
  template<typename XStr>
  typename adapter_enabler<char_type,XStr,bool>::type
  starts(XStr const& str) const{
    return this->starts(stradp<char_type>(str));
  }
  template<typename StrP>
  bool ends(strbase<StrP> const& str) const{
    std::ptrdiff_t offset=this->length()-str.length();
    if(offset<0)return false;
    const_iterator i=this->_beginAt(offset);
    typename strbase<StrP>::const_iterator j=str.begin(),jN=str.end();
    for(;j!=jN;++i,++j)
      if(*i!=*j)return false;
    return true;
  }
  template<typename XStr>
  typename adapter_enabler<char_type,XStr,bool>::type
  ends(XStr const& str) const{
    return this->ends(stradp<char_type>(str));
  }
#pragma%x begin_test
  void test(){
    typedef mwg::stradp<char> _a;
    mwg_assert( (_a("hello world").starts("hel")));
    mwg_assert( (_a("hello world").starts(_a("hel"))));
    mwg_assert(!(_a("hello world").starts("hal")));
    mwg_assert(!(_a("hello world").starts(_a("hal"))));
    mwg_assert(!(_a("hello world").starts("hello world!")));
    mwg_assert(!(_a("hello world").starts(_a("hello world!"))));
    mwg_assert( (_a("hello world").ends("orld")));
    mwg_assert( (_a("hello world").ends(_a("orld"))));
    mwg_assert(!(_a("hello world").ends("olrd")));
    mwg_assert(!(_a("hello world").ends(_a("olrd"))));
    mwg_assert(!(_a("hello world").ends("+hello world")));
    mwg_assert(!(_a("hello world").ends(_a("+hello world"))));
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg::string::find
  //
  //---------------------------------------------------------------------------
#pragma%m mwg::string::find::doc
  /*?lwiki
   * :@fn s.==find==(str&color(red){|}ch&color(red){|}pred&color(red){|}reg,&color(red){[}'''range-spec'''&color(red){]});
   * :@fn s.==rfind==(str&color(red){|}ch&color(red){|}pred&color(red){|}reg,&color(red){[}'''range-spec'''&color(red){]});
   *  指定した文字列を検索して最初に見付かった開始位置を返します。
   *  `find` は範囲先頭から検索を開始し、`rfind` は範囲末端から検索を開始します。
   *  第一引数に検索対象を指定します。第二引数に検索範囲を指定します。検索範囲の指定を省略した場合、文字列全体が検索範囲になります。
   *  :@param[in] '''string''' str;
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
   * :@fn s.==find_any==(s2,&color(red){[}'''range-spec'''&color(red){]});
   * :@fn s.==rfind_any==(s2,&color(red){[}'''range-spec'''&color(red){]});
   *  文字集合の何れかの文字の位置を返します。
   *  :@param[in] s2
   *   文字集合を指定します。
   *  find_any -> c.f. `strpbrk`/`strcspn` (C), find_first_of (C++), `FindOneOf` (ATL/MFC), <?cs IndexOfAny?> (CLR, mwg-string)
   *  rfind_any -> c.f. `find_last_of` (C++), <?cs LastIndexOfAny?> (CLR), `IndexOfAnyR` (mwg-string)
   * :@fn s.==find_not==(s2,&color(red){[}'''range-spec'''&color(red){]});
   * :@fn s.==rfind_not==(s2,&color(red){[}'''range-spec'''&color(red){]});
   *  最初に見付かった、文字集合に含まれない文字の位置を返します。
   *  -`find_not` -> c.f. strspn (C), find_first_not_of (C++), IndexOfNot (mwg-string)
   *  -`rfind_not` -> c.f. find_last_not_of (C++), IndexOfNotR (mwg-string)
   * :@op s1.find(...)>=0;
   *  文字列が他方の文字列に含まれているかどうかを判定する時。
   *  c.f. contains (Boost), Contains (CLR), contains/matches (Java), include? (Ruby), findstring (Makefile).
   * :参考
   *  c.f. Boost find_nth/ifind_first/ifind_last/ifind_nth
   */
#pragma%end

private:
  std::ptrdiff_t _find_impl(char_type const& ch,std::ptrdiff_t i,std::ptrdiff_t j) const{
    const_iterator p=this->_beginAt(i);
    for(;i<j;++i)
      if(*p++==ch)return i;
    return -1;
  }
  std::ptrdiff_t _rfind_impl(char_type const& ch,std::ptrdiff_t i,std::ptrdiff_t j) const{
    const_iterator p=this->_beginAt(j);
    for(;--j>=i;)
      if(*--p==ch)return j;
    return -1;
  }
public:
#define MWG_STRING3_STRING_H__define_find_overloads(FIND) \
  std::ptrdiff_t FIND(char_type const& ch) const{ \
    return this->_##FIND##_impl(ch,0,this->length()); \
  } \
  std::ptrdiff_t FIND(char_type const& ch,std::ptrdiff_t start,std::ptrdiff_t end=mwg::npos) const{ \
    std::size_t const _len=this->length(); \
    return this->_##FIND##_impl(ch,canonicalize_index(start,_len),canonicalize_index(end,_len)); \
  } \
  std::ptrdiff_t FIND(char_type const& ch,mwg::range_i const& r) const{ \
    return this->FIND(ch,r.begin(),r.end()); \
  }

  MWG_STRING3_STRING_H__define_find_overloads(find );
  MWG_STRING3_STRING_H__define_find_overloads(rfind);
#undef MWG_STRING3_STRING_H__define_find_overloads

private:
  template<typename A1,int Swch>
  struct find_enabler
    :mwg::stdm::enable_if<
    Swch==(
      mwg::stdm::is_base_of<strbase_tag<char_type>,A1>::value?1:
      adapter_traits<A1,char_type>::adaptable?2:
      mwg::be_functor<A1,bool(char_type)>::value?3:
      0
    ),std::ptrdiff_t>{};

  template<typename Pred>
  std::ptrdiff_t _find_pred(Pred const& pred,std::ptrdiff_t i,std::ptrdiff_t j) const{
    typedef mwg::functor_traits<Pred,bool(char_type)> _f;
    const_iterator p=this->_beginAt(i);
    for(;i<j;++i)
      if(_f::invoke(pred,*p++))return i;
    return -1;
  }
  template<typename Pred>
  std::ptrdiff_t _rfind_pred(Pred const& pred,std::ptrdiff_t i,std::ptrdiff_t j) const{
    typedef mwg::functor_traits<Pred,bool(char_type)> _f;
    const_iterator p=this->_beginAt(j);
    for(;--j>=i;)
      if(_f::invoke(pred,*--p))return j;
    return -1;
  }
public:
#define MWG_STRING3_STRING_H__define_find_overloads(FIND) \
  template<typename T> \
  typename find_enabler<T,3>::type FIND(T const& pred) const{ \
    return this->_##FIND##_pred(pred,0,this->length()); \
  } \
  template<typename T> \
  typename find_enabler<T,3>::type FIND(T const& pred,std::ptrdiff_t start,std::ptrdiff_t end=mwg::npos) const{ \
    std::size_t const _len=this->length(); \
    return this->_##FIND##_pred(pred,canonicalize_index(start,_len),canonicalize_index(end,_len)); \
  } \
  template<typename T> \
  typename find_enabler<T,3>::type FIND(T const& pred,mwg::range_i const& r) const{ \
    return this->FIND(pred,r.begin(),r.end()); \
  }

  MWG_STRING3_STRING_H__define_find_overloads(find );
  MWG_STRING3_STRING_H__define_find_overloads(rfind);
#undef MWG_STRING3_STRING_H__define_find_overloads

private:
  template<typename StrP>
  bool _find_match_at(std::size_t index,strbase<StrP> const& str) const{
    const_iterator p=this->_beginAt(index);
    typename strbase<StrP>::const_iterator q=str.begin();
    for(std::size_t end=index+str.length();index<end;index++)
      if(*p++!=*q++)return false;
    return true;
  }
  template<typename StrP>
  std::ptrdiff_t _find_impl(strbase<StrP> const& str,std::ptrdiff_t _i0,std::ptrdiff_t _iM) const{
    _iM-=str.length();
    for(std::ptrdiff_t i=_i0;i<=_iM;++i)
      if(_find_match_at(i,str))return i;
    return -1;
  }
  template<typename StrP>
  std::ptrdiff_t _rfind_impl(strbase<StrP> const& str,std::ptrdiff_t _i0,std::ptrdiff_t _iM) const{
    _iM-=str.length();
    for(std::ptrdiff_t i=_iM;i>=_i0;--i)
      if(_find_match_at(i,str))return i;
    return -1;
  }
  template<typename StrP>
  std::ptrdiff_t _find_any_impl(strbase<StrP> const& str,std::ptrdiff_t i,std::ptrdiff_t iN) const{
    return this->_find_pred(_pred_any_of_str<char_type,strbase<StrP> >(str),i,iN);
  }
  template<typename StrP>
  std::ptrdiff_t _find_not_impl(strbase<StrP> const& str,std::ptrdiff_t i,std::ptrdiff_t iN) const{
    return this->_find_pred(_pred_not_of_str<char_type,strbase<StrP> >(str),i,iN);
  }
  template<typename StrP>
  std::ptrdiff_t _rfind_any_impl(strbase<StrP> const& str,std::ptrdiff_t i,std::ptrdiff_t iN) const{
    return this->_rfind_pred(_pred_any_of_str<char_type,strbase<StrP> >(str),i,iN);
  }
  template<typename StrP>
  std::ptrdiff_t _rfind_not_impl(strbase<StrP> const& str,std::ptrdiff_t i,std::ptrdiff_t iN) const{
    return this->_rfind_pred(_pred_not_of_str<char_type,strbase<StrP> >(str),i,iN);
  }
public:
#define MWG_STRING3_STRING_H__define_find_overloads(FIND) \
  template<typename T> \
  typename find_enabler<T,1>::type FIND(T const& str) const{ \
    return this->_##FIND##_impl(str,0,this->length()); \
  } \
  template<typename T> \
  typename find_enabler<T,1>::type FIND(T const& str,std::ptrdiff_t start,std::ptrdiff_t end=mwg::npos) const{ \
    std::size_t const _len=this->length(); \
    return this->_##FIND##_impl(str,canonicalize_index(start,_len),canonicalize_index(end,_len)); \
  } \
  template<typename T> \
  typename find_enabler<T,1>::type FIND(T const& str,mwg::range_i const& r) const{ \
    return this->FIND(str,r.begin(),r.end()); \
  } \
  template<typename T> \
  typename find_enabler<T,2>::type FIND(T const& str) const{ \
    return this->FIND(stradp<char_type>(str)); \
  } \
  template<typename T> \
  typename find_enabler<T,2>::type FIND(T const& str,std::ptrdiff_t start,std::ptrdiff_t end=mwg::npos) const{ \
    return this->FIND(stradp<char_type>(str),start,end); \
  } \
  template<typename T> \
  typename find_enabler<T,2>::type FIND(T const& str,mwg::range_i const& r) const{ \
    return this->FIND(stradp<char_type>(str),r.begin(),r.end()); \
  }

  MWG_STRING3_STRING_H__define_find_overloads(find);
  MWG_STRING3_STRING_H__define_find_overloads(find_any);
  MWG_STRING3_STRING_H__define_find_overloads(find_not);
  MWG_STRING3_STRING_H__define_find_overloads(rfind);
  MWG_STRING3_STRING_H__define_find_overloads(rfind_any);
  MWG_STRING3_STRING_H__define_find_overloads(rfind_not);
#undef MWG_STRING3_STRING_H__define_find_overloads

#pragma%x begin_test
  void test(){
    typedef mwg::stradp<char> _a;
    mwg_assert((_a("0123401234").find("012")==0));
    mwg_assert((_a("0123401234").find("234")==2));
    mwg_assert((_a("0123401234").find("021")<0));
    mwg_assert((_a("0123401234").find("012",1)==5));
    mwg_assert((_a("0123401234").find("012",1,8)==5));
    mwg_assert((_a("0123401234").find("012",1,7)<0));
    mwg_assert((_a("0123401234").rfind("012")==5));
    mwg_assert((_a("0123401234").rfind("234")==7));
    mwg_assert((_a("0123401234").rfind("021")<0));
    mwg_assert((_a("0123401234").rfind("012",1)==5));
    mwg_assert((_a("0123401234").rfind("012",1,8)==5));
    mwg_assert((_a("0123401234").rfind("012",1,7)<0));
    mwg_assert((_a("0123401234").rfind("012",6)<0));
    mwg_assert((_a("0123401234").find('2')==2));
    mwg_assert((_a("0123401234").rfind('2')==7));
    mwg_assert((_a("0123401234").find_any("012")==0));
    mwg_assert((_a("0123401234").find_any("234")==2));
    mwg_assert((_a("0123401234").find_not("012")==3));
    mwg_assert((_a("0123401234").find_not("234")==0));
    mwg_assert((_a("0123401234").rfind_any("012")==7));
    mwg_assert((_a("0123401234").rfind_any("234")==9));
    mwg_assert((_a("0123401234").rfind_not("012")==9));
    mwg_assert((_a("0123401234").rfind_not("234")==6));
  }
#pragma%x end_test

  //---------------------------------------------------------------------------
  //
  // mwg::string::find
  //
  //---------------------------------------------------------------------------
#pragma%m mwg::string::misc::doc
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
  reverse_return_type reverse() const{
    return reverse_return_type(*this);
  }
private:
  typedef strbase<_strtmp_repeat_policy<strbase<policy_type> > > repeat_return_type;
public:
  repeat_return_type repeat(std::size_t count) const{
    return repeat_return_type(*this,count);
  }
#pragma%x begin_test
  void test(){
    typedef mwg::stradp<char> _a;
    mwg_assert( (_a("HELLO").reverse()=="OLLEH"));
    mwg_assert( (_a("HELLO").repeat(3)=="HELLOHELLOHELLO"));
  }
#pragma%x end_test
};

//-----------------------------------------------------------------------------
// strsub, stradp

template<typename XCH>
struct strsub_policy{
  typedef XCH              char_type;
  typedef const char_type& char_at_type;
  typedef strsub_policy    policy_type;
  static const bool has_get_ptr=true;
  typedef pointer_const_iterator<char_type> const_iterator;

  struct buffer_type{
    const char_type* ptr;
    std::size_t len;
  public:
    buffer_type(const char_type* ptr,std::size_t length)
      :ptr(ptr),len(length){}
  public:
    char_at_type operator[](std::size_t index) const{
      return ptr[index];
    }
    std::size_t length() const{
      return this->len;
    }
    const_iterator begin() const{
      return this->ptr;
    }
    const_iterator end() const{
      return this->ptr+this->len;
    }
    const_iterator begin_at(std::ptrdiff_t index) const{return this->ptr+index;}

    const char_type* get_ptr() const{
      return this->ptr;
    }
  };
};

template<typename XCH>
class strsub:public strbase<strsub_policy<XCH> >{
  typedef strbase<strsub_policy<XCH> > base;

private:
  template<typename StrP>
  friend class strbase;

  template<typename BufferType>
  strsub(BufferType const& data,std::size_t start,std::size_t length)
    :base(data.get_ptr()+start,length){}
};

template<typename XCH>
class stradp:public strbase<strsub_policy<XCH> >{
  typedef strbase<strsub_policy<XCH> > base;
  using typename base::char_type;

public:
  stradp(const char_type* ptr,std::size_t length)
    :base(ptr,length){}

  template<typename T>
  stradp(T const& value,typename adapter_enabler<char_type,T,int*>::type=0)
    :base(adapter_traits<T,char_type>::pointer(value),adapter_traits<T,char_type>::length(value)){}
};

//-----------------------------------------------------------------------------
// string

template<typename XCH>
struct string_policy{
  typedef string_policy    policy_type;
  typedef XCH              char_type;
  typedef const char_type& char_at_type;
  static const bool has_get_ptr=true;

  typedef pointer_const_iterator<char_type> const_iterator;

  typedef char_traits<char_type> char_traits_type;

  class buffer_type{
  private:
    friend class string<char_type>;

    struct bucket{
      char_type* data;
      std::size_t len;
    public:
      bucket():data(nullptr),len(-1){
        this->allocate(0);
      }
      bucket(std::size_t len):data(nullptr),len(-1){
        this->allocate(len);
      }
      ~bucket(){
        this->free();
      }
      void reset(std::size_t len){
        this->allocate(len);
      }
    private:
      void free(){
        delete[] this->data;
        this->data=nullptr;
      }
      void allocate(std::size_t length){
        /* 同じ長さでも必ず再確保する。
         * というのも、自分自身を参照しながら自分自身を更新する事があるから:
         * | str = str.reverse()
         */
        // if(this->len==length)return;

        this->free();
        this->len=length;
        this->data=new char_type[length+1];
      }
    };
    mwg::stdm::shared_ptr<bucket> ptr;

  public:
    buffer_type():ptr(new bucket){
      ptr->data[0]=char_traits_type::null();
    }

  public:
    template<typename StrP>
    buffer_type(strbase<StrP> const& str)
      :ptr(new bucket(str.length()))
    {
      this->copy_content(str);
    }
    template<typename StrP>
    void reset(strbase<StrP> const& str){
      this->ptr->reset(str.length());
      this->copy_content(str);
    }
  private:
    template<typename StrP>
    void copy_content(strbase<StrP> const& str){
      typename StrP::const_iterator j=str.begin();
      std::size_t const iN=str.length();
      char_type* const data=ptr->data;
      for(std::size_t i=0;i<iN;++i,++j)
        data[i]=*j;
      data[iN]=char_traits_type::null();
    }

  public:
    char_at_type operator[](std::size_t index) const{
      return ptr->data[index];
    }
    std::size_t length() const{
      return ptr->len;
    }
    const_iterator begin() const{
      return ptr->data;
    }
    const_iterator end() const{
      return ptr->data+ptr->len;
    }
    const_iterator begin_at(std::ptrdiff_t index) const{return ptr->data+index;}

    const char_type* get_ptr() const{
      return ptr->data;
    }
  };
};

template<typename XCH>
class string:public strbase<string_policy<XCH> >{
  typedef strbase<string_policy<XCH> > base;
  using typename base::char_type;

public:
  string(){}

  string(string const& str)
    :base(str.data){}
  string& operator=(string const& rhs){
    this->data=rhs.data;
    return *this;
  }
#ifdef MWGCONF_STD_RVALUE_REFERENCES
  string(string&& str)
    :base(mwg::stdm::move(str.data)){}
  string& operator=(string&& rhs){
    this->data=mwg::stdm::move(rhs.data);
    return *this;
  }
#endif

  template<typename StrP>
  string(strbase<StrP> const& str)
    :base(str){}
  template<typename StrP>
  string& operator=(strbase<StrP> const& str){
    this->data.reset(str);
    return *this;
  }

  template<typename T>
  string(T const& value,typename adapter_enabler<char_type,T,int*>::type=0)
    :base(stradp<char_type>(value)){}

};

#pragma%x begin_check
//-----------------------------------------------------------------------------
// _strtest_repeated_chars_policy

namespace mwg{
namespace string3_detail{

  /// @class _strtest_repeated_chars_policy
  /// 同じ文字が指定した回数だけ繰り返される文字列。
  /// これはデバグ用の StringPolicy である。
  /// has_index に対する処理をテストする為に、
  /// has_index な const_iterator の例として実装された。
  template<typename XCH>
  struct _strtest_repeated_chars_policy{
    typedef _strtest_repeated_chars_policy policy_type;
    typedef XCH char_type;
    typedef XCH char_at_type;
    static const bool has_get_ptr=false;

    class const_iterator{
      char_type value;
      std::ptrdiff_t m_index;
      typedef const_iterator this_type;
    public:
      static const bool has_index=true;
      std::ptrdiff_t index() const{return this->m_index;}

      const_iterator(char_type value,std::ptrdiff_t index):value(value),m_index(index){}

      char_at_type operator*() const{return value;}

      this_type& operator++(){++this->m_index;return *this;}
      this_type& operator--(){--this->m_index;return *this;}
      this_type  operator++(int){return this_type(this->m_index++);}
      this_type  operator--(int){return this_type(this->m_index--);}
      this_type  operator+(std::ptrdiff_t offset) const{return this_type(this->m_index+offset);}
      this_type  operator-(std::ptrdiff_t offset) const{return this_type(this->m_index-offset);}

      std::ptrdiff_t operator- (this_type const& rhs) const{return this->m_index- rhs.m_index;}
      bool           operator==(this_type const& rhs) const{return this->m_index==rhs.m_index;}
      bool           operator!=(this_type const& rhs) const{return this->m_index==rhs.m_index;}
    };

    class buffer_type{
    private:
      char_type value;
      std::size_t m_length;
    public:
      buffer_type(char_type value,std::size_t length)
        :value(value),m_length(length){}

      char_at_type operator[](std::ptrdiff_t index) const{return this->value;}
      std::size_t length() const{return this->m_length;}

      const_iterator begin() const{return const_iterator(this->value,0);}
      const_iterator end() const{return const_iterator(this->value,this->m_length);}
      const_iterator begin_at(std::ptrdiff_t offset) const{return const_iterator(this->value,offset);}
    };
  };
}
}

#pragma%x end_check
//-----------------------------------------------------------------------------
// _strtmp_sub_policy                                                  @tmp.sub

template<typename Iter,bool IterHasIndex=Iter::has_index>
class index_displaced_iterator{ /* not supported */ };

template<typename Iter>
class index_displaced_iterator<Iter,true>:public Iter{
  typedef Iter base;
  std::ptrdiff_t offset;

public:
  index_displaced_iterator(base const& iter,std::ptrdiff_t offset)
    :base(iter),offset(offset){}

  void index() const{
    return this->base::index()-start;
  }
};

template<typename Policy>
struct _strtmp_sub_policy{
public:
  typedef _strtmp_sub_policy            policy_type;
  typedef typename Policy::char_type    char_type;
  typedef typename Policy::char_at_type char_at_type;
  static const bool has_get_ptr=false;

private:
  typedef typename Policy::const_iterator target_iterator;

public:
  // target_iterator::has_index 分岐 (1/2)
  typedef typename stdm::conditional<
    target_iterator::has_index,index_displaced_iterator<target_iterator>,
    target_iterator>::type const_iterator;

  class buffer_type{
    const typename Policy::buffer_type& buff;
    std::size_t m_start;
    std::size_t m_length;

  public:
    buffer_type(const typename Policy::buffer_type& buff,std::size_t start,std::size_t length)
      :buff(buff),m_start(start),m_length(length){}

  public:
    char_at_type operator[](std::size_t index) const{
      return buff[m_start+index];
    }
    std::size_t length() const{
      return this->m_length;
    }

  private:
    // target_iterator::has_index 分岐 (2/2)
    template<typename Iter>
    typename stdm::enable_if<Iter::has_index,const_iterator>::type
    static modify_iterator(Iter const& iter,std::ptrdiff_t offset){
      return const_iterator(iter,offset);
    }
    template<typename Iter>
    typename stdm::enable_if<!Iter::has_index,Iter const&>::type
    static modify_iterator(Iter const& iter,std::ptrdiff_t offset){return iter;}

  public:
    const_iterator begin() const{
      return modify_iterator(buff.begin_at(m_start),m_start);
    }
    const_iterator end() const{
      return modify_iterator(buff.begin_at(m_start+m_length),m_start);
    }
    const_iterator begin_at(std::ptrdiff_t index) const{
      return modify_iterator(buff.begin_at(m_start+index),m_start);
    }
  };
};

#pragma%x begin_test
  void test(){
    using namespace mwg::string3_detail;

    // !has_index な基底 const_iterator から、const_iterator を初期化
    typedef mwg::stradp<char> _a;
    mwg_assert((strbase<_strtmp_sub_policy<_a::policy_type> >(_a::buffer_type("hello",5),1,3)=="ell"));

    // has_index な基底 const_iterator から、const_iterator を初期化
    typedef _strtest_repeated_chars_policy<char> _b;
    mwg_assert((strbase<_strtmp_sub_policy<_b::policy_type> >(_b::buffer_type('A',5),1,3)=="AAA"));
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
template<typename Policy,typename Filter>
struct _strtmp_map_policy{
  typedef _strtmp_map_policy         policy_type;
  typedef typename Policy::char_type char_type;
  typedef char_type                  char_at_type;
  static const bool has_get_ptr=false;

  typedef typename Policy::const_iterator target_iterator;
  typedef typename mwg::stdm::remove_reference<Filter>::type filter_type;
  class const_iterator:public target_iterator{
    filter_type const& m_filter;

    typedef typename Policy::const_iterator base;

  public:
    const_iterator(base const& iter,filter_type const& filter)
      :base(iter),m_filter(filter){}
    const_iterator(base const& iter,const_iterator const& origin)
      :base(iter),m_filter(origin.m_filter){}
    char_at_type operator*() const{return this->m_filter(this->base::operator*());}

    const_iterator& operator++()   {this->base::operator++();return *this;}
    const_iterator& operator--()   {this->base::operator--();return *this;}
    const_iterator  operator++(int){return const_iterator(this->base::operator++(0),*this);}
    const_iterator  operator--(int){return const_iterator(this->base::operator--(0),*this);}

    const_iterator operator+(std::ptrdiff_t offset) const{return const_iterator(this->base::operator+(offset),*this);}
    const_iterator operator-(std::ptrdiff_t offset) const{return const_iterator(this->base::operator-(offset),*this);}
  };

  class buffer_type{
    const typename Policy::buffer_type& buff;
    Filter filter;
  public:
    buffer_type(const typename Policy::buffer_type& buff,filter_type const& filter)
      :buff(buff),filter(filter){}
  public:
    char_at_type operator[](std::size_t index) const{
      return this->filter(this->buff[index]);
    }
    std::size_t length() const{
      return this->buff.length();
    }
    const_iterator begin() const{
      return const_iterator(this->buff.begin(),this->filter);
    }
    const_iterator end() const{
      return const_iterator(this->buff.end(),this->filter);
    }
    const_iterator begin_at(std::ptrdiff_t index) const{
      return const_iterator(this->buff.begin_at(index),this->filter);
    }
  };
};

template<typename Policy,typename Filter>
struct _strtmp_ranged_map_policy{
  typedef _strtmp_ranged_map_policy   policy_type;
  typedef typename Policy::char_type  char_type;
  typedef char_type                   char_at_type;
  static const bool has_get_ptr=false;

private:
  typedef typename mwg::stdm::remove_reference<Filter>::type filter_type;
  class ranged_filter{
    Filter                  m_filter;
    mwg::range<std::size_t> m_range;
  public:
    ranged_filter(filter_type const& filter,std::size_t start,std::size_t end)
      :m_filter(filter),m_range(start,end){}
    char_at_type operator()(char_type const& value,std::ptrdiff_t index) const{
      if(this->m_range.contains(index))
        return this->m_filter(value);
      else
        return value;
    }
  };

private:
  typedef typename Policy::const_iterator target_iterator;
  typedef typename stdm::conditional<target_iterator::has_index,target_iterator,
    indexible_const_iterator<target_iterator> >::type indexed_iterator;
public:
  class const_iterator:public indexed_iterator{
    typedef indexed_iterator base;

    ranged_filter const& m_filter;

  public:
    const_iterator(indexed_iterator const& iter,ranged_filter const& filter)
      :base(iter),m_filter(filter){}
    const_iterator(indexed_iterator const& iter,const_iterator const& origin)
      :base(iter),m_filter(origin.m_filter){}
    char_at_type operator*() const{
      return this->m_filter(this->base::operator*(),this->base::index());
    }

    const_iterator& operator++()   {this->base::operator++();return *this;}
    const_iterator& operator--()   {this->base::operator--();return *this;}
    const_iterator  operator++(int){return const_iterator(this->base::operator++(0),*this);}
    const_iterator  operator--(int){return const_iterator(this->base::operator--(0),*this);}

    std::ptrdiff_t operator-(const_iterator const& rhs) const{return this->m_iter-rhs.m_iter;}
    const_iterator operator+(std::ptrdiff_t offset) const{return const_iterator(this->base::operator+(offset),*this);}
    const_iterator operator-(std::ptrdiff_t offset) const{return const_iterator(this->base::operator-(offset),*this);}
  };

  class buffer_type{
    const typename Policy::buffer_type& buff;
    ranged_filter m_filter;
  public:
    buffer_type(const typename Policy::buffer_type& buff,filter_type const& filter,std::size_t start,std::size_t end)
      :buff(buff),m_filter(filter,start,end){}
  public:
    char_at_type operator[](std::size_t index) const{
      return this->m_filter(this->buff[index],index);
    }
    std::size_t length() const{
      return this->buff.length();
    }

  private:
    template<typename Iter>
    typename stdm::enable_if<Iter::has_index,Iter const&>::type
    static create_indexed(Iter const& iter,std::ptrdiff_t index){
      return iter;
    }

    template<typename Iter>
    typename stdm::enable_if<!Iter::has_index,indexed_iterator>::type
    static create_indexed(Iter const& iter,std::ptrdiff_t index){
      return indexed_iterator(iter,index);
    }

  public:
    const_iterator begin() const{
      return const_iterator(create_indexed(buff.begin(),0),this->m_filter);
    }
    const_iterator end() const{
      return const_iterator(create_indexed(buff.end(),buff.length()),this->m_filter);
    }
    const_iterator begin_at(std::ptrdiff_t index) const{
      return const_iterator(create_indexed(buff.begin_at(index),index),this->m_filter);
    }
  };
};

template<typename XCH>
struct _filt_tolower{
  typedef XCH char_type;
  char_type operator()(const char_type& c) const{
    return char_traits<char_type>::tolower(c);
  }
};
template<typename XCH>
struct _filt_toupper{
  typedef XCH char_type;
  char_type operator()(const char_type& c) const{
    return char_traits<char_type>::toupper(c);
  }
};
template<typename XCH>
struct _filt_replace_char{
  typedef XCH char_type;
  char_type cS;
  char_type cD;
public:
  _filt_replace_char(char_type const& before,char_type const& after)
    :cS(before),cD(after){}
  char_type operator()(const char_type& c) const{
    return c==cS?cD:c;
  }
};

//-----------------------------------------------------------------------------
// _strtmp_pad_policy                                                  @tmp.pad

template<typename Str>
struct _strtmp_pad_policy{
  typedef _strtmp_pad_policy                      policy_type;
  typedef typename Str::policy_type::char_type    char_type;
  typedef typename Str::policy_type::char_at_type char_at_type;
  static const bool has_get_ptr=false;

  typedef default_const_iterator<policy_type> const_iterator;

  class buffer_type{
    Str const& str;
    std::size_t lpad_len;
    std::size_t m_length;
    char_type c;
  public:
    buffer_type(Str const& str,std::size_t lpad_len,std::size_t len,char_type c)
      :str(str),lpad_len(lpad_len),m_length(len),c(c){}
  public:
    char_at_type operator[](std::size_t index) const{
      std::ptrdiff_t index1=std::ptrdiff_t(index)-this->lpad_len;
      if(0<=index1&&(std::size_t)index1<this->str.length())
        return this->str[index1];
      else
        return this->c;
    }
    std::size_t length() const{
      return this->m_length;
    }
    const_iterator begin() const{return const_iterator(*this,0);}
    const_iterator end()   const{return const_iterator(*this,this->length());}
    const_iterator begin_at(std::ptrdiff_t index) const{return const_iterator(*this,index);}
  };
};

//-----------------------------------------------------------------------------
// _strtmp_cat_policy, operator+                                       @tmp.cat

template<typename Str1,typename Str2,typename Str3>
struct _strtmp_cat_policy{
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
      typename policy_type1::char_at_type,
      typename policy_type2::char_at_type
    >::value&&mwg::stdm::is_same<
      typename policy_type1::char_at_type,
      typename policy_type3::char_at_type
    >::value,
    typename policy_type1::char_at_type,
    typename policy_type1::char_type
  >::type char_at_type;

  static const bool has_get_ptr=false;
  typedef default_const_iterator<policy_type> const_iterator;

  class buffer_type{
    Str1 str1;
    Str2 str2;
    Str3 str3;
    std::size_t len1;
    std::size_t len2;
  public:
    buffer_type(string_type1 const& str1,string_type2 const& str2,string_type3 const& str3)
      :str1(str1),str2(str2),str3(str3),len1(str1.length()),len2(str2.length()){}
  public:
    char_at_type operator[](std::size_t index) const{
      std::ptrdiff_t const index2=std::ptrdiff_t(index)-len1;
      if(index2<0)
        return this->str1[index];
      std::ptrdiff_t const index3=std::ptrdiff_t(index2)-len2;
      if(index3<0)
        return this->str2[index2];
      return this->str3[index3];
    }
    std::size_t length() const{
      return len1+len2+this->str3.length();
    }
    const_iterator begin() const{return const_iterator(*this,0);}
    const_iterator end()   const{return const_iterator(*this,this->length());}
    const_iterator begin_at(std::ptrdiff_t index) const{return const_iterator(*this,index);}
  };
};

template<typename Str1,typename Str2>
struct _strtmp_cat_policy<Str1,Str2>{
  typedef _strtmp_cat_policy         policy_type;
  typedef typename mwg::stdm::remove_reference<Str1>::type string_type1;
  typedef typename mwg::stdm::remove_reference<Str2>::type string_type2;
  typedef typename string_type1::policy_type policy_type1;
  typedef typename string_type2::policy_type policy_type2;

  typedef typename policy_type1::char_type char_type;
  typedef typename mwg::stdm::conditional<
    mwg::stdm::is_same<
      typename policy_type1::char_at_type,
      typename policy_type2::char_at_type>::value,
    typename policy_type1::char_at_type,
    typename policy_type1::char_type >::type char_at_type;

  static const bool has_get_ptr=false;

  typedef default_const_iterator<policy_type> const_iterator;

  class buffer_type{
    Str1 str1;
    Str2 str2;
    std::size_t length1;
  public:
    buffer_type(string_type1 const& str1,string_type2 const& str2)
      :str1(str1),str2(str2),length1(str1.length()){}
  public:
    char_at_type operator[](std::size_t index) const{
      std::ptrdiff_t const index2=std::ptrdiff_t(index)-length1;
      if(index2<0)
        return this->str1[index];
      else
        return this->str2[index2];
    }
    std::size_t length() const{
      return length1+this->str2.length();
    }
    const_iterator begin() const{return const_iterator(*this,0);}
    const_iterator end()   const{return const_iterator(*this,this->length());}
    const_iterator begin_at(std::ptrdiff_t index) const{return const_iterator(*this,index);}
  };
};

template<typename Str1,typename Str2>
struct concat_enabler{};

template<typename StrP1,typename StrP2>
struct concat_enabler<strbase<StrP1>,strbase<StrP2> >:mwg::stdm::enable_if<
  mwg::stdm::is_same<typename StrP1::char_type,typename StrP2::char_type >::value,
  strbase<_strtmp_cat_policy<strbase<StrP1> const&,strbase<StrP2> const&> >
  >{};
template<typename StrP1,typename XStr2>
struct concat_enabler<strbase<StrP1>,XStr2 >:mwg::stdm::enable_if<
  adapter_traits<XStr2,typename StrP1::char_type>::adaptable,
  strbase<_strtmp_cat_policy<strbase<StrP1> const&,stradp<typename StrP1::char_type> > >
  >{};
template<typename XStr1,typename StrP2>
struct concat_enabler<XStr1,strbase<StrP2> >:mwg::stdm::enable_if<
  adapter_traits<XStr1,typename StrP2::char_type>::adaptable,
  strbase<_strtmp_cat_policy<stradp<typename StrP2::char_type>,strbase<StrP2> const&> >
  >{};

template<typename StrP1,typename StrP2>
typename concat_enabler<strbase<StrP1>,strbase<StrP2> >::type
operator+(strbase<StrP1> const& lhs,strbase<StrP2> const& rhs){
  typedef typename concat_enabler<strbase<StrP1>,strbase<StrP2> >::type return_type;
  return return_type(lhs,rhs);
}
template<typename StrP1,typename XStr2>
typename concat_enabler<strbase<StrP1>,XStr2 >::type
operator+(strbase<StrP1> const& lhs,XStr2 const& rhs){
  typedef typename concat_enabler<strbase<StrP1>,XStr2 >::type return_type;
  return return_type(lhs,rhs);
}
template<typename XStr1,typename StrP2>
typename concat_enabler<XStr1,strbase<StrP2> >::type
operator+(XStr1 const& lhs,strbase<StrP2> const& rhs){
  typedef typename concat_enabler<XStr1,strbase<StrP2> >::type return_type;
  return return_type(lhs,rhs);
}

#pragma%x begin_test
void test(){
  typedef mwg::stradp<char> _a;
  mwg_assert((_a("hello")+_a(" world")=="hello world"));
  mwg_assert((_a("hello")+" world"=="hello world"));
  mwg_assert(("hello"+_a(" world")+"!"=="hello world!"));
}
#pragma%x end_test

//-----------------------------------------------------------------------------
// reverse, repeat                                         @tmp.rev @tmp.repeat

template<typename Str>
struct _strtmp_reverse_policy{
  typedef _strtmp_reverse_policy                  policy_type;
  typedef typename Str::policy_type::char_type    char_type;
  typedef typename Str::policy_type::char_at_type char_at_type;
  static const bool has_get_ptr=false;

  typedef default_const_iterator<policy_type> const_iterator;

  class buffer_type{
    Str const& str;
  public:
    buffer_type(Str const& str):str(str){}
  public:
    char_at_type operator[](std::size_t index) const{
      return this->str[this->str.length()-1-index];
    }
    std::size_t    length() const{return this->str.length();}
    const_iterator begin()  const{return const_iterator(*this,0);}
    const_iterator end()    const{return const_iterator(*this,this->length());}
    const_iterator begin_at(std::ptrdiff_t index) const{return const_iterator(*this,index);}
  };
};

template<typename Str>
struct _strtmp_repeat_policy{
  typedef _strtmp_repeat_policy                   policy_type;
  typedef typename Str::policy_type::char_type    char_type;
  typedef typename Str::policy_type::char_at_type char_at_type;
  static const bool has_get_ptr=false;

  typedef default_const_iterator<policy_type> const_iterator;

  class buffer_type{
    Str const& m_str;
    std::size_t m_repeatCount;
  public:
    buffer_type(Str const& str,std::size_t repeatCount)
      :m_str(str),m_repeatCount(repeatCount){}
  public:
    char_at_type operator[](std::size_t index) const{
      return this->m_str[index%this->m_str.length()];
    }
    std::size_t    length() const{return this->m_str.length()*this->m_repeatCount;}
    const_iterator begin()  const{return const_iterator(*this,0);}
    const_iterator end()    const{return const_iterator(*this,this->length());}
    const_iterator begin_at(std::ptrdiff_t index) const{return const_iterator(*this,index);}
  };
};

//-----------------------------------------------------------------------------
// 関係演算子                                                           @op.rel

#pragma%m mwg::string::compare::doc
/*?lwiki
 * :@op s1=={==}==s2;
 * :@op s1=={!=}==s2;
 * :@op s1=={<=}==s2;
 * :@op s1=={>=}==s2;
 * :@op s1==<==s2;
 * :@op s1==>==s2;
 *  文字列を比較します。
 * :@fn ==compare==(s1,s2);
 *  二つの文字列を比較します。`s1>s2` の時 `1`, `s1==s2` の時 `0`, `s1<s2` の時 `-1` を返します。
 *  c.f. `strcmp`/`strncmp` (C), `lexicographical_compare` (Boost), \
 *  `Compare` (ATL/MFC), <?cs CompareOriginal?> (CLR), <?java compareTo?> (Java), <?rb operator<=>?> (Ruby)
 * :@fn ==icompare==(s1,s2);
 *  ASCII 大文字・小文字を区別せずに、二つの文字列を比較します。
 *  c.f. `stricmp`/`strcasecmp`/`strnicmp`/`strncasecmp` (C), `ilexicographical_compare` (Boost), \
 *  `CompareNoCase` (ATL/MFC), <?cs Compare?> (CLR), <?java compareToIgnoreCase?> (Java), <?rb casecmp?> (Ruby)
 * :他
 *  c.f. `strcoll`/`wcscoll` (C), `Collate`/`CollateNoCase` (ATL/MFC)
 */
#pragma%end

template<typename StrP1,typename StrP2,typename Ret>
struct compare_enabler:mwg::stdm::enable_if<
  mwg::stdm::is_same<typename StrP1::char_type,typename StrP2::char_type>::value,Ret>{};

template<typename StrP1,typename StrP2>
typename compare_enabler<StrP1,StrP2,int>::type
compare(strbase<StrP1> const& lhs,strbase<StrP2> const& rhs){
  typename StrP1::const_iterator i=lhs.begin(),iN=lhs.end();
  typename StrP2::const_iterator j=rhs.begin(),jN=rhs.end();
  for(;i!=iN&&j!=jN;++i,++j)
    if(*i!=*j)return *i>*j?1:-1;
  return i!=iN?1: j!=jN?-1: 0;
}

template<typename StrP1,typename StrP2>
typename compare_enabler<StrP1,StrP2,int>::type
icompare(strbase<StrP1> const& lhs,strbase<StrP2> const& rhs){
  return compare(lhs.tolower(),rhs.tolower());
}

template<typename StrP1,typename StrP2>
typename compare_enabler<StrP1,StrP2,bool>::type
operator==(strbase<StrP1> const& lhs,strbase<StrP2> const& rhs){
  if(lhs.length()!=rhs.length())return false;
  typename StrP1::const_iterator i=lhs.begin(),iN=lhs.end();
  typename StrP2::const_iterator j=rhs.begin();
  for(;i!=iN;++i,++j)
    if(*i!=*j)return false;
  return true;
}

template<typename StrP1,typename StrP2>
typename compare_enabler<StrP1,StrP2,bool>::type
operator!=(strbase<StrP1> const& lhs,strbase<StrP2> const& rhs){
  return !(lhs==rhs);
}

template<typename StrP1,typename StrP2>
typename compare_enabler<StrP1,StrP2,bool>::type
operator<(strbase<StrP1> const& lhs,strbase<StrP2> const& rhs){
  typename StrP1::const_iterator i=lhs.begin(),iN=lhs.end();
  typename StrP2::const_iterator j=rhs.begin(),jN=rhs.end();
  for(;i!=iN&&j!=jN;++i,++j)
    if(*i!=*j)return *i<*j;
  return i==iN&&j!=jN;
}

template<typename StrP1,typename StrP2>
typename compare_enabler<StrP1,StrP2,bool>::type
operator>(strbase<StrP1> const& lhs,strbase<StrP2> const& rhs){
  return rhs<lhs;
}

template<typename StrP1,typename StrP2>
typename compare_enabler<StrP1,StrP2,bool>::type
operator<=(strbase<StrP1> const& lhs,strbase<StrP2> const& rhs){
  return !(rhs<lhs);
}

template<typename StrP1,typename StrP2>
typename compare_enabler<StrP1,StrP2,bool>::type
operator>=(strbase<StrP1> const& lhs,strbase<StrP2> const& rhs){
  return !(lhs<rhs);
}


template<typename StrP1,typename XStr,typename Ret>
struct compare_enabler2:mwg::stdm::enable_if<
  adapter_traits<XStr,typename StrP1::char_type>::adaptable,Ret>{};

#define MWG_STRING3_STRING_H__overload_compare_adapter(Return,FunctionName) \
template<typename StrP1,typename XStr> \
typename compare_enabler2<StrP1,XStr,Return>::type \
FunctionName(strbase<StrP1> const& lhs,XStr const& rhs){ \
  return FunctionName(lhs,stradp<typename StrP1::char_type>(rhs)); \
} \
template<typename StrP1,typename XStr> \
typename compare_enabler2<StrP1,XStr,Return>::type \
FunctionName(XStr const& lhs,strbase<StrP1> const& rhs){ \
  return FunctionName(stradp<typename StrP1::char_type>(lhs),rhs); \
}
  MWG_STRING3_STRING_H__overload_compare_adapter(int,compare)
  MWG_STRING3_STRING_H__overload_compare_adapter(int,icompare)
  MWG_STRING3_STRING_H__overload_compare_adapter(bool,operator==)
  MWG_STRING3_STRING_H__overload_compare_adapter(bool,operator!=)
  MWG_STRING3_STRING_H__overload_compare_adapter(bool,operator<=)
  MWG_STRING3_STRING_H__overload_compare_adapter(bool,operator>=)
  MWG_STRING3_STRING_H__overload_compare_adapter(bool,operator<)
  MWG_STRING3_STRING_H__overload_compare_adapter(bool,operator>)
#undef MWG_STRING3_STRING_H__overload_compare_adapter

#pragma%x begin_test
void test(){
  typedef mwg::stradp<char> _a;

  mwg_assert(compare(_a("hello"),_a("hello"))== 0);
  mwg_assert(compare(_a("hello"),   "hello" )== 0);
  mwg_assert(compare(   "hello" ,_a("hello"))== 0);
  mwg_assert(compare(_a("hello"),_a("world"))==-1);
  mwg_assert(compare(_a("hello"),   "world" )==-1);
  mwg_assert(compare(   "hello" ,_a("world"))==-1);
  mwg_assert(compare(_a("world"),_a("hello"))== 1);
  mwg_assert(compare(   "world" ,_a("hello"))== 1);
  mwg_assert(compare(_a("world"),   "hello" )== 1);
  mwg_assert(compare(_a("hello"),   "hell"  )== 1);
  mwg_assert(compare(_a("hell" ),   "hello ")==-1);

  // assume ASCII codes
  mwg_assert( compare(_a("hello"),_a("HELLO"))== 1);
  mwg_assert( compare(_a("hello"),_a("WORLD"))== 1);
  mwg_assert( compare(_a("WORLD"),_a("hello"))==-1);
  mwg_assert(icompare(_a("hello"),_a("HELLO"))== 0);
  mwg_assert(icompare(_a("hello"),_a("WORLD"))==-1);
  mwg_assert(icompare(_a("WORLD"),_a("hello"))== 1);

  mwg_assert( (_a("hello")=="hello"));
  mwg_assert(!(_a("hello")!="hello"));
  mwg_assert(!(_a("hello")=="world"));
  mwg_assert( (_a("hello")!="world"));
  mwg_assert(!(_a("hello")=="hell"));
  mwg_assert( (_a("hello")!="hell"));

  mwg_assert(!(_a("hello")<"hello"));
  mwg_assert(!(_a("hello")>"hello"));
  mwg_assert( (_a("hello")<"world"));
  mwg_assert(!(_a("hello")>"world"));
  mwg_assert(!(_a("hello")<"hell"));
  mwg_assert( (_a("hello")>"hell"));

  mwg_assert( (_a("hello")<="hello"));
  mwg_assert( (_a("hello")>="hello"));
  mwg_assert( (_a("hello")<="world"));
  mwg_assert(!(_a("hello")>="world"));
  mwg_assert(!(_a("hello")<="hell"));
  mwg_assert( (_a("hello")>="hell"));

  mwg::string<char> s1;
  mwg_assert( (s1==""));
  mwg::string<char> s2="012345";
  mwg_assert( (s2=="012345"));
  s1="21345";
  mwg_assert( (s1=="21345"));
}
#pragma%x end_test

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
} /* end of namespace string3_detail */
  using string3_detail::string;
  using string3_detail::strsub;
  using string3_detail::stradp;
} /* end of namespace mwg */

//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
//
// char support
//
#include <cstring>
#include <cctype>

namespace mwg{
namespace string3_detail{

template<>
struct char_traits<char>{
  typedef char char_type;
  static std::size_t strlen(const char_type* str){
    return std::strlen(str);
  }
  static mwg_constexpr char_type null(){return '\0';}
  static mwg_constexpr char_type space(){return ' ';}
  static mwg_constexpr char_type tolower(char_type c){
    return 'A'<=c&&c<='Z'?char_type(c+('a'-'A')):c;
  }
  static mwg_constexpr char_type toupper(char_type c){
    return 'a'<=c&&c<='z'?char_type(c+('A'-'a')):c;
  }
  static mwg_constexpr bool isspace(char_type c){
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

namespace mwg{
namespace string3_detail{

template<>
struct char_traits<wchar_t>{
  typedef wchar_t char_type;
  static std::size_t strlen(const char_type* str){
    return std::wcslen(str);
  }
  static mwg_constexpr char_type null(){return L'\0';}
  static mwg_constexpr char_type space(){return L' ';}
  static mwg_constexpr char_type tolower(char_type c){
    return L'A'<=c&&c<=L'Z'?char_type(c+(L'a'-L'A')):c;
  }
  static mwg_constexpr char_type toupper(char_type c){
    return L'a'<=c&&c<=L'z'?char_type(c+(L'A'-L'a')):c;
  }
  static mwg_constexpr bool isspace(char_type c){
    return std::iswspace(c);
  }
};

}
}

#pragma%x begin_test
void test(){
  mwg_assert((mwg::stradp<wchar_t>(L"AbCdE").toupper(1,-1)==L"ABCDE"));
  mwg_assert((mwg::stradp<wchar_t>(L"aBcDe").tolower(1,-1)==L"abcde"));
  mwg_assert((mwg::stradp<wchar_t>(L"  hello  ").trim()==L"hello"));
  mwg_assert((mwg::stradp<wchar_t>(L"  hello  ").ltrim()==L"hello  "));
  mwg_assert((mwg::stradp<wchar_t>(L"  hello  ").rtrim()==L"  hello"));
  mwg_assert((mwg::stradp<wchar_t>(L"world").pad(7)==L" world "));
}
#pragma%x end_test

//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
//
// std::basic_string support
//
#include <string>

namespace mwg{
namespace string3_detail{

template<typename XCH,typename Tr,typename Alloc>
struct adapter_traits<std::basic_string<XCH,Tr,Alloc> >{
  typedef XCH char_type;
  static const XCH* pointer(std::basic_string<XCH,Tr,Alloc> const& str){
    return str.c_str();
  }
  static std::size_t length(std::basic_string<XCH,Tr,Alloc> const& str){
    return str.length();
  }
};

}
}
#pragma%x begin_test
void test(){
  std::string s1("hello");
  mwg_assert((mwg::stradp<char>(s1).toupper(1,4)=="hELLo"));
}
#pragma%x end_test

//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
/*?lwiki
 * *文字列基本機能 (`mwg::string3_detail::strbase`)
 */
#pragma%x mwg::string::strbase::doc
/*?lwiki
 * ***定義: <?cpp* '''range-spec'''?>
 * 以下に繰り返し現れる仮引数として <?cpp* '''range-spec'''?> を定義します。
 * <?cpp* function('''range-spec''')?> は、関数が以下の3つの多重定義を持つことを表します。
 * -`function(i)`
 * -`function(i,j)`
 * -`function(r)`
 * 仮引数 <?cpp* '''range-spec'''?> は、文字列内部の範囲を指定するために用います。
 * それぞれの仮引数は以下の様に指定します。
 * :@param[in] std::ptrdiff_t i,j;
 *  それぞれ範囲の開始位置と終端位置を指定します。
 *  負の数が指定された場合は文字列末端からの相対位置と解釈します。
 *  `mwg::npos` が指定された場合は文字列末端と解釈します。
 *  終端位置が省略された場合は、文字列末端を意味します。
 * :@param[in] mwg::range_i r;
 *  範囲を指定します。開始位置、終端位置は上記 `i`, `j` と同様に解釈されます。
 *
 * **連結・切り出し・挿入・削除
 * :@op s1==+==s2
 *  文字列を連結します。
 */
#pragma%x mwg::string::slice::doc
#pragma%x mwg::string::insert::doc
/*?lwiki
 *
 * **分割・結合
 * :@fn [TODO] s.split(i=0,opt=0);
 * :@fn [TODO] s.split(s1|reg|ch|pred,i=0,opt=0);
 * :@fn [TODO] s.rsplit(i=0,opt=0);
 * :@fn [TODO] s.rsplit(s1|reg|ch|pred,i=-1,opt=0);
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
 *  c.f. split/iter_split (Boost), Split (CLR), split (Java), split (JavaScript), <?rb split?> (Ruby, CLX), \
 *  <?rb partition?>/<?rb rpartition?> (Ruby), split (awk), `split_if` (CLX), `Tokenize(cset,int&)` (ATL/MFC)
 * :@fn [TODO] arr.==join==();
 * :@fn [TODO] arr.==join==(s1); // s1 分割文字列
 *  文字列の集合を連結します。
 *  c.f. `join`/`join_if` (Boost), <?rb join?> (Ruby, CLX)
 *
 * **文字の変換
 */
#pragma%x mwg::string::map::doc
/*?lwiki
 *
 * **空白・幅調節
 */
#pragma%x mwg::string::trim::doc
#pragma%x mwg::string::pad::doc
/*?lwiki
 *
 * **判定
 */
#pragma%x mwg::string::compare::doc
#pragma%x mwg::string::starts::doc
/*?lwiki
 *
 * **検索・置換
 */
#pragma%x mwg::string::find::doc
/*?lwiki
 * :@fn [TODO] s.==replace==(s1,s2,n=mwg::npos); // 文字列を置換
 * :@fn [TODO] s.==replace1==(s1,s2,n=0); // 文字列を置換
 *  c.f. `Replace(s1,s2)` (ATL/MFC), `replace`/`replace_all` (CLX), <?js replace?> (JavaScript)
 *
 * **正規表現
 * :@fn [TODO] s.==replace==(reg,s2); // reg 正規表現
 * :@fn [TODO] s.==replace==(reg,fun); // fun 置換後の文字列を決める関数
 *  c.f. `replace_all`/`replace_regex`/`replace_first`/`replace_last` (Boost), <?cs Relace?> (CLR, mwg-string), \
 *    <?java replace?> (Java, JavaScript), <?java replaceAll?>/<?java replaceFirst?> (Java), \
 *    <?awk sub?>/<?awk gsub?> (awk, Ruby), <?mk subst?>/<?mk patsubst?> (Makefile).
 *  -Boost の replace_nth に対応する関数は、それ程有用とは思われないので提供しない。
 *  -Boost の replace_regex_all, replace_head, replace_tail, \
 *   ireplace_first, ireplace_last, ireplace_nth, ireplace_all \
 *   に対応する関数は正規表現及びそのフラグを用いて表現できるので提供しない。
 *   ■→最適化の観点から行くと head, tail は有用かも知れない。
 *  -Boost の erase_all, erase_regex, erase_regex_all, erase_head, \
 *   erase_tail, erase_first, erase_last, erase_nth, \
 *   ierase_first, ierase_last, ierase_nth, ierase_all \
 *   に対応する関数は置換後の文字列に "" を指定すれば良いだけなので提供しない。
 * :@fn [TODO] s1.==match==(reg,&color(red){[}'''range-spec'''&color(red){]}); // reg 正規表現
 * :@fn [TODO] s1.==rmatch==(reg,&color(red){[}'''range-spec'''&color(red){]});
 * :@fn [TODO] s1.==match_at==(reg,&color(red){[}'''range-spec'''&color(red){]});
 * :@fn [TODO] s1.==rmatch_at==(reg,&color(red){[}'''range-spec'''&color(red){]});
 *  正規表現に対する一致を試す。
 *  -match: 先頭から順に一致を試す。
 *  -rmatch: 末尾から順に一致を試す。
 *  -match_at: 先頭を含む部分列に対してだけ、一致を試す。
 *  -rmatch_at: 末端を含む部分列に対してだけ、一致を試す。
 *  c.f. Search/SearchR/Match/MatchAt (mwg-string), match (JavaScript), match (awk)
 * :@fn [TODO] match_iterator
 *  c.f. find_all/iter_find (Boost), scan (Ruby), match (JavaScript)
 *
 * **他
 */
#pragma%x mwg::string::misc::doc
/*?lwiki
 * :format, operator%
 *  c.f. `sprintf` (C), `AppendFormat, Format, FormatV, FormatMessage, FormatMessageV` (ATL/MFC), `Format` (CLR), format (Java), <?awk sprintf?> (awk, Perl), operator% (Ruby)
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
mwg::string では、文字列の内部形式と文字列に対する操作を分離して実装しています。\
文字列の内部形式は `StringPolicy` を用いて定義されます。\
文字列に対する操作は `strbase<StringPolicy>` によって提供されます。\
ここでは、`StringPolicy` を定義して、新しい文字列の内部形式を追加する方法を説明します。

`StringPolicy` は以下の様なメンバを持つクラスとして定義します。
&pre(!cpp){
#pragma%x mwg::string::policy_requirements
}
:@class class StringPolicy;
 :@class char_type;
  単一の文字を表現する型です。
 :@class char_at_type;
  文字列中への文字を取得する際の型です。\
  内部表現に対応するデータが存在する場合には、そのデータへの参照 (`char_type const&`) になります。\
  それ以外の場合は、単に `char_type` になります。
 :@class buffer_type;
  文字列の内部表現を格納する型です。
 :@class const_iterator;
  文字列に含まれる文字を列挙する反復子です。
 :@var static const bool has_get_ptr;
  文字データが連続した領域に格納され、その先頭へのポインタが得られる場合に true を指定します。

 */
//HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
#endif
#pragma%x begin_test
void test(){
  typedef mwg::stradp<char> _a;
  mwg_assert( (_a("HELLO").repeat(3).tolower(5,-3).reverse()=="OLLehollehOLLEH"));
}
#pragma%x end_test
#pragma%x begin_check
// namespace string_bench{
//   int test_compare1();
//   void test(){
//     for(mwg::i8t i=0;i<10000000LL;i++)
//       string_bench::test_compare1();
//   }
// }

int main(){
  managed_test::run_tests();
  return 0;
}
#pragma%x end_check
