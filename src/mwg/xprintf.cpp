#include <cctype>
#include <utility>
#include <algorithm>
#include <mwg/except.h>
#include <mwg/std/tuple>
#include <mwg/std/type_traits>
#include <mwg/std/cmath>

// 実装用のメモ
//
// 実装は複数の部分に分かれる
// 1 書式指定を読み取る部分
// 2 可変長引数を対象の型に変換する部分
// 3 可変長引数を文字列に変換する部分
//
// 2-3 は可変長引数の型毎に文字列に変換するルーチンを提供する事によって実現した方が良い。

// ChangeLog
//
// 2014-06-04
//   * i2.cpp: agh の sprintf.js を参考にして作り直す。
//

#include "xprintf.h"

// #define XPRINTF__NAN_HAS_NO_SIGN

/**
 * @section printf.functionaname Standard printf Variation
 *
 * @subsection printf.functionname.format 関数名
 *
 * 関数名は以下の形式をしている:
 *   <_> <va> <target> <wide> printf <suffix>
 *   - <_>      ~ /_?/
 *   - <va>     ~ /v?/
 *   - <target> ~ /([fsdc]|sn|as|sc)?/
 *   - <wide>   ~ /w?/
 *   - <suffix> ~ /(_s|_p)?(_l)?/
 *
 * <_>
 *   '_'  (MSC)        MSC では非標準の関数の先頭に '_' を付ける。
 *
 * <va>
 *   ''   (標準)       引数として可変長引数を取る事を表す。
 *   'v'  (標準)       引数として va_list を取る事を表す。
 *                     この関数を使う為には <stdarg.h> が必要。
 *
 * <target>
 *   ''   (標準)       標準出力を書き込み先とする。
 *   'f'  (標準)       第一引数に指定した FILE* を書き込み先とする。
 *   's'  (標準)       第一引数に指定した char* を書き込み先とする。
 *   'sn' (C99)        第一引数に書き込み先の char* buff を受け取り、
 *                     第二引数に buff のバッファサイズを指定する。
 *   'd'  (POSIX:2008) 第一引数に指定した int fd を書き込み先とする。
 *   'as' (GNU)        必要な文字列バッファを malloc で確保しそこに書き込む。
 *                     第一引数に char** を受け取り、確保したバッファを其処に設定する。
 *   'c'  (MSC)        コンソールを書き込み先とする。(標準出力のリダイレクトに関係なく)
 *   'sc' (MSC)        出力しない。文字数だけをカウントする。
 *
 * <wide> は 書き込み先、書式指定文字列、引数の文字列 の形式を指定する。
 *   ''   (標準)       文字の型は char   , 文字列リテラルは ""     である。
 *   'w'  (C99)        文字の型は wchar_t, 文字列リテラルは L""    である。
 *   't'  (MSC)        文字の型は TCHAR  , 文字列リテラルは _T("") である。
 *                     <target> = 'c' の場合は 'c' より先に 't' を指定する。
 *
 * <suffix> (MSC)
 *   '_s' (MSC)        "secure" セキュリティの向上された版の関数である事を表す。
 *   '_p' (MSC)        "positional" 引数の位置指定を使える版の関数である事を表す。
 *                     (secure を包含しているので _s と _p が両方指定される事はない)
 *   '_l' (MSC)        引数に指定された locale_t を使用する。
 *                     locale_t は書式指定文字列の次の引数として受け取る。
 *
 * @subsection printf.functionname.list 関数一覧
 *
 * 標準出力
 *   (標準)       <stdio.h>  printf
 *   (標準)       <stdio.h>  vprintf
 *   (C99)        <wchar.h>  wprintf
 *   (C99)        <wchar.h>  vwprintf
 *
 * ファイル
 *   (標準)       <stdio.h>  fprintf
 *   (標準)       <stdio.h>  vfprintf
 *   (C99)        <wchar.h>  fwprintf
 *   (C99)        <wchar.h>  vfwprintf
 *
 * 文字列バッファ
 *   (標準)       <stdio.h>  sprintf
 *   (標準)       <stdio.h>  vsprintf
 *   (C99)        <wchar.h>  swprintf
 *   (C99)        <wchar.h>  vswprintf
 *
 * 文字列バッファ+サイズ
 *   (C99)        <stdio.h>  snprintf
 *   (C99)        <stdio.h>  vsnprintf
 *
 * ファイルディスクリプタ
 *   (POSIX:2008) <stdio.h>  dprintf
 *   (POSIX:2008) <stdio.h>  vdprintf
 *
 * サイズ
 *   (GNU)        <stdio.h>  asprintf
 *   (GNU)        <stdio.h>  vasprintf
 *
 * コンソール
 *   (MSC)        <conio.h>  _cprintf   _cprintf_l   _cprintf_s   _cprintf_s_l   _cprintf_p   _cprintf_p_l
 *   (MSC)        <conio.h>  _vcprintf  _vcprintf_l  _vcprintf_s  _vcprintf_s_l  _vcprintf_p  _vcprintf_p_l
 *   (MSC)        <conio.h>  _cwprintf  _cwprintf_l  _cwprintf_s  _cwprintf_s_l  _cwprintf_p  _cwprintf_p_l
 *   (MSC)        <conio.h>  _vcwprintf _vcwprintf_l _vcwprintf_s _vcwprintf_s_l _vcwprintf_p _vcwprintf_p_l
 *   (MSC)        <tchar.h>  _tcprintf  _tcprintf_l  _tcprintf_s  _tcprintf_s_l  _tcprintf_p  _tcprintf_p_l
 *   (MSC)        <tchar.h>  _vtcprintf _vtcprintf_l _vtcprintf_s _vtcprintf_s_l _vtcprintf_p _vtcprintf_p_l
 *
 */

//-----------------------------------------------------------------------------
// mwg::xprintf_detail::read_fmtspec

namespace mwg{
namespace xprintf_detail{
namespace{

  int read_posspec(const char*& _p){
    int digits=0;
    int pos=0;
    const char* p=_p;
    while(std::isdigit(*p)){
      digits++;
      pos=pos*10+(*p++-'0');
    }
    if(digits&&*p=='$'){
      _p=p+1;
      return pos;
    }else{
      return 0;
    }
  }

  // <width>? = /(\d+|\*(?:\d+\$)?)?/
  void read_widthspec(int& width,int& width_pos,const char*& _p){
    const char* p=_p;
    width=0;
    width_pos=-1;
    if(std::isdigit(*p)){
      int w=0;
      while(std::isdigit(*p))
        w=w*10+(*p++-'0');
      width=w;
    }else if(*p=='*'){
      p++;
      width_pos=read_posspec(p);
    }
    _p=p;
  }

  fmtspec_type read_typespec(const char*& p){
    switch(*p){
    case 'h':
      if(p[1]=='h'){
        p+=2;
        return type_hh;
      }else{
        p++;
        return type_h;
      }
    case 'l':
      if(p[1]=='l'){
        p+=2;
        return type_ll;
      }else{
        p++;
        return type_l;
      }
    case 'I':
      if(p[1]=='3'){
        if(p[2]=='2'){
          p+=3;
          return type_I32;
        }
      }else if(p[1]=='6'){
        if(p[2]=='4'){
          p+=3;
          return type_I64;
        }
      }else if(p[1]=='1'){
        if(p[2]=='6'){
          p+=3;
          return type_I16;
        }
      }else if(p[1]=='8'){
        p+=2;
        return type_I8;
      }
      p++;
      return type_I;
    case 'L':p++;return type_L;
    case 'j':p++;return type_j;
    case 'z':p++;return type_z;
    case 't':p++;return type_t;
    case 'q':p++;return type_q;
    case 'w':p++;return type_w;
    }

    return type_default;
  }
}

  void read_fmtspec(fmtspec& spec,const char*& _p){
    const char* p=_p;

    // /%{pos}?{flag}{width}?(\.{prec}?)?{type}(.|$)/g
    mwg_assert(*p=='%');
    p++;

    // <pos>? = /(?:(\d+)\$)?/
    spec.pos=read_posspec(p);

    // <flag> = /([-+ 0#']*)/
    spec.flags=0;
    for(;*p;p++){
      switch(*p){
      case '-':spec.flags|=flag_left;continue;
      case '0':spec.flags|=flag_zero;continue;
      case '+':spec.flags|=flag_plus;continue;
      case ' ':spec.flags|=flag_space;continue;
      case '#':spec.flags|=flag_hash;continue;
      case '\'':spec.flags|=flag_quote;continue;
      }
      break;
    }

    // <width>? = /(\d+|\*(?:\d+\$)?)?/
    read_widthspec(spec.width,spec.width_pos,p);

    // (.<prec>?)?
    if(*p=='.'){
      p++;
      // <prec>? = /(?:\d+|\*(?:\d+\$)?)?/
      read_widthspec(spec.precision,spec.precision_pos,p);
    }else{
      spec.precision=-1;
      spec.precision_pos=-1;
    }

    // <type>? = /([hlLjztqw]|hh|ll|I(?:32|64)?)?/
    spec.type=read_typespec(p);

    // <conv> = /(.|$)/
    spec.conv=*p;
    if(*p)p++;

    _p=p;
  }

}
}

//-----------------------------------------------------------------------------
// converters

namespace mwg{
namespace xprintf_detail{
  static const int  GROUPING_DIGITS=3;
  static const char GROUPING_CHAR=',';
  static const char* const digitsLower="0123456789abcdefxnan\0inf\0";
  static const char* const digitsUpper="0123456789ABCDEFXNAN\0INF\0";
  static const int IDIGITS_E  =14;
  static const int IDIGITS_X  =16;
  static const int IDIGITS_NAN=17;
  static const int IDIGITS_INF=21;

  enum integral_conversion_flags{
    iflag_signed    =0x01,
    iflag_group     =0x02,

    iflag_base_hex  =0x10,
    iflag_base_octal=0x20,
  };

  class integer_converter{
    const char* digits;
    int base;
    fmtspec const& spec;
    integral_conversion_flags iflags;
    bool isGrouped;
  public:
    integer_converter(const char* digits,int base,fmtspec const& spec,int iflags)
      :digits(digits),base(base),spec(spec),iflags(integral_conversion_flags(iflags))
    {
      this->isGrouped=spec.flags&flag_quote&&iflags&iflag_group;
    }

  private:
    int nzero;
  public:
    int count_body(mwg::u8t value){
      bool const valueIsZero=value==0;

      if(iflags&iflag_signed&&reinterpret_cast<mwg::i8t const&>(value)<0)
        value=1+~value; // -INT8_MIN は ? なので mwg::u8t のまま変換

      int len=0;
      do{
        value/=base;
        len++;
      }while(value>0);

      if(isGrouped)
        len+=(len-1)/GROUPING_DIGITS;

      nzero=0;
      if(spec.precision>=0&&len<spec.precision)
        nzero=spec.precision-len;
      if((iflags&iflag_base_octal)&&(spec.flags&flag_hash)&&!valueIsZero&&nzero==0)
        nzero=1;

      return len+nzero;
    }

  private:
    template<typename Buff>
    void put_digits(Buff const& buff,mwg::u8t value,int idigit) const{
      char c=digits[value%base];
      value/=base;
      if(value>0){
        this->put_digits(buff,value,idigit+1);
        if(isGrouped&&(idigit+1)%GROUPING_DIGITS==0)
          buff.put(GROUPING_CHAR);
      }
      buff.put(c);
    }

  public:
    template<typename Buff>
    void output_body(Buff const& buff,mwg::u8t value){
      if(iflags&iflag_signed&&reinterpret_cast<mwg::i8t const&>(value)<0)
        value=1+~value; // -INT8_MIN は ? なので mwg::u8t のまま変換

      // nzero padding
      int nzero=this->nzero;
      while(nzero--)buff.put(digits[0]);
      this->put_digits(buff,value,0);
    }

    int count_prefix(mwg::u8t const& value){
      int ret=0;
      if(iflags&iflag_signed){
        if(reinterpret_cast<mwg::i8t const&>(value)<0)
          ret+=1;
        else if(spec.flags&(flag_plus|flag_space))
          ret+=1;
      }

      if(iflags&iflag_base_hex&&spec.flags&flag_hash)
        ret+=2;

      return ret;
    }
    template<typename Buff>
    void output_prefix(Buff const& buff,mwg::u8t const& value){
      if(iflags&iflag_signed){
        if(reinterpret_cast<mwg::i8t const&>(value)<0){
          buff.put('-');
        }else if(spec.flags&flag_plus){
          buff.put('+');
        }else if(spec.flags&flag_space){
          buff.put(' ');
        }
      }

      if(iflags&iflag_base_hex&&spec.flags&flag_hash){
        buff.put('0');
        buff.put(digits[IDIGITS_X]);
      }
    }
    bool has_leading_zeroes(mwg::u8t const&) const{
      // precision を指定している場合は、自分で 0 を出力する
      if(spec.precision>=0)return false;
      return true;
    }
  };

  class character_converter{
  public:
    int count_body(std::wint_t ch) const{mwg_unused(ch);return 1;}
    template<typename Buff>
    void output_body(Buff& buff,std::wint_t ch) const{buff.put(ch);}
    int count_prefix(std::wint_t ch) const{mwg_unused(ch);return 0;}
    template<typename Buff>
    void output_prefix(Buff& buff,std::wint_t ch) const{mwg_unused(buff);mwg_unused(ch);}
    static bool has_leading_zeroes(std::wint_t const&){
      return false;
    }
  };

  class bool_alpha_converter{
    bool upperCase;
  public:
    bool_alpha_converter(bool upperCase):upperCase(upperCase){}

    int count_body(bool value) const{
      return value?4:5;
    }
    template<typename Buff>
    void output_body(Buff& buff,bool value) const{
      if(upperCase)
        xputs(buff,value?"TRUE":"FALSE");
      else
        xputs(buff,value?"true":"false");
    }
    int count_prefix(bool value) const{mwg_unused(value);return 0;}
    template<typename Buff>
    void output_prefix(Buff& buff,bool value) const{mwg_unused(buff);mwg_unused(value);}
    static bool has_leading_zeroes(bool const&){return false;}
  };

  template<typename Buff>
  int basic_convert_impl<Buff>::convert_integer(Buff const& buff,fmtspec const& spec,mwg::u8t value,bool isSigned,int size){
    if(spec.type!=type_default){
      int ssize=0;

      switch(spec.type){
      case type_hh:
        ssize=sizeof(char);
        break;
      case type_h:
        ssize=sizeof(short);
        break;
      case type_l:
        ssize=sizeof(long);
        break;
      case type_ll: // C99
#ifdef MWGCONF_HAS_LONGLONG
        ssize=sizeof(long long);
#else
        ssize=sizeof(long);
#endif
        break;
      case type_t: // C99
        ssize=sizeof(std::ptrdiff_t);
        break;
      case type_z: // C99
        ssize=sizeof(std::size_t);
        break;
      case type_I: // MSC
        ssize=isSigned?sizeof(std::ptrdiff_t):sizeof(std::size_t);
        break;
      case type_j: // C99
        ssize=sizeof(mwg::stdm::intmax_t);
        break;
      case type_I32: // MSC
        ssize=4;
        break;
      case type_I64: // MSC
      case type_q: // BSD
        ssize=8;
        break;
      case type_I8: // 独自
        ssize=1;
        break;
      case type_I16: // 独自
        ssize=2;
        break;
      default:
        ssize=0;
        break;
      }

      if(ssize&&size>ssize)size=ssize;
    }

    mwg::u8t const mask=((std::size_t)size<sizeof(mwg::u8t)?mwg::u8t(1)<<(size<<3):0)-1;
    value&=mask;

    // preferences
    const char* digitCharacters=digitsLower;
    int radix=10;
    int iflags=0;
    switch(spec.conv){
    case 'd':
    case 'i':
      if(isSigned){
        if(value<<1&~mask)value|=~mask; // sign extension
        iflags=iflag_signed|iflag_group;
      }else{
        iflags=iflag_group;
      }
      goto integer;
    case 'X':
      digitCharacters=digitsUpper;
      /*FALLTHROUGH*/
    case 'x':
      radix=16;
      iflags=iflag_base_hex;
      goto integer;
    case 'o':
      radix=8;
      iflags=iflag_base_octal;
      goto integer;
    case 'u':
      iflags=iflag_group;
      goto integer;
    integer:
      {
        integer_converter conv(digitCharacters,radix,spec,iflags);
        return convert_aligned(buff,spec,value,conv);
      }
    case 'c':
      {
        character_converter conv;
        return convert_aligned(buff,spec,value,conv);
      }
    case 's':case 'S':
      {
        bool_alpha_converter conv(spec.conv=='S');
        return convert_aligned(buff,spec,value,conv);
      }
    case 'e':case 'E':
    case 'f':case 'F':
    case 'g':case 'G':
    case 'a':case 'A':
      if(isSigned)
        return basic_convert_impl<Buff>::convert_floating_point(buff,spec,(double)(mwg::i8t)value);
      else
        return basic_convert_impl<Buff>::convert_floating_point(buff,spec,(double)(mwg::u8t)value);
    default:
      return xprint_convert_unknown_conv;
    }
  }
}
}

namespace mwg{
namespace xprintf_detail{

  enum fptype{
    fptype_fixed,       // %f
    fptype_exponent,    // %e
    fptype_significant, // %g
  };

  class floating_point_converter{
  protected:
    fmtspec const& spec;
    int type;
    char echar;
    int radix;
    double _log2;
    const char* digits;
  public:
    floating_point_converter(fmtspec const& spec)
      :spec(spec),type(fptype_fixed),echar('?'),radix(10),_log2(M_LN2/M_LN10),digits(digitsLower){}

    void set_type(char ch,char exponentChar){
      this->type=ch;
      this->echar=exponentChar;
    }
    void set_radix(int radix,double _log2){
      this->radix=radix;
      this->_log2=_log2;
    }
    void set_digits(const char* digits){
      this->digits=digits;
    }

  protected:
    bool   isGrouped;
    bool   hasPoint;
    bool   hasExponent;
    double frac;
    int    exp;
    int    fprec;
    int    iprec;


    // 繰り上がりで桁が増えるかどうかを判定
    // (0.9999999... の場合に起こる)
    bool checkFullCarry(int ndigit,int& nzero) const{
      double value=frac;
      nzero=0; // 末尾の零の数

      int ncarry=0;
      bool carryStop=false;
      while(--ndigit>0){
        int nextDigit=int(value*=radix);
        value-=nextDigit;
        if(nextDigit==radix-1)
          ncarry++;
        else{
          ncarry=0;
          carryStop=true;
        }

        if(nextDigit==0)
          nzero++;
        else
          nzero=0;
      }

      int last=(int)stdm::round(value*radix);
      if(last==0){
        nzero++;
      }else if(last==radix){
        nzero=ncarry+1;
        if(!carryStop)return true;
      }else{
        nzero=0;
      }

      return false;
    }

    void _frexp(double const& value,double& frac,int& exp) const{
      if(value==0.0){
        exp=1; // 0.0 の時は一の位の 0 を most-significant digit とする
        frac=0.0;
        return;
      }

      int exp2;
      double const frac2=std::frexp(value,&exp2);
      double const expf=exp2*_log2;
      exp=(int)std::ceil(expf);
      frac=frac2*std::pow((double)radix,expf-exp);
      if(frac*radix<1.0){
        frac*=radix;
        exp--;
      }
      mwg_assert(frac==0.0||(1.0/radix<=frac&&frac<1.0));
    }

  public:
    int count_body(double value){
      if(!stdm::isfinite(value))
        return 3;

      if(value<0.0)
        value=-value;

      int precision=spec.precision;
      if(precision<0)precision=6;

      this->isGrouped=(spec.flags&flag_quote)!=0;

      bool fOmitTrailingZero=false;
      if(type==fptype_significant){
        fOmitTrailingZero=(spec.flags&flag_hash)==0;
        if((value<std::pow((double)radix,(double)-4.0)&&value!=0.0)||std::pow((double)radix,(double)precision)<=value+0.5){
          type=fptype_exponent;
          precision--;
        }
      }

      int nzero;
      if(type==fptype_exponent){
        this->hasExponent=true;

        this->_frexp(value,this->frac,this->exp);
        if(checkFullCarry(1+precision,nzero)){
          frac/=radix;
          exp++;
          nzero--;
        }

        iprec=1;
        exp-=iprec;
        fprec=precision;

      }else{
        this->hasExponent=false;
        if(type==fptype_significant){
          // precision = 有効数字桁数
          this->_frexp(value,this->frac,this->exp);
          if(exp>0){
            if(checkFullCarry(precision,nzero))exp++;

            fprec=precision-exp;
            if(fprec<0)fprec=0;
          }else{
            if(checkFullCarry(precision,nzero)){
              mwg_assert(nzero==precision);
              exp++;
              nzero--;
            }
            fprec=precision-exp;

            this->frac=value/radix;
            this->exp=1;
          }

          iprec=exp;
          exp=0;
          mwg_assert(fprec>=0);
        }else{
          // precision = 小数部桁数
          fprec=precision;
          if(value>=1.0){
            this->_frexp(value,this->frac,this->exp);
            if(exp==0){
              frac/=radix;
              exp++;
            }
          }else{
            this->frac=value/radix;
            this->exp=1;
          }

          mwg_assert(0.0<=frac&&frac<1.0&&exp>0,"frac=%g exp=%d",frac,exp);

          iprec=exp;
          exp=0;
          if(checkFullCarry(iprec+fprec,nzero))iprec++;
        }
      }

      if(fOmitTrailingZero){
        if(fprec<nzero)
          fprec=0;
        else
          fprec-=nzero;
      }
      this->hasPoint=fprec>0||spec.flags&flag_hash;

      int cseq=iprec+fprec;
      if(this->isGrouped)
        cseq+=(iprec-1)/GROUPING_DIGITS;
      if(this->hasPoint)
        cseq++;
      if(hasExponent){
        cseq+=5;
        if(exp>=1000)cseq++;
      }
      return cseq;
    }

  private:
    int pd_pos;
    int pd_nzero;
    template<typename Buff>
    void put_digit(Buff& buff,int digit){
      buff.put(digits[digit]);
      if(--pd_pos>=0){
        if(pd_pos==0){
          if(hasPoint)
            buff.put('.');
        }else if(isGrouped&&pd_pos%3==0)
          buff.put(GROUPING_CHAR);
      }
    }

  protected:
    template<typename Buff>
    void generateFloatingSequence(Buff& buff,int integralDigits,int fractionDigits){
      double value=frac;
      pd_pos=integralDigits;
      pd_nzero=0;

      int ndigit=pd_pos+fractionDigits;

      int prevDigit=-1;
      int nreach=0;
      while(--ndigit>0){
        int nextDigit=int(value*=radix);
        value-=nextDigit;
        if(nextDigit==radix-1){
          nreach++;
        }else{
          if(prevDigit>=0)
            put_digit(buff,prevDigit);
          for(;nreach>0;nreach--)
            put_digit(buff,radix-1);

          prevDigit=nextDigit;
        }
      }

      int last=(int)stdm::round(value*radix);
      if(last==radix){
        // 繰り上がり
        nreach++;
        if(prevDigit>=0)
          put_digit(buff,prevDigit+1);
        else{
          put_digit(buff,1); //★桁数が一つ多くなる
          nreach--;
        }

        for(;nreach>0;nreach--)
          put_digit(buff,0);
      }else{
        if(prevDigit>=0)
          put_digit(buff,prevDigit);
        for(;nreach>0;nreach--)
          put_digit(buff,radix-1);
        put_digit(buff,last);
      }
    }

  public:
    template<typename Buff>
    void output_body(Buff const& buff,double const& value){
      if(!stdm::isfinite(value)){
        if(stdm::isnan(value))
          xputs(buff,&digits[IDIGITS_NAN]);
        else
          xputs(buff,&digits[IDIGITS_INF]);
        return;
      }

      generateFloatingSequence(buff,iprec,fprec);

      if(hasExponent){
        buff.put(digits[IDIGITS_E]);
        if(exp<0){
          buff.put('-');
          exp=-exp;
        }else
          buff.put('+');
        if(exp>=1000) // 4倍精度では 4932 まで可能
          buff.put(digits[exp/1000%10]);
        buff.put(digits[exp/100%10]);
        buff.put(digits[exp/10%10]);
        buff.put(digits[exp%10]);
      }
    }

  public:
    int count_prefix(double const& value){
      int ret=0;
#ifdef XPRINTF__NAN_HAS_NO_SIGN
      if(stdm::isnan(value))return 0;
#endif
      if(value<0.0||spec.flags&(flag_plus|flag_space))
        ret++;
      return ret;
    }
    template<typename Buff>
    void output_prefix(Buff const& buff,double const& value){
#ifdef XPRINTF__NAN_HAS_NO_SIGN
      if(stdm::isnan(value))return;
#endif
      if(value<0.0)
        buff.put('-');
      else if(spec.flags&flag_plus)
        buff.put('+');
      else if(spec.flags&flag_space)
        buff.put(' ');
    }
    static bool has_leading_zeroes(double const& value){
      return stdm::isfinite(value);
    }
  };

  template<typename Buff>
  int basic_convert_impl<Buff>::convert_floating_point(Buff const& buff,fmtspec const& spec,double const& value){
    floating_point_converter conv(spec);

    switch(spec.conv){
    case 'f':break;
    case 'F':
      conv.set_digits(digitsUpper);
      break;
    case 'e':
      conv.set_type(fptype_exponent,'e');
      break;
    case 'E':
      conv.set_type(fptype_exponent,'E');
      conv.set_digits(digitsUpper);
      break;
    case 'g':
      conv.set_type(fptype_significant,'e');
      break;
    case 'G':
      conv.set_type(fptype_significant,'E');
      conv.set_digits(digitsUpper);
      break;
    case 'a':
      conv.set_type(fptype_exponent,'p');
      conv.set_radix(16,0.25);
      break;
    case 'A':
      conv.set_type(fptype_exponent,'P');
      conv.set_radix(16,0.25);
      conv.set_digits(digitsUpper);
      break;
    default:
      return xprint_convert_unknown_conv;
    }

    return convert_aligned(buff,spec,value,conv);
  }

  class string_converter{
    std::size_t s;
  public:
    string_converter(fmtspec const& spec,std::size_t len)
      :s(len)
    {
      if(spec.precision>=0&&(std::size_t)spec.precision<len)
        this->s=spec.precision;
    }
  public:
    int count_body(const char* str) const{mwg_unused(str);return s;}
    template<typename Buff>
    void output_body(Buff& buff,const char* str) const{
      for(std::size_t i=0;i<s;i++)buff.put(str[i]);
    }
    int count_prefix(const char* str) const{mwg_unused(str);return 0;}
    template<typename Buff>
    void output_prefix(Buff& buff,const char* str) const{mwg_unused(buff);mwg_unused(str);}
    static bool has_leading_zeroes(const char*){
      return false;
    }
  };

  template<typename Buff>
  int basic_convert_impl<Buff>::convert_string(Buff const& buff,fmtspec const& spec,const char* str,std::size_t len){
    string_converter conv(spec,len);
    switch(spec.conv){
    case 's':break;
    default:
      return xprint_convert_unknown_conv;
    }
    return convert_aligned(buff,spec,str,conv);
  }

}
}

namespace mwg{
namespace xprintf_detail{

  template class basic_convert_impl<xprintf_writer>;
  template class basic_convert_impl<empty_writer>;
  template class basic_convert_impl<cfile_writer>;
  template class basic_convert_impl<ostream_writer>;
  template class basic_convert_impl<string_writer>;

  // template class _instantiate_for_writers<_instantiate_basic_convert>;
}
}

//-----------------------------------------------------------------------------
