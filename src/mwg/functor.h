// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef MWG_FUNCTOR_H
#define MWG_FUNCTOR_H
#pragma%x begin_check
#include <cstdio>
#include <cstring>
#include <mwg/std/type_traits>
#include <mwg/except.h>
#include <mwg/concept.h>
#include <mwg/functor.h>

//-----------------------------------------------------------------------------
// Test targets

int func1(){
  return 123;
}

class C{
  int x;
public:
  C(int x):x(x){}
  void print() const{
    std::printf("C::print(): this->x =%d\n",this->x);
  }

  int getValue() const{return this->x;}
};

class F{
  int x;
public:
  F(int x):x(x){}
  int operator()(int y) const{
    //std::printf("F::print(int): x+y=%d\n",x+y);
    return x+y;
  }
};

class F2{
public:
  int operator()(int) const{return 1;}
  int operator()(float) const{return 1;}
  int operator()(int,int) const{return 2;}
  int operator()(int x,int y,int z) const{return x+y+z;}
};

struct Str{
  int x;
  int y;
};

#pragma%x end_check
#include <new>
#include <algorithm>
#include <cstring>
#include <mwg/std/type_traits>
#include <mwg/std/utility>
#include <mwg/concept.h>
#include "funcsig.h"
#include "functor.proto.h"

#define mwg_attribute(X) mwg_attribute_##X
#if MWGCONF_GCC_VER>30300
# define mwg_attribute_may_alias __attribute__((may_alias))
#else
# define mwg_attribute_may_alias
#endif

#pragma%include "impl/VariadicMacros.pp"
#pragma%include "bits/functor/functor.variadic.pp"
#pragma%include "bits/functor.type_traits.pp"

namespace mwg{
namespace functor_detail{
  namespace sig=mwg::funcsig;
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
//
//    Functor Traits
//
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  concept functor_traits
//------------------------------------------------------------------------------
  /*?lwiki
   * &pre*(!cpp){
   * template<typename F>
   * struct functor_traits{
   *   static const bool is_functor;
   *   typedef F fct_t;
   *   typedef auto sgn_t;
   *
   *   typedef auto ref_tr; // functor への参照を保持
   *   typedef auto ins_tr; // functor の複製インスタンスを保持
   *   static '''return-type''' invoke(const fct_t& f,...){
   *     f(...);
   *   }
   * };
   * template<typename F,typename S>
   * struct functor_traits{
   *   static const bool is_functor;
   *   typedef F fct_t;
   *   typedef S sgn_t;
   *
   *   typedef auto ref_tr; // functor への参照を保持
   *   typedef auto ins_tr; // functor の複製インスタンスを保持
   *   static '''return-type''' invoke(const fct_t& f,...){
   *     f(...);
   *   }
   * };
   * }
   */
  template<typename F>
  struct functor_case_traits{
    typedef F fct_t;
    typedef functor_traits<F> fct_tr;
    typedef fct_t case_data;
    static const fct_t& endata(const fct_t& f){return f;}
    static const fct_t& dedata(const fct_t& f){return f;}
  };
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  functor_traits implementation helpers
//------------------------------------------------------------------------------
  struct functor_traits_empty{
    // not functor
    static const bool is_functor=false;

    typedef void fct_t;
    typedef void sgn_t;
    typedef void ref_tr;
    typedef void ins_tr;
  };

  /*?lwiki
   * :@class class functor_traits_signature<S>;
   *  `functor_traits` 特殊化の実装に使うヘルパクラスです。以下の物を定義します。
   *  &pre(!cpp){
   *  static const bool is_functor=true;
   *  typedef S sgn_t;
   *  }
   *  `struct functor_traits_signature` で定義される物の他に以下の物を定義する必要があります。
   *  &pre(!cpp){
   *  typedef auto fct_t;                // raw functor type
   *  typedef auto ref_tr;               // traits of mwg::functor_ref case
   *  typedef auto ins_tr;               // traits of mwg::functor data case
   *  static R invoke(const fct_t&,...); // invoke functor object
   *  }
   */
  template<typename S>
  struct functor_traits_signature{};
#pragma%m 1
  template<typename R,typename... A>
  struct functor_traits_signature<R(A...)>:functor_traits_empty{
    static const bool is_functor=true;
    typedef R (sgn_t)(A...);
  };
  template<typename R,typename... A>
  struct functor_traits_signature<R(A...,...)>:functor_traits_empty{
    static const bool is_functor=true;
    typedef R (sgn_t)(A...,...);
  };
#pragma%end
#pragma%x variadic_expand_0toArN
  template<
    typename S,
    typename F,
    typename RefCaseTr=functor_case_traits<F>,
    typename InsCaseTr=functor_case_traits<F> >
  struct functor_traits_impl:functor_traits_signature<S>{
    typedef F fct_t;
    typedef RefCaseTr ref_tr;
    typedef InsCaseTr ins_tr;
  };
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT

  struct functor_traits_end:functor_traits_empty{};

  /*?lwiki
   * :@class template<int L,typename F,typename=void> struct functor_traits_chain;
   *  指定した型 `T` を関手として取り扱う方法を提供するリストです。
   *  `L` について特殊化を定義することにより新しい方法を追加します。
   *  具体的な `L` の値の決定は <?pp functor_traits_chain::register?>
   *  マクロを使用します。実際の使用例を参照して下さい。
   */
  template<int L,typename F,typename=void>
  struct functor_traits_chain:functor_traits_empty{};
  template<
    typename F,int L=0,
    bool=functor_traits_chain<L,F>::is_functor,
    bool=!stdm::is_base_of<functor_traits_end,functor_traits_chain<L,F> >::value>
  struct functor_traits_selector:functor_traits_empty{};
  template<typename F,int L,bool B>
  struct functor_traits_selector<F,L,true ,B>:functor_traits_chain<L,F>{};
  template<typename F,int L>
  struct functor_traits_selector<F,L,false,true>:functor_traits_selector<F,L+1>{};
  template<typename F,int L>
  struct functor_traits_selector<F,L,false,false>:functor_traits_empty{};

  /*?lwiki
   * @class template<int L,typename F,typename S,typename=void> struct functor_traits_chain2;
   *  指定した型 `T` を関手 `S` として取り扱う方法を提供するリストです。
   *  `L` について特殊化を定義することにより新しい方法を追加します。
   *  具体的な `L` の値の決定は <?pp functor_traits_chain::register?>
   *  マクロを使用します。実際の使用例を参照して下さい。
   */
  template<int L,typename F,typename S,typename=void>
  struct functor_traits_chain2:functor_traits_empty{};
  template<
    typename F,typename S=void,int L=0,
    bool=functor_traits_chain2<L,F,S>::is_functor,
    bool=!stdm::is_base_of<functor_traits_end,functor_traits_chain2<L,F,S> >::value>
  struct functor_traits_selector2:functor_traits_empty{};
  template<typename F,typename S,int L,bool B>
  struct functor_traits_selector2<F,S,L,true ,B>:functor_traits_chain2<L,F,S>{};
  template<typename F,typename S,int L>
  struct functor_traits_selector2<F,S,L,false,true>:functor_traits_selector2<F,S,L+1>{};
  template<typename F,typename S,int L>
  struct functor_traits_selector2<F,S,L,false,false>:functor_traits_empty{};

#pragma%[functor_traits_chain_count=0]

#pragma%m functor_traits_chain::register
#pragma%%x 1.r/@/$"functor_traits_chain_count"/.i
#pragma%%[functor_traits_chain_count++]
#pragma%end

#pragma%m functor_traits_chain::terminate
#pragma%%x
  template<typename F>
  struct functor_traits_chain<$"functor_traits_chain_count",F,void>:functor_traits_end{};
  template<typename F,typename S>
  struct functor_traits_chain2<$"functor_traits_chain_count",F,S,void>:functor_traits_end{};
#pragma%%end.i
#pragma%end
}
}

#pragma%include "bits/functor/functor.varargs.pp"

namespace mwg{
namespace functor_detail{

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class functor_traits<R (*)(params)>
//  class functor_traits<R(params)>
//------------------------------------------------------------------------------

  template<typename S,typename F>
  struct functor_invoker_function{};
#pragma%m 1
  template<typename F,typename R,typename... A>
  struct functor_invoker_function<R(A...),F>{
    static R invoke(F f,A... arg,...){
      return R(f(arg...));
    }
  };
#pragma%end
#pragma%x variadic_expand_0toArN

#pragma%m 1
  template<typename F>
  struct functor_traits_chain<@,F*,typename stdm::enable_if<stdm::is_function<F>::value>::type>
    :functor_traits_impl<F,F*>,functor_invoker_function<F,F*>{};
  template<typename F>
  struct functor_traits_chain<@,F,typename stdm::enable_if<stdm::is_function<F>::value>::type>
    :functor_traits_impl<F,F*>,functor_invoker_function<F,F*>{};
#pragma%end
#pragma%x functor_traits_chain::register

}
}

namespace mwg{
namespace functor_detail{

  // [traits.mfp] R (C::*)(params)
  // [traits.mfp] R (C::*)(params...) ■TODO
  template<typename S,typename Mfp>
  struct functor_invoker_mfp;
#pragma%m 1
  template<typename Mfp,typename R,typename CRef,typename... A>
  struct functor_invoker_mfp<R(CRef,A...),Mfp>{
    static R invoke(Mfp f,CRef c,A... a){
      return R((c.*f)(a...));
    }
  };
#pragma%end
#pragma%x variadic_expand_0toArNm1

  // [traits.mp] T C::*
  template<typename S,typename Mop>
  struct functor_invoker_mop;
  template<typename Mop,typename R,typename C>
  struct functor_invoker_mop<R(C),Mop>{
    static typename stdm::add_lvalue_reference<R>::type invoke(Mop f,C& c,...){return c.*f;}
    static typename stdx::add_const_reference<R>::type invoke(Mop f,C const& c,...){return c.*f;}
  };

#pragma%m 1
  template<typename Mfp>
  struct functor_traits_chain<@,Mfp,typename stdm::enable_if<stdm::is_member_function_pointer<Mfp>::value>::type>
    : functor_traits_impl<typename is_memfun_pointer<Mfp>::functor_sgn,Mfp>
    , functor_invoker_mfp<typename is_memfun_pointer<Mfp>::functor_sgn,Mfp>{};
  template<typename T,typename C>
  struct functor_traits_chain<@,T C::*,typename stdm::enable_if<stdm::is_member_object_pointer<T C::*>::value>::type>
    : functor_traits_impl<T&(C&),T C::*>
    , functor_invoker_mop<T(C),T C::*>{};

  template<typename T,typename C,typename S>
  struct functor_traits_chain2<
    @,T C::*,S,
    typename stdm::enable_if<
      stdm::is_member_object_pointer<T C::*>::value&&(
        is_variant_function<typename stdm::add_lvalue_reference<T>::type (C&),S>::value||
        is_variant_function<typename stdx::add_const_reference<T>::type (C const&),S>::value
      )
    >::type >:functor_traits<T C::*>{typedef S sgn_t;};
#pragma%end
#pragma%x functor_traits_chain::register

}
}

namespace mwg{
namespace functor_detail{

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class functor_traits<F /* having operator() */>
//------------------------------------------------------------------------------
#pragma%begin
  template<typename C>
  struct functor_case_traits_FunctorRef;
  template<typename Mfp>
  struct functor_traits_ftorF;
  template<typename P,typename Mfp>
  struct functor_traits_ftorP;
//------------------------------------------------------------------------------
/* TODO:
  ■operator() と operator() const の両方が定義されている場合
    どちらか一方を優先させる
    ・decltype(&F::operator()) では望んだ方のポインタを取得出来ない
*/
//=============================================================================
#pragma%end
  template<typename C>
  struct functor_case_traits_FunctorRef:functor_case_traits<C>{
    typedef const C* case_data;
    static const C* endata(const C& ins){return &ins;}
    static const C& dedata(const C* p){return *p;}
  };
//-----------------------------------------------------------------------------

  template<typename S,typename F>
  struct functor_invoker_functor{};
#pragma%m 1
  template<typename R,typename C,typename... A>
  struct functor_invoker_functor<R(A...),C>{
    static R invoke(C const& f,A... a,...){
      return R(const_cast<C&>(f)(a...));
    }
  };
#pragma%end
#pragma%x variadic_expand_0toArN

  template<typename S,typename P,typename C>
  struct functor_invoker_pfunctor{};
#pragma%m 1
  template<typename P,typename C,typename R,typename... A>
  struct functor_invoker_pfunctor<R(A...),P,C>{
    static R invoke(const P& f,A... a,...){
      return R(const_cast<C&>(*f)(a...));
    }
  };
#pragma%end
#pragma%x variadic_expand_0toArN
//=============================================================================
#if defined(MWGCONF_STD_DECLTYPE)
  template<typename Mfp>
  struct functor_traits_ftorF;
  template<typename P,typename Mfp>
  struct functor_traits_ftorP;
//-----------------------------------------------------------------------------
// [traits.functor] F with R F::operator()(%types%)
  template<typename Mfp>
  struct functor_traits_ftorF
    :functor_traits_impl<
      typename is_memfun_pointer<Mfp>::member_type,
      typename is_memfun_pointer<Mfp>::object_type,
      functor_case_traits_FunctorRef<typename is_memfun_pointer<Mfp>::object_type>
    >
    ,functor_invoker_functor<
      typename is_memfun_pointer<Mfp>::member_type,
      typename is_memfun_pointer<Mfp>::object_type
    >
  {};

#pragma%m 1
  template<typename F>
  struct functor_traits_chain<@,F,typename stdm::enable_if<has_single_operator_functor<T>::value>::type>
    :functor_traits_ftorF<decltype(&F::operator())>{};
#pragma%end
#pragma%x functor_traits_chain::register

//-----------------------------------------------------------------------------
// [traits.pfunctor] P with F P::operator*() / R F::operator()(%types%)
  template<typename P,typename SOp>
  struct functor_traits_ftorP
    :functor_traits_impl<typename is_memfun_pointer<SOp>::member_type,P>
    ,functor_invoker_pfunctor<
      typename is_memfun_pointer<SOp>::member_type,
      P,
      typename is_memfun_pointer<SOp>::object_type
    >
  {};

#pragma%m 1
  template<typename P>
  struct functor_traits_chain<@,P,typename stdm::enable_if<is_pointer_to_single_operator_functor<T>::value>::type>
    :functor_traits_ftorP<P,typename is_pointer_to_single_operator_functor<P>::operator_type>{};
#pragma%end
#pragma%x functor_traits_chain::register
#endif
}
}

namespace mwg{
namespace functor_detail{
  /*?lwiki
   * :@var mwg::functor_detail::==is_variant_functor==<typename From,typename To>::value;
   *  関手 `From` を関手 `To` に変換できるかどうかを判定します。
   */
  template<typename F,typename T,bool=(functor_traits<F>::is_functor&&functor_traits<T>::is_functor)>
  struct is_variant_functor:stdm::false_type{};
  template<typename F,typename T>
  struct is_variant_functor<F,T,true>
    :is_variant_function<typename functor_traits<F>::sgn_t,typename functor_traits<T>::sgn_t>{};

#pragma%m 1
  template<typename F,typename S>
  struct functor_traits_chain2<@,F,S,typename stdm::enable_if<is_variant_functor<F,S>::value>::type>
    :functor_traits<F>{typedef S sgn_t;};
#pragma%end
#pragma%x functor_traits_chain::register

#pragma%m 1
  template<typename F,typename S>
  struct functor_traits_chain2<@,F,S,typename stdm::enable_if<can_be_called_as<F,S>::value>::type>
    :functor_traits_signature<typename can_be_called_as<F,S>::signature_type>
    ,functor_invoker_functor<typename can_be_called_as<F,S>::signature_type,F>
  {
    typedef F fct_t;
    struct ref_tr:functor_case_traits_FunctorRef<F>{typedef functor_traits_chain2 fct_tr;};
    struct ins_tr:functor_case_traits           <F>{typedef functor_traits_chain2 fct_tr;};
  };
#pragma%end
#pragma%x functor_traits_chain::register
}
}

namespace mwg{
namespace functor_detail{

#pragma%x functor_traits_chain::terminate

  template<typename F,typename S>
  struct functor_traits:functor_traits_selector2<F,S>{};
  template<typename F>
  struct functor_traits<F,void>:functor_traits_selector<F>{};

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class is_functor/be_functor
//------------------------------------------------------------------------------
  template<typename F,typename S>
  struct is_functor:stdm::is_same<S,typename mwg::functor_traits<F>::sgn_t>{};
  template<typename F,typename S>
  struct be_functor:stdm::integral_constant<bool,functor_traits<F,S>::is_functor>{};

#pragma%m 1
  template<typename S,typename F,typename... A>
  typename stdm::enable_if<mwg::be_functor<F,S>::value,typename sig::returns<S>::type>::type
  functor_invoke(const F& f,A mwg_forward_rvalue... a){
    return mwg::functor_traits<F,S>::invoke(f,stdm::forward<A>(a)...);
  }
#pragma%end
#pragma%x variadic_expand_0toArN

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
//
//    Functor Classes
//
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  interface functor_case
//------------------------------------------------------------------------------
#pragma%m 1
  template<typename R %s_typenames%>
  struct functor_case<R(%types%)>{
    virtual R call(%types%) const=0;
    virtual ~functor_case(){}
    virtual functor_case* placement_clone(void* ptr) const=0;
  };
#pragma%end
#pragma%x mwg::functor::arities
  template<typename S,typename T,bool INTERIOR>
  class functor_case_data:public functor_case<S>{
    char m_data[sizeof(T)];
  private:
    typedef typename mwg::stdm::remove_cv<T>::type data_type;
#if defined(__GNUC__)&&MWGCONF_GCC_VER<40000
    const data_type* ptr() const{return (const data_type*)(&this->m_data);}
    data_type* ptr(){return (data_type*)(&this->m_data);}
#else
    const data_type* ptr() const{return reinterpret_cast<const data_type*>(&this->m_data);}
    data_type* ptr(){return reinterpret_cast<data_type*>(&this->m_data);}
#endif
  protected:
    functor_case_data(const T& value){new(this->ptr()) T(value);}
    ~functor_case_data(){get_ref().~T();}
    const T& get_ref() const{return *this->ptr();}
  private:
    functor_case_data& operator=(const functor_case_data&) mwg_std_deleted;
  };
  template<typename S,typename T>
  class functor_case_data<S,T,false>:functor_case<S>{
    T* ptr;
  protected:
    functor_case_data(const T& value):ptr(new T(value)){}
    functor_case_data(const functor_case_data& c):ptr(new T(*c.ptr)){}
    ~functor_case_data(){delete this->ptr;this->ptr=nullptr;}
    const T& get_ref() const{return *ptr;}
  private:
    functor_case_data& operator=(const functor_case_data&) mwg_std_deleted;
  };

  // CHK: sizeof(void*)*2 の値は妥当か?
  template<typename F,bool IsFunction>
  struct functor_case_data__is_interior__impl
    :stdm::integral_constant<bool,(sizeof(F)<=sizeof(void*)*2)>{};
  template<typename F>
  struct functor_case_data__is_interior__impl<F,true>
    :stdm::integral_constant<bool,(sizeof(F*)<=sizeof(void*)*2)>{};
  template<typename F>
  struct functor_case_data__is_interior
    :functor_case_data__is_interior__impl<F,stdm::is_function<F>::value>{};

#pragma%m 1
  template<typename Tr,typename R %s_typenames%>
  class functor_case_impl<R(%types%),Tr>
    :public functor_case_data<R(%types%),typename Tr::case_data,functor_case_data__is_interior<R(%types%)>::value>
  {
  public:
    typedef Tr case_tr; // used by others
    typedef R (sgn_t)(%types%);
    typedef typename Tr::case_data case_data;
    typedef functor_case_data<sgn_t,case_data,functor_case_data__is_interior<sgn_t>::value> base;
  public:
    functor_case_impl(const case_data& f):base(f){} // used by placement clone
    template<typename F> functor_case_impl(const F& f):base(Tr::endata(f)){}
    virtual R call(%params%) const{
      typedef typename Tr::fct_tr fct_tr; // gcc-2.95.3 work around 一旦型に入れる必要有り。
      return R(fct_tr::invoke(Tr::dedata(this->get_ref()) %s_args%));
    }
    virtual functor_case_impl* placement_clone(void* ptr) const{
      return new(ptr) functor_case_impl(this->get_ref());
    }
  };
#pragma%end
#pragma%x mwg::functor::arities

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  interface functor_base
//------------------------------------------------------------------------------
#pragma%m 1
  template<typename R %s_typenames%>
  struct functor_base<R(%types%)>{
    functor_case<R(%types%)>* h;
    R operator()(%params%) const{
      return this->h->call(%args%);
    }
  };
#pragma%end
#pragma%x mwg::functor::arities
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class functor
//------------------------------------------------------------------------------
  // 2016-03-25 gcc-2.95.3 bug work around:
  //   enable_if に複雑な式を指定すると ICE になる。
  template<typename F,typename S>
  struct is_explicit_functor:stdm::integral_constant<bool,(!is_functor<F,S>::value&&be_functor<F,S>::value)>{};

#pragma%m 1
  template<typename S>
  class functor_ref:public functor_base<S>{
    char buffer[3*sizeof(void*)];
  protected:
    functor_ref(){}
    template<typename F,typename Case>
    void init(const F& f){
#ifdef MWGCONF_STD_STATIC_ASSERT
      static_assert(sizeof(Case)<=sizeof(this->buffer),"sizeof(Case) too large");
#else
      static_assert(sizeof(Case)<=sizeof(mwg::declval<functor_ref>().buffer),"sizeof(Case) too large");
#endif
      this->h=new(this->buffer) Case(f);
    }
  public:
    template<typename F>
    functor_ref(const F& f,typename stdm::enable_if<is_functor<F,S>::value,mwg::invalid_type*>::type=nullptr){
      this->init<F,functor_case_impl<S,typename functor_traits<F>::ref_tr> >(f);
    }
    template<typename F>
    explicit functor_ref(const F& f,typename stdm::enable_if<is_explicit_functor<F,S>::value,mwg::invalid_type*>::type=0){
      this->init<F,functor_case_impl<S,typename functor_traits<F,S>::ref_tr> >(f);
    }
    ~functor_ref(){this->free();}

    void swap(functor_ref& right){
      std::swap(this->h,right.h);
      char temp[sizeof this->buffer];
      std::memcpy(temp,        this->buffer,sizeof this->buffer);
      std::memcpy(this->buffer,right.buffer,sizeof this->buffer);
      std::memcpy(right.buffer,temp,        sizeof this->buffer);
    }
    functor_ref(const functor_ref& f){
      this->h=f.h->placement_clone(this->buffer);
    }
    functor_ref& operator=(const functor_ref& f){
      this->free();
      this->h=f.h->placement_clone(this->buffer);
      return *this;
    }
    template<typename F>
    typename stdm::enable_if<is_explicit_functor<F,S>::value,functor_ref&>::type
    operator=(const F& f){
      this->free();
      this->init<F,functor_case_impl<S,typename functor_traits<F,S>::ref_tr> >(f);
      return *this;
    }
#ifdef MWGCONF_STD_RVALUE_REFERENCES
    functor_ref(functor_ref&& f){
      if(this==&f)return;
      this->h=nullptr;
      this->swap(f);
    }
    functor_ref& operator=(functor_ref&& f){
      if(this==&f)return *this;
      this->swap(f);
      return *this;
    }
#endif
  private:
    void free(){
      if(this->h){
        this->h->~functor_case<S>();
        this->h=nullptr;
      }
    }
  };
  template<typename S>
  class vfunctor_ref:public functor_ref<S>{
  public:
    vfunctor_ref() mwg_std_deleted;
    template<typename F>
    vfunctor_ref(const F& f,typename stdm::enable_if<is_functor<F,S>::value,mwg::invalid_type*>::type=nullptr){
      this->template init<F,functor_case_impl<S,typename functor_traits<F>::ref_tr> >(f);
    }
    template<typename F>
    vfunctor_ref(const F& f,typename stdm::enable_if<!is_functor<F,S>::value&&be_functor<F,S>::value,mwg::invalid_type*>::type=nullptr){
      this->template init<F,functor_case_impl<S,typename functor_traits<F,S>::ref_tr> >(f);
    }
  };
#pragma%end
#pragma%x 1
#pragma%x 1.r#functor_ref#functor#.r#ref_tr#ins_tr#
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
} // end of functor_detail
  using functor_detail::functor_invoke;
} // end of mwg
//EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
/*?lwiki
 * &pre(!cpp){
 * template<typename F>
 * typename mwg::stdm::enable_if<mwg::is_functor<bool(),F>::value,void>::type
 * test1(const F& f){
 *   puts(mwg::functor_traits<F>::invoke(f)?"O":"X");
 * }
 * template<typename F>
 * typename mwg::stdm::enable_if<mwg::be_functor<bool(),F>::value,void>::type
 * test2(const F& f){
 *   puts(mwg::functor_traits<F,bool()>::invoke(f)?"O":"X");
 * }
 * }
 */
//EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
#endif
#pragma%x begin_check

//------------------------------------------------------------------------------
// for debug

#ifdef MWG_FUNCTOR_H__VariantFunctorEnabled

#ifdef mwg_concept_is_valid_expression
template<typename F>
mwg_concept_is_valid_expression(has_single_operator_functor,F,F_,((F_*)0)->operator());
#elif defined(mwg_concept_is_valid_expression_vc2010A)
template<typename F>
mwg_concept_is_valid_expression_vc2010A(has_single_operator_functor,F,F_,((F_*)0)->operator());
#else
template<typename F>
struct has_single_operator_functor:mwg::stdm::false_type{};
#define mwgconf_mwg_functor_nosupport
#endif

template<typename P>
struct is_pointer_to_single_operator_functor{
#ifdef mwg_concept_is_valid_expression
  mwg_concept_is_valid_expression(c1_1,P,P_,((P_*)0)->operator*());
#elif defined(mwg_concept_is_valid_expression_vc2010A)
  mwg_concept_is_valid_expression_vc2010A(c1_1,P,P_,((P_*)0)->operator*());
#else
  struct c1_1:mwg::stdm::false_type{};
#endif

  struct c1:mwg::stdm::integral_constant<bool,c1_1::value||mwg::stdm::is_pointer<P>::value>{};

  template<typename T> static T expr();

  template<typename P_,bool B> struct c2:mwg::stdm::false_type{
    typedef mwg::unknown_type operator_type;
  };
  template<typename P_> struct c2<P_,true>{
    typedef typename std::remove_reference<decltype(*expr<P_>())>::type functor_type;
    static const bool value=has_single_operator_functor<functor_type>::value;
    typedef decltype(&functor_type::operator()) operator_type;
  };

  mwg_concept_condition(c2<P,c1::value>::value);
  typedef typename c2<P,c1::value>::operator_type operator_type;
};

#endif

//-----------------------------------------------------------------------------
void debug_support_member_function_pointer(){
  mwg_check( mwg::stdm::is_member_function_pointer<void(C::*)() const>::value);
  mwg_check(!mwg::stdm::is_member_function_pointer<void(const C&)>::value);

  // mwg_check(mwg::functor_detail::signature_mfp<void(C::*)()>::is_mfp==true);
  // mwg_check(mwg::functor_detail::signature_mfp<int(C::*)()>::is_mfp==true);
  // mwg_check(mwg::functor_detail::signature_mfp<int(C::*)(int)>::is_mfp==true);

  mwg_check(( mwg::functor_detail::is_memfun_pointer<void(C::*)() const>::value));
  mwg_check((!mwg::functor_detail::is_memfun_pointer<void(const C&)>::value));
  mwg_check(( mwg::stdm::is_same<void(const C&),mwg::functor_detail::is_memfun_pointer<void(C::*)() const>::functor_sgn>::value));
  mwg_check(( mwg::is_functor<void(C::*)() const,void(const C&)>::value));
  mwg_check(( mwg::be_functor<void(C::*)() const,void(const C&)>::value));
}

void debug_support_functor_object(){
#ifdef MWG_FUNCTOR_H__VariantFunctorEnabled
  mwg_check(!(is_pointer_to_single_operator_functor<F>::c1_1::value));
  mwg_check(!(is_pointer_to_single_operator_functor<F>::c1::value));
  mwg_check(!(is_pointer_to_single_operator_functor<F>::value));
  mwg_check(!(mwg::functor_detail::is_pointer_to_single_operator_functor<F>::value));

  mwg_check( (mwg::functor_traits<F>::is_functor));
#else
  mwg_check(!(mwg::functor_detail::is_pointer_to_single_operator_functor<F>::value));

  mwg_check(!(mwg::functor_traits<F>::is_functor));
  mwg_check( (mwg::functor_detail::can_be_called_as<F,int(int)>::value));
  //mwg_check( (mwg::functor_detail::functor_traits_chain2<6,F,int(int)>::is_functor));
  mwg_check( (mwg::functor_detail::functor_traits_signature<int(int)>::is_functor));
  mwg_check( (mwg::functor_traits<F,int(int)>::is_functor));
#endif

#if 0
  // debug
  typedef mwg::functor_detail::functor_case_traits_FunctorRef<F> case_tr;
  typedef mwg::functor_detail::functor_case_impl<void(int),case_tr> case_t;
  mwg_check((mwg::stdm::is_same<case_tr::case_data,const F*>::value));
  mwg_check((mwg::stdm::is_same<case_t::base,mwg::functor_detail::functor_case_data<void(int),const F*,true> >::value));
  //case_t::base x(reinterpret_cast<const F*>(0));
#endif
}

//-----------------------------------------------------------------------------

void check_function_pointer(){
  // 1. function pointer
  mwg::functor<int()> f1(&func1);
  mwg_check(f1()==func1());
  f1=func1;
  mwg_check(f1()==func1());
}

void check_member_function_pointer(){
  C instance(123);
  mwg::functor<int(const C&)> f2(&C::getValue);
  mwg_check(f2(instance)==123);
  mwg::functor<void(C&)> f2_2(&C::getValue);
  f2_2(instance);

  //mwg::functor<void(const C&)> f2(&C::print);f2(C(123));
  //mwg::functor<void(C&)> f2_2(&C::print);
}

void check_functor_object(){
  F functor(5);
  mwg::functor<int(int)> f3(functor);
  mwg_check(f3(4)==functor(4));
  mwg::functor_ref<int(int)> f3_1(functor);
  mwg_check(f3_1(4)==functor(4));
}

void check_member_object_pointer(){
  mwg::functor<int&(Str&)> f4(&Str::x);
  Str hoge={2011,2012};
  mwg_check(f4(hoge)==hoge.x);
  mwg::functor<const int&(const Str&)> f4_1(&Str::x);
  mwg_check(f4_1(hoge)==2011);
  mwg::functor<int(const Str&)> f4_2(&Str::x);
  mwg_check(f4_2(hoge)==2011);
  mwg_check( (mwg::stdm::is_convertible<const int&,int>::value));
}

void check_variance(){
  {
    mwg::functor<int(int,int)> g1(&func1);
    mwg_check(g1(1,2)==func1());
    g1=func1;
    mwg_check(g1(1,2)==func1());
  }

  {
    mwg::functor<void(char*,const char*,int)> g2(sprintf);
    char buff[100];
    g2(buff,"hello! %dth world!\n",4);
    mwg_check(std::strcmp(buff,"hello! 4th world!\n")==0);
  }

#ifdef MWG_FUNCTOR_H__VariantFunctorEnabled
  mwg_check( (mwg::functor_detail::can_be_called_as<F,int(int)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F,int(short)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F,int(char)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F,int(float)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F,int(double)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F,int*(int)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F,int()>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F,int(int,int)>::value));

  mwg_check(!(mwg::functor_detail::can_be_called_as<F2,void()>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2,void(int)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2,void(int,int)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2,void(int,int,int)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2,void(int,int,int,int)>::value));
#else
  mwg_check( (mwg::functor_detail::can_be_called_as<F,int(int)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F,int(short)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F,int(char)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F,int(float)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F,int(double)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F,int*(int)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F,void()>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F,int(int,int)>::value));

  mwg_check(!(mwg::functor_detail::can_be_called_as<F2,int()>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2,int(int)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2,int(int,int)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2,int(int,int,int)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2,int(int,int,int,int)>::value));
#endif

  mwg::functor<int(int,int,int)> g3=mwg::functor<int(int,int,int)>(F2());
  mwg_check( (g3(1,2,3)==6));
}

int main(){
  typedef mwg::functor_traits<void(C::*)()const> mfp_ftr;

  // sizeof(functor_case)
  std::fprintf(stderr,"  sizeof(functor_case)=%zd sizeof(ins_t)=%zd\n",
    sizeof(mwg::functor_detail::functor_case<int()>),
    sizeof(mwg::functor_detail::functor_case_impl<mfp_ftr::sgn_t,mfp_ftr::ins_tr>)
  );

  check_can_be_called_as();

  //----------------------------------------------------------------------------
  check_function_pointer();

  debug_support_member_function_pointer();
  check_member_function_pointer();

  debug_support_functor_object();
  check_functor_object();

  check_member_object_pointer();

  //----------------------------------------------------------------------------
  check_variance();

  return 0;
}
#pragma%x end_check
