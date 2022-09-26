// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BIO_TAPE_STREAM_INL
#define MWG_BIO_TAPE_STREAM_INL
#include <string>
namespace mwg{
namespace bio{

  class istream_tape;
  class ostream_tape;

  template<typename ITape=const itape&,typename Tr=std::char_traits<char> > class basic_tape_streambuf;
  template<typename ITape=const itape&,typename Tr=std::char_traits<char> > class basic_tape_stream;
  template<typename ITape=const itape&,typename Tr=std::char_traits<char> > class basic_tape_istream;
  template<typename ITape=const itape&,typename Tr=std::char_traits<char> > class basic_tape_ostream;

  typedef basic_tape_streambuf<>  taperef_streambuf;
  typedef basic_tape_stream<>     taperef_stream;
  typedef basic_tape_istream<>    taperef_istream;
  typedef basic_tape_ostream<>    taperef_ostream;
}
}
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
#include <cstring>
#include <iostream>
#include <mwg/concept.h>
#include <mwg/std/type_traits>
#include <mwg/std/memory>
#include "defs.h"
#include "tape.h"
namespace mwg{
namespace bio{
//-----------------------------------------------------------------------------
//  class istream_tape, ostream_tape
//-----------------------------------------------------------------------------
namespace detail{
  template<typename Stream>
  class stream_tape_base:public itape{
    typedef stream_tape_base self;
  protected:
    Stream& str;
    bool flag_seek;
    stream_tape_base(Stream& str,const char* mode=""):str(str){
      flag_seek=std::strchr(mode,'s')!=nullptr;
    }

  public:
    virtual bool can_read() const {return false;}
    virtual bool can_write() const{return false;}
    virtual bool can_seek() const {return this->flag_seek;}
    virtual bool can_trunc() const{return false;}

    virtual int read(void* buff,int size,int n=1) const{
      mwg_unused(buff);
      mwg_unused(size);
      mwg_unused(n);
      mwg_assert(self::can_read(),"not supported operation.");
      return 0;
    }
    virtual int write(const void* buff,int size,int n=1) const{
      mwg_unused(buff);
      mwg_unused(size);
      mwg_unused(n);
      mwg_assert(self::can_write(),"not supported operation.");
      return 0;
    }
    virtual int flush() const{return EOF;}

    virtual u8t size() const{
      mwg_assert(this->can_seek());

      i8t pos=this->tell();
      mwg_assert(pos!=-1,"unable to tell.");
      mwg_verify(this->seek(0,SEEK_END)==0,"failed to seek the end position.");
      i8t ret=this->tell();
      mwg_verify(this->seek(pos)==0,"failed to restore the position.");
      return ret!=-1?ret:0;
    }
    virtual int trunc(u8t size) const{
      mwg_unused(size);
      mwg_assert(false);
      return -1;
    }

    virtual bool is_alive() const{return !str.eof();}
    // virtual int seek(i8t offset,int whence=SEEK_SET) const
    // virtual i8t tell() const
  };

}

  class istream_tape:public detail::stream_tape_base<std::istream>{
    typedef detail::stream_tape_base<std::istream> base;

  public:
    istream_tape(std::istream& str,const char* mode=""):base(str,mode){}

  public:
    virtual bool can_read() const {return true;}

  public:
    virtual int read(void* buff,int size,int n=1) const{
      str.read(reinterpret_cast<char*>(buff),size*n);
      return str?n:str.gcount()/size;
    }
    virtual int seek(i8t offset,int whence=SEEK_SET) const{
      mwg_assert(this->can_seek());
      if(whence==SEEK_END)
        str.seekg((std::streamoff)offset,std::ios_base::end);
      else if(whence==SEEK_CUR)
        str.seekg((std::streamoff)offset,std::ios_base::cur);
      else
        str.seekg((std::streampos)offset);
      return str.fail()?1:0;
    }
    virtual i8t tell() const{
      mwg_assert(this->can_seek());
      return str.tellg();
    }
  };

  class ostream_tape:public detail::stream_tape_base<std::ostream>{
    typedef detail::stream_tape_base<std::ostream> base;
  public:
    ostream_tape(std::ostream& str,const char* mode=""):base(str,mode){}

  public:
    virtual bool can_write() const{return true;}

  public:
    virtual int write(const void* buff,int size,int n=1) const{
      // std::ostream::write に失敗した時、実際に書き込まれた量を取得する方法はない様だ
      // (あるいは put を1文字ずつ実行するしかないのだろうか)。
      //
      // c.f. http://stackoverflow.com/questions/14238572/how-many-bytes-actually-written-by-ostreamwrite
      //   特に書き込み先がファイルシステムやネットワークの場合、実際に書き込まれたか・相手が受信したかどうかはその場では分からない。
      //   (書き込み先がメモリ等の場合など、書き込まれた量を原理的に把握できるケースも存在するが、特殊ケース扱いなのだろう。)
      str.write(reinterpret_cast<const char*>(buff),size*n);
      return str?n:0;
    }
    virtual int seek(i8t offset,int whence=SEEK_SET) const{
      mwg_assert(this->can_seek());
      if(whence==SEEK_END)
        str.seekp((std::streamoff)offset,std::ios_base::end);
      else if(whence==SEEK_CUR)
        str.seekp((std::streamoff)offset,std::ios_base::cur);
      else
        str.seekp((std::streampos)offset);
      return str.fail()?1:0;
    }
    virtual i8t tell() const{
      mwg_assert(this->can_seek());
      return str.tellp();
    }
    virtual int flush() const{
      str.flush();
      return str.fail()?EOF:0;
    }
  };

//-----------------------------------------------------------------------------
//  class basic_tape_streambuf
//-----------------------------------------------------------------------------
template<typename ITape,typename Tr>
class basic_tape_streambuf:public std::basic_streambuf<char,Tr>{
  ITape tape;
  typedef Tr traits_type;
  typedef typename Tr::char_type    char_type;
  typedef typename Tr::int_type     int_type;
  typedef typename stdm::remove_reference<ITape>::type tape_type;
//---------------------------------------------------------
// buffering
private:
  bool buff_to_delete;
  char_type* gbuff;
  char_type* gbuff_end;
  char_type* pbuff;
  char_type* pbuff_end;
protected:
  static const int DEFAULT_BUFFSIZE=32*1024;
  //static const int DEFAULT_BUFFSIZE=256;
  void buff_alloc(int psize=DEFAULT_BUFFSIZE,int gsize=DEFAULT_BUFFSIZE){
    this->buff_free();
    this->buff_to_delete=true;
    this->pbuff=new char_type[psize];
    this->pbuff_end=pbuff+psize;
    this->gbuff=new char_type[gsize];
    this->gbuff_end=gbuff+gsize;
    this->setp(pbuff,pbuff_end);
    this->setg(gbuff,gbuff_end,gbuff_end);
  }
  void buff_free(){
    if(this->pbuff!=nullptr)this->sync();
    if(this->buff_to_delete){
      this->buff_to_delete=false;
      delete this->pbuff;
      delete this->gbuff;
    }
    this->pbuff=nullptr;
    this->pbuff_end=nullptr;
    this->gbuff=nullptr;
    this->gbuff_end=nullptr;
  }
protected:
  virtual std::basic_streambuf<char,Tr>* setbuf(char_type* buffer,std::streamsize count){
    this->buff_free();
    this->buff_to_delete=false;
    this->gbuff=buffer;
    this->gbuff_end=buffer+count;
    this->pbuff=buffer;
    this->pbuff_end=buffer+count;
    this->setp(pbuff,pbuff_end);
    this->setg(gbuff,gbuff_end,gbuff_end);
    return this;
  }
//---------------------------------------------------------
// ctor/dtor
public:
  basic_tape_streambuf(const tape_type& tape)
    :tape(tape),buff_to_delete(false),pbuff(nullptr),gbuff(nullptr)
  {
    this->buff_alloc();
  }
  basic_tape_streambuf(const basic_tape_streambuf& r)
    :tape(r.tape),buff_to_delete(false),pbuff(nullptr),gbuff(nullptr)
  {
    this->buff_alloc();
  }
  basic_tape_streambuf& operator=(const basic_tape_streambuf& r) mwg_std_deleted;
  ~basic_tape_streambuf(){
    this->buff_free();
  }
//---------------------------------------------------------
// i/o operations
protected:
  virtual std::streampos seekoff(
    std::streamoff off,
    std::ios_base::seekdir dir,
    int nMode=std::ios::in|std::ios::out
  ){
    mwg_unused(nMode);
    this->sync();
    tape.seek(
      off,
      dir==std::ios_base::beg?SEEK_SET:
      dir==std::ios_base::cur?SEEK_CUR:
      dir==std::ios_base::end?SEEK_END:
      SEEK_SET
    );
    this->setg(gbuff,gbuff_end,gbuff_end);
    return EOF;
  }
  virtual std::streampos seekpos(
    std::streampos pos,
    int nMode=std::ios::in|std::ios::out
  ){
    mwg_unused(nMode);
    this->sync();
    tape.seek(pos,SEEK_SET);
    this->setg(gbuff,gbuff_end,gbuff_end);
    return EOF;
  }
protected:
  virtual int sync(){
    if(this->pbuff==nullptr)return 0;
    // flush output buffer
    char_type* pb=this->pbase();
    char_type* pp=this->pptr();
    if(pp!=pb){
      this->tape.write(pb,pp-pb);
      this->pbump(pb-pp); // 巻き戻し
    }
    return 0;
  }
  virtual int_type overflow(
    int_type meta=traits_type::eof()
  ){
    if(this->sync()!=0)
      return traits_type::eof();

    if(meta!=traits_type::eof()){
      char_type ch=traits_type::to_char_type(meta);
      if(this->pbase()<this->epptr()){
        *this->pptr()=ch;
        this->pbump(1);
      }else{
        this->tape.write(&ch,sizeof(char));
      }
    }
    return traits_type::not_eof(meta);
  }
  virtual int_type underflow(){
    char_type* gp=this->gptr();
    char_type* ge=this->egptr();
    if(gp<ge){
      return traits_type::to_int_type(*gp);
    }else{
      int c=this->tape.read(gbuff,1,gbuff_end-gbuff);
      if(0==c)return traits_type::eof();
      this->setg(gbuff,gbuff,gbuff+c);
      return traits_type::to_int_type(*gbuff);
    }
  }
};

//-----------------------------------------------------------------------------
//  class basic_tape_stream
//-----------------------------------------------------------------------------

template<typename ITape, typename Tr>
class basic_tape_stream: public std::basic_iostream<char, Tr> {
  typedef typename stdm::remove_reference<ITape>::type tape_type;
public:
  explicit basic_tape_stream(const tape_type& tape):
    std::basic_iostream<char, Tr>(new basic_tape_streambuf<ITape, Tr>(tape)) {}
};
template<typename ITape, typename Tr>
class basic_tape_istream: public std::basic_istream<char, Tr> {
  typedef typename stdm::remove_reference<ITape>::type tape_type;
public:
  basic_tape_istream() {}
  explicit basic_tape_istream(const tape_type& tape):
    std::basic_istream<char, Tr>(new basic_tape_streambuf<ITape, Tr>(tape)) {}
};
template<typename ITape, typename Tr>
class basic_tape_ostream: public std::basic_ostream<char, Tr> {
  typedef typename stdm::remove_reference<ITape>::type tape_type;
public:
  basic_tape_ostream() {}
  explicit basic_tape_ostream(const tape_type& tape):
    std::basic_ostream<char, Tr>(new basic_tape_streambuf<ITape, Tr>(tape)) {}
private:
  basic_tape_ostream(const basic_tape_ostream&);
};

template<typename T>
class shared_ref {
  stdm::shared_ptr<T> ptr;
  template<class U> friend class shared_ref;
public:
  template<typename U>
  shared_ref(U* ptr, typename stdm::enable_if<stdm::is_convertible<U*, T*>::value, mwg::invalid_type*>::type = 0): ptr(ptr) {}
  template<typename U>
  explicit shared_ref(const shared_ref<U>& ref, typename stdm::enable_if<stdm::is_convertible<U*, T*>::value, mwg::invalid_type*>::type = 0): ptr(ref.ptr) {}
  template<typename U>
  explicit shared_ref(const U& value): ptr(new T(value)) {}
  operator T&() const { return *this->ptr; }
  T& get() const { return *this->ptr; }
};

namespace detail{
  //template<typename A>
  //struct adapter_cast_impl<std::iostream,A,adapter_cast_from_tape>
  //  :adapter_cast_simple_construct<basic_tape_stream<A>,A>{};
  //template<typename A>
  //struct adapter_cast_impl<std::istream,A,adapter_cast_from_tape>
  //  :adapter_cast_simple_construct<basic_tape_istream<A>,A>{};
  //template<typename A>
  //struct adapter_cast_impl<std::ostream,A,adapter_cast_from_tape>
  //  :adapter_cast_simple_construct<basic_tape_ostream<A>,A>{};

  //template<typename A>
  //struct adapter_cast_impl<std::iostream&,A,adapter_cast_from_tape>
  //  :adapter_cast_simple_construct<taperef_stream,A>{};
  //template<typename A>
  //struct adapter_cast_impl<std::istream&,A,adapter_cast_from_tape>
  //  :adapter_cast_simple_construct<taperef_istream,A>{};
  //template<typename A>
  //struct adapter_cast_impl<std::ostream&,A,adapter_cast_from_tape>
  //  :adapter_cast_simple_construct<taperef_ostream,A>{};

  template<typename A>
  struct adapter_cast_impl<std::iostream,A,adapter_cast_from_tape>
    :adapter_cast_simple_construct<shared_ref<basic_tape_stream<A> >,A>{};
  template<typename A>
  struct adapter_cast_impl<std::istream,A,adapter_cast_from_tape>
    :adapter_cast_simple_construct<shared_ref<basic_tape_istream<A> >,A>{};
  template<typename A>
  struct adapter_cast_impl<std::ostream,A,adapter_cast_from_tape>
    :adapter_cast_simple_construct<shared_ref<basic_tape_ostream<A> >,A>{};

  template<typename A>
  struct adapter_cast_impl<std::iostream&,A,adapter_cast_from_tape>
    :adapter_cast_simple_construct<shared_ref<taperef_stream>,A>{};
  template<typename A>
  struct adapter_cast_impl<std::istream&,A,adapter_cast_from_tape>
    :adapter_cast_simple_construct<shared_ref<taperef_istream>,A>{};
  template<typename A>
  struct adapter_cast_impl<std::ostream&,A,adapter_cast_from_tape>
    :adapter_cast_simple_construct<shared_ref<taperef_ostream>,A>{};
}

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
