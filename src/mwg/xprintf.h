// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef MWG_XPRINTF_H
#define MWG_XPRINTF_H
#include <cstdlib>
#include <cstring>
#include <mwg/std/tuple>
#include <mwg/std/utility>
#include <mwg/std/type_traits>

/*
 * ★型 Target を新しく出力先として登録する方法
 *
 * in myheader.h
 * | namespace MyNamespace{
 * |   class custom_writer:public mwg::xprintf_detail::xprintf_writer{
 * |     virtual void put(std::wint_t ch) const{ ... }
 * |   };
 * | }
 * | namespace mwg{
 * | namespace xprintf_detail{
 * |   MyNamespace::custom_writer create_xprintf_writer(Target& target,adl_helper*);
 * | }
 * | }
 *
 * namespace mwg::xprintf_detail の中に create_xprintf_writer という関数を定義する。
 * この関数は Target& target を受け取って、
 * インターフェイス xprintf_writer を実装するクラスのインスタンスを生成する。
 *
 * インターフェイス xprintf_writer は void put(std::wint_t) const; という純粋仮想関数を持つ。
 * この put メンバ関数は文字を受け取って、その文字を出力する処理を行う。
 * 内部でバッファリングをする場合はデストラクタで flush するのを忘れない様に。
 *
 *
 * ★型 T の引数に対する書式出力を定義する方法
 *
 * template<typename Buff>
 * int mwg::xprintf_detail::xprintf_convert(
 *   Buff const& buff,fmtspec const& spec,MyType const& value,
 *   mwg::xprintf_detail::adl_helper*);
 * を実装すれば良い。
 *
 * in xxx.h
 * | namespace mwg{
 * | namespace xprintf_detail{
 * |   template<typename Writer>
 * |   int xprintf_convert(Writer const& buff,fmtspec const& spec,MyType const& value,adl_helper*){
 * |     実装
 * |   }
 * | }
 * | }
 *
 * 実装を隠蔽または分離したい場合は、
 * Writer = xprintf_writer, cfile_writer, ostream_writer, string_writer の4つについて、
 * 関数テンプレートのインスタンス化をする。
 *
 * in yyy.h
 * | namespace mwg{
 * | namespace xprintf_detail{
 * |   template<typename Writer>
 * |   int xprintf_convert(Writer const& buff,fmtspec const& spec,MyType const& value,adl_helper*);
 * | }
 * | }
 *
 * in yyy.cpp
 * | namespace mwg{
 * | namespace xprintf_detail{
 * |   template<typename Writer>
 * |   int xprintf_convert(Writer const& buff,fmtspec const& spec,MyType const& value,adl_helper*){
 * |     実装
 * |   }
 * |
 * |   // template int xprintf_convert<xprintf_writer>(xprintf_writer const& buff,fmtspec const& spec,MyType const& value);
 * |   // template int xprintf_convert<cfile_writer  >(cfile_writer   const& buff,fmtspec const& spec,MyType const& value);
 * |   // template int xprintf_convert<ostream_writer>(ostream_writer const& buff,fmtspec const& spec,MyType const& value);
 * |   // template int xprintf_convert<string_writer >(string_writer  const& buff,fmtspec const& spec,MyType const& value);
 * |   template void _instantiate_xprintf_convert<MyType>(MyType const&);
 * | }
 * | }
 *
 */

//-----------------------------------------------------------------------------
// mwg::vararg::evaluate_element

namespace mwg{
namespace vararg{
  using namespace mwg;

  // 二分探索
  template<std::size_t L,std::size_t U,typename TT,bool Binary=(L+1<U)>
  struct tuple_element_selector__impl{
    static const std::size_t M=(L+U)/2;
    template<typename F>
    static typename F::return_type eval(F const& obj,int index,TT const& tp){
      if(std::size_t(index)<M)
        return tuple_element_selector__impl<L,M,TT>::eval(obj,index,tp);
      else
        return tuple_element_selector__impl<M,U,TT>::eval(obj,index,tp);
    }
  };

  template<std::size_t L,std::size_t U,typename TT>
  struct tuple_element_selector__impl<L,U,TT,false>{
    template<typename F>
    static typename F::return_type eval(F const& obj,int index,TT const& tp){
      return obj.eval(mwg::stdm::get<L>(tp));
    }
  };

  template<std::size_t L,std::size_t U,typename TT>
  struct tuple_element_selector:tuple_element_selector__impl<L,U,TT>{};

  template<typename TT,typename Eval>
  typename Eval::return_type
  evaluate_element(Eval const& evaluater,TT const& tp,int index){
    const int U=mwg::stdm::tuple_size<TT>::value;
    if(index<0||U<=index)
      return evaluater.out_of_range();
    else
      return tuple_element_selector<0,U,TT>::eval(evaluater,index,tp);
  }

  template<typename XTuple,typename YTuple>
  struct pack_forward_enabler{};
  template<>
  struct pack_forward_enabler<mwg::stdm::tuple<>,mwg::stdm::tuple<> >:mwg::stdm::true_type{};
  template<typename X,typename Y,typename... XArgs,typename... YArgs>
  struct pack_forward_enabler<mwg::stdm::tuple<X,XArgs...>,mwg::stdm::tuple<Y,YArgs...> >{
    static const bool head_value=
      (!mwg::stdm::is_lvalue_reference<X>::value||mwg::stdm::is_lvalue_reference<Y>::value)
      &&mwg::stdm::is_convertible<
        typename mwg::stdm::remove_reference<X>::type*,
        typename mwg::stdm::remove_reference<Y>::type*>::value;

    static const bool value=head_value
      &&pack_forward_enabler<mwg::stdm::tuple<XArgs...>,mwg::stdm::tuple<YArgs...> >::value;
  };

  template<typename... XArgs,typename... YArgs>
  typename mwg::stdm::enable_if<
    pack_forward_enabler<mwg::stdm::tuple<XArgs...>,mwg::stdm::tuple<YArgs...> >::value,
    mwg::stdm::tuple<XArgs&&...>
    >::type
  va_forward(YArgs&&... args){
    typedef mwg::stdm::tuple<XArgs&&...> return_type;
    return return_type(mwg::stdm::forward<XArgs>(args)...);
  }
}
}

//-----------------------------------------------------------------------------
// mwg::xprintf_detail::read_fmtspec

namespace mwg{
namespace xprintf_detail{
  struct adl_helper{};

  enum fmtspec_flags{
    // 左右位置 (型非依存)
    flag_left =0x01,
    flag_zero =0x02,

    // 符号 (型依存)
    flag_plus =0x04,
    flag_space=0x08,

    // 装飾 (型依存)
    flag_hash =0x10,
    flag_quote=0x20,

    // 出力抑制 (実装用)
    flag_suppress=0x100,
  };

  enum fmtspec_type{
    type_default,
    type_h,
    type_l,
    type_hh,
    type_ll,
    type_L,
    type_j,
    type_z,
    type_t,
    type_q,
    type_w,
    type_I,
    type_I8,
    type_I16,
    type_I32,
    type_I64,
  };

  struct fmtspec{
    int pos;
    int flags;
    int width;
    int width_pos;
    int precision;
    int precision_pos;
    fmtspec_type type;
    char conv;
  };

  void read_fmtspec(fmtspec& spec,const char*& _p);

}
}

//-----------------------------------------------------------------------------
// mwg::xprintf_detail::va_getIndex

namespace mwg{
namespace xprintf_detail{

  inline int va_getIndex(...){
    throw mwg::except("mwg::xprintf(): invalid argument type");
  }
  inline int va_getIndex(int const& value){
    return value;
  }

  struct va_getIndex_eval{
    typedef int return_type;

    template<typename T>
    static int eval(T const& value){
      return va_getIndex(value);
    }

    static int out_of_range(){return -1;}
  };

  template<typename TT>
  int va_getIndex(int index,TT const& tp){
    return mwg::vararg::evaluate_element(va_getIndex_eval(),tp,index);
  }


  inline const char* va_getFormat(...){
    throw mwg::except("mwg::xprintf(): invalid argument type (expecting const char* fmt)");
  }
  inline const char* va_getFormat(const char* value){
    return value;
  }
  struct va_getFormat_eval{
    typedef const char* return_type;

    template<typename T>
    static const char* eval(T const& value){
      return va_getFormat(value);
    }

    static const char* out_of_range(){
      return 0;
    }
  };
  template<typename TT>
  const char* va_getFormat(int index,TT const& tp){
    return mwg::vararg::evaluate_element(va_getFormat_eval(),tp,index);
  }

}
}

//-----------------------------------------------------------------------------
// converters

namespace mwg{
namespace xprintf_detail{

  template<typename Buff>
  int xputs(Buff const& buff,const char* str){
    const char* p=str;
    while(*p)buff.put(*p++);
    return p-str;
  }

  enum xprintf_convert_returns{
    xprint_convert_argument_out_of_range=-1,
    xprint_convert_unknown_conv=-2,
  };

  template<typename Buff>
  int xprintf_convert(Buff const& buff,fmtspec const& spec,...){
    return xputs(buff,"(xprintf: not supported argument type)");
  }
  template<typename Buff>
  struct xprintf_convert_eval{
    typedef int return_type;
    Buff const& buff;
    fmtspec const& spec;
  public:
    xprintf_convert_eval(Buff const& buff,fmtspec const& spec):buff(buff),spec(spec){}

    template<typename T>
    int eval(T mwg_forward_rvalue value) const{
      return xprintf_convert(this->buff,this->spec,mwg::stdm::forward<T>(value));
    }

    static int out_of_range(){
      return xprint_convert_argument_out_of_range;
    }
  };

  // helper function

  template<typename T,typename Buff,typename Converter>
  int convert_aligned(Buff const& buff,fmtspec const& spec,T const& value,Converter& conv){
    int pw=conv.count_prefix(value);
    int bw=conv.count_body(value);

    int pad=spec.width-pw-bw;
    int rpad=0,zero=0,lpad=0;
    if(pad>=1){
      if(spec.flags&flag_left){
        // POSIX に従うと - の方が優先
        rpad=pad;
      }else if(spec.flags&flag_zero&&conv.has_leading_zeroes(value)){
        zero=pad;
      }else
        lpad=pad;
    }

    int const ret=lpad+pw+zero+bw+rpad;

    if(!(spec.flags&flag_suppress)){
      // write
      while(lpad--)buff.put(' ');
      conv.output_prefix(buff,value);
      while(zero--)buff.put('0');
      conv.output_body(buff,value);
      while(rpad--)buff.put(' ');
    }

    return ret;
  }

  // default converters

  template<typename Buff>
  int xprintf_convert(Buff const& buff,fmtspec const& spec,mwg::u8t value,bool isSigned,int size);

  template<typename Buff,typename T>
  typename stdm::enable_if<stdm::is_signed<T>::value&&!stdm::is_same<T,char>::value,int>::type
  xprintf_convert(Buff const& buff,fmtspec const& spec,T const& value){
    return xprintf_convert(buff,spec,(mwg::i8t)value,true,sizeof(T));
  }

  template<typename Buff,typename T>
  typename stdm::enable_if<stdm::is_unsigned<T>::value||stdm::is_same<T,char>::value,int>::type
  xprintf_convert(Buff const& buff,fmtspec const& spec,T const& value){
    return xprintf_convert(buff,spec,(mwg::i8t)value,true,sizeof(T));
  }

  template<typename Buff>
  int xprintf_convert(Buff const& buff,fmtspec const& spec,double const& value);

  // string
  template<typename Buff>
  int xprintf_convert(Buff const& buff,fmtspec const& spec,const char* str,std::size_t len);

  template<typename Buff>
  int xprintf_convert(Buff const& buff,fmtspec const& spec,const char* str){
    return xprintf_convert(buff,spec,str,std::strlen(str));
  }

  // template<typename Buff,std::size_t N>
  // int xprintf_convert(Buff const& buff,fmtspec const& spec,const char (&str)[N]){
  //   std::size_t s;
  //   for(s=0;s<N;s++)if(!str[s])break;
  //   return xprintf_convert(buff,spec,str,s);
  // }

  template<typename Buff>
  int xprintf_convert(Buff const& buff,fmtspec const& spec,std::string const& str){
    return xprintf_convert(buff,spec,str.data(),str.size());
  }


}
}

//-----------------------------------------------------------------------------
// create_xprintf_writer

#include <cstdio>
#include <ostream>
#include <string>
#include <iostream>

namespace mwg{
namespace xprintf_detail{
  struct xprintf_writer{
    virtual void put(std::wint_t ch) const=0;
  };

  struct cfile_writer{
    std::FILE* file;
  public:
    cfile_writer(std::FILE* file):file(file){}
    void put(std::wint_t ch) const{
      std::putc(ch,this->file);
    }
  };

  struct ostream_writer{
    std::ostream& ostr;
  public:
    ostream_writer(std::ostream& ostr):ostr(ostr){}
    void put(std::wint_t ch) const{
      this->ostr.put(ch);
    }
  };

  struct string_writer{
    std::string& str;
  public:
    string_writer(std::string& str):str(str){}
    void put(std::wint_t ch) const{
      this->str+=ch;
    }
  };

  inline xprintf_detail::cfile_writer create_xprintf_writer(std::FILE* file,adl_helper*){
    return xprintf_detail::cfile_writer(file);
  }
  inline xprintf_detail::ostream_writer create_xprintf_writer(std::ostream& ostr,adl_helper*){
    return xprintf_detail::ostream_writer(ostr);
  }
  inline xprintf_detail::string_writer create_xprintf_writer(std::string& str,adl_helper*){
    return xprintf_detail::string_writer(str);
  }

  template<typename T>
  void _instantiate_xprintf_convert(T const& value){
    fmtspec spec={0};
    xprintf_convert(
      mwg::declval<xprintf_writer const&>(),
      spec,value,(adl_helper*)0);
    xprintf_convert(
      cfile_writer(stdout),
      spec,value,(adl_helper*)0);
    xprintf_convert(
      ostream_writer(std::cout),
      spec,value,(adl_helper*)0);
    std::string dummy;
    xprintf_convert(
      string_writer(dummy),
      spec,value,(adl_helper*)0);
  }
}
}

//-----------------------------------------------------------------------------
// printf functions

namespace mwg{
namespace xprintf_detail{

  template<typename Buff,typename... Args>
  int vxprintf_impl(Buff const& buff,const char* fmt,mwg::stdm::tuple<Args...> const& args){
    using namespace mwg::xprintf_detail;
    namespace detail=mwg::xprintf_detail;
    int nchar=0;
    int iarg=0;
    int iarg_base=-1;

    for(;*fmt;){
      if(*fmt=='%'){
        fmtspec spec;
        detail::read_fmtspec(spec,fmt);
        if(spec.conv=='%'){
          // 色々な引数を指定していた場合どうするか?
          buff.put('%');
          nchar++;
          continue;
        }

        // instanciate spec.width
        if(spec.width_pos==0)
          spec.width=detail::va_getIndex(iarg++,args);
        else if(spec.width_pos>0)
          spec.width=detail::va_getIndex(iarg_base+spec.width_pos,args);
        else if(spec.width<0)
          spec.width=0; // default

        // instanciate spec.precision
        if(spec.precision_pos==0)
          spec.precision=detail::va_getIndex(iarg++,args);
        else if(spec.precision_pos>0)
          spec.precision=detail::va_getIndex(iarg_base+spec.precision_pos,args);
        // a default value is determined by the corresponding converter.

        // instanciate spec.pos
        int jarg=spec.pos>0?iarg_base+spec.pos:iarg++;
        if(spec.conv=='\0'){
          if(!(fmt=detail::va_getFormat(jarg,args))){
            xputs(buff,"(xprintf: argument index out of range)");
            break;
          }
          iarg_base=jarg;
          continue;
        }

        xprintf_convert_eval<Buff> ev(buff,spec);
        int w=mwg::vararg::evaluate_element(ev,args,jarg);
        if(w>=0){
          nchar+=w;
        }else{
          if(w==xprint_convert_argument_out_of_range){
            // out of range
            xputs(buff,"(xprintf: argument index out of range)");
          }else if(w==xprint_convert_unknown_conv){
            // unknown conversion char
            xputs(buff,"(xprintf: unknown conversion '");
            buff.put(spec.conv);
            xputs(buff,"')");
          }else{
            // unknown error
            xputs(buff,"(xprintf: unknown error)");
          }
        }

      }else{
        buff.put(*fmt++);
        nchar++;
      }
    }

    return nchar;
  }

//---------------------------------------------------------------------------

  template<typename Buff,typename... Args>
  int vxprintf(Buff& buff,const char* fmt,mwg::stdm::tuple<Args...> const& args){
    return vxprintf_impl(
      create_xprintf_writer(buff,(adl_helper*)0),
      fmt,args);
  }

  template<typename Buff,typename... Args>
  int xprintf(Buff& buff,const char* fmt,Args&&... args){
    return vxprintf_impl(
      create_xprintf_writer(buff,(adl_helper*)0),
      fmt,mwg::vararg::va_forward<Args...>(args...));
  }

  template<typename... Args>
  std::string vsprintf(const char* fmt,mwg::stdm::tuple<Args...> const& args){
    std::string buff;
    vxprintf(buff,fmt,args);
    return mwg::stdm::move(buff);
  }

  template<typename Buff,typename... Args>
  std::string sprintf(const char* fmt,Args&&... args){
    std::string buff;
    vxprintf(
      buff,fmt,
      fmt,mwg::vararg::va_forward<Args...>(args...));
    return mwg::stdm::move(buff);
  }

}
}
namespace mwg{
  using mwg::xprintf_detail::vxprintf;
  using mwg::xprintf_detail::xprintf;
  using mwg::xprintf_detail::vsprintf;
  using mwg::xprintf_detail::sprintf;
}

#endif
