// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_CHAR_H
#define MWG_CHAR_H
#include <mwg/defs.h>
namespace mwg{

  // クラス前方宣言、c?t typedefs は mwg/defs.h にある。
  template<typename T, int CP>
  struct char_data {
    T value;
  };

  template<typename T, int CP>
  class char_t {
    T value;
  public:
    char_t(): value(0){}
    char_t(const char_t& ch): value(ch.value) {}
    char_t(const char_data<T, CP>& ch): value(ch.value) {}
    char_t(const T& ch): value(ch) {}
    explicit char_t(int ch): value(ch) {}
    operator T() const {return this->value;}
    operator bool() const {return this->value != 0;}
    operator char_data<T, CP>() const {char_data<T, CP> ret = {this->value}; return ret;}
  public:
    char_t& operator=(const char_t& ch) {this->value = ch.value; return *this;}
    char_t& operator=(const T& ch) {this->value = ch; return *this;}
    char_t& operator+=(const T& d) {this->value += d; return *this;}
    char_t& operator-=(const T& d) {this->value -= d; return *this;}
  public:
    char_t operator+(const T& d) const {return this->value + d;}
    char_t operator-(const T& d) const {return this->value - d;}
    T operator-(const char_t& ch) const {return this->value - ch.value;}
    bool operator==(const char_t& ch) const {return this->value == ch.value;}
    bool operator!=(const char_t& ch) const {return this->value != ch.value;}
    bool operator<=(const char_t& ch) const {return this->value <= ch.value;}
    bool operator>=(const char_t& ch) const {return this->value >= ch.value;}
    bool operator< (const char_t& ch) const {return this->value <  ch.value;}
    bool operator> (const char_t& ch) const {return this->value >  ch.value;}
  };
  template<typename T, int CP>
  char_t<T, CP> operator+(const T& d, const char_t<T,CP>& c) {return c.value + d;}

  static const i4t MS_CP  = 1 << 16;
  static const i4t IBM_CP = 2 << 16;

}
#endif
