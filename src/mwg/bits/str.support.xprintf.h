// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef MWG_STR_SUPPORT_XPRINTF_H
#define MWG_STR_SUPPORT_XPRINTF_H
#include <cstddef>
#include <vector>
#include <string>
#include <mwg/std/utility>
#include <mwg/xprintf.h>
#include <mwg/str.h>
#pragma%include "../impl/VariadicMacros.pp"

//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
// xprintf(strbuf<char>,fmt,...);

namespace mwg{
namespace str_detail{
  template<typename XCH>
  class strbuf_xprintf_writer:public mwg::xprintf_detail::xprintf_writer{
    mwg::strbuf<XCH>& target;
  public:
    strbuf_xprintf_writer(mwg::strbuf<XCH>& target):target(target){}
    virtual void put(std::wint_t ch) const{target.append((XCH)ch);}
  };
}

namespace xprintf_detail{
  template<typename XCH>
  mwg::str_detail::strbuf_xprintf_writer create_xprintf_writer(mwg::strbuf<XCH>& target,adl_helper){return target;}
}
}

//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
// mwg::str(xputf(fmt,...));

namespace mwg{
namespace str_detail{

#pragma%m 1
  template<typename... Args>
  struct adapter_traits<mwg::xprintf_detail::_vxputf_temporary_object<Args...> >
#pragma%end
#pragma%x variadic_expand_ArN
  {
    static const bool available=true;
    typedef char char_type;

    class adapter_type:public strbase<strbuff_policy<char_type> >{
      typedef strbase<strbuff_policy<char_type> > base;
#pragma%m 1
      typedef mwg::xprintf_detail::_vxputf_temporary_object<Args...> source_type;
#pragma%end
#pragma%x variadic_expand_ArN
    public:
      // 下手な事をするよりも xprintf 側で std::string を作ってもらうのが速い。
      // xprintf_writer の実装で std::string を特別扱いしているので。
      adapter_type(source_type const& source):base(std::string(source)){}
    };
  };

}
}

//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
#endif
