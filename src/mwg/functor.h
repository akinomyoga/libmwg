// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef MWG_FUNCTOR_H
#define MWG_FUNCTOR_H
#include <new>
#include <algorithm>
#include <cstring>
#include <mwg/std/type_traits>
#include <mwg/concept.h>
#include "funcsig.h"
#include "functor.proto.h"
namespace mwg{

#%include "bits/functor/functor.variadic.pp"
namespace functor_detail{
#%include "bits/functor/functor.ftypes.pp"
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
//
//    Functor Traits
//
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  concept functor_traits
//------------------------------------------------------------------------------
#%(
  template<typename F>
  concept functor_traits{
    typedef F fct_t;
    static const bool is_functor;
    static const int arity;
    static const bool has_varargs;


    typedef auto ret_t;
    typedef auto sgn_t;
    typedef auto arg1_t;
    typedef auto arg2_t;
    typedef auto arg3_t;
    ...

    typedef auto ref_tr; // functor への参照を保持
    typedef auto ins_tr; // functor の複製インスタンスを保持
    static ret_t invoke(const fct_t& f,...){
      f(...);
    }
  };
#%)
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
    static const int arity=0;
    static const bool has_varargs=false;

    typedef void fct_t;
    typedef void sgn_t;
    typedef void ret_t;
#%define 1
    ${.for|K|1|ARITY_MAX+1|typedef void argK_t;|
    }
#%define end
#%expand 1.i
    typedef void ref_tr;
    typedef void ins_tr;
  };
#ifdef _MSC_VER /* VCBUG */
  template<typename S>
  struct functor_traits_signature{};
#endif
#%define 1
  template<typename R %s_typenames%>
  struct functor_traits_signature<R(%types%)>:functor_traits_empty{
    static const bool is_functor=true;
    static const int arity=%AR%;

    typedef R (sgn_t)(%types%);
    typedef R ret_t;
    ${.for|K|1|%AR%+1|typedef AK argK_t;|
    }
    #%(
    //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // requirements to be functor_traits:
    typedef fct_t;                        // raw functor type
    typedef ref_tr;                        // traits of mwg::functor_ref case
    typedef ins_tr;                        // traits of mwg::functor data case
    static R invoke(const fct_t&,%types%); // invoke functor object
    //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    #%)
  };
  template<typename R %s_typenames%>
  struct functor_traits_signature<R(%types...%)>
    :functor_traits_signature<R(%types%)>
  {
    static const bool has_varargs=true;
    typedef R (sgn_t)(%types...%);
  };
#%define end
#%expand mwg::functor::arities
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
  template<typename F,typename S,int L>
  struct functor_traits_switch:functor_traits_empty{};
  // L=1 [traits.functor]
  // L=2 [traits.pfunctor]
  // L=3 [traits.mfp]
  // L=4 [traits.mp]
  // L=5 [traits.vararg]
  // L=101: is_variant_signature
  // L=102: can_be_called_as_impl1 (overloaded functor)
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class functor_traits<R (*)(params)>
//  class functor_traits<R(params)>
//------------------------------------------------------------------------------
#%define 1
  template<typename R %s_typenames%>
  struct functor_traits<R(*)(%types%)>:functor_traits_impl<R(%types%),R(*)(%types%)>{
    typedef R(*fct_t)(%types%);
    static R invoke(fct_t f %s_params%,...){
      return R(f(%args%));
    }
  };
  template<typename R %s_typenames%>
  struct functor_traits<R(%types%)>:functor_traits<R(*)(%types%)>{
    typedef R (fct_t)(%types%);
  };
#%define end
#%expand mwg::functor::arities
//------------------------------------------------------------------------------
// [traits.mfp] R (C::*)(params)
// [traits.mfp] R (C::*)(params...) ■TODO
  template<typename Mfp>
  struct functor_invoker_mfp;
#%define 1
  template<typename R,typename C %s_typenames%>
  struct functor_invoker_mfp<R (C::*)(%types%) %const%>{
    static R invoke(R(C::*f)(%types%) %const%,%const% C& c %s_params%,...){
      return R((c.*f)(%args%));
    }
  };
#%define end
#%define 1 (
#%  expand 1.r#%const%##
#%  expand 1.r#%const%#const#
#%)
#%expand mwg::functor::arities.r|#ARITY_MAX+1#|#ARITY_MAX#|
  template<typename Mfp>
  struct functor_traits_switch<Mfp,void,3>
    :functor_traits_impl<typename is_memfun_pointer<Mfp>::functor_sgn,Mfp>
    ,functor_invoker_mfp<Mfp>
  {};
//------------------------------------------------------------------------------
// [traits.mp] T C::*
  template<typename T,typename C>
  struct functor_traits_switch<T C::*,void,4>:functor_traits_impl<T&(C&),T C::*>{
    typedef T C::*fct_t;
    static T& invoke(const fct_t& f,C& c,...){return c.*f;}
    static const T& invoke(const fct_t& f,const C& c,...){return c.*f;}
  };
  template<typename T,typename C,typename S>
  struct functor_traits_switch<T C::*,S,4>:functor_traits_switch<
    T C::*,S,
    is_variant_signature<functor_traits_signature<T&(C&)>,functor_traits<S*> >::value?101:
    is_variant_signature<functor_traits_signature<const T&(const C&)>,functor_traits<S*> >::value?101:
    0
  >{};
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class functor_traits<F /* having operator() */>
//------------------------------------------------------------------------------
#%(
  template<typename F,typename S,int L>
  struct functor_traits_switch;
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
#%)
  template<typename F,typename S>
  struct functor_invoker_functor;
  template<typename P,typename C,typename S>
  struct functor_invoker_pfunctor;
#ifdef _MSC_VER /* VCBUG */
  template<typename F,typename S>
  struct functor_invoker_functor{};
  template<typename P,typename C,typename S>
  struct functor_invoker_pfunctor{};
#endif
  template<typename C>
  struct functor_case_traits_FunctorRef:functor_case_traits<C>{
    typedef const C* case_data;
    static const C* endata(const C& ins){return &ins;}
    static const C& dedata(const C* p){return *p;}
  };
//-----------------------------------------------------------------------------
#%define 1
  template<typename R,typename C %s_typenames%>
  struct functor_invoker_functor<C,R(%types%)>{
    static R invoke(const C& f %s_params%,...){
      return R(const_cast<C&>(f)(%args%));
    }
  };
#%define end
#%expand mwg::functor::arities
#%define 1 (
  template<typename P,typename C,typename R %s_typenames%>
  struct functor_invoker_pfunctor<P,C,R(%types%)>{
    static R invoke(const P& f %s_params%,...){
      return R(const_cast<C&>(*f)(%args%));
    }
  };
#%)
#%expand mwg::functor::arities
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
      typename is_memfun_pointer<Mfp>::object_type,
      typename is_memfun_pointer<Mfp>::member_type
    >
  {};
  template<typename F>
  struct functor_traits_switch<F,void,1>
    :functor_traits_ftorF<decltype(&F::operator())>{};
//-----------------------------------------------------------------------------
// [traits.pfunctor] P with F P::operator*() / R F::operator()(%types%)
  template<typename P,typename SOp>
  struct functor_traits_ftorP
    :functor_traits_impl<typename is_memfun_pointer<SOp>::member_type,P>
    ,functor_invoker_pfunctor<
      P,
      typename is_memfun_pointer<SOp>::object_type,
      typename is_memfun_pointer<SOp>::member_type
    >
  {};
  template<typename P>
  struct functor_traits_switch<P,void,2>
    :functor_traits_ftorP<P,typename is_pointer_to_single_operator_functor<P>::operator_type>{};
//=============================================================================
  template<typename T>
  struct functor_traits<T>:functor_traits_switch<
    T,void,
    is_vararg_function<T>::value?5:
    is_vararg_function_pointer<T>::value?5:
    stdm::is_member_function_pointer<T>::value?3:
    stdm::is_member_object_pointer<T>::value?4:
    has_single_operator_functor<T>::value?1:
    is_pointer_to_single_operator_functor<T>::value?2:
    0
  >{};
#else
  template<typename T>
  struct functor_traits<T>:functor_traits_switch<
    T,void,
    is_vararg_function<T>::value?5:
    is_vararg_function_pointer<T>::value?5:
    stdm::is_member_function_pointer<T>::value?3:
    stdm::is_member_object_pointer<T>::value?4:
    //has_single_operator_functor<T>::value?1:
    //is_pointer_to_single_operator_functor<T>::value?2:
    0
  >{};
#endif
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  関数共変反変
//------------------------------------------------------------------------------
#%(
  template<typename F,typename S,int L>
  struct functor_traits_switch;
  template<typename F,typename S>
  struct functor_traits_switch<F,S,101>;
  template<typename F,typename S>
  struct functor_traits;
//------------------------------------------------------------------------------
#%)
  template<typename F,typename S>
  struct functor_traits_switch<F,S,101>:functor_traits<F>{typedef S sgn_t;};
  template<typename F,typename S>
  struct functor_traits_switch<F,S,102>
    :functor_traits_signature<typename can_be_called_as<F,S>::signature_type>
    ,functor_invoker_functor<F,typename can_be_called_as<F,S>::signature_type>
  {
    typedef F fct_t;
    struct ref_tr:functor_case_traits_FunctorRef<F>{
      typedef functor_traits_switch fct_tr;
    };
    struct ins_tr:functor_case_traits<F>{
      typedef functor_traits_switch fct_tr;
    };
  };

  template<typename F,typename S>
  struct functor_traits:functor_traits_switch<
    F,S,
    is_vararg_function<F>::value?5:
    is_vararg_function_pointer<F>::value?5:
    stdm::is_member_object_pointer<F>::value?4:
    is_variant_signature<functor_traits<F>,functor_traits<S*> >::value?101:
    can_be_called_as<F,S>::value?102:
    0
  >{};

#%include "bits/functor/functor.varargs.pp"

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class is_functor/be_functor
//------------------------------------------------------------------------------
  template<typename F,typename S>
  struct is_functor:stdm::is_same<S,typename mwg::functor_traits<F>::sgn_t>{};
  template<typename F,typename S>
  struct be_functor:stdm::integral_constant<bool,functor_traits<F,S>::is_functor>{};

#ifdef MWGCONF_STD_RVALUE_REFERENCES
//%define 1 (
  template<typename S,typename F %s_typenames%> typename stdm::enable_if<
    mwg::be_functor<F,S>::value,
    typename mwg::functor_traits<F,S>::ret_t
  >::type functor_invoke(const F& f $".for|K|1|%AR%+1|,AK&& aK|"){
    return mwg::functor_traits<F,S>::invoke(f $".for|K|1|%AR%+1|,stdm::forward<AK>(aK)|");
  }
//%)
//%expand mwg::functor::arities
#else
////%define 1 (
//  template<typename S,typename F> typename stdm::enable_if<
//    mwg::be_functor<F,S>::value,//&&mwg::funcsig::get_arity<S>::value==%AR%,
//    typename mwg::functor_traits<F,S>::ret_t
//  >::type functor_invoke(const F& f $".for|K|1|%AR%+1|
//    ,typename mwg::functor_traits<F,S>::argK_t aK|"
//  ){
//    return mwg::functor_traits<F,S>::invoke(f %s_args%);
//  }
////%)
////%expand mwg::functor::arities
//%define 1 (
  template<typename S,typename F %s_typenames%> typename stdm::enable_if<
    mwg::be_functor<F,S>::value,
    typename mwg::functor_traits<F,S>::ret_t
  >::type functor_invoke(const F& f $".for|K|1|%AR%+1|,AK& aK|"){
    return mwg::functor_traits<F,S>::invoke(f %s_args%);
  }
//%)
//%expand mwg::functor::arities
#endif

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
//
//    Functor Classes
//
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  interface functor_case
//------------------------------------------------------------------------------
#%define 1
  template<typename R %s_typenames%>
  struct functor_case<R(%types%)>{
    virtual R call(%types%) const=0;
    virtual ~functor_case(){}
    virtual functor_case* placement_clone(void* ptr) const=0;
  };
#%define end
#%expand mwg::functor::arities
  template<typename S,typename T,bool INTERIOR>
  class functor_case_data:public functor_case<S>{
    char buff[sizeof(T)];
  protected:
    functor_case_data(const T& value){new(buff) T(value);}
    ~functor_case_data(){get_ref().~T();}
#if defined(__GNUC__)&&MWGCONF_GCC_VER<40000
    const T& get_ref() const{return *(const T*)(this->buff);}
#else
    const T& get_ref() const{return *reinterpret_cast<const T*>(this->buff);}
#endif
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
#%define 1
  template<typename Tr,typename R %s_typenames%>
  class functor_case_impl<R(%types%),Tr>
    :public functor_case_data<R(%types%),typename Tr::case_data>
  {
  public:
    typedef Tr case_tr; // used by others
    typedef R (sgn_t)(%types%);
    typedef typename Tr::case_data case_data;
    typedef functor_case_data<sgn_t,case_data> base;
  public:
    functor_case_impl(const case_data& f):base(f){} // used by placement clone
    template<typename F> functor_case_impl(const F& f):base(Tr::endata(f)){}
    virtual R call(%params%) const{
      return R(Tr::fct_tr::invoke(Tr::dedata(this->get_ref()) %s_args%));
    }
    virtual functor_case_impl* placement_clone(void* ptr) const{
      return new(ptr) functor_case_impl(this->get_ref());
    }
  };
#%define end
#%expand mwg::functor::arities

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  interface functor_base
//------------------------------------------------------------------------------
#%define 1
  template<typename R %s_typenames%>
  struct functor_base<R(%types%)>{
    functor_case<R(%types%)>* h;
    R operator()(%params%) const{
      return this->h->call(%args%);
    }
  };
#%define end
#%expand mwg::functor::arities
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class functor
//------------------------------------------------------------------------------
#%define 1
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
    functor_ref(const F& f,mwg_requires((is_functor<F,S>::value),void*) =nullptr){
      this->init<F,functor_case_impl<S,typename functor_traits<F>::ref_tr> >(f);
    }
    template<typename F>
    explicit functor_ref(const F& f,mwg_requires((!is_functor<F,S>::value&&be_functor<F,S>::value),void*) =nullptr){
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
    template<typename F> mwg_requires((!is_functor<F,S>::value&&be_functor<F,S>::value),
    functor_ref&) operator=(const F& f){
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
    vfunctor_ref(const F& f,mwg_requires((is_functor<S,F>::value),void*) =nullptr){
      this->template init<F,functor_case_impl<S,typename functor_traits<F>::ref_tr> >(f);
    }
    template<typename F>
    vfunctor_ref(const F& f,mwg_requires((!is_functor<S,F>::value&&be_functor<S,F>::value),void*) =nullptr){
      this->template init<F,functor_case_impl<S,typename functor_traits<F,S>::ref_tr> >(f);
    }
  };
#%define end
#%expand 1
#%expand 1.r#functor_ref#functor#.r#ref_tr#ins_tr#
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
} // end of functor_detail
  using functor_detail::functor_invoke;
} // end of mwg
//EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
#if 0
  template<typename F> mwg_requires((mwg::is_functor<bool(),F>::value),
  void) test1(const F& f){
    puts(mwg::functor_traits<F>::invoke(f)?"O":"X");
  }
  template<typename F> mwg_requires((mwg::be_functor<bool(),F>::value),
  void) test2(const F& f){
    puts(mwg::functor_traits<F,bool()>::invoke(f)?"O":"X");
  }
#endif
//EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
#endif
