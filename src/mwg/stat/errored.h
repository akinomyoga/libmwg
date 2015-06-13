// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef MWG_STAT_ERRORED_VALUE_H
#define MWG_STAT_ERRORED_VALUE_H
#include <cstdio>
#include <cmath>
#include <mwg/range.h>
namespace mwg{
namespace stat{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

  template<typename T=double>
  class errored;
  static class variance_tag_type*  const variance_tag=0;//nullptr;
  static class deviation_tag_type* const deviation_tag=0;//nullptr;

//-----------------------------------------------------------------------------
  template<typename T>
  class errored{
    T m_value;
    T m_var; // e^2
  public:
    T value() const{return m_value;}
    T error() const{return std::sqrt(m_var);}
    T relative_error() const{return std::sqrt(m_var)/m_value;}
    T relative_variance() const{return m_var/(m_value*m_value);}
    T variance() const{return m_var;}
    T upper1sigma() const{return m_value+std::sqrt(m_var);}
    T lower1sigma() const{return m_value-std::sqrt(m_var);}
  public:
    errored():m_value(0),m_var(HUGE_VAL){}
    errored(T value,T var,variance_tag_type*)
      :m_value(value),m_var(var){}
    errored(T value,T delta,deviation_tag_type* =0)
      :m_value(value),m_var(delta*delta){}
    explicit errored(mwg::range_t<T> const& range)
      :m_value(0.5*(range.begin()+range.end())),m_var(0.25*range.length()*range.length()){}
    errored operator+(const errored& r) const{
      return errored(m_value+r.m_value,this->m_var+r.m_var,variance_tag);
    }
    errored operator-(const errored& r) const{
      return errored(m_value-r.m_value,this->m_var+r.m_var,variance_tag);
    }
    errored operator+() const{
      return *this;
    }
    errored operator-() const{
      return errored(-m_value,this->m_var,variance_tag);
    }
    errored operator*(const errored& r) const{
      T const v(m_value*r.m_value);
      return errored(v,v*v*(relative_variance()+r.relative_variance()),variance_tag);
    }
    errored operator/(const errored& r) const{
      T const v(m_value/r.m_value);
      return errored(v,v*v*(relative_variance()+r.relative_variance()),variance_tag);
    }

    errored operator/(const T& scalar) const{
      return errored(m_value/scalar,m_var/(scalar*scalar),variance_tag);
    }
    errored operator*(const T& scalar) const{
      return errored(m_value*scalar,m_var*(scalar*scalar),variance_tag);
    }
  public:
    errored squared() const{
      T const v(m_value*m_value);
      return errored(v,4*m_var*v,variance_tag); // (2 x dx)^2 = 4 x^2 dx^2
    }
    errored cubed() const{
      T const v(m_value*m_value*m_value);
      return errored(v,9*m_var*v*m_value,variance_tag); // (3 x^2 dx)^2 = 9 x^4 dx^2
    }
    static errored sqrt_impl(const errored& value){
      return errored(std::sqrt(value.m_value),value.m_var/(4.0*value.m_value),variance_tag);
    }
  };

  template<typename T>
  errored<T> operator*(const T& scalar,const errored<T>& errored){
    return errored*scalar;
  }

  template<typename T>
  std::ostream& operator<<(std::ostream& ostr,const errored<T>& v){
    return ostr<<v.value()<<" "<<v.error();
  }

  template<typename T>
  std::istream& operator>>(std::istream& istr,errored<T>& v){
    T value;
    T error;
    istr>>value>>error;
    v=errored<T>(value,error);
    return istr;
  }

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
namespace std{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

  template<typename T>
  mwg::stat::errored<T> abs(const mwg::stat::errored<T>& value){
    return mwg::stat::errored<T>(std::abs(value.value()),value.variance(),mwg::stat::variance_tag);
  }
  template<typename T>
  mwg::stat::errored<T> sqrt(const mwg::stat::errored<T>& value){
    return mwg::stat::errored<T>::sqrt_impl(value);
  }
  template<typename T>
  mwg::stat::errored<T> pow(const mwg::stat::errored<T>& value,const T& exponent){
    T const v=std::pow(value.value(),exponent);
    T const t=v*exponent/value.value();
    return mwg::stat::errored<T>(v,t*t*value.variance(),mwg::stat::variance_tag);
  }
  template<typename T>
  mwg::stat::errored<T> pow(const T& value,const mwg::stat::errored<T>& exponent){
    T const v=std::pow(value,exponent.value());
    T const t=v*std::log(value);
    return mwg::stat::errored<T>(v,t*t*exponent.variance(),mwg::stat::variance_tag);
  }
  template<typename T>
  mwg::stat::errored<T> pow(const mwg::stat::errored<T>& x,const mwg::stat::errored<T>& y){
    T const v(std::pow(x.value(),y.value()));
    T const ty=std::log(x.value());
    T const tx=y.value()/x.value();
    return mwg::stat::errored<T>(v,v*v*(ty*ty*y.variance()+tx*tx*x.variance()),mwg::stat::variance_tag);
  }
  template<typename T>
  mwg::stat::errored<T> exp(const mwg::stat::errored<T>& x){
    T const v=std::exp(x.value());
    return mwg::stat::errored<T>(v,v*v*x.variance(),mwg::stat::variance_tag);
  }
  template<typename T>
  mwg::stat::errored<T> log(const mwg::stat::errored<T>& x){
    return mwg::stat::errored<T>(
      std::log(x.value()),
      x.variance()/(x.value()*x.value()),
      mwg::stat::variance_tag
    );
  }
  template<typename T>
  mwg::stat::errored<T> log10(const mwg::stat::errored<T>& x){
    T const t=1.0/(std::log(10)*x.value());
    return mwg::stat::errored<T>(
      std::log10(x.value()),
      t*t*x.variance(),
      mwg::stat::variance_tag
    );
  }
  template<typename T>
  mwg::stat::errored<T> sin(const mwg::stat::errored<T>& x){
    T const t=std::cos(x.value()); // dsin(x) = cos(x) dx
    return mwg::stat::errored<T>(
      std::sin(x.value()),
      t*t*x.variance(),
      mwg::stat::variance_tag
    );
  }
  template<typename T>
  mwg::stat::errored<T> cos(const mwg::stat::errored<T>& x){
    T const t=std::sin(x.value()); // dcos(x) = - sin(x) dx
    return mwg::stat::errored<T>(
      std::cos(x.value()),
      t*t*x.variance(),
      mwg::stat::variance_tag
    );
  }
  template<typename T>
  mwg::stat::errored<T> tan(const mwg::stat::errored<T>& x){
    T const v=std::tan(x.value());
    T const t=v*v+1;               // dtan(x) = (tan^2(x) + 1) dx
    return mwg::stat::errored<T>(v,t*t*x.variance(),mwg::stat::variance_tag);
  }
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
#endif
