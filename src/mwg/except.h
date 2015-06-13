// -*- mode:C++;coding:utf-8 -*-
#ifndef MWG_EXCEPT_H
#define MWG_EXCEPT_H
#include <cstdio>
#include <cerrno>
#include <string>
#include <typeinfo>
#include <exception>
#include <stdexcept>

namespace mwg{
  // エラーコード
  typedef unsigned ecode_t;

  static const ecode_t ecode_error=0;
  //static const ecode_t ecode_base=0x00100000;

  struct ecode{
    static const ecode_t error=0;

    static const ecode_t CATEGORY_MASK=0xFFFF0000;

    //--------------------------------------------------------------------------
    //  Flags
    //--------------------------------------------------------------------------
    static const ecode_t FLAG_MASK=0x0FF00000;
    // bit 27
    static const ecode_t EAssert  =0x08000000; // assert 失敗。プログラム自体が誤っている

    // bit 24-26 (種別)
    static const ecode_t ESupport =0x01000000; // サポートしていない操作・値
    static const ecode_t ETrial   =0x02000000; // 試行失敗 (通信など潜在的に失敗の可能性を含む物)
    static const ecode_t EAccess  =0x03000000; // アクセス権限
    static const ecode_t EAbort   =0x04000000; // 操作中止の為の人為的例外

    // bit 20-21 (主格)
    static const ecode_t EArgument  =0x00100000; // (明らかな)引数指定の誤りに起因
    static const ecode_t EData      =0x00200000; // 自身・引数の内部状態に起因
    static const ecode_t EOperation =0x00300000; // 操作の実装

    // bit 22-23 (原因)
    static const ecode_t ENull      =0x00400000; // 値が NULL 又は未初期化である事による物
    static const ecode_t ERange     =0x00800000; // 値の範囲がおかしい物
    static const ecode_t EClose     =0x00C00000; // 既に始末処理が行われている物

    // 複合
    static const ecode_t EArgRange=EArgument|ERange;
    static const ecode_t EArgNull =EArgument|ENull;
    static const ecode_t EDatRange=EData|ERange;
    static const ecode_t EDatNull =EData|ENull;
    static const ecode_t EImpl    =EOperation|ENull;    // 未実装
    static const ecode_t EInvalid =EOperation|ESupport; // 禁止操作・無効操作
    //--------------------------------------------------------------------------
    
    static const ecode_t io=0x00020000;
  };

  //****************************************************************************
  //    class except
  //============================================================================
  class except;
  class assertion_error;
  class invalid_operation;
  // std::bad_alloc
  // std::bad_cast
  // std::bad_typeid
  // std::bad_exception
  // std::ios_base::failure
  //
  // 恐らく投げるべきはこれ。 
  // std::logic_error
  //   std::out_of_range
  //   std::invalid_argument
  //   std::length_error
  //   std::domain_error
  //
  // 以下は処理系に依存して発生する例外。
  // 特に浮動小数点の取り扱い。
  // std::runtime_error
  //   std::range_error
  //   std::overflow_error
  //   std::underflow_error

  class except:public std::exception{
  protected:
    std::string msg;
    ecode_t ecode;
    mwg::except* original;
  //--------------------------------------------------------------------------
  //  初期化
  //--------------------------------------------------------------------------
  public:
    explicit except(const std::string& message,ecode_t ecode,const std::exception& orig)
      :msg(message),
       ecode(ecode),original(CopyException(&orig))
    {}
    explicit except(const std::string& message,ecode_t ecode=mwg::ecode_error)
      :msg(message),
       ecode(ecode),original(NULL)
    {}
    except()
      :msg("mwg::except"),ecode(0),original(NULL)
    {}
    virtual ~except() throw(){
      this->internal_free();
    }
  private:
    void internal_free(){
      if(this->original!=NULL){
        // virtual ~except(){} なので、全て delete で処分しておけば OK。
        delete this->original;
        this->original=NULL;
      }
    }
  public:
    except& operator=(const except& cpy){
      if(&cpy==this)return *this;
      this->internal_free();
      this->msg=cpy.msg;
      this->ecode=cpy.ecode;
      this->original=CopyException(cpy.original);

      return *this;
    }
    except(const except& cpy)
      :msg(cpy.msg),
       ecode(cpy.ecode),
       original(CopyException(cpy.original))
    {}
  //--------------------------------------------------------------------------
  //  original exception の為のメモリ管理
  //--------------------------------------------------------------------------
  protected:
    virtual except* copy() const{return new except(*this);}

  private:
    static except* CopyException(const except* e){
      if(e==NULL)return NULL;
      return e->copy();
    }
    static except* CopyException(const std::exception* e){
      if(e==NULL)return NULL;

      const except* err=dynamic_cast<const except*>(e);
      if(err)return CopyException(err);

      return new except(
        std::string("type<")+typeid(*e).name()+">: "+e->what(),
        (ecode_t)0);
    }
  //--------------------------------------------------------------------------
  //  機能
  //--------------------------------------------------------------------------
  public:
    virtual const char* what() const throw(){return this->msg.c_str();}
    ecode_t code() const{return this->ecode;}
  };

#define mwg_define_class_error(errorName) mwg_define_class_error_ex(errorName,mwg::except,mwg::ecode_error)
#define mwg_define_class_error_ex(errorName,BASE,ECODE)                                      \
  class errorName:public BASE{                                                               \
  public:                                                                                    \
    explicit errorName(const std::string& message,ecode_t ecode,const std::exception& orig)  \
      :BASE(message,ecode,orig)                                                              \
    {ecode|=(ECODE&ecode::CATEGORY_MASK);}                                                   \
    explicit errorName(const std::string& message,ecode_t ecode=ECODE)                       \
      :BASE(message,ecode)                                                                   \
    {ecode|=(ECODE&ecode::CATEGORY_MASK);}                                                   \
    errorName():BASE(#errorName,ECODE){}                                                     \
    errorName(const errorName& err):BASE(err){}                                              \
  public:                                                                                    \
    virtual errorName* copy() const{return new errorName(*this);}                            \
  }                                                                                          /**/

//mwg_define_class_error_ex(argument_error,except,ecode::EArgument);
//→ std::invalid_argument
//*****************************************************************************
//    ASSERTION
//=============================================================================
mwg_define_class_error_ex(assertion_error  ,except,ecode::EAssert);
mwg_define_class_error_ex(invalid_operation,except,ecode::EInvalid);
}
/**----------------------------------------------------------------------------
mwg_check/mwg_assert

概要
  プログラムの実行中に特定の条件が満たされている事を確認します。

  マクロ NDEBUG 及び MWG_DEBUG の値によって、デバグ用と配布用で動作が変わります。
  マクロ NDEBUG が定義されている場合は条件式のチェックとエラーメッセージの出力が抑制されます。
  NDEBUG は、通常コンパイラに最適化オプションを指定すると自動で定義されます。
  明示的に -DNDEBUG オプションを指定して NDEBUG を定義する事も可能です。
  一方で、重い研鑽を実行しながら開発・デバグをする場合、最適化をしつつ条件式のチェックも実行したい事があるかも知れません。
  その場合にはコンパイラに -DMWB_DEBUG オプションを指定する事によって、条件式のチェックとエラーメッセージの出力を強制する事ができます。
  即ち、<code>defined(NDEBUG)&&!defined(MWG_DEBUG)</code> の条件で、チェック・メッセージ出力が抑制されます。

関数一覧
  @fn int r=mwg_check_nothrow(条件式,書式,引数...);
    条件式が偽の場合に、その事を表すエラーメッセージを出力します。
  @fn mwg_check(条件式,書式,引数...);
    条件式が偽の場合に、エラーメッセージを出力して例外を発生させます。
    <code>defined(NDEBUG)&&!defined(MWG_DEBUG)</code> の場合には、単に例外を発生させます。
  @fn int r=mwg_assert_nothrow(条件式,書式,引数...);
    条件式が偽の場合に、その事を表すエラーメッセージを出力します。
    <code>defined(NDEBUG)&&!defined(MWG_DEBUG)</code> の場合には、条件式の評価も含めて何も実行しません。
  @fn mwg_assert(条件式,書式,引数...);
    条件式が偽の場合に、エラーメッセージを出力して例外を発生させます。
    <code>defined(NDEBUG)&&!defined(MWG_DEBUG)</code> の場合には、条件式の評価も含めて何も実行しません。
  @fn int r=mwg_verify_nothrow(条件式,書式,引数...);
    条件式が偽の場合に、その事を表すエラーメッセージを出力します。
    <code>defined(NDEBUG)&&!defined(MWG_DEBUG)</code> の場合には、条件式の評価のみを実行し結果を確認しません。
  @fn mwg_verify(条件式,書式,引数...);
    条件式が偽の場合に、エラーメッセージを出力して例外を発生させます。
    <code>defined(NDEBUG)&&!defined(MWG_DEBUG)</code> の場合には、条件式の評価のみを実行し結果を確認しません。
  @def MWG_NOTREACHED;
    制御がそこに到達しない事を表します。

  マクロ              通常              NDEBUG
  ~~~~~~~~~~~~~~~~~~  ~~~~~~~~~~~~~~~~  ~~~~~~~~~~~~~~~~~
  mwg_check           評価・出力・例外  評価・例外
  mwg_check_nothrow   評価・出力        評価・出力
  mwg_verify          評価・出力・例外  評価
  mwg_verify_nothrow  評価・出力        評価
  mwg_assert          評価・出力・例外  -
  mwg_assert_nothrow  評価・出力        -

引数と戻り値
  @param 条件式 : bool
    判定対象の条件式を指定します。評価した結果として真になる事が期待される物です。
  @param 書式 : const char*
    エラーが発生した時のメッセージを決める書式指定文字列です。printf の第一引数と同様の物です。
  @param 引数...
    書式に与える引数です。
  @return : int
    今迄に失敗した条件式の回数が返ります。

-----------------------------------------------------------------------------*/
#ifdef _MSC_VER
# define mwg_noinline __declspec(noinline)
#else
# define mwg_noinline
#endif
//!
//! @def mwg_assert_funcname
//!
#ifdef _MSC_VER
# define mwg_assert_funcname __FUNCSIG__
#elif defined(__INTEL_COMPILER)
// # define mwg_assert_funcname __FUNCTION__
# define mwg_assert_funcname __PRETTY_FUNCTION__
#elif defined(__GNUC__)
# define mwg_assert_funcname __PRETTY_FUNCTION__
#else
# define mwg_assert_funcname 0
#endif
//!
//! @def mwg_assert_position
//!
#define mwg_assert_stringize2(...) #__VA_ARGS__
#define mwg_assert_stringize(...) mwg_assert_stringize2(__VA_ARGS__)
#define mwg_assert_position __FILE__ mwg_assert_stringize(:__LINE__)

#include <cstdarg>
#include <string>
#include <mwg_config.h>

#if defined(__unix__)
# include <unistd.h>
#endif

namespace mwg{
namespace except_detail{
  struct sgr{
    int value;
    sgr(int value):value(value){}
  };

  class dbgput{
    FILE* file;
    bool m_isatty;
  public:
#if defined(__unix__)
    dbgput(FILE* file):file(file),m_isatty(isatty(fileno(file))){}
#else
    dbgput(FILE* file):file(file),m_isatty(false){}
#endif
    dbgput& operator<<(const char* str){
      while(*str)std::putc(*str++,file);
      return *this;
    }
    dbgput& operator<<(char c){
      std::putc(c,file);
      return *this;
    }
  private:
    void putsgr(int num){
      if(!m_isatty)return;

      std::putc('\x1b',file);
      std::putc('[',file);
      if(num>0){
        int x=1;
        while(x*10<=num)x*=10;
        for(;x>=1;x/=10)std::putc('0'+num/x%10,file);
      }
      std::putc('m',file);
    }
  public:
    dbgput& operator<<(sgr const& sgrnum){
      putsgr(sgrnum.value);
      return *this;
    }
  };
}
}

static mwg_noinline void mwg_vprintd_(const char* pos,const char* func,const char* fmt,va_list arg){
  using namespace ::mwg::except_detail;
  dbgput d(stderr);

  std::fprintf(stderr,"%s:",pos);
  if(fmt&&*fmt){
    d<<' '<<sgr(94);//sgr(35);
    std::vfprintf(stderr,fmt,arg);
    d<<sgr(0);
    if(func)
      d<<" @ "<<sgr(32)<<'"'<<func<<'"'<<sgr(0);
    d<<'\n';
  }else{
    if(func){
      d<<" mwg_printd @ "<<sgr(32)<<'"'<<func<<'"'<<sgr(0)<<'\n';
    }else
      d<<" mwg_printd\n";
  }
  std::fflush(stderr);
}
static void mwg_printd_(const char* pos,const char* func,const char* fmt,...){
  va_list arg;
  va_start(arg,fmt);
  mwg_vprintd_(pos,func,fmt,arg);
  va_end(arg);
}

static mwg_noinline int mwg_vcheckfn(bool condition,const char* expr,const char* pos,const char* func,const char* fmt,va_list arg){
  static int i=0;
  if(!condition){
    ++i; // dummy operation to set break point

    using namespace ::mwg::except_detail;
    dbgput d(stderr);
    // first line
    d<<sgr(1)<<"mwg_assertion_failure!"<<sgr(0)<<' '<<sgr(91)<<expr<<sgr(0);
    if(fmt&&*fmt){
      d<<", \""<<sgr(31);
      std::vfprintf(stderr,fmt,arg);
      d<<"\""<<sgr(0);
    }
    d<<'\n';

    // second line
    d<<"  @ "<<sgr(4)<<pos<<sgr(0);
    if(func)
      d<<':'<<func<<'\n';

    // std::fprintf(stderr,"%s:",pos);
    // if(func)
    //   d<<sgr(32)<<'"'<<func<<'"'<<sgr(0)<<':';
    // d<<" mwg_assertion_failure! ";
    // if(fmt&&*fmt){
    //   d<<sgr(35);
    //   std::vfprintf(stderr,fmt,arg);
    //   d<<sgr(0);
    // }
    // d<<"[expr: "<<sgr(94)<<expr<<sgr(0)<<"]\n";
    std::fflush(stderr);
  }
  return i;
}

static mwg_noinline void mwg_vcheckft(bool condition,const char* expr,const char* pos,const char* func,const char* fmt,va_list arg){
  if(condition)return;
  std::string buff("assertion failed! ");
  if(fmt&&*fmt){
    char message[1024];
#ifdef MWGCONF_HAS_VSNPRINTF
    // C99 vsnprintf
    /*?mconf
     * # X snprintf    cstdio           'char b[9];::snprintf(b,9,"");'             
     * X vsnprintf -h'cstdio' -h'cstdarg' 'char b[9];va_list a;::vsnprintf(b,9,"",a);'
     */
    ::vsnprintf(message,sizeof message,fmt,arg);
#else
    std::vsprintf(message,fmt,arg);
#endif
    buff+=message;
    buff+=" ";
  }
  buff+="[expr = ";
  buff+=expr;
  buff+=" @ ";
  buff+=pos;
  buff+=" ]";
  throw mwg::assertion_error(buff);
}
static int mwg_checkfn(bool condition,const char* expr,const char* pos,const char* func,const char* fmt,...){
  va_list arg;
  va_start(arg,fmt);
  int r=mwg_vcheckfn(condition,expr,pos,func,fmt,arg);
  va_end(arg);
  return r;
}
static void mwg_checkft(bool condition,const char* expr,const char* pos,const char* func,const char* fmt,...){
#if MWG_DEBUG||!defined(NDEBUG)
  va_list args1;
  va_start(args1,fmt);
  mwg_vcheckfn(condition,expr,pos,func,fmt,args1);
  va_end(args1);
#endif
  va_list args2;
  va_start(args2,fmt);
  mwg_vcheckft(condition,expr,pos,func,fmt,args2);
  va_end(args2);
}

#define mwg_printd(...)                    mwg_printd_(mwg_assert_position,mwg_assert_funcname,"" __VA_ARGS__)
// #define mwg_check_nothrow(condition,...)   mwg_checkfn(condition,#condition,mwg_assert_position,mwg_assert_funcname,"" __VA_ARGS__)
// #define mwg_check(condition,...)           mwg_checkft(condition,#condition,mwg_assert_position,mwg_assert_funcname,"" __VA_ARGS__)
#define mwg_check_nothrow(condition,...)   ((condition)||(mwg_checkfn(false,#condition,mwg_assert_position,mwg_assert_funcname,"" __VA_ARGS__),false))
#define mwg_check(condition,...)           ((condition)||(mwg_checkft(false,#condition,mwg_assert_position,mwg_assert_funcname,"" __VA_ARGS__),false))

#if MWG_DEBUG||!defined(NDEBUG)
// # define mwg_assert_nothrow(condition,...) mwg_checkfn(condition,#condition,mwg_assert_position,mwg_assert_funcname,"" __VA_ARGS__)
// # define mwg_assert(condition,...)         mwg_checkft(condition,#condition,mwg_assert_position,mwg_assert_funcname,"" __VA_ARGS__)
# define mwg_verify_nothrow(condition,...) mwg_check_nothrow(condition,__VA_ARGS__)
# define mwg_verify(condition,...)         mwg_check(condition,__VA_ARGS__)
# define mwg_assert_nothrow(condition,...) mwg_check_nothrow(condition,__VA_ARGS__)
# define mwg_assert(condition,...)         mwg_check(condition,__VA_ARGS__)
#else
  static inline int mwg_assert_empty(){return 1;}
  static inline int mwg_assert_empty(int value){return value;}
# define mwg_verify(condition,...)         mwg_assert_empty(condition)
# define mwg_verify_nothrow(condition,...) mwg_assert_empty(condition)
# ifdef _MSC_VER
#   define mwg_assert(condition,...)         __assume(condition)
#   define mwg_assert_nothrow(condition,...) __assume(condition)
# else
#   define mwg_assert(...)                   mwg_assert_empty()
#   define mwg_assert_nothrow(...)           mwg_assert_empty()
# endif
#endif
#define MWG_NOTREACHED /* NOTREACHED */ mwg_assert(0,"NOTREACHED")

#endif
