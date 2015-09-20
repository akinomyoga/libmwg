// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef MWG_XPRINTF_H
#define MWG_XPRINTF_H
#include <cstdlib>
#include <cstring>
#include <mwg/std/tuple>
#include <mwg/std/utility>
#include <mwg/std/type_traits>
#pragma%begin
//-----------------------------------------------------------------------------

#pragma%[ArN=10]

#pragma%m variadic_expand::with_arity
#pragma%%m _ 1
#pragma%%m _ _.R|,[[:space:]]*typename[[:space:]]*\.\.\.[[:space:]]*([_[:alpha:]][_[:alnum:]]*)\y|$".for/%K/0/__arity__/,typename $1%K/"|
#pragma%%m _ _.R|[[:space:]]*\ytypename[[:space:]]*\.\.\.[[:space:]]*([_[:alpha:]][_[:alnum:]]*)\y|$".for/%K/0/__arity__/typename $1%K/,"|
#pragma%%m _ _.R|,[[:space:]]*([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.[[:space:]]*([_[:alnum:]]+)\y|$".for/%K/0/__arity__/,$1%K$2 $3%K/"|
#pragma%%m _ _.R|[[:space:]]*\y([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.[[:space:]]*([_[:alnum:]]+)\y|$".for/%K/0/__arity__/$1%K$2 $3%K/,"|
#pragma%%m _ _.R|,[[:space:]]*([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.|$".for/%K/0/__arity__/,$1%K$2/"|
#pragma%%m _ _.R|[[:space:]]*\y([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.|$".for/%K/0/__arity__/$1%K$2/,"|
#pragma%%m _ _.R|,[[:space:]]*mwg::stdm::forward<([_[:alnum:]]+)>\(([_[:alnum:]]+)\)...|$".for/%K/0/__arity__/,mwg::stdm::forward<$1%K>($2%K)/"|
#pragma%%m _ _.R|[[:space:]]*\ymwg::stdm::forward<([_[:alnum:]]+)>\(([_[:alnum:]]+)\)...|$".for/%K/0/__arity__/mwg::stdm::forward<$1%K>($2%K)/,"|
#pragma%%x _.i
#pragma%end

#pragma%m variadic_expand_0toArN
#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
#pragma%%x 1
#else
#pragma%%m a
#pragma%%x variadic_expand::with_arity.f/__arity__/0/ArN+1/
#pragma%%end
#pragma%%m a a.R/\ytemplate<>([[:space:][:cntrl:]]*(struct|union|class))/template<--->$1/
#pragma%%m a a.r|\ytemplate<>([[:space:]]*typename\y)?|inline|
#pragma%%m a a.r|\ytemplate\<---\>|template<>|
#pragma%%x a
#endif
#pragma%end

#pragma%m variadic_expand_ArN
#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
#pragma%%x 1
#else
#pragma%%x variadic_expand::with_arity.r/__arity__/ArN/
#endif
#pragma%end

#pragma%m variadic_expand_ArNm1
#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
#pragma%%x 1
#else
#pragma%%x variadic_expand::with_arity.r/__arity__/ArN-1/
#endif
#pragma%end

//-----------------------------------------------------------------------------
#pragma%end

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

  namespace detail{
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
        mwg_unused(index);
        return obj.eval(mwg::stdm::get<L>(tp));
      }
    };
    template<>
    struct tuple_element_selector__impl<0,0,stdm::tuple<>,false>{
      template<typename F>
      static typename F::return_type eval(F const& obj,int index,stdm::tuple<> const& tp){
        mwg_unused(obj);
        mwg_unused(index);
        mwg_unused(tp);
        return obj.out_of_range();
      }
    };

    template<std::size_t L,std::size_t U,typename TT>
    struct tuple_element_selector:tuple_element_selector__impl<L,U,TT>{};
  }

  template<typename TT,typename Eval>
  typename Eval::return_type
  evaluate_element(Eval const& evaluater,TT const& tp,int index){
    const int U=mwg::stdm::tuple_size<TT>::value;
    if(index<0||U<=index)
      return evaluater.out_of_range();
    else
      return detail::tuple_element_selector<0,U,TT>::eval(evaluater,index,tp);
  }

  template<typename XTuple,typename YTuple>
  struct pack_forward_enabler:mwg::stdm::false_type{};
  template<>
  struct pack_forward_enabler<mwg::stdm::tuple<>,mwg::stdm::tuple<> >:mwg::stdm::true_type{};

#pragma%m 1
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
#pragma%end
#pragma%x variadic_expand_ArNm1

#pragma%m 1
  template<typename... XArgs,typename... YArgs>
  typename mwg::stdm::enable_if<
    pack_forward_enabler<mwg::stdm::tuple<XArgs...>,mwg::stdm::tuple<YArgs...> >::value,
    mwg::stdm::tuple<XArgs mwg_forward_rvalue...>
    >::type
  va_forward(YArgs mwg_forward_rvalue... args){
    typedef mwg::stdm::tuple<XArgs mwg_forward_rvalue...> return_type;
    return return_type(mwg::stdm::forward<XArgs>(args)...);
  }
#pragma%end
#pragma%x variadic_expand_0toArN
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
    mwg_unused(spec);
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
    return xprintf_convert(buff,spec,(mwg::i8t)value,false,sizeof(T));
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
    str="";
    return xprintf_detail::string_writer(str);
  }

  template<typename T>
  void _instantiate_xprintf_convert(T const& value){
    fmtspec spec={};
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

  template<typename Buff,typename Tuple>
  int vxprintf_impl(Buff const& buff,const char* fmt,Tuple const& args){
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
            nchar+=xputs(buff,"(xprintf: argument index out of range)");
          }else if(w==xprint_convert_unknown_conv){
            // unknown conversion char
            nchar+=xputs(buff,"(xprintf: unknown conversion '");
            buff.put(spec.conv);
            nchar++;
            nchar+=xputs(buff,"')");
          }else{
            // unknown error
            nchar+=xputs(buff,"(xprintf: unknown error)");
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

  template<typename Buff,typename Tuple>
  typename mwg::stdm::enable_if<mwg::stdx::is_tuple<Tuple>::value,int>::type
  vxprintf(Buff& buff,const char* fmt,Tuple const& args){
    return vxprintf_impl(
      create_xprintf_writer(buff,(adl_helper*)0),
      fmt,args);
  }

#pragma%m 1
  template<typename Buff,typename... Args>
  int xprintf(Buff& buff,const char* fmt,Args mwg_forward_rvalue... args){
    return vxprintf_impl(
      create_xprintf_writer(buff,(adl_helper*)0),
      fmt,mwg::stdm::forward_as_tuple(mwg::stdm::forward<Args>(args)...));
  }
#pragma%end
#pragma%x variadic_expand_0toArN

  template<typename Tuple>
  typename mwg::stdm::enable_if<mwg::stdx::is_tuple<Tuple>::value,std::string>::type
  vsprintf(const char* fmt,Tuple const& args){
    std::string buff;
    vxprintf(buff,fmt,args);
    return mwg::stdm::move(buff);
  }

#pragma%m 1
  template<typename Buff,typename... Args>
  std::string sprintf(const char* fmt,Args mwg_forward_rvalue... args){
    std::string buff;
    vxprintf(buff,fmt,mwg::stdm::forward_as_tuple(mwg::stdm::forward<Args>(args)...));
    return mwg::stdm::move(buff);
  }
#pragma%end
#pragma%x variadic_expand_0toArN
  
}
}
namespace mwg{
  using mwg::xprintf_detail::vxprintf;
  using mwg::xprintf_detail::xprintf;
  using mwg::xprintf_detail::vsprintf;
  using mwg::xprintf_detail::sprintf;
}

#endif
#pragma%x begin_check
// mmake_check_flags: -L "$CFGDIR" -lmwg

#include <cstdio>
#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>
#include <mwg/except.h>
#include <mwg/std/cmath>
#include <mwg/xprintf.h>

template<typename Tuple>
void check_vprintf(const char* expectedResult,const char* fmt,Tuple const& args){
  std::string buff;
  int n=mwg::vxprintf(buff,fmt,args);
  mwg_check(
    buff==expectedResult,
    "fmt=%s: result[#%d]=(%s) expected[#%d]=(%s)",
    fmt,buff.size(),buff.c_str(),
    std::strlen(expectedResult),expectedResult);
  mwg_check((std::size_t)n==buff.size(),"fmt=%s: len: calculated=%d actual=%d (%s)",fmt,n,buff.size(),buff.c_str());
}

#pragma%m 1
template<typename... Args>
void check_printf(const char* expectedResult,const char* fmt,Args mwg_forward_rvalue... args){
  check_vprintf(expectedResult,fmt,mwg::stdm::forward_as_tuple(mwg::stdm::forward<Args>(args)...));
}
#pragma%end
#pragma%x variadic_expand_0toArN

#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wformat-security"
#endif
#pragma%m 1
template<typename... Args>
void check_printf_with_stdio(const char* fmt,Args mwg_forward_rvalue... args){
  std::string buff;
  mwg::vxprintf(buff,fmt,mwg::stdm::forward_as_tuple(mwg::stdm::forward<Args>(args)...));

  std::vector<char> buff2(buff.size()+100,'\n');
  char* ptr=&buff[0];
  std::sprintf(&buff2[0],fmt,args...);

  mwg_check(buff==ptr,"expected=(%s) result=(%s)",ptr,buff.c_str());
}
#pragma%end
#pragma%x variadic_expand_0toArN
#ifdef __clang__
# pragma clang diagnostic pop
#endif

void check_read_fmtspec(){
  const char* fmt="aiueo kakikukeko %s %10.12s %#010.123f\n";

  std::string buff;

  //"aiueo kakikukeko (flg=00 w=0 p=-1 conv=s) (flg=00 w=10 p=12 conv=s) (flg=22 w=10 p=123 conv=f)\n";
  int data[][4]={
    {000,0 ,-1 ,(int)'s'},
    {000,10,12 ,(int)'s'},
    {022,10,123,(int)'f'}
  };
  const int ndata=sizeof data/sizeof*data;

  int idata=0;

  for(;*fmt;){
    if(*fmt=='%'){
      mwg_check(idata<ndata);
      int const* ent=data[idata];

      mwg::xprintf_detail::fmtspec spec;
      mwg::xprintf_detail::read_fmtspec(spec,fmt);
      mwg_check(
        spec.flags==ent[0]&&
        spec.width==ent[1]&&
        spec.precision==ent[2]&&
        spec.conv==ent[3],
        "fmtspec[%d] = (flg=%03o w=%d p=%d conv=%c), expected: (flg=%03o w=%d p=%d conv=%c)",
        idata,
        spec.flags,spec.width,spec.precision,spec.conv?spec.conv:'$',
        ent[0],ent[1],ent[2],ent[3]?ent[3]:'$'
      );

      buff.append(1,'*');
      idata++;
    }else{
      buff.append(1,*fmt);
      fmt++;
    }
  }

  mwg_check(buff=="aiueo kakikukeko * * *\n");
}

void test1(){
  std::ostringstream ss;
  mwg::xprintf(ss,"(%.6d,%6o,%#-5x,%d)\n",100,100,100,"a");
  mwg_check(ss.str()=="(000100,   144,0x64 ,(xprintf: unknown conversion 'd'))\n");

  check_printf("(                   123,456,789)\n","(%'*d)\n",30,123456789);

  std::string line;
  mwg::xprintf(line,"(%'*d)\n",30,123456789);

  check_printf_with_stdio("%x %#x\n",123,123);
  check_printf_with_stdio("%o %#o %o %#o\n",0,0,0644,0644);

  // test of continue '%'
  check_printf(
    // expected result
    "# hello world\n"
    "1 2 3\n"
    "-4 -3 -2 -4\n",

    // format
    "# hello world\n%",
    "%d %d %d\n%",1,2,3,
    "%d %d %d %1$d\n",-4,-3,-2
  );

  // test of default size
  check_printf(
    // expected result
    "d=-1 u=255 x=ff X=FF o=377\n"
    "d=-1 u=65535 x=ffff X=FFFF o=177777\n"
    "d=-1 u=4294967295 x=ffffffff X=FFFFFFFF o=37777777777\n"
    "d=-1 u=18446744073709551615 x=ffffffffffffffff X=FFFFFFFFFFFFFFFF o=1777777777777777777777\n"
    "d=46 u=46 x=2e X=2E o=56\n"
    "d=-722 u=64814 x=fd2e X=FD2E o=176456\n"
    "d=-1234567890 u=3060399406 x=b669fd2e X=B669FD2E o=26632376456\n"
    "d=-1234567890 u=18446744072474983726 x=ffffffffb669fd2e X=FFFFFFFFB669FD2E o=1777777777766632376456\n",

    // format
    "d=%1$d u=%1$u x=%1$x X=%1$X o=%1$o\n"
    "d=%2$d u=%2$u x=%2$x X=%2$X o=%2$o\n"
    "d=%3$d u=%3$u x=%3$x X=%3$X o=%3$o\n"
    "d=%4$d u=%4$u x=%4$x X=%4$X o=%4$o\n%5$",
    mwg::i1t(-1),mwg::i2t(-1),mwg::i4t(-1),mwg::i8t(-1),
    "d=%1$d u=%1$u x=%1$x X=%1$X o=%1$o\n"
    "d=%2$d u=%2$u x=%2$x X=%2$X o=%2$o\n"
    "d=%3$d u=%3$u x=%3$x X=%3$X o=%3$o\n"
    "d=%4$d u=%4$u x=%4$x X=%4$X o=%4$o\n",
    mwg::i1t(-1234567890),mwg::i2t(-1234567890),mwg::i4t(-1234567890),mwg::i8t(-1234567890)
  );

  // test of size spec
  check_printf(
    "hhd=46 hd=-722 ld=-1234567890 lld=-1234567890\n"
    "hhx=2e hx=fd2e lx=b669fd2e llx=ffffffffb669fd2e\n",
    "hhd=%1$hhd hd=%1$hd ld=%1$ld lld=%1$lld\n"
    "hhx=%1$hhx hx=%1$hx lx=%1$lx llx=%1$llx\n",
    mwg::i8t(-1234567890)
  );

  check_printf("pi=3.141593 1=1.000000\n","pi=%f 1=%f\n",M_PI,1.0);

  mwg::xprintf(line,"pi=%.20f\n",M_PI);
  mwg_check(line.compare(0,15,"pi=3.1415926535")==0&&line.size()==26&&line[25]=='\n');

  mwg::xprintf(line,"0.1=%.100f\n",0.1);
  mwg_check(line.compare(0,10,"0.1=0.1000")==0&&line.size()==6+100+1&&line[106]=='\n');

  mwg::xprintf(line,"0.01=%.100f\n",0.01);
  mwg_check(line.compare(0,10,"0.01=0.010")==0&&line.size()==7+100+1&&line[107]=='\n',"result=%s",line.c_str());

  check_printf("bool: true FALSE\n","bool: %s %S\n",true,false);
}

void test2(){
  // %d
  check_printf("0 -1","%d %d",0,-1);
  check_printf("12345 -12345" ,"%d %d",12345,-12345);
  check_printf("+12345 -12345","%+d %+d",12345,-12345);
  check_printf(" 12345 -12345","% d % d",12345,-12345);
  check_printf("   12345","%8d",12345);
  check_printf("12345   ","%-8d",12345);
  check_printf("00012345","%08d",12345);
  check_printf("00012345","%.8d",12345);
  check_printf("  012345","%8.6d",12345);
  check_printf("012345  ","%-8.6d",12345);
  check_printf("12,345"  ,"%'d",12345);
  check_printf("  012345","%*.*d",8,6,12345);

  // %x %o
  check_printf("1e240 1E240" ,"%x %1$X",123456);
  check_printf("0x1e240 0X1E240" ,"%#x %1$#X",123456);
  check_printf("361100 0361100" ,"%o %1$#o",123456);
  check_printf("0 0" ,"%o %1$#o",0);
  check_printf(" 0  0" ,"%2o %1$#2o",0);
  check_printf("00 00" ,"%02o %1$02o",0);

  // %f %e
  check_printf("1.234568","%f",1.2345678);
  check_printf("12.345678","%f",12.345678);
  check_printf("123.456780","%f",123.45678);
  check_printf("1234567.800000","%f",1234567.8);
  check_printf("12345678.000000","%f",12345678);

  // -carry
  check_printf("0.999999 1.000000 9.999994e-001 9.999996e-001"  ,"%f %f %1$e %2$e",0.9999994,0.9999996);
  check_printf("0.991999 0.992000 9.919994e-001 9.919996e-001"  ,"%f %f %1$e %2$e",0.9919994,0.9919996);
  check_printf("0.000000 0.000001 4.000000e-007 5.000000e-007"  ,"%f %f %1$e %2$e",0.0000004,0.0000005);
  check_printf("1.000000 2.000000 1.000000e+000 2.000000e+000"  ,"%f %f %1$e %2$e",1.0000000,2.0000000);
  check_printf("0.100000 0.200000 1.000000e-001 2.000000e-001"  ,"%f %f %1$e %2$e",0.1000000,0.2000000);
  check_printf("10.000000 20.000000 1.000000e+001 2.000000e+001","%f %f %1$e %2$e",10.0,20.0);
  check_printf("1.000000","%f",0.9999999);
  check_printf("0.999999","%f",0.9999991);
  check_printf("0.000244","%f",0.0002436);
  check_printf("0.244","%.3f",0.2436912);

  // -grouping
  check_printf("1.234568","%'f",1.2345678);
  check_printf("12.345678","%'f",12.345678);
  check_printf("123.456780","%'f",123.45678);
  check_printf("1,234,567.800000","%'f",1234567.8);
  check_printf("12,345,678.000000","%'f",12345678);

  // %g
  check_printf("1.23456","%g",1.2345612345);
  check_printf("0.000243612","%g",0.000243612);
  check_printf("243612","%g",243612.0);
  check_printf("2.43612e+006","%g",2436120.0);
  check_printf("2.43612E-005","%G",0.0000243612);
  check_printf("1","%g",0.9999999);
  check_printf("0.999999","%g",0.9999991);
  check_printf("99.9999","%g",99.9999);
  check_printf("99.9999","%#g",99.9999);

  // %#g & 四捨五入
  check_printf("1.00000e+006 1e+006","%#g %1$g",999999.9);
  check_printf("100000. 100000","%#g %1$g",99999.99);
  check_printf("10000.0 10000" ,"%#g %1$g",9999.999);
  check_printf("100.000 100"   ,"%#g %1$g",99.99999);
  check_printf("10.0000 10"    ,"%#g %1$g",9.999999);
  check_printf("1.00000 1"     ,"%#g %1$g",0.9999999);
  check_printf("0.100000 0.1"  ,"%#g %1$g",0.09999999);
  check_printf("0.0100000 0.01","%#g %1$g",0.009999999);
  check_printf("999999. 999999"       ,"%#g %1$g",999999.0);
  check_printf("99999.9 99999.9"      ,"%#g %1$g",99999.9);
  check_printf("9999.99 9999.99"      ,"%#g %1$g",9999.99);
  check_printf("99.9999 99.9999"      ,"%#g %1$g",99.9999);
  check_printf("9.99999 9.99999"      ,"%#g %1$g",9.99999);
  check_printf("0.999999 0.999999"    ,"%#g %1$g",0.999999);
  check_printf("0.0999999 0.0999999"  ,"%#g %1$g",0.0999999);
  check_printf("0.00999999 0.00999999","%#g %1$g",0.00999999);

  // zero
  check_printf("0.000000 0.000000e+000","%f %e",0.0,0.0);
  check_printf("0.00000 0","%#g %1$g",0.0);

  // nan/inf
  check_printf("inf INF +inf -INF","%f %1$F %1$+f %F",1.0/0.0,-1.0/0.0);
  check_printf("inf INF +inf -INF","%e %1$E %1$+e %E",1.0/0.0,-1.0/0.0);
  check_printf("inf INF +inf -INF","%g %1$G %1$+g %G",1.0/0.0,-1.0/0.0);
  check_printf("inf INF +inf -INF","%a %1$A %1$+a %A",1.0/0.0,-1.0/0.0);

  double nan=std::sqrt(-1.0);
  check_printf("nan NAN +nan NAN","%f %1$F %1$+f %F",nan,-nan);
  check_printf("nan NAN +nan NAN","%e %1$E %1$+e %E",nan,-nan);
  check_printf("nan NAN +nan NAN","%g %1$G %1$+g %G",nan,-nan);
  check_printf("nan NAN +nan NAN","%a %1$A %1$+a %A",nan,-nan);
}

int main(){
  check_read_fmtspec();

  test1();
  test2();
  return 0;
}

#pragma%x end_check
