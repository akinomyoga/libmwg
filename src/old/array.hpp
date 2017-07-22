// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_ARRAY_H
#define MWG_ARRAY_H
#include <new>
#include <stdexcept>
#include <mwg/defs.h>
#include <mwg/std/cstdint>
#include <mwg/std/type_traits>
#include <mwg/functor.h>
#include <mwg/range.h>
#include <mwg/bits/mpl.integer.h>
namespace mwg{
//------------------------------------------------------------------------------
template<
  typename T, int ALIGN = 1,
  bool TemplateArgCheck = (mwg::mpl::is_pow2<std::size_t, sizeof(void*)>::value && mwg::mpl::is_pow2<int, ALIGN>::value)
>
class internal_buffer;

template<typename T,int D=1> class Array;
template<typename T,int D=1> class RangedArray;

//******************************************************************************
//  内部バッファ
//==============================================================================
template<typename T>
class internal_buffer<T,1,true>{
private:
  int   size;
public:
  T* data;
  operator T*() const{return this->data;}
  T* operator->() const{return this->data;}
public:
  internal_buffer():data(nullptr){}
  ~internal_buffer(){
    this->free();
  }
  void set_size(int len){
    int buffsz=len*sizeof(T);
    if(buffsz>this->size){
      this->alloc(len);
    }else if(4*buffsz<this->size){
      this->alloc(len);
    }
  }
  static void swap(internal_buffer& l,internal_buffer& r){
    {
      int size=l.size;
      l.size=r.size;
      r.size=size;
    }{
      T* data=l.data;
      l.data=r.data;
      r.data=data;
    }
  }
private:
  void alloc(int len){
    int buffsz=len*sizeof(T);
    this->free();
    this->size=buffsz;
    this->data=reinterpret_cast<T*>(new byte[this->size]);
  }
  void free(){
    delete[] reinterpret_cast<byte*>(this->data);
    this->size=0;
    this->data=nullptr;
  }
};
template<typename T,int ALIGN>
class internal_buffer<T,ALIGN,true>{
private:
  static const int B=sizeof(void*);
  static const stdm::intptr_t A_1=ALIGN-1;
  static int aligned_size(int sz){
    return sz==0?B: sz+(B-1)&~(B-1);
  }
private:
  int   size;
  byte* buff;
public:
  T* data;
  operator T*() const{return this->data;}
  T* operator->() const{return this->data;}
public:
  internal_buffer():data(nullptr){}
  ~internal_buffer(){
    this->free();
  }
  void set_size(int len){
    int buffsz=aligned_size(len*sizeof(T)+A_1);
    if(buffsz>this->size){
      this->alloc(len);
    }else if(4*buffsz<this->size){
      this->alloc(len);
    }
  }
  static void swap(internal_buffer& l,internal_buffer& r){
    {
      int size=l.size;
      l.size=r.size;
      r.size=size;
    }{
      byte* buff=l.buff;
      l.buff=r.buff;
      r.buff=buff;
    }{
      T* data=l.data;
      l.data=r.data;
      r.data=data;
    }
  }
private:
  void alloc(int len){
    int buffsz=aligned_size(len*sizeof(T)+A_1);
    this->free();
    this->size=buffsz;
    this->buff=new byte[this->size];
    this->data=reinterpret_cast<T*>(
      reinterpret_cast<stdm::intptr_t>(this->buff)+A_1 & ~A_1
      );
  }
  void free(){
    delete[] this->buff;
    this->size=0;
    this->buff=nullptr;
    this->data=nullptr;
  }
};
//******************************************************************************
//  一次元配列
//==============================================================================
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#if 0
template<typename T>
struct NormalArrayData{
private:
  typedef NormalArrayData<T> data_t;
  int dim;
  internal_buffer<T> buff;
public:
  typedef int index_t;
  bool contains(index_t index) const;

  typedef const T* const_iterator;
  typedef T* iterator;
  iterator       begin();
  iterator       end();
  const_iterator begin() const;
  const_iterator end() const;
  index_t        begin_index() const; // functor に現在位置を知らせる為
  index_t        end_index() const;   // functor に現在位置を知らせる為
};
/*
template<typename T>
class Array<T,1>{
  int n;
  internal_buffer<T> data;
public:
  Array(int n){
    this->alloc(n);
    this-init();
  }
private:
  void alloc(int n){
    if(n<0)
      throw std::invalid_argument("指定された要素数は負です。");

    this->free();
    this->n=n;
    this->data.set_size(n);
  }
  void init(){
    // call ctor
    T* p=this->data;
    T* pM=p+n;
    while(p<pM)new(p++) T;
  }
  void free(){
    // call dtor
    T* p=this->data;
    T* pM=p+n;
    while(p<pM)p++->~T();
  }
};
//*/
#endif
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//------------------------------------------------------------------------------
template<typename T>
class RangedArray<T,1>{
  irange dim;
  internal_buffer<T> data;
//------------------------------------------------------------------------------
//  初期化
//------------------------------------------------------------------------------
private:
  template<typename U,int D> friend class RangedArray;
  RangedArray(){}
public:
  RangedArray(const irange& dim){
    this->alloc(dim);
    this->init();
  }
  RangedArray(int n){
    this->alloc(irange(0,n));
    this->init();
  }
  ~RangedArray(){
    this->free();
  }
  RangedArray(const RangedArray& copye){
    this->operator=(copye);
  }
  RangedArray& operator=(const RangedArray& r){
    if(this==&r)return *this;

    this->alloc(r.dim);
    T* p=this->data;
    for(int i=0,iM=dim.length();i<iM;i++)
      new(p+i) T(r.data[i]);
  }
#ifdef MWGCONF_STD_RVALUE_REFERENCES
  RangedArray(RangedArray&& copye){
    swap(*this,copye);
  }
  RangedArray& operator=(RangedArray&& r){
    if(this==&r)return *this;
    swap(*this,r);
  }
#  ifdef _MSC_VER
  RangedArray(const RangedArray&& copye){
    swap(*this,const_cast<RangedArray&>(copye));
  }
  RangedArray& operator=(const RangedArray&& r){
    if(this==&r)return *this;
    swap(*this,const_cast<RangedArray&>(r));
  }
#  endif
#endif
private:
  void alloc(const irange& ndim){
    if(ndim.is_empty())
      throw std::invalid_argument("添字の範囲が空です。");

    this->free();
    this->dim=ndim;
    this->data.set_size(ndim.length());
  }
  void init(){
    // call ctor
    T* p=this->data;
    T* pM=p+dim.length();
    while(p<pM)new(p++) T;
  }
  void free(){
    // call dtor
    T* p=this->data;
    T* pM=p+dim.length();
    while(p<pM)p++->~T();
  }
//------------------------------------------------------------------------------
//  基本動作
//------------------------------------------------------------------------------
public:
  const T& operator[](int index) const{
    return const_cast<T*>(this)->operator[](index);
  }
  T& operator[](int index){
    assert((T*)this->data&&this->dim.contains(index));
    return this->data[index-dim.begin()];
  }
public:
  static void swap(RangedArray& l,RangedArray& r){
    internal_buffer<T>::swap(l.data,r.data);
    irange tdim(l.dim);
    l.dim=r.dim;
    r.dim=tdim;
  }
//------------------------------------------------------------------------------
//  共通動作
//------------------------------------------------------------------------------
public: /* indices */
  typedef int index_t;
  bool     contains(int index) const{return this->dim.contains(index);}
  int            length()      const{return this->dim.length();}
  index_t        begin_index() const{return this->dim.begin();}
  index_t        end_index()   const{return this->dim.end();}
public: /* iterators */
  typedef T* iterator;
  typedef const T* const_iterator;
  iterator       begin()            {return this->data.data;}
  iterator       end()              {return this->data.data+this->dim.length();}
  const_iterator begin()       const{return this->data.data;}
  const_iterator end()         const{return this->data.data+this->dim.length();}
//------------------------------------------------------------------------------
//  機能
//------------------------------------------------------------------------------
public:
  // TODO>:foreach      :-> each/eachR               std::for_each
  // TODO>:reduce       :-> reduce/reduceR           std::accumulate std::inner_product std::partial_sum std::adjacent_difference
  // TODO>:map          :-> map/mapR/mapD/mapRD      std::transform
  // TODO>:reverse      :-> reverse/reverseD         std::reverse
  // TODO:                                           std::rotate
  // TODO:                                           std::partition std::stable_partition
  // TODO:                                           std::swap_ranges
  // TODO:                                           std::sort std::stable_sort std::partial_sort std::nth_element
  // TODO:                                           std::binary_search std:: std::lower_bound std::upper_bound std::equal_range
  // TODO: slice/first/last
  // TODO: copy/replace                              std::replace std::replace_if
  // TODO: indexof lastindexof                       std::find
  // TODO: transpose (二次元)
  // TODO: count                                     std::count
  // TODO: max/min                                   std::min_element std::max_element
  // TODO: every/some                                std::count
  // TODO: shuffle                                   std::random_shuffle
  // array では実装しない物
  // TODO: remove                                    std::remove std::remove_if
  // TODO: filter                                    std::remove_copy std::remove_copy_if
  // TODO: uniq                                      std::unique std::unique_copy
  // TODO: shift/unshift/push/pop/insert
  void fill(const T& value){
    T* p=this->data;
    T* pM=p+dim.length();
    while(p<pM)*p++=value;
  }
  //--------------------------------------------------------------------------
  // each
  //--------------------------------------------------------------------------
//%define const_nonconst (
//%%expand 1.r|\<_const\>||
//%%expand 1.r|\<_const\>|const|
//%)
//%define 1 (
  template<typename F> typename mwg::stdm::enable_if<
    (mwg::be_functor<F,bool(_const T&,int)>::value),
    int
  >::type each(const F& handler) _const{
    _const T* p  =this->begin();
    int i =this->begin_index();
    int iM=this->end_index();
    for(;i<iM;p++,i++)
      if(!mwg::functor_invoke<bool(_const T&,int)>(handler,*p,i))break;
    return i;
  }
  template<typename F> typename mwg::stdm::enable_if<
    (!mwg::be_functor<F,bool(_const T&,int)>::value&&mwg::be_functor<F,void(_const T&,int)>::value),
    int
  >::type each(const F& handler mwg_gcc3_concept_overload(1)) _const{
    _const T* p  =this->begin();
    int i =this->begin_index();
    int iM=this->end_index();
    for(;i<iM;p++,i++)
      mwg::functor_invoke<void(_const T&,int)>(handler,*p,i);
    return i;
  }
  template<typename F> typename mwg::stdm::enable_if<
    (mwg::be_functor<F,bool(_const T&,int)>::value),
    int
  >::type eachR(const F& handler) _const{
    _const T* p  =this->end()-1;
    int i =this->end_index()-1;
    int iM=this->begin_index();
    for(;i>=iM;p--,i--)
      if(!mwg::functor_invoke<bool(_const T&,int)>(handler,*p,i))break;
    return i;
  }
  template<typename F> typename mwg::stdm::enable_if<
    (!mwg::be_functor<F,bool(_const T&,int)>::value&&mwg::be_functor<F,void(_const T&,int)>::value),
    int
  >::type eachR(const F& handler mwg_gcc3_concept_overload(1)) _const{
    _const T* p  =this->end()-1;
    int i =this->end_index()-1;
    int iM=this->begin_index();
    for(;i>=iM;p--,i--)
      mwg::functor_invoke<void(_const T&,int)>(handler,*p,i);
    return i;
  }
//%)
//%expand const_nonconst
  //--------------------------------------------------------------------------
  // map
  //--------------------------------------------------------------------------
#if defined(MWGCONF_STD_DECLTYPE)&&defined(MWGCONF_STD_AUTO_TYPE)
  template<typename F>
  auto map(const F& converter) const
    -> RangedArray<decltype(converter( mwg::declval<T>() ))>
  {
    typedef decltype(converter( mwg::declval<T>() )) U;
    RangedArray<U> ret;
    ret.alloc(this->dim);

    // init
    const T* src=this->begin();
    const T* srcM=this->end();
    U* dst=ret.data;
    while(src<srcM)new(dst++) U(converter(*src++));

    return ret;
  }
  template<typename F>
  auto mapR(const F& converter) const
    -> RangedArray<decltype(converter( mwg::declval<T>() ))>
  {
    typedef decltype(converter( mwg::declval<T>() )) U;
    RangedArray<U> ret;
    ret.alloc(this->dim);

    // init
    const T* src=this->begin();
    const T* srcM=this->end();
    U* dst=ret.end();
    while(src<srcM)new(--dst) U(converter(*src++));

    return ret;
  }
#endif
  template<typename F>
  RangedArray& mapD(const F& converter){
    T* src=this->begin();
    T* srcM=this->end();
    for(;src<srcM;src++)*src=converter(*src++);
    return *this;
  }
  template<typename F>
  RangedArray& mapRD(const F& converter){
    T* inc=this->begin();
    T* dec=this->end()-1;
    for(;inc<dec;inc++,dec--){
      T value(converter(*dec));
      *dec=converter(*inc);
      *inc=value;
    }
    if(inc==dec)
      *inc=converter(*inc);

    return *this;
  }
  //--------------------------------------------------------------------------
  // reverse
  //--------------------------------------------------------------------------
  template<typename F>
  RangedArray reverse(const F& converter) const{
    RangedArray<T> ret;
    ret.alloc(this->dim);

    // init
    const T* src=this->begin();
    const T* srcM=this->end();
    T* dst=ret.end();
    while(src<srcM)new(--dst) T(*src++);

    return ret;
  }
  template<typename F>
  RangedArray& reverseD(const F& converter){
    T* inc=this->begin();
    T* dec=this->end()-1;
    for(;inc<dec;inc++,dec--){
      std::swap(*dec,*inc);
    }

    return *this;
  }
  //--------------------------------------------------------------------------
  // reduce
  //--------------------------------------------------------------------------
  template<typename V,typename F>
  V reduce(const V& initial,const F& reducer) const{
    const T* p =this->begin();
    const T* pM=this->end();
    V value=initial;
    while(p<pM)value=reducer(const_cast<const V&>(value),*p++);
    return value;
  }
  template<typename V,typename F>
  V reduceR(const V& initial,const F& reducer) const{
    const T* p =this->end();
    const T* pM=this->begin();
    V value=initial;
    while(p>pM)value=reducer(const_cast<const V&>(value),*--p);
    return value;
  }
  template<typename F>
  T reduce(const F& reducer) const{
    const T* p =this->begin();
    const T* pM=this->end();
    if(p==pM)return T();
    T value=*p++;
    while(p<pM)value=reducer(const_cast<const T&>(value),*p++);
    return value;
  }
  template<typename F>
  T reduceR(const F& reducer) const{
    const T* p =this->end();
    const T* pM=this->begin();
    if(p==pM)return T();
    T value=*--p;
    while(p>pM)value=reducer(const_cast<const T&>(value),*--p);
    return value;
  }
};
//------------------------------------------------------------------------------
}
namespace std{
  template<typename T>
  void swap(mwg::RangedArray<T,1>& l,mwg::RangedArray<T,1>& r){
    mwg::RangedArray<T,1>::swap(l,r);
  }
  template<typename T,int ALIGN>
  void swap(mwg::internal_buffer<T,ALIGN>& l,mwg::internal_buffer<T,ALIGN>& r){
    mwg::internal_buffer<T,ALIGN>::swap(l,r);
  }
}
#endif
