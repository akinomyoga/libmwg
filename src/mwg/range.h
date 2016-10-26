// -*- mode:C++;coding:utf-8 -*-
#ifndef MWG_RANGE_H
#define MWG_RANGE_H
#include <algorithm>
#include <mwg/defs.h>
#include <mwg/std/type_traits>
#include <mwg/concept.h>
#include <mwg/functor.h>

namespace mwg{
//==============================================================================

namespace range_detail{
  template<typename F,typename T>
  struct foreach_switch{
    static const int value=
      mwg::be_functor<F,bool(const T&)>::value?1:
      0;
  };

  template<typename T,int S>
  struct range_traits_impl{
    typedef T value_type;
    typedef int difference_type;
    static void increment_inclusive(T& end){end++;}
  };
  template<typename T>
  struct range_traits_impl<T,1>{
    // for floating-point numbers
    typedef T value_type;
    typedef T difference_type;
    static void increment_inclusive(T& /* end */){}
  };
}

template<typename T>
struct range_traits
  :range_detail::range_traits_impl<T,mwg::stdm::is_floating_point<T>::value?1:0>{};

template<typename T>
class range_base{
  typedef range_traits<T> traits_type;
  typedef T value_type;
  typedef typename traits_type::difference_type difference_type;
private:
  value_type m_begin;
  value_type m_end;
public:
  value_type begin() const{return this->m_begin;}
  value_type end() const{return this->m_end;}
public:
  range_base(const value_type& begin,const value_type& end,bool inclusive=false)
    :m_begin(begin),m_end(end)
  {
    if(inclusive)
      traits_type::increment_inclusive(m_end);
  }
  difference_type length() const{
    return this->m_end-this->m_begin;
  }
  bool is_empty() const{
    return this->length()<=0;
  }
  bool contains(const value_type& value) const{
    return this->m_begin<=value&&value<this->m_end;
  }
  bool contains(const range_base& r) const{
    return this->m_begin<=r.m_begin&&r.m_end<=this->m_end;
  }
  //--------------------------------------------------------------------------
  bool operator==(const range_base& r) const{
    return m_begin==r.m_begin&&m_end==r.m_end;
  }
  // //--------------------------------------------------------------------------
  // template<typename F>
  // typename stdm::enable_if<range_detail::foreach_switch<F,T>::value==0,void>::type
  // foreach(const F& _f) const{
  //   F& f=const_cast<F&>(_f);
  //   for(value_type i=m_begin;i<m_end;i++)
  //     mwg::functor_invoke<void(const value_type&)>(f,i);
  // }
  // template<typename F>
  // typename stdm::enable_if<range_detail::foreach_switch<F,T>::value==1,void>::type
  // foreach(const F& _f mwg_gcc3_concept_overload(1)) const{
  //   F& f=const_cast<F&>(_f);
  //   for(value_type i=m_begin;i<m_end;i++)
  //     if(!mwg::functor_invoke<bool(const value_type&)>(f,i))break;
  // }
};

template<typename T>
class range:public range_base<T>{
public:
  range():range_base<T>(0,0){}
  range(const T& begin,const T& end,bool inclusive=false)
    :range_base<T>(begin,end,inclusive){}
public:
  range operator&(const range_base<T>& r) const{
    return new range(
      std::max(this->m_begin,r.m_begin),
      std::min(this->m_end,r.m_end)
    );
  }
};

//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//  mwg::make_range
//    指定した引数から range を生成します。
//------------------------------------------------------------------------------
template<typename T,int N>
range<T*> make_range(T (&arr)[N]){
  return range<T>(arr,arr+N);
}
template<typename T>
range<T> make_range(const T& begin,const T& end,bool inclusive=false){
  return range<T>(begin,end,inclusive);
}
template<typename T>
typename stdm::enable_if<!mwg::stdm::is_same<T,int>::value,range<T> >::type
make_range(const T& begin,int len){
  return range<T>(begin,begin+len);
}

typedef range<int> irange;

//------------------------------------------------------------------------------

template<typename T>
T clamp(T const& value,mwg::range<T> r){
  return (value<r.begin())?r.begin(): (value>r.end())?r.end(): value;
}

//==============================================================================
}
#endif
