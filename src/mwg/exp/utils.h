// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_EXP_UTILS_H
#define MWG_EXP_UTILS_H
#include <mwg/defs.h>
#include <mwg/except.h>
#include <mwg/concept.h>
#include <mwg/std/iterator>
#include <mwg/std/type_traits>
#include <mwg/std/memory>
namespace mwg{
namespace exp{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

  // このクラスは現在は使われていない
  // x C++03 では、その型のコンパイル時定数を宣言することができない
  // x CRTP::enum_t を代替として定数を宣言しても、
  //   underlying_type とサイズが異なるかもしれない。
  template<typename CRTP,typename I=unsigned int>
  class enum_class_base{
  public:
    typedef I underlying_type;
    mwg_constexpr explicit enum_class_base(const underlying_type& value):value(value){}

    typedef typename CRTP::enum_t enum_type;
    mwg_constexpr enum_class_base(const enum_type& value):value(value){}

    bool operator==(const CRTP& r) const{return this->value==r.value;}
    bool operator!=(const CRTP& r) const{return this->value!=r.value;}
    CRTP operator&(const CRTP& r) const{return this->value&r.value;}
    CRTP operator^(const CRTP& r) const{return this->value^r.value;}
    CRTP operator|(const CRTP& r) const{return this->value|r.value;}
    CRTP operator~() const{return ~this->value;}

    // // sizeof(underlying_type) > sizeof(enum_type) の時に危険
    // operator enum_type() const{return (enum_type)value;}

    static bool equals(const enum_class_base& l,const underlying_type& r){return l.value==r;}
  protected:
    underlying_type value;
  };

  template<typename T,typename I>
  bool operator==(const enum_class_base<T,I>& l,const typename T::enum_t& r){
    return enum_class_base<T,I>::equals(l,(I)r);
  }
  template<typename T,typename I>
  bool operator==(const typename T::enum_t& r,const enum_class_base<T,I>& l){
    return enum_class_base<T,I>::equals(l,(I)r);
  }
  template<typename T,typename I>
  bool operator!=(const enum_class_base<T,I>& l,const typename T::enum_t& r){
    return !enum_class_base<T,I>::equals(l,(I)r);
  }
  template<typename T,typename I>
  bool operator!=(const typename T::enum_t& r,const enum_class_base<T,I>& l){
    return !enum_class_base<T,I>::equals(l,(I)r);
  }

  template<typename T,typename I>
  T operator&(enum_class_base<T,I> const& lhs,typename T::enum_t const& rhs){return enum_class_base<T,I>(rhs)&lhs;}
  template<typename T,typename I>
  T operator&(typename T::enum_t const& lhs,enum_class_base<T,I> const& rhs){return enum_class_base<T,I>(lhs)&rhs;}
  template<typename T,typename I>
  T operator|(enum_class_base<T,I> const& lhs,typename T::enum_t const& rhs){return enum_class_base<T,I>(rhs)|lhs;}
  template<typename T,typename I>
  T operator|(typename T::enum_t const& lhs,enum_class_base<T,I> const& rhs){return enum_class_base<T,I>(lhs)|rhs;}
  template<typename T,typename I>
  T operator^(enum_class_base<T,I> const& lhs,typename T::enum_t const& rhs){return enum_class_base<T,I>(rhs)^lhs;}
  template<typename T,typename I>
  T operator^(typename T::enum_t const& lhs,enum_class_base<T,I> const& rhs){return enum_class_base<T,I>(lhs)^rhs;}

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  static_flags
//-----------------------------------------------------------------------------
  template<typename Tag,int I>
  struct static_flags{
    static_flags(){}

    template<int J>
    const static_flags<Tag,(I^J)>& operator^(const static_flags<Tag,J>&) const{
      return reinterpret_cast<static_flags<Tag,(I^J)> >(0);
    }
    template<int J>
    const static_flags<Tag,(I|J)>& operator|(const static_flags<Tag,J>&) const{
      return reinterpret_cast<static_flags<Tag,(I|J)> >(0);
    }
    template<int J>
    const static_flags<Tag,(I&J)>& operator&(const static_flags<Tag,J>&) const{
      return reinterpret_cast<static_flags<Tag,(I&J)> >(0);
    }
    const static_flags<Tag,(~I)>& operator~() const{
      return reinterpret_cast<static_flags<Tag,~I> >(0);
    }
  };
#define mwg_static_flags_cref(TAG,VALUE) \
  const mwg::exp::static_flags<TAG,VALUE>&

/* #define mwg_static_flags_define(NAME,TAG,VALUE)                                      \
 *   static mwg_static_flags_cref(TAG,VALUE) NAME                                       \
 *     =reinterpret_cast<mwg_static_flags_cref(TAG,VALUE)>(*reinterpret_cast<int*>(0)); \
 *   static const int NAME##_flag=VALUE;                                               \/\*\*\/
 */
//※null reference はコンパイラの警告が出る。

#define mwg_static_flags_define(NAME,TAG,VALUE)                         \
  static mwg::exp::static_flags<TAG,VALUE> const NAME;                  \
  static const int NAME##_flag=VALUE;
//※クラス内で定義すると、NAME を実際に使った時、リンク時に NAME が存在しないと怒られる。

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  enumeration
//-----------------------------------------------------------------------------
  struct ienumerator_end_tag{};
  template<typename T,typename Derived=void>
  class ienumerator;
  template<typename T>
  class enumerator;

  namespace detail{
    template<typename T>
    class ienumerator_null_enumerator;
  }

  template<typename T>
  class ienumerator<T,void>{
  public:
    virtual void next()=0;
    virtual operator bool() const=0;
    virtual T operator*() const=0;
    virtual ~ienumerator(){}
  //-------------------------------------------------------
  // input_iterator
  public:
    typedef std::ptrdiff_t difference_type;
    typedef typename stdm::remove_reference<T>::type value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef std::input_iterator_tag iterator_category;
    ienumerator& operator++(){
      this->next();
      return *this;
    }
    // CHK: T が参照でない場合?
    value_type operator++(int){
      value_type ret=this->operator*();
      this->next();
      return ret;
    }
    pointer operator->() const{return &this->operator*();}
    bool operator==(const ienumerator& r) const{
      bool ll=this->operator bool();
      bool rr=r.operator bool();
      if(ll&&rr)
        return this==&r; /* 適当... */
      else
        return ll==rr;
    }
    bool operator!=(const ienumerator& r) const{
      return !this->operator==(r);
    }
  //-------------------------------------------------------
  // range
  public:
    typedef detail::ienumerator_null_enumerator<T> null_enumerator;
    // class null_enumerator:public ienumerator{
    // public:
    //   virtual void next(){}
    //   virtual operator bool() const{return false;}
    //   virtual T operator*() const{
    //     throw mwg::invalid_operation("invalid operation");
    //   }
    // };
  };

  namespace detail{
    template<typename T>
    class ienumerator_null_enumerator:public ienumerator<T,void>{
    public:
      virtual void next(){}
      virtual operator bool() const{return false;}
      virtual T operator*() const{
        throw mwg::invalid_operation("invalid operation");
      }
    };
  }

  template<typename T,typename Derived>
  class ienumerator:public ienumerator<T>{
  //-------------------------------------------------------
  // input_iterator
  public:
    Derived& operator++(){
      this->next();
      return static_cast<Derived&>(*this);
    }
    Derived operator++(int){
      Derived ret=static_cast<Derived&>(*this);
      this->next();
      return ret;
    }
  //-------------------------------------------------------
  // range
  public:
    Derived& begin(){
      return static_cast<Derived&>(*this);
    }
    Derived end(){
      return Derived(ienumerator_end_tag());
    }
  };

  template<typename T>
  class enumerator:public ienumerator<T,enumerator<T> >{
    stdm::unique_ptr<ienumerator<T> > ptr;
  public:
    enumerator():ptr(nullptr){}
    enumerator(ienumerator<T>* p):ptr(p){}
    enumerator(ienumerator_end_tag):ptr(nullptr){}
    virtual void next(){this->ptr->next();}
    virtual operator bool() const{return ptr.get()&&ptr->operator bool();}
    virtual T operator*() const{return this->ptr->operator*();}

    enumerator& operator=(ienumerator<T>* p){this->ptr.reset(p);return *this;}

#if mwg_has_feature(cxx_rvalue_references)
    enumerator(enumerator&& movee){this->ptr=stdm::move(movee.ptr);}
    enumerator& operator=(enumerator&& movee){
      this->ptr=stdm::move(movee.ptr);
      return *this;
    }
//#else
//    enumerator(const enumerator& movee)=default;
//    enumerator& operator=(const enumerator& movee)=default;
#endif
  };

//-----------------------------------------------------------------------------
#define mwg_yield_start   switch(_yld_flag){case 0:
#define mwg_yield_start_with(flag,value,T)            \
  int& _yld_flag(flag);                               \
  mwg::identity<T>::type& _yld_value(value);          \
  mwg_yield_start
#define mwg_yield_end  default:goto _yld_default;_yld_default:_yld_flag=-1;_yld_exit:;}
#define mwg_yield_last(VALUE)         \
  do{                                 \
    _yld_value=VALUE;                 \
    _yld_flag=-1;                     \
    goto _yld_exit;                   \
  }while(0)
#define mwg_yield_returnID(ID,VALUE)  \
  do{                                 \
    _yld_value=VALUE;                 \
    _yld_flag=ID;                     \
    goto _yld_exit;                   \
    case ID:;                         \
  }while(0)
#define mwg_yield_allvalID(ID,ENUM)   \
  do{                                 \
    for(;(ENUM);++(ENUM))             \
      mwg_yield_returnID(ID,*(ENUM)); \
  }while(0)
#define mwg_yield_allptrID(ID,ENUM)   \
  do{                                 \
    for(;(ENUM);++(ENUM))             \
      mwg_yield_returnID(ID,&*(ENUM));\
  }while(0)
#define mwg_yield_return(VALUE)       mwg_yield_returnID(__LINE__,VALUE)
#define mwg_yield_allval(ENUM)        mwg_yield_allvalID(__LINE__,ENUM)
#define mwg_yield_allptr(ENUM)        mwg_yield_allptrID(__LINE__,ENUM)
#define mwg_yield_break   goto _yld_default
//-----------------------------------------------------------------------------
#define mwg_fiber_start   switch(_fib_flag){case 0:
#define mwg_fiber_start_with(flag)    \
  int& _fib_flag(flag);               \
  mwg_fiber_start
#define mwg_fiber_end default:goto _fib_default;_fib_default:_fib_flag=-1;_fib_exit:;}
#define mwg_fiber_continueID(ID,VALUE)  \
  do{                                 \
    _fib_flag=ID;                     \
    goto _fib_exit;                   \
    case ID:;                         \
  }while(0)
#define mwg_fiber_continue(VALUE)     mwg_fiber_continueID(__LINE__,VALUE)
#define mwg_fiber_break   goto _fib_default
//-----------------------------------------------------------------------------

  template<typename Iterator>
  class range_enumerator:public mwg::exp::ienumerator<typename std::iterator_traits<Iterator>::reference,range_enumerator<Iterator> >{
    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    typedef typename std::iterator_traits<Iterator>::reference  reference;

    Iterator i;
    Iterator iN;
    int _yld_flag;
    int _yld_value; // dummy
  public:
    range_enumerator(const Iterator& begin,const Iterator& end):i(begin),iN(end),_yld_flag(0){this->next();}
    range_enumerator(mwg::exp::ienumerator_end_tag):_yld_flag(-1){}

    int index;
    virtual void next(){
      mwg_yield_start;

      for(;i!=iN;i++)
        mwg_yield_returnID(1,0);

      mwg_yield_end;
    }
    virtual operator bool() const{
      return _yld_flag>=0;
    }
    virtual reference operator*() const{
      return *this->i;
    }
  };

//EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
// T 型の列挙子を定義する場合:
//  1. int _yld_flag を定義します。これは何処まで列挙したかを覚えておく為の物です。
//  2. T _yld_value を定義します。これは現在の値を記録する為の物です。
//  3. 列挙子を初期化する部分で _yld_flag に 0 を設定します。
//  4. 反復部分を mwg_yield_start と mwg_yield_end で囲みます。
//  5. value を yield したい所に mwg_yield_return(value); を記述します。
// 注意 "変数の宣言について"
//  mwg_yield_start と mwg_yield_end の間では変数を宣言出来ません。(yield を含
//  まないブロックの中では宣言する事が出来ます。) 寿命が yield を跨ぐ様な変数を
//  宣言する場合には、_yld_flag や _yld_value と同じ寿命になる様に宣言して下さい。
// 注意 "yield の id について"
//  同じ行内に複数の yield は記述出来ません。これは、行番号を用いて最後の yield
//  の位置を記録している為です。記録の為に用いる番号を明示的に指定するには、
//  mwg_yield_returnID(番号,value) を使用して下さい。但し、0 は特別な意味を持つ
//  ので指定しないで下さい。また、番号として 1 2 3 を順に使う事で最適化を促す事
//  が出来る…場合があるかも知れません。
#if 0
  class MyEnumerator:public mwg::exp::ienumerator<int,MyEnumerator>{
    int _yld_flag;
    int _yld_value;
  public:
    MyEnumerator():_yld_flag(0){this->next();}
    MyEnumerator(mwg::exp::ienumerator_end_tag):_yld_flag(-1){}

    int index;
    virtual void next(){
      mwg_yield_start;

      for(index=0;index<10;index++){
        mwg_yield_return(index*index);
      }

      mwg_yield_end;
    }
    virtual operator bool() const{
      return _yld_flag>=0;
    }
    virtual int operator*() const{
      return this->_yld_value;
    }
  };
#endif
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
