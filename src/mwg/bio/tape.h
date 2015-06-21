// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef MWG_BIO_TAPE_H
#define MWG_BIO_TAPE_H
#include <cstring>
#include <mwg/std/type_traits>
#include <mwg/exp/utils.h> /* for static_flags */
#include "defs.h"

#include <mwg/impl/warning_push.inl>

namespace mwg{
namespace bio{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  class itape;

  template<typename ITape=itape,int RWFlags=0>
  class tape_head;

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
// class itape
//-----------------------------------------------------------------------------
class itape{
public:
  virtual bool can_read() const=0;
  virtual bool can_write() const=0;
  virtual bool can_seek() const=0;
  virtual bool can_trunc() const=0;
  virtual int read(void* buff,int size,int n=1) const=0;
  virtual int write(const void* buff,int size,int n=1) const=0;
  virtual int seek(i8t offset,int whence=SEEK_SET) const=0;
  virtual i8t tell() const=0;
  virtual u8t size() const=0;
  virtual int trunc(u8t size) const=0;
  virtual int flush() const=0;
  // seek 戻り値. 0: 成功
  // tell 戻り値. -1: 失敗 他: 現在位置

  virtual ~itape(){}
  virtual bool is_alive() const{return true;}
  operator bool() const{return this->is_alive();}
};
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
// class memory_tape
//-----------------------------------------------------------------------------
class memory_tape:public itape{
  byte* data;
  bool f_write;
  std::size_t m_size;
  mutable std::size_t m_pos;
public:
  memory_tape(void* data,std::size_t size)
    :data((byte*)data),f_write(true),m_size(size),m_pos(0){}
  memory_tape(void* begin,void* end)
    :data((byte*)begin),f_write(true),m_size((byte*)end-(byte*)end),m_pos(0){}
  memory_tape(const void* data,std::size_t size)
    :data((byte*)data),f_write(false),m_size(size),m_pos(0){}
  memory_tape(const void* begin,const void* end)
    :data((byte*)begin),f_write(false),m_size((const byte*)end-(const byte*)end),m_pos(0){}
public:
  virtual bool can_read() const{return true;}
  virtual bool can_write() const{return f_write;}
  virtual bool can_seek() const{return true;}
  virtual bool can_trunc() const{return false;}
  virtual int read(void* buff,int size,int n=1) const{
    int n2=std::min<int>((m_size-m_pos)/size,n);
    int len=n2*size;
    std::memcpy(buff,data+m_pos,len);
    m_pos+=len;
    return n2;
  }
  virtual int write(const void* buff,int size,int n=1) const{
    if(!f_write)return 0;
    int n2=std::min<int>((m_size-m_pos)/size,n);
    int len=n2*size;
    std::memcpy(data+m_pos,buff,n2*size);
    m_pos+=len;
    return n2;
  }
  virtual int seek(i8t offset,int whence) const{
    i8t pos;
    switch(whence){
    case SEEK_CUR:
      pos=m_pos+offset;
      goto set_pos;
    case SEEK_END:
      pos=m_size+offset;
      goto set_pos;
    case SEEK_SET:
      pos=offset;
      goto set_pos;
    set_pos:
      if(0<=pos&&static_cast<u8t>(pos)<=m_size){
        m_pos=pos;
        return 0;
      }else
        return -1;
    default:
      return EINVAL;
    }
  }
  virtual i8t tell() const{return this->m_pos;}
  virtual u8t size() const{return this->m_size;}
  virtual int trunc(u8t size) const{
    mwg_unused(size);
    mwg_assert(false,"truncation not supported");
    return -1;
  }
  virtual int flush() const{return 0;}
};

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  template<typename ITape> class tape_head
//-----------------------------------------------------------------------------
  // タグとして可能な物
  // - 整数  : エンディアン
  // - 小数  : エンディアン / IEEE754, 固定小数点
  // - 真偽値: 1 byte 2 bytes 4 bytes
  // - 文字列: 文字列の形式 (nullTerm, withLen,) / 文字コード / BOM
namespace  rwflags_detail{
  struct rwflags_tag{};
  template<typename T,int I> struct rwflags_impl;
}
  mwg_static_flags_define(little_endian,rwflags_detail::rwflags_tag,0x1);
  mwg_static_flags_define(big_endian,rwflags_detail::rwflags_tag,0x2);

template<typename ITape,int RWFlags>
class tape_head{
  const ITape* m_tape;
public:
  const ITape* tape() const{return this->m_tape;}
public:
  //tape_head():m_tape(nullptr){} // cannot be assigned because m_tape is const
  tape_head():m_tape(nullptr){}
  tape_head(const ITape* tape):m_tape(tape){}
  tape_head(const ITape& tape):m_tape(&tape){}

  operator bool() const{return m_tape&&m_tape->is_alive();}

public:
  //
  // itape interfaces
  //
  int write_data(const void* buff,int size,int n=1) const{
    return this->m_tape->write(buff,size,n);
  }
  int read_data(void* buff,int size,int n=1) const{
    return this->m_tape->read(buff,size,n);
  }
  int seek(i8t offset,int whence=SEEK_SET) const{
    return this->m_tape->seek(offset,whence);
  }
  i8t tell() const{
    return this->m_tape->tell();
  }
  u8t size() const{
    return this->m_tape->size();
  }
  int trunc(u8t size) const{
    return this->m_tape->trunc(size);
  }
  int flush() const{
    return this->m_tape->flush();
  }
  bool can_read() const{return this->m_tape->can_read();}
  bool can_write() const{return this->m_tape->can_write();}
  bool can_seek() const{return this->m_tape->can_seek();}
  bool can_trunc() const{return this->m_tape->can_trunc();}

public:
  //
  // basic read/write
  //
  template<typename T>
  typename rwflags_detail::rwflags_impl<T,RWFlags>::wtype write(const T& value) const{
    return rwflags_detail::rwflags_impl<T,RWFlags>::template write<ITape,RWFlags>(this,value);
  }
  template<typename T>
  typename rwflags_detail::rwflags_impl<T,RWFlags>::rtype read(T& value) const{
    return rwflags_detail::rwflags_impl<T,RWFlags>::template read<ITape,RWFlags>(this,value);
  }
  template<typename T,int I>
  typename rwflags_detail::rwflags_impl<T,I>::wtype write(const T& value,mwg_static_flags_cref(rwflags_detail::rwflags_tag,I)) const{
    return rwflags_detail::rwflags_impl<T,I>::template write<ITape,RWFlags>(this,value);
  }
  template<typename T,int I>
  typename rwflags_detail::rwflags_impl<T,I>::rtype read(T& value,mwg_static_flags_cref(rwflags_detail::rwflags_tag,I)) const{
    return rwflags_detail::rwflags_impl<T,I>::template read<ITape,RWFlags>(this,value);
  }
  template<int I,typename T>
  typename rwflags_detail::rwflags_impl<T,I>::wtype write(const T& value) const{
    return rwflags_detail::rwflags_impl<T,I>::write(this,value);
  }
  template<int I,typename T>
  typename rwflags_detail::rwflags_impl<T,I>::rtype read(T& value) const{
    return rwflags_detail::rwflags_impl<T,I>::read(this,value);
  }

  // 型を明示的に指定する場合
  template<typename T,typename U>
  typename mwg::stdm::enable_if<
    !mwg::stdm::is_same<T,U>::value,
    typename rwflags_detail::rwflags_impl<T,RWFlags>::rtype>::type
  read(U& value) const{
    typedef typename rwflags_detail::rwflags_impl<T,RWFlags>::rtype return_type;
    T _value;
    return_type ret=rwflags_detail::rwflags_impl<T,RWFlags>::template read<ITape,RWFlags>(this,_value);
    value=_value;
    return ret;
  }

public:
  u4t align_fill(int align,byte c=0) const{
    // align must be some power of 2.
    i8t pos=this->m_tape->tell();
    i4t res=i4t(pos&align-1);
    return res==0?0:memset(c,align-res);
  }

private:
  u4t read_skip(u4t size) const{
    u4t r=0;u4t dummy;
    for(u4t n=4;n<=size;n+=4)r+=4*m_tape->read(&dummy,4);
    if(size&=3)r+=m_tape->read(&dummy,1,size);
    return r;
  }
public:

  u4t align(int align) const{
    // align must be some power of 2.
    i8t pos=this->m_tape->tell();
    i4t res=i4t(pos&align-1);
    if(res==0)return 0;

    i4t advance=align-res;
    // i4t pad=pos+advance-this->m_tape->size();
    // if(pad>0){
    //   this->m_tape->seek(res,SEEK_END);
    //   if(this->m_tape->can_write())memset(c,pad);
    // }else{
    //   this->m_tape->seek(advance,SEEK_CUR);
    // }
    if(this->m_tape->can_seek())
      this->m_tape->seek(advance,SEEK_CUR);
    else
      this->read_skip(advance);

    return advance;
  }

  u4t memset(byte c,u4t size) const{
    u4t r=0;
    i4t p=c|c<<8;p|=p<<16;
    for(u4t n=4;n<=size;n+=4)r+=4*m_tape->write(&p,4);
    if(size&=3)r+=m_tape->write(&p,1,size);
    return r;
  }

  //
  // 場所の記憶
  //
  class mark_t{
    tape_head const* const head;
    i8t const offset;
  public:
    mark_t(tape_head const& head):head(&head),offset(head->tell()){}
    mark_t(tape_head const* head):head(head),offset(head->tell()){}
    ~mark_t(){head->seek(offset);}
  };
  mark_t mark() const{return mark_t(this);}
};

  template<typename IT>
  typename stdm::enable_if<stdm::is_base_of<itape,IT>::value,tape_head<IT> >::type
  make_head(const IT& tape){return tape_head<IT>(tape);}
  template<typename IT,int RWF>
  typename stdm::enable_if<stdm::is_base_of<itape,IT>::value,tape_head<IT,RWF> >::type
  make_head(const IT& tape,mwg_static_flags_cref(rwflags_detail::rwflags_tag,RWF)){return tape_head<IT,RWF>(tape);}
  template<int RWF,typename IT>
  typename stdm::enable_if<stdm::is_base_of<itape,IT>::value,tape_head<IT,RWF> >::type
  make_head(const IT& tape){return tape_head<IT,RWF>(tape);}

namespace rwflags_detail{

  template<typename T>
  struct rwflags_impl_default{
    typedef int wtype;
    template<typename IT,int RWF>
    static int write(const tape_head<IT,RWF>* head,const T& value){
      return head->write_data(&value,sizeof(T),1);
    }
    typedef int rtype;
    template<typename IT,int RWF>
    static int read(const tape_head<IT,RWF>* head,T& value){
      return head->read_data(&value,sizeof(T),1);
    }
  };
  template<typename T,int I,typename X>
  struct rwflags_impl_reinterpret{
    typedef int wtype;
    template<typename IT,int RWF>
    static int write(const tape_head<IT,RWF>* head,const T& value){
      return rwflags_impl<X,I>::template write<IT,RWF>(head,reinterpret_cast<const X&>(value));
    }
    typedef int rtype;
    template<typename IT,int RWF>
    static int read(const tape_head<IT,RWF>* head,T& value){
      return rwflags_impl<X,I>::template read<IT,RWF>(head,reinterpret_cast<X&>(value));
    }
  };

  template<typename T,int I>
  struct rwflags_impl:rwflags_impl_default<T>{};

  template<typename T,int I>
  struct rwflags_impl2:rwflags_impl_default<T>{};

#ifdef MWG_SYS_BIGENDIAN
# define other_endian_flag little_endian_flag
#else
# define other_endian_flag big_endian_flag
#endif
  template<> struct rwflags_impl<u8t,other_endian_flag>{
    typedef int wtype;
    template<typename IT,int RWF>
    static int write(const tape_head<IT,RWF>* head,const u8t& value){
      return rwflags_impl_default<u8t>::template write<IT,RWF>(
        head,
        value<<56&0xFF00000000000000LL|
        value<<40&0x00FF000000000000LL|
        value<<24&0x0000FF0000000000LL|
        value<< 8&0x000000FF00000000LL|
        value>> 8&0x00000000FF000000|
        value>>24&0x0000000000FF0000|
        value>>40&0x000000000000FF00|
        value>>56&0x00000000000000FF
      );
    }
    typedef int rtype;
    template<typename IT,int RWF>
    static int read(const tape_head<IT,RWF>* head,u8t& value){
      u8t t;
      int r=rwflags_impl_default<u8t>::template read<IT,RWF>(head,t);
      if(r)
        value=(
          t<<56&0xFF00000000000000LL|
          t<<40&0x00FF000000000000LL|
          t<<24&0x0000FF0000000000LL|
          t<< 8&0x000000FF00000000LL|
          t>> 8&0x00000000FF000000|
          t>>24&0x0000000000FF0000|
          t>>40&0x000000000000FF00|
          t>>56&0x00000000000000FF
        );
      return r;
    }
  };
  template<> struct rwflags_impl<u4t,other_endian_flag>{
    typedef int wtype;
    template<typename IT,int RWF>
    static int write(const tape_head<IT,RWF>* head,const u4t& value){
      return rwflags_impl_default<u4t>::template write<IT,RWF>(
        head,
        value<<24&0xFF000000|
        value<< 8&0x00FF0000|
        value>> 8&0x0000FF00|
        value>>24&0x000000FF
      );
    }
    typedef int rtype;
    template<typename IT,int RWF>
    static int read(const tape_head<IT,RWF>* head,u4t& value){
      u4t t;
      int r=rwflags_impl_default<u4t>::template read<IT,RWF>(head,t);
      if(r)
        value=(
          t<<24&0xFF000000|
          t<< 8&0x00FF0000|
          t>> 8&0x0000FF00|
          t>>24&0x000000FF
        );
      return r;
    }
  };
  template<> struct rwflags_impl<u2t,other_endian_flag>{
    typedef int wtype;
    template<typename IT,int RWF>
    static int write(const tape_head<IT,RWF>* head,const u2t& value){
      return rwflags_impl_default<u2t>::template write<IT,RWF>(
        head,
        value<<8&0xFF00|
        value>>8&0x00FF
      );
    }
    typedef int rtype;
    template<typename IT,int RWF>
    static int read(const tape_head<IT,RWF>* head,u2t& value){
      u2t t;
      int r=rwflags_impl_default<u2t>::template read<IT,RWF>(head,t);
      if(r)
        value=(
          t<<8&0xFF00|
          t>>8&0x00FF
        );
      return r;
    }
  };
#undef other_endian_flag

  template<int I> struct rwflags_impl<   u2t,I>:rwflags_impl2<u2t,I&(little_endian_flag|big_endian_flag)>{};
  template<int I> struct rwflags_impl<   u4t,I>:rwflags_impl2<u4t,I&(little_endian_flag|big_endian_flag)>{};
  template<int I> struct rwflags_impl<   u8t,I>:rwflags_impl2<u8t,I&(little_endian_flag|big_endian_flag)>{};
  template<int I> struct rwflags_impl<   i2t,I>:rwflags_impl_reinterpret<   i2t,I,u2t>{};
  template<int I> struct rwflags_impl<   i4t,I>:rwflags_impl_reinterpret<   i4t,I,u4t>{};
  template<int I> struct rwflags_impl<   i8t,I>:rwflags_impl_reinterpret<   i8t,I,u8t>{};
  template<int I> struct rwflags_impl< float,I>:rwflags_impl_reinterpret< float,I,u4t>{};
  template<int I> struct rwflags_impl<double,I>:rwflags_impl_reinterpret<double,I,u8t>{};
} /* endof namespace rwflags_detail */

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#include "tape.util.inl"
#include "tape.stdio.inl"
#include "tape.stream.inl"
#include <mwg/impl/warning_pop.inl>
#endif
// #pragma%x begin_check
// #include <mwg/except.h>
// #include <mwg/bio/tape.h>

// int main(){
//   return 0;
// }
// #pragma%x end_check
