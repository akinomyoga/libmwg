// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BIO_TAPE_H_stdio
#define MWG_BIO_TAPE_H_stdio
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
# include <io.h>
#else
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
#endif

#if defined(MSDOS)||defined(OS2)||defined(WIN32)||defined(__CYGWIN__)
# include <fcntl.h>
# include <io.h>
# define MWG_BIO_TAPE_H_stdio__require_setmode
#endif

#include <mwg/except.h>
#include <mwg/std/memory>
#include "defs.h"
#include "tape.h"
namespace mwg{
namespace bio{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

class cstd_file_tape;
typedef cstd_file_tape ftape;

class cstd_file_tape:public itape{
  stdm::shared_ptr<FILE> _file;
  std::FILE* file;
  bool f_read;
  bool f_write;
  bool f_seek;
//------------------------------------------------------------------------------
//  Initialization
//------------------------------------------------------------------------------
public:
  cstd_file_tape():
    file(nullptr),
    f_read(false), f_write(false), f_seek(false)
  {}

  cstd_file_tape(FILE* file,const char* mode){
    //std::printf("dbg:cstd_file_tape: file\n");
    this->file=file;
    f_read  = nullptr!=std::strchr(mode,'r');
    f_write = nullptr!=std::strchr(mode,'w');
    f_seek  = nullptr!=std::strchr(mode,'s');

#ifdef MWG_BIO_TAPE_H_stdio__require_setmode
    if(nullptr!=std::strchr(mode,'b'))
      setmode(fileno(file), O_BINARY);
#endif
  }
  // ディスク上のファイルから
  cstd_file_tape(const char* filepath,const char* mode) {
    this->open(filepath, mode);
  }
public:
  bool open(const char* filepath, const char* mode) {
    this->close();

#ifdef _MSC_VER
    file=std::fopen(filepath,mode);
#elif defined(__MINGW32__)
    file=::fopen64(filepath,mode);
#elif defined(__GNUC__)&&(defined(__USE_FILE_OFFSET64)||defined(__USE_LARGEFILE64))
    file=::fopen64(filepath,mode);
#else
    file=std::fopen(filepath,mode);
#endif
    if(file!=nullptr)
      _file.reset(file,std::fclose);

    bool is_a=nullptr!=std::strchr(mode,'a');
    bool is_r=nullptr!=std::strchr(mode,'r');
    bool is_w=nullptr!=std::strchr(mode,'w');
    bool is_p=nullptr!=std::strchr(mode,'+');
    f_read=is_r||is_p;
    f_write=is_w||is_p||is_a;
    f_seek=!is_a;

    return is_alive();
  }

  bool is_alive() const{
    return file!=nullptr;
  }
  //bool is_eof() const{return feof(file);}
  void close(){
    if(this->_file){
      this->_file.reset();
      this->file=nullptr;
    }
  }

//------------------------------------------------------------------------------
//  itape implementation
//------------------------------------------------------------------------------
public:
  bool can_read() const{return f_read;}
  bool can_write() const{return f_write;}
  bool can_seek() const{return f_seek;}
  bool can_trunc() const{return f_seek&&f_write;}
  int read(void* buff,int size,int n=1) const{
    mwg_assert(this->f_read,"must be can_read.");
    return std::fread(buff,size,n,file);
  }
  int write(const void* buff,int size,int n=1) const{
    mwg_assert(this->f_write,"must be can_write.");
    return std::fwrite(buff,size,n,file);
  }
  int flush() const{
    return std::fflush(file);
  }
  int seek(i8t offset,int whence=SEEK_SET) const{
    mwg_assert(this->f_seek,"must be can_seek.");
#ifdef _MSC_VER
    //if(whence==SEEK_CUR)
    //  std::printf("dbg: offset=%lld whence=SEEK_CUR\n",offset);
    return ::_fseeki64(file,offset,whence);
#elif defined(__GNUC__)&&defined(_LARGEFILE_SOURCE)
    std::fflush(file); // 特定の環境ではこれがないと変な事に?
    return ::fseeko(file,offset,whence);
#else
    return std::fseek(file,offset,whence);
#endif
  }
  i8t tell() const{
    mwg_assert(this->f_seek,"must be can_seek.");
#ifdef _MSC_VER
    return ::_ftelli64(file);
#elif defined(__GNUC__)&&defined(_LARGEFILE_SOURCE)
    return ::ftello(file);
#elif defined(__GNUC__)
    fpos_t ret;
    ::fgetpos(file,&ret);
    return (i8t)ret;
#else
    return std::ftell(file);
#endif
  }
  u8t size() const{
    mwg_assert(this->f_seek,"must be can_seek.");
    std::fflush(file);
#ifdef _MSC_VER
    return ::_filelengthi64(::_fileno(file));
#elif defined(__MINGW32__)
    struct _stati64 st;
    ::_fstati64(_fileno(file),&st); // MinGW では fileno はマクロ
    return st.st_size;
#elif defined(__GNUC__)&&(defined(__USE_FILE_OFFSET64)||defined(__USE_LARGEFILE64))
    // ↑条件分岐が之で正しいのかは不明 (何も宣言しなくても使える環境も多い)
    struct stat64 st;
    ::fstat64(::fileno(file),&st);
    return st.st_size;
#else
    struct stat st;
    ::fstat(::fileno(file),&st);
    return st.st_size;
#endif
  }
  int trunc(u8t size) const{
    mwg_assert(this->can_trunc(),"must be can_trunc.");
    std::fflush(file);
#if defined(_MSC_VER)
    return ::_chsize_s(::_fileno(file),size);
#elif defined(__MINGW32__)
    // 何故か MinGW では _chsize_s にも ftruncate64 にも対応していない...??
    if(size>0x100000000LL)return errno=EFBIG;
    return ::_chsize(_fileno(file),size); // MinGW では fileno はマクロ
#elif defined(__GNUC__)&&(defined(__USE_FILE_OFFSET64)||defined(__USE_LARGEFILE64))
    return ::ftruncate64(::fileno(file),size);
#else
    return ::ftruncate(::fileno(file),size);
#endif
  }
};

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
