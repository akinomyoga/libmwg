// -*- mode:C++;coding:utf-8 -*-
#pragma once
#include <memory>
#include <algorithm>
#include <utility>
#include <mwg/defs.h>
#include "type_traits"
namespace mwg{
namespace stdm{
namespace detail{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  template<typename T>
  class default_delete;
  template<typename T,typename Del=default_delete<T> >
  class unique_ptr;
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  default_delete
//-----------------------------------------------------------------------------
  template<class T>
  struct default_delete{
    default_delete(){}
    template<class T2>
    default_delete(const default_delete<T2>&){}
    void operator()(T *p) const{
      if(0<sizeof(T))delete p;
    }
  };
  template<class T>
  struct default_delete<T[]>{
    default_delete(){}
    void operator()(T *p) const{
      if(0<sizeof(T))delete[] p;
    }
  private:
    template<class U>
    void operator()(U*) const mwg_std_deleted;
  };
//-----------------------------------------------------------------------------
  template<typename T>
  struct has_pointer_type{
    typedef char true_t [1];
    typedef char false_t[2];

    struct checker{
      template<typename X>
      static true_t & eval(X* a,typename X::pointer* dummy=0);
      static false_t& eval(...);
    };

    static const bool value=sizeof(true_t)==sizeof(checker::eval(*(T*)0));
  };

  template<typename P>
  struct hold_pointer{typedef P pointer;};

  template<typename T,typename D,bool DeriveFromD>
  class unique_ptr_base{
  protected:
    typedef typename remove_reference<D>::type D_noreference;
    typedef typename conditional<
      has_pointer_type<D_noreference>::value,
      D_noreference,
      hold_pointer<T*>
    >::type::pointer pointer;
    typedef D deleter_type;

    unique_ptr_base(pointer p,D d):ptr(p),del(d){}
    template<typename P2,typename D2>
    unique_ptr_base(P2 p,D2 d):ptr(p),del(d){}

  public:
    D_noreference& get_deleter(){return this->del;}
    const D_noreference& get_deleter() const{return this->del;}

  protected:
    pointer ptr;
    deleter_type del;
  };

  template<typename T,typename D>
  class unique_ptr_base<T,D,true>:public D{
  protected:
    typedef typename remove_reference<D>::type D_noreference;
    typedef typename conditional<
      has_pointer_type<D_noreference>::value,
      D_noreference,
      hold_pointer<T*>
    >::type::pointer pointer;

    unique_ptr_base(pointer p,D d):D(d),ptr(p){}
    template<typename P2,typename D2>
    unique_ptr_base(P2 p,D2 d):D(d),ptr(p){}

  public:
    D_noreference& get_deleter(){return *this;}
    const D_noreference& get_deleter() const{return *this;}

  protected:
    pointer ptr;
  };
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class unique_ptr<T,D>;
//-----------------------------------------------------------------------------
  template<typename T,typename D>
  class unique_ptr:public unique_ptr_base<
      T,D,is_empty<D>::value||is_same<default_delete<T>,D>::value
  >{
  public:
    typedef T element_type;
    typedef D deleter_type;
    typedef unique_ptr_base<
      T,D,is_empty<D>::value||is_same<default_delete<T>,D>::value
    > base;
    typedef typename base::pointer pointer;
  //---------------------------------------------------------------------------
  // 初期化
  public:
    unique_ptr():base(pointer(),D()){
      static_assert(!is_pointer<D>::value,"unique_ptr constructed with null deleter pointer");
    }
    unique_ptr(stdm::nullptr_t):base(pointer(),D()){
      static_assert(!is_pointer<D>::value,"unique_ptr constructed with null deleter pointer");
    }
    explicit unique_ptr(pointer p):base(p,D()){
      static_assert(!is_pointer<D>::value,"unique_ptr constructed with null deleter pointer");
    }
    unique_ptr(pointer p,typename stdm::add_lvalue_reference<D>::type d):base(p,d){
    }
    unique_ptr& operator=(stdm::nullptr_t){
      this->reset();
      return (*this);
    }
#ifdef MWGCONF_STD_RVALUE_REFERENCES
  public:
    unique_ptr(pointer p,typename remove_reference<D>::type&& d):base(p,stdm::move(d)){}
    unique_ptr(unique_ptr&& movee)
      :base(movee.release(),stdm::forward<D>(movee.get_deleter()))
    {}
    template<typename T2,typename D2>
    unique_ptr(unique_ptr<T2,D2>&& movee)
      :base(movee.release(),stdm::forward<D2>(movee.get_deleter()))
    {}
    template<typename T2,typename D2>
    unique_ptr& operator=(unique_ptr<T2,D2>&& movee){
      reset(movee.release());
      this->get_deleter()=stdm::move(movee.get_deleter());
      return *this;
    }
    unique_ptr& operator=(unique_ptr&& movee){
      if(this!=&movee){
        reset(movee.release());
        this->get_deleter()=stdm::move(movee.get_deleter());
      }
      return *this;
    }
  private:
    unique_ptr(const unique_ptr&) mwg_std_deleted;
    unique_ptr& operator=(const unique_ptr&) mwg_std_deleted;
    template<typename T2,typename D2>
    unique_ptr(const unique_ptr<T2,D2>&) mwg_std_deleted;
    template<typename T2,typename D2>
    unique_ptr& operator=(const unique_ptr<T2, D2>&) mwg_std_deleted;
#else
  public:
    // 不正なポインタ移譲を検知出来ない可能性有り。
    //unique_ptr(pointer p,typename remove_reference<D>::type const& d):base(p,d){}
    unique_ptr(unique_ptr const& movee)
      :base(const_cast<unique_ptr&>(movee).release(),movee.get_deleter())
    {}
    unique_ptr& operator=(unique_ptr const& movee){
      if(this!=&movee){
        reset(const_cast<unique_ptr&>(movee).release());
        this->get_deleter()=movee.get_deleter();
      }
      return *this;
    }

    template<typename T2,typename D2>
    unique_ptr(unique_ptr<T2,D2> const& movee)
      :base(const_cast<unique_ptr<T2,D2>&>(movee).release(),movee.get_deleter())
    {}
    template<typename T2,typename D2>
    unique_ptr& operator=(unique_ptr<T2,D2> const& movee){
      reset(const_cast<unique_ptr<T2,D2>&>(movee).release());
      this->get_deleter()=movee.get_deleter();
      return *this;
    }
#endif
  //---------------------------------------------------------------------------
  // 始末
  private:
    void internal_delete(){
      if (this->ptr!=pointer())
        this->get_deleter()(this->ptr);
    }
  public:
    ~unique_ptr(){
      this->internal_delete();
    }
  //---------------------------------------------------------------------------
  // 代入・交換
  public:
    void swap(unique_ptr& right){	// swap elements
      std::swap(this->ptr,right.ptr);
      std::swap(this->get_deleter(),right.get_deleter());
    }

#ifdef MWGCONF_STD_RVALUE_REFERENCES
    void swap(unique_ptr&& right){
      if(this!=&right){
        std::swap(this->ptr,right.ptr);
        std::swap(this->get_deleter(),right.get_deleter());
      }
    }
#endif
  //---------------------------------------------------------------------------
  // 設定
  public:
    pointer release(){
      pointer ret(this->ptr);
      this->ptr=pointer();
      return ret;
    }
    void reset(pointer p=pointer()){
      if(p!=this->ptr){
        this->internal_delete();
        this->ptr=p;
      }
    }
  //---------------------------------------------------------------------------
  // 取得
  public:
    typename stdm::add_lvalue_reference<T>::type operator*() const{return *this->ptr;}
    pointer operator->() const{return this->ptr;}
    pointer get() const{return this->ptr;}
    mwg_explicit_operator bool() const{return this->ptr!=pointer();}
  };
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class unique_ptr<T[N],D>;
//-----------------------------------------------------------------------------
  template<typename T,typename D>
  class unique_ptr<T[],D>
    :public unique_ptr_base<T,D,is_empty<D>::value||is_same<default_delete<T[]>,D>::value>
  {
  public:
    typedef unique_ptr_base<T,D,is_empty<D>::value||is_same<default_delete<T[]>,D>::value> base;
    typedef typename base::pointer pointer;
    typedef T element_type;
    typedef D deleter_type;
  //---------------------------------------------------------------------------
  // 初期化
  public:
    unique_ptr():base(pointer(),D()){
      static_assert(!is_pointer<D>::value,"unique_ptr constructed with null deleter pointer");
    }
    explicit unique_ptr(pointer p):base(p,D()){
      static_assert(!is_pointer<D>::value,"unique_ptr constructed with null deleter pointer");
    }
    unique_ptr(pointer p,typename stdm::add_lvalue_reference<D>::type d):base(p,d){}

    unique_ptr(stdm::nullptr_t):base(pointer(),D()){
      static_assert(!is_pointer<D>::value,"unique_ptr constructed with null deleter pointer");
    }
    unique_ptr& operator=(stdm::nullptr_t){
      this->reset();
      return *this;
    }
  private:
    template<typename P2> explicit unique_ptr(P2) mwg_std_deleted;
    template<typename P2,typename D2> unique_ptr(P2,D2) mwg_std_deleted;
    unique_ptr(const unique_ptr&) mwg_std_deleted;
    template<typename T2,typename D2> unique_ptr(const unique_ptr<T2,D2>&) mwg_std_deleted;
    unique_ptr& operator=(const unique_ptr&) mwg_std_deleted;
    template<typename T2,typename D2> unique_ptr& operator=(const unique_ptr<T2,D2>&) mwg_std_deleted;
#ifdef MWGCONF_STD_RVALUE_REFERENCES
  public:
    unique_ptr(
      pointer p,
      typename stdm::remove_reference<D>::type&& d
    ):base(p,stdm::move(d)){}
    unique_ptr(unique_ptr&& movee)
      :base(movee.release(),stdm::forward<D>(movee.get_deleter())){}
    unique_ptr& operator=(unique_ptr&& movee){	// assign by moving movee
      if(this!=&movee){
        this->reset(movee.release());
        this->get_deleter()=stdm::move(movee.get_deleter());
      }
      return *this;
    }
  private:
    template<typename T2,typename D2>
    unique_ptr(unique_ptr<T2,D2>&& movee) mwg_std_deleted;
    template<typename T2,typename D2>
    unique_ptr& operator=(unique_ptr<T2,D2>&& movee) mwg_std_deleted;
#endif
  //---------------------------------------------------------------------------
  // 始末
  public:
    ~unique_ptr(){
      this->internal_delete();
    }
  private:
    void internal_delete(){
      this->get_deleter()(this->ptr);
    }
  //---------------------------------------------------------------------------
  // 交換・設定
  public:
    void swap(unique_ptr& right){
      std::swap(this->ptr,right.ptr);
      std::swap(this->get_deleter(),right.get_deleter());
    }
#ifdef MWGCONF_STD_RVALUE_REFERENCES
    void swap(unique_ptr&& right){
      if(this!=&right){
        std::swap(this->ptr,right.ptr);
        std::swap(this->get_deleter(),right.get_deleter());
      }
    }
#endif

    void reset(stdm::nullptr_t){
      if(this->ptr!=0){
        this->internal_delete();
        this->ptr=0;
      }
    }
    void reset(pointer p=pointer()){
      if(p!=this->ptr){
        this->internal_delete();
        this->ptr=p;
      }
    }
  private:
    template<typename P2> void reset(P2) mwg_std_deleted;
  //---------------------------------------------------------------------------
  // 取得
  public:
    typename stdm::add_lvalue_reference<T>::type operator[](size_t index) const{return this->ptr[index];}
    pointer get() const{return this->ptr;}
    mwg_explicit_operator bool() const{return this->ptr!=0;}
    pointer release(){
      pointer ret(this->ptr);
      this->ptr=pointer();
      return ret;
    }
  };
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//  unique_ptr operators
//-----------------------------------------------------------------------------
  template<typename T,typename D>
  void swap(unique_ptr<T,D>& l,unique_ptr<T,D>& r){l.swap(r);}
#ifdef MWGCONF_STD_RVALUE_REFERENCES
  template<typename T,typename D>
  void swap(unique_ptr<T,D>& l,unique_ptr<T,D>&& r){l.swap(r);}
  template<typename T,typename D>
  void swap(unique_ptr<T,D>&& l,unique_ptr<T,D>& r){r.swap(l);}
#endif

#define MWG_TEMP_DEFINE_COMPARE(OP)                                       \
  template<typename T1,typename D1,typename T2,typename D2>               \
  bool operator OP(const unique_ptr<T1,D1>& l,const unique_ptr<T2,D2>& r){\
    return l.get() OP r.get();                                            \
  }                                                                       /**/
  MWG_TEMP_DEFINE_COMPARE(==)
  MWG_TEMP_DEFINE_COMPARE(!=)
  MWG_TEMP_DEFINE_COMPARE(<)
  MWG_TEMP_DEFINE_COMPARE(>)
  MWG_TEMP_DEFINE_COMPARE(<=)
  MWG_TEMP_DEFINE_COMPARE(>=)
#undef MWG_TEMP_DEFINE_COMPARE

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
} /* end of namespace detail */

  using mwg::stdm::detail::unique_ptr;
  using mwg::stdm::detail::default_delete;

} /* end of namespace stdm */
} /* end of namespace mwg */
namespace std{
  using mwg::stdm::detail::swap;
}
