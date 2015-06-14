// -*- mode:C++;coding:utf-8-dos -*-
#pragma once
#ifndef MWG_EXP_STRING3_STRING_H
#define MWG_EXP_STRING3_STRING_H
//HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
#include <cstddef>
#include <cstring>
#include <string>
#include <mwg/std/utility>
#include <mwg/std/type_traits>
#include <mwg/std/memory>
#include <mwg/mpl.h>           /* mwg::mpl::integral_limits */
#include <mwg/range.h>
namespace mwg{
namespace string3_detail{
  template<typename XCH>
  struct char_traits;
  template<typename Policy>
  struct strbase;

  template<typename XCH>
  class strsub;
  template<typename XCH>
  class stradp;
  template<typename XCH>
  class string;

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

}

  static const std::ptrdiff_t npos
    =mwg::mpl::integral_limits<std::ptrdiff_t>::min_value;
}
namespace mwg{
namespace string3_detail{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  inline std::size_t canonicalize_index(std::ptrdiff_t const& index,std::size_t const& len){
    if(index==mwg::npos){
      return len;
    }else if(index<0){
      std::size_t const _index=index+len;
      return _index>=0?_index:0;
    }else
      return index<len?index:len;
  }

#if 0
struct str_policy{
  typedef char        char_type;
  typedef const char& char_at_type; // e.g. char, const char&
  typedef str_policy  policy_type;

  static const bool has_get_ptr;

  struct const_iterator{
    char_at_type operator*() const;

    const_iterator& operator++();
    const_iterator operator++(int);
  };

  struct buffer_type{
    char_at_type operator[]() const;
    // or: const char_type& operator[]{} const;

    std::size_t length() const;

    const_iterator begin() const;
    const_iterator end() const;

    [[if(has_get_ptr)]]
    const char_type* get_ptr() const;
  };
};
#endif

//-----------------------------------------------------------------------------
//  char_traits

// Tr 要件
template<typename XCH>
struct char_traits{
  typedef XCH char_type;

  static std::size_t strlen(const char_type* str);
  static mwg_constexpr char_type null();
};

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
    return ::isspace(c);
  }

};

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

//-----------------------------------------------------------------------------
//  predicaters

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

template<typename StrP1>
class default_const_iterator{
  typedef StrP1 policy_type;
  typedef typename policy_type::char_at_type char_at_type;
  typedef typename policy_type::buffer_type  buffer_type;

  buffer_type const& data;
  std::size_t index;
public:
  default_const_iterator(buffer_type const& data,std::size_t index)
    :data(data),index(index){}
  char_at_type operator*() const{
    return data[index];
  }
  default_const_iterator& operator++(){
    this->index++;
    return *this;
  }
  default_const_iterator operator++(int){
    default_const_iterator ret(*this);
    this->index++;
    return ret;
  }
  default_const_iterator& operator--(){
    this->index--;
    return *this;
  }
  default_const_iterator operator--(int){
    default_const_iterator ret(*this);
    this->index--;
    return ret;
  }

  bool operator==(default_const_iterator const& rhs) const{
    return this->index==rhs.index;
  }
  bool operator!=(default_const_iterator const& rhs) const{
    return this->index!=rhs.index;
  }
  bool operator<(default_const_iterator const& rhs) const{
    return this->index<rhs.index;
  }
  bool operator<=(default_const_iterator const& rhs) const{
    return this->index<=rhs.index;
  }
  bool operator>(default_const_iterator const& rhs) const{
    return this->index>rhs.index;
  }
  bool operator>=(default_const_iterator const& rhs) const{
    return this->index>=rhs.index;
  }
};

//-----------------------------------------------------------------------------
// strbase

template<typename XCH>
struct strbase_tag{};

template<typename Policy>
struct strbase:strbase_tag<typename Policy::char_type>{
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
# include "i001.strbase_constructor_variadic.inl"
#endif

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

  const_iterator begin() const{return data.begin();}
  const_iterator end()   const{return data.end();}

  typedef typename mwg::stdm::conditional<
    policy_type::has_get_ptr,strsub<char_type>,strbase<_strtmp_sub_policy<policy_type> > >::type slice_return_type;
  slice_return_type slice(std::ptrdiff_t start,std::ptrdiff_t end=mwg::npos) const{
    std::size_t const len=this->length();
    std::size_t const _start=canonicalize_index(start,len);
    std::size_t _end=canonicalize_index(end,len);
    if(_start>_end)_end=_start;
    return slice_return_type(this->data,_start,_end-_start);
  }
  slice_return_type slice(mwg::range_i const& range) const{
    return slice(range.begin(),range.end());
  }
  slice_return_type substr(std::ptrdiff_t start,std::size_t length) const{
    std::size_t const len=this->length();
    std::size_t const _start=canonicalize_index(start,len);
    std::size_t _end=canonicalize_index(start+length,len);
    if(_start>_end)_end=_start;
    return slice_return_type(this->data,_start,_end-_start);
  }
  char_type head() const{
    std::size_t const _len=this->length();
    return _len==0?char_traits_type::null():data[0];
  }
  slice_return_type head(std::size_t len) const{
    std::size_t const _len=this->length();
    if(len>_len)len=_len;
    return slice_return_type(this->data,0,len);
  }
  char_type tail() const{
    std::size_t const _len=this->length();
    return _len==0?char_traits_type::null():data[_len-1];
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
  remove_return_type remove(mwg::range_i const& range) const{
    return this->remove(range.begin(),range.end());
  }

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
  ranged_tolower_return_type tolower(mwg::range_i const& range) const{
    return tolower(range.begin(),range.end());
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
  ranged_toupper_return_type toupper(mwg::range_i const& range) const{
    return toupper(range.begin(),range.end());
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
  typename ranged_map_enabler<F>::type map(F const& filter,mwg::range_i const& range) const{
    return map(filter,range.begin(),range.end());
  }

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
    std::size_t _len=this->length();
    std::size_t i=0;
    while(i<_len&&_f::invoke(pred,data[i]))i++;
    std::size_t j=_len-1;
    while(j>i&&_f::invoke(pred,data[j]))j--;
    return slice_return_type(this->data,i,j+1-i);
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
    std::size_t _len=this->length();
    std::size_t i=0;
    while(i<_len&&_f::invoke(pred,data[i]))i++;
    return slice_return_type(this->data,i,_len-i);
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
    std::size_t _len=this->length();
    std::size_t j=_len-1;
    while(j>=0&&_f::invoke(pred,data[j]))j--;
    return slice_return_type(this->data,0,j+1);
  }

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
    std::ptrdiff_t i=this->length()-str.length();
    if(i<0)return false;
    typename strbase<StrP>::const_iterator j=str.begin(),jN=str.end();
    for(;j!=jN;++i,++j)
      if(this->data[i]!=*j)return false;
    return true;
  }
  template<typename XStr>
  typename adapter_enabler<char_type,XStr,bool>::type
  ends(XStr const& str) const{
    return this->ends(stradp<char_type>(str));
  }

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

  template<typename StrP>
  bool _find_match_at(std::size_t index,strbase<StrP> const& str) const{
    typename strbase<StrP>::const_iterator j=str.begin(),jN=str.end();
    for(;j!=jN;++index,++j)
      if(this->data[index]!=*j)
        return false;
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
  template<typename Pred>
  std::ptrdiff_t _find_pred(Pred const& pred,std::ptrdiff_t i,std::ptrdiff_t iN) const{
    typedef mwg::functor_traits<Pred,bool(char_type)> _f;
    for(;i<iN;++i)
      if(_f::invoke(pred,this->data[i]))return i;
    return -1;
  }
  template<typename Pred>
  std::ptrdiff_t _rfind_pred(Pred const& pred,std::ptrdiff_t i,std::ptrdiff_t iN) const{
    typedef mwg::functor_traits<Pred,bool(char_type)> _f;
    for(;--iN>=i;)
      if(_f::invoke(pred,this->data[i]))return i;
    return -1;
  }
  template<typename StrP>
  std::ptrdiff_t _find_any_impl(strbase<StrP> const& str,std::ptrdiff_t i,std::ptrdiff_t iN) const{
    return this->_find_pred(_pred_any_of_str<char_type,strbase<StrP> >(str));
  }
  template<typename StrP>
  std::ptrdiff_t _find_not_impl(strbase<StrP> const& str,std::ptrdiff_t i,std::ptrdiff_t iN) const{
    return this->_find_pred(_pred_not_of_str<char_type,strbase<StrP> >(str));
  }
  template<typename StrP>
  std::ptrdiff_t _rfind_any_impl(strbase<StrP> const& str,std::ptrdiff_t i,std::ptrdiff_t iN) const{
    return this->_rfind_pred(_pred_any_of_str<char_type,strbase<StrP> >(str));
  }
  template<typename StrP>
  std::ptrdiff_t _rfind_not_impl(strbase<StrP> const& str,std::ptrdiff_t i,std::ptrdiff_t iN) const{
    return this->_rfind_pred(_pred_not_of_str<char_type,strbase<StrP> >(str));
  }
public:
#define MWG_STRING2_STRING_H__define_find_overloads(FIND) \
  template<typename T> \
  typename find_enabler<T,1>::type FIND(T const& str) const{ \
    return this->_##FIND##_impl(str,0,this->length()); \
  } \
  template<typename T> \
  typename find_enabler<T,2>::type FIND(T const& str) const{ \
    return this->FIND(stradp<char_type>(str)); \
  } \
  template<typename T> \
  typename find_enabler<T,1>::type FIND(T const& str,std::ptrdiff_t start,std::ptrdiff_t end=mwg::npos) const{ \
    std::size_t const _len=this->length(); \
    return this->_##FIND##_impl(str,canonicalize_index(start,_len),canonicalize_index(end,_len)); \
  } \
  template<typename T> \
  typename find_enabler<T,2>::type FIND(T const& str,std::ptrdiff_t start,std::ptrdiff_t end=mwg::npos) const{ \
    return this->FIND(stradp<char_type>(str),start,end); \
  } \
  template<typename T> \
  typename find_enabler<T,1>::type FIND(T const& str,mwg::range_i const& range) const{ \
    return this->FIND(str,range.begin(),range.end()); \
  } \
  template<typename T> \
  typename find_enabler<T,2>::type FIND(T const& str,mwg::range_i const& range) const{ \
    return this->FIND(stradp<char_type>(str),range.begin(),range.end()); \
  }

  MWG_STRING2_STRING_H__define_find_overloads(find)
  MWG_STRING2_STRING_H__define_find_overloads(find_any)
  MWG_STRING2_STRING_H__define_find_overloads(find_not)
  MWG_STRING2_STRING_H__define_find_overloads(rfind)
  MWG_STRING2_STRING_H__define_find_overloads(rfind_any)
  MWG_STRING2_STRING_H__define_find_overloads(rfind_not)
#undef MWG_STRING2_STRING_H__define_find_overloads

  template<typename T>
  typename find_enabler<T,3>::type find(T const& pred) const{
    return this->_find_pred(pred,0,this->length());
  }
  template<typename T>
  typename find_enabler<T,3>::type rfind(T const& pred) const{
    return this->_rfind_pred(pred,0,this->length());
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
  ranged_char_replace_return_type replace(char_type const& before,char_type const& after,mwg::range_i const& range) const{
    return this->replace(before,after,range.begin(),range.end());
  }

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
  replace(mwg::range_i const& range,T const& str) const{
    return this->replace(range.begin(),range.end(),str);
  }
  template<typename T>
  typename range_replace_enabler<T,2>::type
  replace(mwg::range_i const& range,T const& str) const{
    return this->replace(range.begin(),range.end(),str);
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
};

//-----------------------------------------------------------------------------

template<typename XCH>
struct strsub_policy{
  typedef XCH              char_type;
  typedef const char_type& char_at_type;
  typedef strsub_policy    policy_type;
  typedef const char_type* const_iterator;
  static const bool has_get_ptr=true;

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
  friend struct strbase;

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

template<typename XCH>
struct string_policy{
  typedef string_policy    policy_type;
  typedef XCH              char_type;
  typedef const char_type& char_at_type;
  static const bool has_get_ptr=true;

  typedef const char_type* const_iterator;

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
      char* const data=ptr->data;
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

//-----------------------------------------------------------------------------

template<typename StrP1>
struct _strtmp_sub_policy{
  typedef _strtmp_sub_policy           policy_type;
  typedef typename StrP1::char_type    char_type;
  typedef typename StrP1::char_at_type char_at_type;
  static const bool has_get_ptr=false;

  struct buffer_type;
  typedef default_const_iterator<_strtmp_sub_policy> const_iterator;

  struct buffer_type{
    const typename StrP1::buffer_type& buff;
    std::size_t start;
    std::size_t len;
  public:
    buffer_type(const typename StrP1::buffer_type& buff,std::size_t start,std::size_t length)
      :buff(buff),start(start),len(length){}
  public:
    char_at_type operator[](std::size_t index) const{
      return buff[start+index];
    }
    std::size_t length() const{
      return this->length;
    }
    const_iterator begin() const{
      return const_iterator(*this,0);
    }
    const_iterator end() const{
      return const_iterator(*this,this->len);
    }
  };
};

template<typename StrP,typename Filter>
struct _strtmp_map_policy{
  typedef _strtmp_map_policy       policy_type;
  typedef typename StrP::char_type char_type;
  typedef char_type                char_at_type;
  static const bool has_get_ptr=false;

  typedef default_const_iterator<_strtmp_map_policy> const_iterator;

  struct buffer_type{
    const typename StrP::buffer_type& buff;
    Filter filter;
    typedef typename mwg::stdm::remove_reference<Filter>::type filter_type;
  public:
    buffer_type(const typename StrP::buffer_type& buff,filter_type const& filter)
      :buff(buff),filter(filter){}
  public:
    char_at_type operator[](std::size_t index) const{
      return this->filter(this->buff[index]);
    }
    std::size_t length() const{
      return this->buff.length();
    }
    const_iterator begin() const{
      return const_iterator(*this,0);
    }
    const_iterator end() const{
      return const_iterator(*this,this->length());
    }
  };
};

template<typename StrP,typename Filter>
struct _strtmp_ranged_map_policy{
  typedef _strtmp_ranged_map_policy policy_type;
  typedef typename StrP::char_type  char_type;
  typedef char_type                 char_at_type;
  static const bool has_get_ptr=false;

  typedef default_const_iterator<policy_type> const_iterator;

  struct buffer_type{
    const typename StrP::buffer_type& buff;
    Filter filter;
    mwg::range_t<std::size_t> range;
  public:
    typedef typename mwg::stdm::remove_reference<Filter>::type filter_type;
    buffer_type(const typename StrP::buffer_type& buff,filter_type const& filter,std::size_t start,std::size_t end)
      :buff(buff),filter(filter),range(start,end){}
  public:
    char_at_type operator[](std::size_t index) const{
      if(range.contains(index))
        return this->filter(this->buff[index]);
      else
        return this->buff[index];
    }
    std::size_t length() const{
      return this->buff.length();
    }
    const_iterator begin() const{
      return const_iterator(*this,0);
    }
    const_iterator end() const{
      return const_iterator(*this,this->length());
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

template<typename Str>
struct _strtmp_pad_policy{
  typedef _strtmp_pad_policy                 policy_type;
  typedef typename Str::policy_type::char_type    char_type;
  typedef typename Str::policy_type::char_at_type char_at_type;
  static const bool has_get_ptr=false;

  typedef default_const_iterator<policy_type> const_iterator;

  struct buffer_type{
    Str const& str;
    std::size_t lpad_len;
    std::size_t m_length;
    char c;
  public:
    buffer_type(Str const& str,std::size_t lpad_len,std::size_t len,char_type c)
      :str(str),lpad_len(lpad_len),m_length(len),c(c){}
  public:
    char_at_type operator[](std::size_t index) const{
      std::ptrdiff_t index1=std::ptrdiff_t(index)-this->lpad_len;
      if(0<=index1&&index1<this->str.length())
        return this->str[index1];
      else
        return c;
    }
    std::size_t length() const{
      return this->m_length;
    }
    const_iterator begin() const{
      return const_iterator(*this,0);
    }
    const_iterator end() const{
      return const_iterator(*this,this->length());
    }
  };
};

//-----------------------------------------------------------------------------

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

  struct buffer_type{
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
    const_iterator begin() const{
      return const_iterator(*this,0);
    }
    const_iterator end() const{
      return const_iterator(*this,this->length());
    }
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

  struct buffer_type{
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
    const_iterator begin() const{
      return const_iterator(*this,0);
    }
    const_iterator end() const{
      return const_iterator(*this,this->length());
    }
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

//-----------------------------------------------------------------------------
// 関係演算子

template<typename StrP1,typename StrP2,typename Ret>
struct compare_enabler:mwg::stdm::enable_if<
  mwg::stdm::is_same<typename StrP1::char_type,typename StrP2::char_type>::value,Ret>{};

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

#define MWG_STRING2_STRING_H__define_relational_operator(Op) \
template<typename StrP1,typename XStr> \
typename compare_enabler2<StrP1,XStr,bool>::type \
operator Op(strbase<StrP1> const& lhs,XStr const& rhs){ \
  return lhs Op stradp<typename StrP1::char_type>(rhs); \
} \
template<typename StrP1,typename XStr> \
typename compare_enabler2<StrP1,XStr,bool>::type \
operator Op(XStr const& lhs,strbase<StrP1> const& rhs){ \
  return stradp<typename StrP1::char_type>(lhs) Op rhs; \
}
  MWG_STRING2_STRING_H__define_relational_operator(==)
  MWG_STRING2_STRING_H__define_relational_operator(!=)
  MWG_STRING2_STRING_H__define_relational_operator(<=)
  MWG_STRING2_STRING_H__define_relational_operator(>=)
  MWG_STRING2_STRING_H__define_relational_operator(<)
  MWG_STRING2_STRING_H__define_relational_operator(>)
#undef MWG_STRING2_STRING_H__define_relational_operator

//-----------------------------------------------------------------------------
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

template<typename XCH,typename Tr,typename Alloc>
struct adapter_traits<std::basic_string<XCH,Tr,Alloc> >{
  typedef XCH char_type;
  static const XCH* pointer(std::basic_string<XCH,Tr,Alloc> const& str){
    return str.c_ptr();
  }
  static std::size_t length(std::basic_string<XCH,Tr,Alloc> const& str){
    return str.length();
  }
};

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
} /* end of namespace string3_detail */
  using string3_detail::string;
  using string3_detail::strsub;
  using string3_detail::stradp;
} /* end of namespace mwg */
//HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
#endif
