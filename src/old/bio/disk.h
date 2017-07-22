// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BIO_DISK_H
#define MWG_BIO_DISK_H
#include "defs.h"
namespace mwg{
namespace bio{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

class idisk{
public:
  virtual bool can_read() const=0;
  virtual bool can_write() const=0;
  virtual int read(void* buff,u8t pos,int size,int n=1) const=0;
  virtual int write(void* buff,u8t pos,int size,int n=1) const=0;
  virtual u8t size() const=0;

  virtual ~idisk(){}
public:
  template<typename T>
  bool read_val(T& buff,u8t pos) const{
    return 1==this->read(&buff,pos,sizeof buff,1);
  }
  template<typename T>
  bool write_val(T& buff,u8t pos) const{
    return 1==this->write(&buff,pos,sizeof buff,1);
  }
  template<typename T>
  bool read_arr(T& buff,u8t index) const{
    return 1==this->read(&buff,index*sizeof buff,sizeof buff,1);
  }
  template<typename T>
  bool write_arr(T& buff,u8t index) const{
    return 1==this->write(&buff,index*sizeof buff,sizeof buff,1);
  }
};

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
