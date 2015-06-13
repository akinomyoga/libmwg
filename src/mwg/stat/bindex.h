// -*- mode:c++ -*-
#pragma once
#ifndef LIBMWG_STAT_BINDEX_H
#define LIBMWG_STAT_BINDEX_H
namespace mwg{
namespace stat{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  class bindex{
    int value;
  public:
    bindex():value(0){}
    bindex(int value):value(value){}

    operator int() const{return this->value;}
    bindex& operator++(){++this->value;return *this;}
    bindex& operator--(){--this->value;return *this;}
    bindex operator++(int){return bindex(this->value++);}
    bindex operator--(int){return bindex(this->value--);}
    bindex operator+(int delta) const{return bindex(this->value+delta);}
    bindex operator-(int delta) const{return bindex(this->value-delta);}
    bindex operator+=(int delta){this->value+=delta;return *this;}
    bindex operator-=(int delta){this->value-=delta;return *this;}

    int operator-(const bindex& right) const{return this->value-right.value;}
  };

  typedef bindex bindex_t;
  // //inline bool operator==(const bindex_t& left,int right){return int(left)==right;}
  // //inline bool operator==(int left,const bindex_t& right){return left==int(right);}
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
