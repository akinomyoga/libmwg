// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BIO_VIEW_H
#define MWG_BIO_VIEW_H
#include "defs.h"
namespace mwg{
namespace bio{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class locked_win/locked_ptr
//=============================================================================
class iunlocker;
class locked_win_base;
class locked_win;
class const_locked_win;
template<typename T,typename locked_win_t>
class locked_ptr_base;
template<typename T>
class locked_ptr;
//-----------------------------------------------------------------------------
class iunlocker{
public:
  virtual int increment()=0;
  virtual int decrement()=0;
  virtual ~iunlocker(){}
};
//-----------------------------------------------------------------------------
class locked_win_base{
  friend class locked_win;
  friend class const_locked_win;
protected:
  byte* ptr;
  iunlocker* unlocker;
public:
  ~locked_win_base(){
    this->free();
  }
protected:
  void init(byte* ptr,iunlocker* unlocker){
    this->ptr=ptr;
    this->unlocker=unlocker;
    if(unlocker!=nullptr)unlocker->increment();
  }
  void free(){
    this->ptr=nullptr;
    if(this->unlocker!=nullptr){
      if(0==this->unlocker->decrement())
        delete this->unlocker;
      this->unlocker=nullptr;
    }
  }
};

class locked_win:public locked_win_base{
public:
  void* get_ptr() const{return this->ptr;}
public:
  locked_win(){
    this->init(nullptr,nullptr);
  }
  locked_win(const locked_win& copye){
    this->init(copye.ptr,copye.unlocker);
  }
  locked_win(void* ptr,iunlocker* unlocker){
    this->init((byte*)ptr,unlocker);
  }
  locked_win& operator=(const locked_win& copye){
    if(this==&copye)return *this;
    this->free();
    this->init(copye.ptr,copye.unlocker);
    return *this;
  }
};

class const_locked_win:public locked_win_base{
public:
  const void* get_ptr() const{return this->ptr;}
public:
  const_locked_win(){
    this->init(nullptr,nullptr);
  }
  const_locked_win(const const_locked_win& copye){
    this->init(copye.ptr,copye.unlocker);
  }
  const_locked_win(const void* ptr,iunlocker* unlocker){
    this->init((byte*)ptr,unlocker);
  }
  const_locked_win& operator=(const const_locked_win& copye){
    if(this==&copye)return *this;
    this->free();
    this->init(copye.ptr,copye.unlocker);
    return *this;
  }
  const_locked_win(const locked_win& copye){
    this->init(copye.ptr,copye.unlocker);
  }
  const_locked_win& operator=(const locked_win& copye){
    this->free();
    this->init(copye.ptr,copye.unlocker);
    return *this;
  }
};
//-----------------------------------------------------------------------------
template<typename T,typename locked_win_t>
class locked_ptr_base:public locked_win_t{
protected:
  locked_ptr_base(const locked_win_t& mem):locked_win_t(mem){}
public: // ポインタ演算
#define AddOperator(O,OE,OO)                          \
  locked_ptr_base& operator OE(std::ptrdiff_t i){     \
    this->ptr OE sizeof(T)*i;                         \
    return *this;                                     \
  }                                                   \
  locked_ptr_base operator O(std::ptrdiff_t i) const{ \
    locked_ptr_base ret(*this);                       \
    ret.ptr OE sizeof(T)*i;                           \
    return ret;                                       \
  }                                                   \
  locked_ptr_base& operator OO(){                     \
    this->ptr OE sizeof(T);                           \
    return *this;                                     \
  }                                                   \
  locked_ptr_base operator OO(int){                   \
    locked_ptr_base ret(*this);                       \
    this->ptr OE sizeof(T);                           \
    return ret;                                       \
  }                                                   /**/
  AddOperator(+,+=,++)
  AddOperator(-,-=,--)
  std::ptrdiff_t operator-(const locked_ptr_base& r) const{
    return (this->ptr-r.ptr)/sizeof(T);
  }
#undef AddOperator
public: // 比較演算
#define CompareOperator(O)                        \
  bool operator O(const locked_ptr_base& right){  \
    return this->ptr O right.ptr;                 \
  }                                               /**/
  CompareOperator(==)
  CompareOperator(!=)
  CompareOperator(<)
  CompareOperator(>)
  CompareOperator(<=)
  CompareOperator(>=)
#undef CompareOperator
public: // 参照など
  T* get_ptr() const{
    return reinterpret_cast<T*>(this->ptr);
  }
  operator T*() const{
    return reinterpret_cast<T*>(this->ptr);
  }
  T* operator->() const{
    return reinterpret_cast<T*>(this->ptr);
  }
  T& operator*() const{
    return *reinterpret_cast<T*>(this->ptr);
  }
  T& operator[](std::ptrdiff_t index) const{
    return reinterpret_cast<T*>(this->ptr)[index];
  }
};

template<typename T,typename M>
inline locked_ptr_base<T,M> operator+(std::ptrdiff_t i,const locked_ptr_base<T,M>& p){
  return p+i;
}

template<typename T>
class locked_ptr:public locked_ptr_base<T,locked_win>{
  typedef locked_ptr_base<T,locked_win> base;
public:
  locked_ptr(const locked_win& mem):base(mem){}
};

template<typename T>
class locked_ptr<const T>:public locked_ptr_base<const T,const_locked_win>{
  typedef locked_ptr_base<const T,const_locked_win> base;
public:
  locked_ptr(const locked_win& mem):base(const_locked_win(mem)){}
  locked_ptr(const const_locked_win& mem):base(mem){}
};

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class iview // definition
//=============================================================================
class iview;

class iview{
public:
  virtual bool             can_read()               const=0;
  virtual bool             can_write()              const=0;
  virtual bool             can_trunc()              const=0;
  virtual u8t              size()                   const=0;
  virtual locked_win       lockrw(u8t pos,int size) const=0;
  virtual locked_win       lockw(u8t pos,int size)  const=0;
  virtual const_locked_win lockr(u8t pos,int size)  const=0;
  virtual int              trunc(u8t size)          const=0;
  virtual ~iview(){}
};

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class mem_view // iview on RAM
//=============================================================================
class flat_mem_view;
class ranged_mem_view;
class const_ranged_mem_view;

class flat_mem_view:public iview{
public:
  bool can_read() const{return true;}
  bool can_write() const{return true;}
  bool can_trunc() const{return false;}
  u8t size() const{return u8t(-1);}
  locked_win lockrw(u8t pos,int size) const{
    return locked_win((void*)(mwg::uPt)pos,nullptr);
  }
  locked_win lockw(u8t pos,int size) const{
    return locked_win((void*)(mwg::uPt)pos,nullptr);
  }
  const_locked_win lockr(u8t pos,int size) const{
    return const_locked_win((void*)(mwg::uPt)pos,nullptr);
  }
  int trunc(u8t size) const{
    throw mwg::bio::nosupport_error("the size of this view cannot be changed.");
  }
};

class ranged_mem_view:public iview{
  byte* start;
  std::size_t m_size;
public:
  ranged_mem_view(void* start,std::size_t size)
    :start((byte*)start),m_size(size){}
public:
  bool can_read() const{return true;}
  bool can_write() const{return true;}
  bool can_trunc() const{return false;}
  u8t size() const{return this->m_size;}
  locked_win lockrw(u8t pos,int size) const{
    mwg_assert(pos>=0||pos+size<=this->m_size);
    return locked_win(start+(std::ptrdiff_t)pos,nullptr);
  }
  locked_win lockw(u8t pos,int size) const{
    mwg_assert(pos>=0||pos+size<=this->m_size);
    return locked_win(start+(std::ptrdiff_t)pos,nullptr);
  }
  const_locked_win lockr(u8t pos,int size) const{
    mwg_assert(pos>=0||pos+size<=this->m_size);
    return const_locked_win(start+(std::ptrdiff_t)pos,nullptr);
  }
  int trunc(u8t size) const{
    throw mwg::bio::nosupport_error("the size of this view cannot be changed.");
  }
};

class const_ranged_mem_view:public iview{
  byte* start;
  std::size_t m_size;
public:
  const_ranged_mem_view(void* start,std::size_t size)
    :start((byte*)start),m_size(size){}
public:
  bool can_read() const{return true;}
  bool can_write() const{return false;}
  bool can_trunc() const{return false;}
  u8t size() const{return this->m_size;}
  locked_win lockrw(u8t pos,int size) const{
    throw mwg::bio::nosupport_error("readonly iview.");
  }
  locked_win lockw(u8t pos,int size) const{
    throw mwg::bio::nosupport_error("readonly iview.");
  }
  const_locked_win lockr(u8t pos,int size) const{
    mwg_assert(pos>=0||pos+size<=this->m_size);
    return const_locked_win(start+(std::ptrdiff_t)pos,nullptr);
  }
  int trunc(u8t size) const{
    throw mwg::bio::nosupport_error("the size of this view cannot be changed.");
  }
};

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
