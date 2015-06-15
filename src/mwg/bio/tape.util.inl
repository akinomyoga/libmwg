// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef MWG_BIO_TAPE_UTIL_H
#define MWG_BIO_TAPE_UTIL_H
#include <climits>
#include <mwg/concept.h>
#include <mwg/functor.h>
#include <mwg/std/type_traits>
#include <mwg/std/memory>
#include "defs.h"
#include "tape.h"
namespace mwg{
namespace bio{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//  tapes
//-----------------------------------------------------------------------------
template<typename BaseTape=itape>
class subsequence_tape;
template<typename BaseTape=itape>
class shared_tape;
//■ cached_tape (読取専用/書込専用/読み書きシーク で別々の実装)
//-----------------------------------------------------------------------------
// コピーを禁止されている tape への参照を保持します。
template<typename BaseTape>
class shared_tape:public itape{
  stdm::shared_ptr<const BaseTape> tape;
public:
  shared_tape(const BaseTape* tape):tape(tape){}

  bool can_read() const{return tape->can_read();}
  bool can_write() const{return tape->can_write();}
  bool can_seek() const{return tape->can_seek();}
  bool can_trunc() const{return tape->can_trunc();}
  int read(void* buff,int size,int n=1) const{return tape->read(buff,size,n);}
  int write(const void* buff,int size,int n=1) const{return tape->write(buff,size,n);}
  int seek(i8t offset,int whence=SEEK_SET) const{return tape->seek(offset,whence);}
  i8t tell() const{return tape->tell();}
  u8t size() const{return tape->size();}
  int trunc(u8t size) const{return tape->trunc(size);}
  int flush() const{return tape->flush();}
};

template<typename BaseTape>
class subsequence_tape:public itape{
  const BaseTape& tape;
  u8t const offset;
  u8t const length;
  bool const readOnly;
  mutable u8t position;
public:
  subsequence_tape(const BaseTape& tape,const u8t& offset,const u8t& length,bool readOnly=false)
    :tape(tape),offset(offset),length(length),readOnly(readOnly)
    ,position(0)
  {
    if(!tape.can_seek())
      throw std::invalid_argument("!tape.can_seek()");
  }
  bool can_read() const{return this->tape.can_read();}
  bool can_write() const{return !this->readOnly&&this->tape.can_write();}
  bool can_seek() const{return this->tape.can_seek();}
  bool can_trunc() const{return false;}
  int read(void* buff,int size,int n=1) const{
    n=std::min<int>(n,std::min<u8t>((length-position)/size,INT_MAX));
    if(n<1)return 0;
    u8t pos=offset+position;
    if(pos!=tape.tell())tape.seek(pos);
    n=tape.read(buff,size,n);
    position+=size*n;
    return n;
  }
  int write(const void* buff,int size,int n=1) const{
    n=std::min<int>(n,std::min<u8t>((length-position)/size,INT_MAX));
    if(n<1)return 0;
    u8t pos=offset+position;
    if(pos!=tape.tell())tape.seek(pos);
    n=tape.write(buff,size,n);
    position+=size*n;
    return n;
  }
  int seek(i8t offset,int whence=SEEK_SET) const{
    i8t pos;
    switch(whence){
    case SEEK_SET:
      pos=offset;
      break;
    case SEEK_CUR:
      pos=this->position+offset;
    case SEEK_END:
      pos=this->length+offset;
      break;
    default:
      errno=EINVAL;
      return -1;
    }

    if(pos<0||pos>length){
      errno=EINVAL;
      return -1;
    }

    this->position=pos;
    return 0;
  }
  i8t tell() const{return this->position;}
  u8t size() const{return this->length;}
  int trunc(u8t size) const{errno=EACCES;return -1;}
  int flush() const{return tape.flush();}
};
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//  operators
//-----------------------------------------------------------------------------
#ifdef MWG_CONCEPT_OVERLOAD_FAIL
inline const itape& operator|(const itape& left,const itape& right)
#else
template<typename T,typename U>
typename stdm::enable_if<stdm::is_base_of<itape,T>::value&&stdm::is_base_of<itape,U>::value,const U&>::type
operator|(const T& left,const U& right)
#endif
{
  if(!left.can_read())
    throw std::invalid_argument("!left.can_read()");
  if(!right.can_write())
    throw std::invalid_argument("!right.can_write()");

  // copy
  int const BUFFSIZE(1024);
  byte buff[1024]; /* ←何故か BUFFSIZE だとコンパイルできない環境がある */
  int c;
  while((c=left.read(buff,1,BUFFSIZE)))
    if(0==right.write(buff,1,c))break;

  return right;
}
//-----------------------------------------------------------------------------

template<typename BaseTape,typename Filter>
class filtered_rtape:public itape{
  //static const int BSIZE=32; // for debugging
  static const int BSIZE=16384;//1024

  struct filtered_rtape_buffer{
    const BaseTape& tape;
    const Filter& filter;
    byte* s;
    byte* sN;
    byte* d;
    byte* dN;
    void* state;
    byte sbuf[BSIZE];
    byte dbuf[BSIZE];
  public:
    filtered_rtape_buffer(const BaseTape& tape,const Filter& filter)
      :tape(tape),filter(filter),state(nullptr)
    {
      if(!tape.can_read())
        throw std::invalid_argument("!tape.can_read()");
      s=sN=sbuf;
      d=dN=dbuf;
    }
    ~filtered_rtape_buffer(){
      this->free();
    }
  private:
    void free(){
      // if(state!=nullptr)
      //   filter(const_cast<const byte*&>(s),s,d=dbuf,dbuf+BSIZE,state); // free
      if(state!=nullptr)
        filter(const_cast<const byte*&>(s),s,dN,dN,state); // free
    }
  private:
    void throw_filter_error(int r){
      char buff[100];
      std::sprintf(buff,"filtered_rtape::read: error_code %d from filter function",r);
      throw ill_format_error(buff);
    }
  public:
    int read(void* buff_,int size,int n){
      // tape -> s -> d -> buff
      // ■TODO: d is redundant. filter can directly write to buff

      std::size_t len  =size*(std::size_t)n;
      byte* const buff0=reinterpret_cast<byte*>(buff_);
      byte* buff       =buff0;
      bool fEOF        =false;

      for(;;){
        if(d==dN){
          if(!fEOF&&s==sN){
            // read from tape
            s=sbuf;
            sN=s+tape.read(s,1,BSIZE);
            fEOF=s==sN;
          }

          // read from sbuf
          d=dN=dbuf;
          //mwg_assert(sbuf<=s&&sN<=sbuf+BSIZE&&dbuf<=dN&&dN+BSIZE<=dbuf+BSIZE);
          int r=filter(const_cast<const byte*&>(s),sN,dN,dbuf+BSIZE,state);
          if(r!=0)this->throw_filter_error(r);

          if(fEOF&&d==dN){
            this->free();
            return (buff-buff0)/size;
          }
        }

        // read from dbuf
        if(len<=std::size_t(dN-d)){
          std::memcpy(buff,d,(std::size_t)len);
          d+=(std::size_t)len;
          return n;
        }else{
          std::size_t ncpy=dN-d;
          std::memcpy(buff,d,ncpy);
          buff+=ncpy;
          len-=ncpy;
          d=dN;
        }
      }
    }
  };

  mwg::stdm::shared_ptr<filtered_rtape_buffer> ptr;
public:
  filtered_rtape(const BaseTape& tape,const Filter& filter)
    :ptr(new filtered_rtape_buffer(tape,filter)){}
public:
  int read(void* buff_,int size,int n=1) const{
    return ptr->read(buff_,size,n);
  }
public:
  bool can_read() const{return true;}
  bool can_write() const{return false;}
  bool can_seek() const{return false;}
  bool can_trunc() const{return false;}
  int write(const void* buff,int size,int n=1) const{return 0;}
  int seek(i8t offset,int whence=SEEK_SET) const{errno=EACCES;return -1;}
  i8t tell() const{return 0;}
  u8t size() const{return 0;}
  int trunc(u8t size) const{errno=EACCES;return -1;}
  int flush() const{return 0;}
};

//-----------------------------------------------------------------------------

template<typename BaseTape,typename Filter>
class filtered_wtape:public itape{
  //static const int BSIZE=32; // for debugging
  static const int BSIZE=16384;//1024

  struct filtered_wtape_buffer{
    const BaseTape& tape;
    const Filter& filter;
    byte* p;
    void* state;
    int wret;
    byte pbuf[BSIZE];
  public:
    filtered_wtape_buffer(const BaseTape& tape,const Filter& filter)
      :tape(tape),filter(filter),state(nullptr)
    {
      if(!tape.can_write())
        throw std::invalid_argument("filtered_wtape: !tape.can_write()");

      p=pbuf;
      wret=1;
    }
    ~filtered_wtape_buffer(){
      this->terminate();
    }
  private:
    void terminate(){
      if(state==nullptr)return;

      const byte* dummy=p;
      for(;;){
        byte* p0=p;
        int r=filter(dummy,dummy,p,pbuf+BSIZE,state); // flush internal buffer
        if(r!=0)throw_filter_error(r);

        if(p==p0)break;

        if(p==pbuf+BSIZE)
          this->flush();
      }

      this->flush();

      filter(const_cast<const byte*&>(p),p,p,p,state); // free
      state=nullptr;
    }
  private:
    void throw_filter_error(int r){
      char buff[100];
      std::sprintf(buff,"filtered_wtape::write: error_code %d from filter function",r);
      throw ill_format_error(buff);
    }
  public:
    int write(const void* buff_,int size,int n){
      const byte* const buff0=reinterpret_cast<const byte*>(buff_);
      const byte* const buffN=buff0+size*n;
      if(buff0==buffN)return 0;

      const byte* buff=buff0;
      // buff -(filter)-> pbuf -> tape
      //   pbuf が一杯になるまで書き込まない

      for(;;){
        int r=filter(buff,buffN,p,pbuf+BSIZE,state);
        if(r!=0)throw_filter_error(r);

        if(p==pbuf+BSIZE)
          this->flush();

        if(wret==0)
          return 0;

        if(buff==buffN)
          return n;
      }
    }
    int flush(){
      if(p!=pbuf){
        wret=tape.write(pbuf,p-pbuf);
        p=pbuf;
      }
      return tape.flush();
    }
  };

  mwg::stdm::shared_ptr<filtered_wtape_buffer> ptr;
public:
  filtered_wtape(const BaseTape& tape,const Filter& filter)
    :ptr(new filtered_wtape_buffer(tape,filter)){}
public:
  int read(void* buff_,int size,int n=1) const{
    return 0;
  }
public:
  bool can_read() const{return false;}
  bool can_write() const{return true;}
  bool can_seek() const{return false;}
  bool can_trunc() const{return false;}
  int write(const void* buff,int size,int n=1) const{return ptr->write(buff,size,n);}
  int seek(i8t offset,int whence=SEEK_SET) const{errno=EACCES;return -1;}
  i8t tell() const{return 0;}
  u8t size() const{return 0;}
  int trunc(u8t size) const{errno=EACCES;return -1;}
  int flush() const{return ptr->flush();}
};

#ifdef MWG_CONCEPT_OVERLOAD_FAIL
struct tag_filter_type{
  virtual ~tag_filter_type(){}
  virtual int operator()(const byte*& s,const byte* sN,byte*& d,byte* dN,void*& state) const=0;
};

filtered_rtape<itape,tag_filter_type> operator|(const itape& rtape,const tag_filter_type& filter){
  if(!rtape.can_read())
    throw std::invalid_argument("operator|(rtape,filter)! rtape.can_read()");

  return filtered_rtape<itape,tag_filter_type>(rtape,filter);
}
filtered_wtape<itape,tag_filter_type> operator|(const tag_filter_type& filter,const itape& wtape){
  if(!wtape.can_write())
    throw std::invalid_argument("operator|(filter,wtape)! wtape.can_write()");

  return filtered_wtape<itape,tag_filter_type>(wtape,filter);
}
#else
struct tag_filter_type{};

template<typename T,typename F>
typename stdm::enable_if<
  stdm::is_base_of<itape,T>::value&&stdm::is_base_of<tag_filter_type,F>::value,
  filtered_rtape<T,F> >::type
operator|(const T& rtape,const F& filter){
  if(!rtape.can_read())
    throw std::invalid_argument("operator|(rtape,filter)! rtape.can_read()");

  return filtered_rtape<T,F>(rtape,filter);
}

template<typename T,typename F>
typename stdm::enable_if<
  stdm::is_base_of<itape,T>::value&&stdm::is_base_of<tag_filter_type,F>::value,
  filtered_wtape<T,F> >::type
operator|(const F& filter,const T& wtape){
  if(!wtape.can_write())
    throw std::invalid_argument("operator|(filter,wtape)! wtape.can_write()");
  
  return filtered_wtape<T,F>(wtape,filter);
}
#endif

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  filters
//-----------------------------------------------------------------------------
// Concept: filter_function(s,sN,d,dN,state)
//  state は処理の間、値が保持される。初期値は nullptr である。必要が在れば
//  state に自由にメモリを確保して良い。少なくとも処理の終了・中断時に state
//  を解放する事を指示する呼出がある。(後述の s==sN&&d==dN で呼び出した時。)
//  この折に正しくメモリを解放すれば問題が起こらない様になっている。勿論、こ
//  の「解放の呼出」を待たなくても、関数が不要になったと判断した時に自由に解
//  放して良いし、再度確保しても良い。或いは state をメモリブロックへのポイン
//  タとしてではなく、整数として状態保持に使用しても良い。使い方は完全に自由
//  である。
//
//  s<sN
//    関数は入力からデータを読み取って、出力に書込を行う事。
//    state==nullptr の時、必要が在れば state にメモリを確保して良い。
//    ※必ず一文字以上読み取るか書き込むかする。
//    ※必ずしも入力から読むとは限らない。
//    ※必ずしも出力に書くとは限らない。
//  s==sN
//    関数はストリーム末端の書込 (state flush) を行う事。
//    ※必ず一回で書込が終了するとは限らない (dN-d が足りない時)
//  s==sN&&d==dN
//    関数は state のメモリ解放を行い nullptr を代入する事。
//    処理が終了した後に state!=nullptr の時に呼び出される。
//
//-----------------------------------------------------------------------------

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4180) /* `const' modifier to function reference */
#endif
template<typename F>
class filter_function_filter:public tag_filter_type{
  const F& filter_function;
  typedef int(sgn_t)(const byte*&,const byte*,byte*&,byte*,void*&);
public:
  filter_function_filter(const F& func):filter_function(func){}
  int operator()(const byte*& s,const byte* sN,byte*& d,byte* dN,void*& state) const{
    return mwg::functor_traits<F,sgn_t>::invoke(filter_function,s,sN,d,dN,state);
  }
};
#ifdef _MSC_VER
# pragma warning(pop)
#endif

typedef int filter_function_type(const byte*&,const byte*,byte*&,byte*,void*&);

template<typename F>
typename mwg::stdm::enable_if<mwg::be_functor<F,filter_function_type>::value,filter_function_filter<F> >::type
filtered(const F& filterFunction){
  return filter_function_filter<F>(filterFunction);
}

//-----------------------------------------------------------------------------

filter_function_type hex_encode;
filter_function_type hex_decode;
filter_function_type base64_encode;
filter_function_type base64_decode;

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}

#ifndef MWG_CONCEPT_OVERLOAD_FAIL
template<typename F,typename T> typename mwg::stdm::enable_if<
  (mwg::be_functor<F,void(const T&)>::value&&mwg::stdm::is_base_of<mwg::bio::itape,T>::value),
void>::type operator|(const F& writer,const T& tape){
  if(!tape.can_write())
    throw std::invalid_argument("operator|(writer,tape): !tape.can_write()");
  writer(tape);
}
#endif

#endif
#pragma%x begin_check
// mmake_check_flags: -L "$CFGDIR" -lmwg

#include <sstream>
#include <mwg/except.h>
#include <mwg/bio/tape.util.inl>

#include <cstring>
#include <mwg/bio/tape.util.inl>
#include <mwg/bio/tape.stream.inl>

void test_filters(){
  static struct{
    const char* text;
    const char* base64;
    const char* hex;
  } data[]={
    {"hello","aGVsbG8=","68656c6c6f"},
    {"world","d29ybGQ=","776f726c64"},
    {
      "17:23:45 up 27 days, 6:28, 3 users, load average: 0.14, 0.30, 0.37",
      "MTc6MjM6NDUgdXAgMjcgZGF5cywgNjoyOCwgMyB1c2VycywgbG9hZCBhdmVyYWdlOiAwLjE0LCAwLjMwLCAwLjM3",
      "31373a32333a343520757020323720646179732c20363a32382c203320757365"
      "72732c206c6f616420617665726167653a20302e31342c20302e33302c20302e3337"
    },
  };

  std::ostringstream dst;
  for(int i=0;i<sizeof data/sizeof*data;i++){
    std::istringstream src1(data[i].text);
    mwg::bio::istream_tape(src1)
      |mwg::bio::filtered(mwg::bio::base64_encode)
      |mwg::bio::ostream_tape(dst);
    mwg_assert(dst.str()==data[i].base64);
    dst.str("");dst.clear();

    std::istringstream src2(data[i].base64);
    mwg::bio::istream_tape(src2)
      |mwg::bio::filtered(mwg::bio::base64_decode)
      |mwg::bio::ostream_tape(dst);
    mwg_assert(dst.str()==data[i].text);
    dst.str("");dst.clear();

    std::istringstream src3(data[i].text);
    mwg::bio::istream_tape(src3)
      |mwg::bio::filtered(mwg::bio::hex_encode)
      |mwg::bio::ostream_tape(dst);
    mwg_assert(dst.str()==data[i].hex);
    dst.str("");dst.clear();

    std::istringstream src4(data[i].hex);
    mwg::bio::istream_tape(src4)
      |mwg::bio::filtered(mwg::bio::hex_decode)
      |mwg::bio::ostream_tape(dst);
    mwg_assert(dst.str()==data[i].text);
    dst.str("");dst.clear();
  }
}

int main(){
  test_filters();

  // mwg::bio::ostream_tape tape(dst);
  // mwg::bio::tape_head<> head(tape);
  
  return 0;
}
#pragma%x end_check
