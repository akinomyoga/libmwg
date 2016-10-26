// -*- mode:C++;coding:utf-8 -*-
#ifndef MWG_BITS_STR_SUPPORT_XPRINTF_H
#define MWG_BITS_STR_SUPPORT_XPRINTF_H
#include <cstddef>
#include <vector>
#include <string>
#include <mwg/std/utility>
#include <mwg/xprintf.h>
#include <mwg/str.h>
#pragma%include "../impl/VariadicMacros.pp"
#pragma%include "../impl/ManagedTest.pp"
#pragma%x begin_check
// mmake_check_flags: -L "$CFGDIR" -lmwg
#include <mwg/xprintf.h>
#include <mwg/str.h>
#ifndef MWG_BITS_STR_SUPPORT_XPRINTF_H
# error <mwg/bits/str.support.xprintf.h> not automatically included
#endif

#pragma%x end_check

//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
// xprintf(strbuf<char>,fmt,...);

namespace mwg{
namespace str_detail{
  template<typename XCH>
  class strbuf_xprintf_writer_proc{
    mwg::strbuf<XCH>& target;
  public:
    strbuf_xprintf_writer_proc(mwg::strbuf<XCH>& target):target(target){}
    void operator()(XCH const* buffer,int count) const{
      target.append(buffer,buffer+count);
    }
  };
}

namespace xprintf_detail{
  template<typename XCH>
  xprintf_writer create_xprintf_writer(mwg::strbuf<XCH>& target,bool flagClear,adl_helper){
    if(flagClear)target.clear();
    return mwg::str_detail::strbuf_xprintf_writer_proc<XCH>(target);
  }
}
}

#pragma%x begin_test
void test(){
  mwg_check( (mwg::be_functor<mwg::str_detail::strbuf_xprintf_writer_proc<char>,void(char const*,int)>::value));
  mwg::strbuf<char> buff;
  mwg::xprintf(buff,"%s/ph%03d/dens%06d.dat","work",12,12345);
  mwg_check((buff=="work/ph012/dens012345.dat"));
}
#pragma%x end_test

//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
// mwg::str(xputf(fmt,...));

namespace mwg{
namespace str_detail{

  template<typename Tmp>
  class _stradp_xputf_tmp:public strbase<strbuf_policy<char> >{
    typedef strbase<strbuf_policy<char> > base;
    typedef Tmp source_type;
  public:
    // 下手な事をするよりも xprintf 側で std::string を作ってもらうのが速い。
    // xprintf_writer の実装で std::string を特別扱いしているので。
    _stradp_xputf_tmp(source_type const& source):base(std::string(source)){}
  };

  template<typename Tuple>
  struct adapter_traits<mwg::xprintf_detail::_vxputf_temporary_object<Tuple> >{
    static const bool available=true;
    typedef char char_type;
    typedef _stradp_xputf_tmp<mwg::xprintf_detail::_vxputf_temporary_object<Tuple> > adapter_type;
  };

  template<typename Tuple>
  struct adapter_traits<mwg::xprintf_detail::_xputf_temporary_object<Tuple> >{
    static const bool available=true;
    typedef char char_type;
    typedef _stradp_xputf_tmp<mwg::xprintf_detail::_xputf_temporary_object<Tuple> > adapter_type;
  };

#pragma%x begin_test
  void test(){
    typedef mwg::xprintf_detail::_xputf_temporary_object<mwg::vararg::packed_forward<char[6],int> > xputf_return_type;
    mwg_check((mwg::str_detail::adapter_traits<xputf_return_type>::available));
    mwg_check((mwg::str(mwg::xputf("%s/ph%03d.bin","hydro",12))=="hydro/ph012.bin"));
  }
#pragma%x end_test

}
}
//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
#endif
#pragma%x begin_check
int main(){
  managed_test::run_tests();
  return 0;
}
#pragma%x end_check
