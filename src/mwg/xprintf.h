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
#pragma%# // typename... args
#pragma%%m _ _.R|,[[:space:]]*typename[[:space:]]*\.\.\.[[:space:]]*([_[:alpha:]][_[:alnum:]]*)\y|$".for/%K/0/__arity__/,typename $1%K/"|
#pragma%%m _ _.R|[[:space:]]*\ytypename[[:space:]]*\.\.\.[[:space:]]*([_[:alpha:]][_[:alnum:]]*)\y|$".for/%K/0/__arity__/typename $1%K/,"|
#pragma%# // Args qualifiers... args
#pragma%%m _ _.R|,[[:space:]]*([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.[[:space:]]*([_[:alnum:]]+)\y|$".for/%K/0/__arity__/,$1%K$2 $3%K/"|
#pragma%%m _ _.R|[[:space:]]*\y([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.[[:space:]]*([_[:alnum:]]+)\y|$".for/%K/0/__arity__/$1%K$2 $3%K/,"|
#pragma%# // Args qualifiers...
#pragma%%m _ _.R|,[[:space:]]*([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.|$".for/%K/0/__arity__/,$1%K$2/"|
#pragma%%m _ _.R|[[:space:]]*\y([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.|$".for/%K/0/__arity__/$1%K$2/,"|
#pragma%# // mwg::stdm::forward<Args>(args)...
#pragma%%m _ _.R|,[[:space:]]*mwg::stdm::forward<([_[:alnum:]]+)>\(([_[:alnum:]]+)\)...|$".for/%K/0/__arity__/,mwg::stdm::forward<$1%K>($2%K)/"|
#pragma%%m _ _.R|[[:space:]]*\ymwg::stdm::forward<([_[:alnum:]]+)>\(([_[:alnum:]]+)\)...|$".for/%K/0/__arity__/mwg::stdm::forward<$1%K>($2%K)/,"|
#pragma%%x _.i.r/<##>|##//
#pragma%end

#pragma%m variadic_expand_0toArN
#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
#pragma%%x 1.r/##//
#else
#pragma%%m a
#pragma%%x variadic_expand::with_arity.f/__arity__/0/ArN+1/
#pragma%%end
#pragma%%m a a.R/\ytemplate<>([[:space:][:cntrl:]]*(struct|union|class))/template<--->$1/
#pragma%%m a a.r|\ytemplate<>([[:space:]]*typename\y)?|inline|
#pragma%%m a a.r|\ytemplate<--->|template<>|
#pragma%%x a
#endif
#pragma%end

#pragma%m variadic_expand_ArN
#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
#pragma%%x 1.r/##//
#else
#pragma%%x variadic_expand::with_arity.r/__arity__/ArN/
#endif
#pragma%end

#pragma%m variadic_expand_ArNm1
#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
#pragma%%x 1.r/##//
#else
#pragma%%x variadic_expand::with_arity.r/__arity__/ArN-1/
#endif
#pragma%end

//-----------------------------------------------------------------------------
#pragma%end

/*?lwiki
 * *関数一覧
 * <?cpp #include <mwg/xprintf.h>?>
 * :@fn int mwg::''xprintf''(buff,fmt,args...);
 * :@fn int mwg::''vxprintf''(buff,fmt,tuple);
 *  指定した出力先に書式指定文字列で整形された文字列を書き込みます。
 *  :@param[in,out] template<typename Buff> Buff& ''buff'';
 *   出力先を指定します。\
 *   現在 <?cpp std::ostream, std::string, std::FILE*?> に対応しています。
 *  :@param[in] const char* ''fmt'';
 *   書式指定文字列を指定します。
 *  :@param[in] template<typename... Args> Args... ''args'';
 *  :@param[in] template<typename... Args> std::tuple<Args...> ''tuple'';
 *   引数を指定します。
 * :@fn '''xputf-temp''' mwg::''xputf''(fmt,args...);
 * :@fn '''xputf-temp''' mwg::''vxputf''(fmt,tuple);
 *  :@param[in] template<typename... Args> Args... ''args'';
 *  :@param[in] template<typename... Args> std::tuple<Args...> ''tuple'';
 *  :@class '''xputf-temp'''
 *   :@fn std::size_t '''xputf-temp'''::''count''() const;
 *   :@fn std::string '''xputf-temp'''::''str''() const;
 *   :@fn explicit '''xputf-temp'''::''operator'' std::string() const;
 *  :@fn Buff& ''operator<<''(buff,'''xputf-temp''');
 *   :@param[in,out] template<typename Buff> Buff& ''buff'';
 *
 * *注意
 * **xputf の戻り値の参照は複製しない
 *
 * 例:
 * &pre(!cpp){
 * auto& f=xputf("%d",1);
 * std::cout << f; // undefined
 *
 * auto&& g(){return xputf("%d",1);}
 * std::cout << g(); // undefined
 * }
 *
 * 但し以下は OK:
 * &pre(!cpp){
 * h(auto const& formatter){std::cout << formatter;}
 * h(xputf("%d",1));
 * }
 *
 * ''説明''
 * xputf の結果は一時オブジェクトとして使う事を想定している。
 * 具体的には xputf の結果 (temp とする) は xputf の実引数に対する参照を保持する。
 * 従って、temp の有効寿命は temp を構築した完全式の寿命と同じになる。
 * 誤って temp を有効寿命を超えて持ち越さない様に、
 * temp のコピー演算子・コピー構築を削除している。
 * しかしこの対策は完全でない。
 * 参照の複製に関してまでは禁止することができないからである。
 * 寿命を超えて参照の複製が起こるのは 2 パターンある。
 *
 * +auto& a=xputf(...); で参照を捕獲する。
 *  C++ の例外的既定で捕獲されたオブジェクトの寿命 temp は参照変数 a の寿命にまで延長される。
 *  しかし、''temp の部分式評価で生成された実引数の寿命は延長されない'' ので、
 *  temp の有効寿命はこの完全式の評価が終わった時点で尽きる。
 * +auto&& f(){return xputf(...);}: 関数の戻り値として参照を返す。
 *  これも駄目。関数を抜けた時点で xputf は消滅してなくなる。
 *
 * *拡張: 型 Target を新しく出力先として登録する方法
 * &pre(!cpp,title=myheader.h){
 * namespace MyNamespace{
 *   class custom_writer:public mwg::xprintf_detail::xprintf_writer{
 *     virtual void put(std::wint_t ch) const{ ... }
 *   };
 * }
 * namespace mwg{
 * namespace xprintf_detail{
 *   MyNamespace::custom_writer create_xprintf_writer(Target& target,adl_helper);
 * }
 * }
 * }
 *
 * <?cpp namespace mwg::xprintf_detail?> の中に <?cpp create_xprintf_writer?> という関数を定義する。
 * この関数は <?cpp Target& target?> を受け取って、
 * インターフェイス <?cpp xprintf_writer?> を実装するクラスのインスタンスを生成する。
 *
 * インターフェイス xprintf_writer は void put(std::wint_t) const; という純粋仮想関数を持つ。
 * この put メンバ関数は文字を受け取って、その文字を出力する処理を行う。
 * 内部でバッファリングをする場合はデストラクタで flush するのを忘れない様に。
 *
 *
 * *拡張: 型 T の引数に対する書式出力を定義する方法
 * &pre(!cpp){
 * template<typename Buff>
 * int mwg::xprintf_detail::xprintf_convert(
 *   Buff const& buff,fmtspec const& spec,MyType const& value,
 *   mwg::xprintf_detail::adl_helper);
 * }
 * を実装すれば良い。
 *
 * &pre(!cpp,title=xxx.h){
 * namespace mwg{
 * namespace xprintf_detail{
 *   template<typename Writer>
 *   int xprintf_convert(Writer const& buff,fmtspec const& spec,MyType const& value,adl_helper){
 *     実装
 *   }
 * }
 * }
 * }
 *
 * 実装を隠蔽または分離したい場合は、
 * Writer = xprintf_writer, empty_writer, cfile_writer, ostream_writer, string_writer の5つについて、
 * 関数テンプレートのインスタンス化をする。例えば以下の様にする。
 *
 * &pre(!cpp,title=yyy.h){
 * namespace mwg{
 * namespace xprintf_detail{
 *   template<typename Writer>
 *   int xprintf_convert(Writer const& buff,fmtspec const& spec,MyType const& value,adl_helper);
 * }
 * }
 * }
 *
 * &pre(!cpp,title=yyy.cpp){
 * namespace mwg{
 * namespace xprintf_detail{
 *   template<typename Writer>
 *   int xprintf_convert(Writer const& buff,fmtspec const& spec,MyType const& value,adl_helper){
 *     実装
 *   }
 * 
 *   template int xprintf_convert<xprintf_writer>(xprintf_writer const& buff,fmtspec const& spec,MyType const& value,adl_helper);
 *   template int xprintf_convert<empty_writer  >(cfile_writer   const& buff,fmtspec const& spec,MyType const& value,adl_helper);
 *   template int xprintf_convert<cfile_writer  >(cfile_writer   const& buff,fmtspec const& spec,MyType const& value,adl_helper);
 *   template int xprintf_convert<ostream_writer>(ostream_writer const& buff,fmtspec const& spec,MyType const& value,adl_helper);
 *   template int xprintf_convert<string_writer >(string_writer  const& buff,fmtspec const& spec,MyType const& value,adl_helper);
 * }
 * }
 * }
 *
 */

//-----------------------------------------------------------------------------
// mwg::vararg::evaluate_element

namespace mwg{
namespace vararg{
  using namespace mwg;

  namespace detail{
    // 二分探索
    template<std::size_t L,std::size_t U,typename TT,bool=(L+1<U)>
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

    template<std::size_t L,std::size_t U,typename TT>
    struct tuple_element_selector:tuple_element_selector__impl<L,U,TT>{};
  }

  template<typename Eval>
  typename Eval::return_type
  evaluate_element(Eval const& evaluater,mwg::stdm::tuple<> const& tp,int index){
    mwg_unused(tp);
    mwg_unused(index);
    return evaluater.out_of_range();
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

#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
#pragma%m 1
  template<typename... Args>
  class packed_forward:public mwg::stdm::tuple<Args mwg_forward_rvalue...>{
  public:
    typedef mwg::stdm::tuple<Args mwg_forward_rvalue...> tuple_type;

    packed_forward(Args mwg_forward_rvalue... args):tuple_type(mwg::stdm::forward<Args>(args)...){}

    // copy constructor
    //   右辺値参照も左辺値参照も全部コピーする。
    //   基底が tuple<Args mwg_forward_rvalue...> なので要素は全て参照である。
    //   従って rhs を右辺値参照に強制しても問題は生じない。
    packed_forward(packed_forward const& rhs):tuple_type(mwg::stdm::move((tuple_type&)rhs)){}
  };
#pragma%end
#pragma%x 1
#else
#pragma%x
#pragma%x variadic_expand::with_arity.r/__arity__/ArN/
#pragma%end.r/typename Args[0-9]/&=void/
#pragma%m 1 1.r/class packed_forward/&<Args...>/
#pragma%x variadic_expand::with_arity.f/__arity__/0/ArN/
#endif

#pragma%m 1
  template<typename... Args>
  packed_forward<Args...> pack_forward(Args mwg_forward_lvalue... args){
    return packed_forward<Args...>(mwg::stdm::forward<Args>(args)...);
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
  template<typename T>
  inline typename stdm::enable_if<stdm::is_floating_point<T>::value,int>::type
  va_getIndex(T const&){
    throw mwg::except("mwg::xprintf(): a floating point number cannot used for an index.");
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
      return xprintf_convert(this->buff,this->spec,mwg::stdm::forward<T>(value),adl_helper());
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
  struct basic_convert_impl{
    // implemented in xprintf.cpp
    static int convert_integer(Buff const& buff,fmtspec const& spec,mwg::u8t value,bool isSigned,int size);
    static int convert_floating_point(Buff const& buff,fmtspec const& spec,double const& value);
    static int convert_string(Buff const& buff,fmtspec const& spec,const char* str,std::size_t len);
  };

  // integral conversion

  template<typename Buff,typename T>
  typename stdm::enable_if<stdm::is_signed<T>::value&&!stdm::is_same<T,char>::value,int>::type
  xprintf_convert(Buff const& buff,fmtspec const& spec,T const& value,adl_helper){
    return basic_convert_impl<Buff>::convert_integer(buff,spec,(mwg::i8t)value,true,sizeof(T));
  }

  template<typename Buff,typename T>
  typename stdm::enable_if<stdm::is_unsigned<T>::value||stdm::is_same<T,char>::value,int>::type
  xprintf_convert(Buff const& buff,fmtspec const& spec,T const& value,adl_helper){
    return basic_convert_impl<Buff>::convert_integer(buff,spec,(mwg::i8t)value,false,sizeof(T));
  }

  // floating point conversion

  template<typename Buff>
  int xprintf_convert(Buff const& buff,fmtspec const& spec,double const& value,adl_helper){
    return basic_convert_impl<Buff>::convert_floating_point(buff,spec,value);
  }

  // string conversion

  template<typename Buff>
  int xprintf_convert(Buff const& buff,fmtspec const& spec,const char* str,adl_helper){
    return basic_convert_impl<Buff>::convert_string(buff,spec,str,std::strlen(str));
  }

  // template<typename Buff,std::size_t N>
  // int xprintf_convert(Buff const& buff,fmtspec const& spec,const char (&str)[N],adl_helper){
  //   std::size_t s;
  //   for(s=0;s<N;s++)if(!str[s])break;
  //   return basic_convert_impl<Buff>::convert_string(buff,spec,str,s);
  // }

  template<typename Buff>
  int xprintf_convert(Buff const& buff,fmtspec const& spec,std::string const& str,adl_helper){
    return basic_convert_impl<Buff>::convert_string(buff,spec,str.data(),str.size());
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

  struct empty_writer{
    void put(std::wint_t) const{}
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

  inline xprintf_detail::cfile_writer create_xprintf_writer(std::FILE* file,bool flagClear,adl_helper){
    mwg_unused(flagClear);
    return xprintf_detail::cfile_writer(file);
  }
  inline xprintf_detail::ostream_writer create_xprintf_writer(std::ostream& ostr,bool flagClear,adl_helper){
    mwg_unused(flagClear);
    return xprintf_detail::ostream_writer(ostr);
  }
  inline xprintf_detail::string_writer create_xprintf_writer(std::string& str,bool flagClear,adl_helper){
    if(flagClear)str="";
    return xprintf_detail::string_writer(str);
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
    return vxprintf_impl(create_xprintf_writer(buff,true,adl_helper()),fmt,args);
  }

#pragma%m 1
  template<typename Buff,typename... Args>
  int xprintf(Buff& buff,const char* fmt,Args mwg_forward_rvalue... args){
    return vxprintf(buff,fmt,mwg::stdm::forward_as_tuple(mwg::stdm::forward<Args>(args)...));
  }
#pragma%end
#pragma%x variadic_expand_0toArN

//---------------------------------------------------------------------------

  template<typename Tuple>
  class _vxputf_temporary_object{
    const char* m_fmt;
    Tuple const& m_args;

    _vxputf_temporary_object(const char* fmt,Tuple const& args):m_fmt(fmt),m_args(args){}

#if defined(MWGCONF_STD_DEFAULTED_FUNCTIONS)&&defined(MWGCONF_STD_RVALUE_REFERENCES)
    _vxputf_temporary_object(_vxputf_temporary_object const& c) = default;
    _vxputf_temporary_object(_vxputf_temporary_object&& c) = default;
#endif

    template<typename Tuple2>
    friend _vxputf_temporary_object<typename mwg::stdm::enable_if<mwg::stdx::is_tuple<Tuple2>::value,Tuple2>::type> const
    vxputf(const char* fmt,Tuple2 const& args);

  public:
    template<typename Buff>
    int print(Buff& buff) const{
      return vxprintf_impl(create_xprintf_writer(buff,false,adl_helper()),m_fmt,m_args);
    }

    std::string str() const{
      std::string buff;
      vxprintf_impl(create_xprintf_writer(buff,false,adl_helper()),m_fmt,m_args);
      return mwg::stdm::move(buff);
    }

    mwg_explicit_operator std::string() const{
      return mwg::stdm::move(this->str());
    }

    std::size_t count() const{
      return vxprintf_impl(empty_writer(),m_fmt,m_args);
    }
  };

  template<typename Tuple>
  _vxputf_temporary_object<typename mwg::stdm::enable_if<mwg::stdx::is_tuple<Tuple>::value,Tuple>::type> const
  vxputf(const char* fmt,Tuple const& args){
    return _vxputf_temporary_object<Tuple>(fmt,args);
  }

  template<typename pack_type>
  class _xputf_temporary_object{
    const char* m_fmt;
    pack_type m_args;

    _xputf_temporary_object(const char* fmt,pack_type mwg_forward_rvalue args):m_fmt(fmt),m_args(mwg::stdm::move(args)){}

#if defined(MWGCONF_STD_DEFAULTED_FUNCTIONS)&&defined(MWGCONF_STD_RVALUE_REFERENCES)
    _xputf_temporary_object(_xputf_temporary_object const& c) = default;
    _xputf_temporary_object(_xputf_temporary_object&& c) = default;
#endif

#pragma%m 1
    template<typename... Args>
    friend _xputf_temporary_object<mwg::vararg::packed_forward<Args...> > const
    xputf(const char* fmt,Args mwg_forward_rvalue... args);
#pragma%end
#pragma%x variadic_expand_0toArN

    typename pack_type::tuple_type const& get_args() const{return this->m_args;}

  public:
    template<typename Buff>
    int print(Buff& buff) const{
      return vxprintf_impl(create_xprintf_writer(buff,false,adl_helper()),m_fmt,this->get_args());
    }

    std::string str() const{
      std::string buff;
      vxprintf_impl(create_xprintf_writer(buff,false,adl_helper()),m_fmt,this->get_args());
      return mwg::stdm::move(buff);
    }

    mwg_explicit_operator std::string() const{
      return mwg::stdm::move(this->str());
    }

    std::size_t count() const{
      return vxprintf_impl(empty_writer(),m_fmt,this->get_args());
    }
  };

#pragma%m 1
  template<typename... Args>
  _xputf_temporary_object<mwg::vararg::packed_forward<Args...> > const
  xputf(const char* fmt,Args mwg_forward_rvalue... args){
    typedef _xputf_temporary_object<mwg::vararg::packed_forward<Args...> > const return_type;
    return return_type(fmt,mwg::vararg::pack_forward<Args...##>(args...));
  }
#pragma%end
#pragma%x variadic_expand_0toArN

  template<typename Buff,typename Tuple>
  Buff& operator<<(Buff& buff,_vxputf_temporary_object<Tuple> const& _vxputf){
    _vxputf.print(buff);
    return buff;
  }
  template<typename Buff,typename Tuple>
  Buff& operator<<(Buff& buff,_xputf_temporary_object<Tuple> const& _vxputf){
    _vxputf.print(buff);
    return buff;
  }
}
}
namespace mwg{
  using mwg::xprintf_detail::vxprintf;
  using mwg::xprintf_detail::xprintf;
  using mwg::xprintf_detail::vxputf;
  using mwg::xprintf_detail::xputf;
}

#endif
#pragma%x begin_check
// mmake_check_flags: -L "$CFGDIR" -lmwg

#ifdef _MSC_VER
# pragma warning(disable:4413) // 参照を捕獲すると文句を言う
#endif

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
  mwg_check(ss.str()=="(000100,   144,0x64 ,(xprintf: unknown conversion 'd'))\n","ss=%s",ss.str().c_str());

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
    sizeof(long)==4?
    "hhd=46 hd=-722 ld=-1234567890 lld=-1234567890\n"
    "hhx=2e hx=fd2e lx=b669fd2e llx=ffffffffb669fd2e\n":
    "hhd=46 hd=-722 ld=-1234567890 lld=-1234567890\n"
    "hhx=2e hx=fd2e lx=ffffffffb669fd2e llx=ffffffffb669fd2e\n",
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

void check_xputf_interface(){
  std::string buff;
  mwg::xprintf(buff,"%#x",1234);
  mwg_check(buff=="0x4d2","result=%s",buff.c_str());

  // std::ostream
  {
    std::ostringstream line;
    line << mwg::xputf("hello %05d!\n",1234)
         << "simple" << std::endl
         << mwg::xputf("world %#x!\n",1234);
    mwg_check(line.str()=="hello 01234!\nsimple\nworld 0x4d2!\n","line=%s",line.str().c_str());
  }

  // std::string
  {
    std::string line;
    line << mwg::xputf("hello %05d!\n",1234)
         << mwg::xputf("simple\n")
         << mwg::xputf("world %#x!\n",1234);
    mwg_check(line=="hello 01234!\nsimple\nworld 0x4d2!\n","line=%s",line.c_str());
  }

#ifdef __linux__
  {
    // std::FILE*
    // Only compile&run tests. Results are not tested below.
    std::FILE* file=std::fopen("/dev/null","wb");
    file << mwg::xputf("hello %05d!\n");
    std::fclose(file);
  }
#endif

  // std::string(mwg::xputf())
  // mwg::xputf().str()
  // mwg::xputf().count()
  mwg_check(std::string(mwg::xputf("hello %05d!",1234))=="hello 01234!","result=%s",std::string(mwg::xputf("hello %05d!")).c_str());
  mwg_check(mwg::xputf("hello %05d!",1234).str()=="hello 01234!");
  mwg_check(mwg::xputf("hello %05d!",1234).count()==12);
  mwg_check(mwg::vxputf("hello %05d!",mwg::stdm::forward_as_tuple(1234)).count()==12);
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
#if defined(_MSC_VER)||(0<MWGCONF_GCC_VER&&MWGCONF_GCC_VER<40000)
  double const inf=double(float(1e100));
#else
  double const inf=1.0/0.0;
#endif
  check_printf("inf INF +inf -INF","%f %1$F %1$+f %F",inf,-inf);
  check_printf("inf INF +inf -INF","%e %1$E %1$+e %E",inf,-inf);
  check_printf("inf INF +inf -INF","%g %1$G %1$+g %G",inf,-inf);
  check_printf("inf INF +inf -INF","%a %1$A %1$+a %A",inf,-inf);

  double nan=std::sqrt(-1.0);
  check_printf("nan NAN +nan NAN","%f %1$F %1$+f %F",nan,-nan);
  check_printf("nan NAN +nan NAN","%e %1$E %1$+e %E",nan,-nan);
  check_printf("nan NAN +nan NAN","%g %1$G %1$+g %G",nan,-nan);
  check_printf("nan NAN +nan NAN","%a %1$A %1$+a %A",nan,-nan);
}

int main(){
  check_read_fmtspec();

  check_xputf_interface();

  test1();
  test2();
  return 0;
}

#pragma%x end_check
