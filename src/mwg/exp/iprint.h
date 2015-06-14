// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef MWG_EXP_IPRINT_H
#define MWG_EXP_IPRINT_H
#include <cstdlib>
#include <iostream>
#include <typeinfo>
#if defined(__GNUC__)&&__GNUC__>=3
# include <cxxabi.h>
#endif
#include <mwg/defs.h>
#include <mwg/std/type_traits>
namespace mwg{
namespace exp{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
struct iformat{
  virtual ~iformat(){}
};

struct iprint{
  virtual void print(std::ostream& ostr) const{
    ostr<<"[object ";
#if defined(__GNUC__)&&__GNUC__>=3
    {
      int status;
      char* clsname=abi::__cxa_demangle(typeid(*this).name(),0,0,&status);
      if(clsname!=nullptr){
        ostr<<clsname;
        std::free(clsname);
      }else{
        ostr<<typeid(*this).name();
      }
    }
#else
    ostr<<typeid(*this).name();
#endif
    ostr<<"]";
  }
  virtual void print(std::ostream& ostr,const iformat& fmt) const{
    this->print(ostr);
  }
};

namespace sfmt{
  struct sfmt_tag{};
  struct plain_t :public sfmt_tag,iformat{};
  struct human_t :public sfmt_tag,iformat{};
  struct xml_t   :public sfmt_tag,iformat{};
  struct json_t  :public sfmt_tag,iformat{};
  struct ttx_t   :public sfmt_tag,iformat{};
  static plain_t *const plain =nullptr;
  static human_t *const human =nullptr; // human readable format
  static xml_t   *const xml   =nullptr;
  static json_t  *const json  =nullptr;
  static ttx_t   *const ttx   =nullptr;

  // 新しいフォーマットを定義したい場合は、sfmt 名前空間内で定義する事
  //  フラグで指定したい場合には、そういう型を作成する。
  //  template<int Flag> struct myfmt; 的な
}

//fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// operator<<
inline std::ostream& operator<<(std::ostream& ostr,const iprint& obj){
  obj.print(ostr);
  return ostr;
}

class formatted_stream{
  std::ostream& ostr;
  const iformat& fmt;
public:
  formatted_stream(std::ostream& ostr,const iformat& fmt):ostr(ostr),fmt(fmt){}
  std::ostream& operator<<(const iprint& obj) const{
    obj.print(ostr,fmt);
    return ostr;
  }
};
inline formatted_stream operator<<(std::ostream& ostr,const iformat& fmt){
  return formatted_stream(ostr,fmt);
}

template<typename Fmt>
class sfmt_formatted_stream{
  std::ostream& ostr;
public:
  explicit sfmt_formatted_stream(std::ostream& ostr):ostr(ostr){}
  template<typename T>
  typename stdm::enable_if<stdm::is_base_of<iprint,T>::value,std::ostream&>::type
  operator<<(const T& obj) const{
    obj.print(ostr,reinterpret_cast<Fmt*>(0));
    return ostr;
  }
};
template<typename Fmt>
inline typename stdm::enable_if<stdm::is_base_of<sfmt::sfmt_tag,Fmt>::value,sfmt_formatted_stream<Fmt> >::type
operator<<(std::ostream& ostr,Fmt*){
  return sfmt_formatted_stream<Fmt>(ostr);
}
/*?lwiki
 *
 * *例
 *
 * &pre(agh-lang-cpp) <<<EOF
 * struct C:public iprint{
 *   typedef iprint base;
 *   void print(std::ostream& ostr,mwg::exp::sfmt::xml_t*) const{
 *     ostr<<"<C />";
 *   }
 *   void print(std::ostream& ostr,mwg::exp::sfmt::ttx_t*) const{
 *     ostr<<"C{},";
 *   }
 *   virtual void print(std::ostream& ostr,const iformat& fmt) const{
 *     if(typeid(fmt)==typeid(mwg::exp::sfmt::xml_t))
 *       this->print(ostr,mwg::exp::sfmt::xml);
 *     else if(typeid(fmt)==typeid(mwg::exp::sfmt::ttx_t))
 *       this->print(ostr,mwg::exp::sfmt::ttx);
 *     else
 *       this->base::print(ostr,fmt);
 *   }
 * };
 * void example(){
 *   namespace sfmt=mwg::exp::sfmt;
 *   C c;
 *   c.print(std::cout);
 *   c.print(std::cout,sfmt::xml);
 *   c.print(std::cout,sfmt::json);
 *
 *   iprint& p(c);
 *   p.print(std::cout,sfmt::xml_t());
 *   p.print(std::cout,sfmt::json_t());
 *
 *   std::ostr<<c<<std::endl;
 *   std::ostr<<sfmt::xml<<c<<std::endl; // as sfmt_tag*
 *   std::ostr<<sfmt::xml_t()<<c<<std::endl; // as iformat
 * }
 * EOF
 */
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
