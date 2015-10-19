// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef MWG_STDM_TUPLE__NONVARIADIC_TUPLE_INL
#define MWG_STDM_TUPLE__NONVARIADIC_TUPLE_INL
#include <cstddef>
#include <mwg/defs.h>
#include <mwg/std/type_traits>
#include <mwg/std/utility>
#pragma%expand
namespace mwg{
namespace stdm{
namespace detail{
  template<std::size_t I,typename R,typename TT>
  struct tuple_get_impl;

#pragma%if MWGCONF_STD_VARIADIC
  template<typename... Ts>
  struct tuple_is_default_constructible;
#pragma%else
  template<$".for/K/0/ArN/typename TK=void/,">
  struct tuple_is_default_constructible
    :mwg::stdm::integral_constant<bool,mwg::stdm::is_default_constructible<T0>::value&&tuple_is_default_constructible<$".for/K/1/ArN/TK/,">::value>{};

  template<>
  struct tuple_is_default_constructible<>:mwg::stdm::true_type{};

#pragma%end
}
#pragma%if MWGCONF_STD_VARIADIC
  template<typename... Ts>
  class tuple;
#pragma%else
  template<$".for/K/0/ArN/typename TK=void/,",bool IsDefaultConstructible=detail::tuple_is_default_constructible<$".for/K/0/ArN/TK/,">::value >
  class tuple;
#pragma%end

  static struct ignore_type{
    ignore_type(){}
    template<typename T> void operator=(T) const{}
  } const ignore;

//fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
  template<typename TT>
  class tuple_size{};
  template<typename TT>
  class tuple_size<TT const>:public tuple_size<TT>{};
  template<typename TT>
  class tuple_size<TT volatile>:public tuple_size<TT>{};
  template<typename TT>
  class tuple_size<TT const volatile>:public tuple_size<TT>{};
#pragma%if MWGCONF_STD_VARIADIC
  template<typename... Ts>
  class tuple_size<tuple<Ts...> >
    :public mwg::stdm::integral_constant<std::size_t,sizeof...(Ts)>{};
#pragma%else
  template<$".for:K:0:ArN:typename TK:,">
  class tuple_size<tuple<$".for:K:0:ArN:TK:,"> >
    :public mwg::stdm::integral_constant<std::size_t,1+tuple_size<tuple<$".for:K:1:ArN:TK:,"> >::value>{};
  template<>
  class tuple_size<tuple<> >
    :public mwg::stdm::integral_constant<std::size_t,0>{};
#pragma%end
  template<typename A0,typename A1>
  class tuple_size<stdm::pair<A0,A1> >
    :public mwg::stdm::integral_constant<std::size_t,2>{};

  namespace detail{
    template<typename TT>
    struct is_tuple_nocv:stdm::false_type{static const std::size_t size=0;};
#pragma%if MWGCONF_STD_VARIADIC
    template<typename... Ts>
    struct is_tuple_nocv<tuple<Ts...> >:stdm::true_type{
      static const std::size_t size=tuple_size<tuple<Ts...> >::value;
    };
#pragma%else
    template<$".for:K:0:ArN:typename TK:,">
    struct is_tuple_nocv<tuple<$".for:K:0:ArN:TK:,"> >:stdm::true_type{
      static const std::size_t size=tuple_size<tuple<$".for:K:0:ArN:TK:,"> >::value;
    };
#pragma%end
    template<typename TT>
    struct is_tuple:is_tuple_nocv<typename mwg::stdm::remove_cv<TT>::type>{};

    template<std::size_t I,typename TT>
    struct tuple_element_nocv:mwg::identity<void>{};
#pragma%if MWGCONF_STD_VARIADIC
    template<typename T0,typename... T1s>
    struct tuple_element_nocv<0,tuple<T0,T1s...> >
      :mwg::identity<T0>{};
    template<std::size_t I,typename T0,typename... T1s>
    struct tuple_element_nocv<I,tuple<T0,T1s...> >
      :tuple_element_nocv<I-1,tuple<T1s...> >{};
#pragma%else
    template<$".for:K:0:ArN:typename TK:,",bool B>
    struct tuple_element_nocv<0,tuple<$".for:K:0:ArN:TK:,",B> >
      :mwg::identity<T0>{};
    template<std::size_t I$".for:K:0:ArN:,typename TK:",bool B>
    struct tuple_element_nocv<I,tuple<$".for:K:0:ArN:TK:,",B> >
      :tuple_element_nocv<I-1,tuple<$".for:K:1:ArN:TK:,"> >{};
#pragma%end
    template<>
    struct tuple_element_nocv<0,tuple<> >:mwg::identity<void>{};
    template<std::size_t I>
    struct tuple_element_nocv<I,tuple<> >:mwg::identity<void>{};
  }

  template<std::size_t I,typename TT>
  class tuple_element:public stdm::enable_if<
    detail::is_tuple<TT>::value&&(I<detail::is_tuple<TT>::size),
    typename mwg::stdx::copy_cv<
      typename detail::tuple_element_nocv<I,typename mwg::stdm::remove_cv<TT>::type>::type,
      TT>::type>{};

  template<std::size_t I,typename A0,typename A1>
  class tuple_element<I,stdm::pair<A0,A1> >
    :public stdm::enable_if<I==0||I==1,typename stdm::conditional<I==0,A0,A1>::type>{};
  template<std::size_t I,typename A0,typename A1>
  class tuple_element<I,stdm::pair<A0,A1> const>
    :public stdm::enable_if<I==0||I==1,typename stdm::conditional<I==0,A0,A1>::type const>{};
  template<std::size_t I,typename A0,typename A1>
  class tuple_element<I,stdm::pair<A0,A1> volatile>
    :public stdm::enable_if<I==0||I==1,typename stdm::conditional<I==0,A0,A1>::type volatile>{};
  template<std::size_t I,typename A0,typename A1>
  class tuple_element<I,stdm::pair<A0,A1> const volatile>
    :public stdm::enable_if<I==0||I==1,typename stdm::conditional<I==0,A0,A1>::type const volatile>{};

  // traits
  /* you can define a specialization for your type */
  template<typename TT,typename Alloc>
  struct uses_allocator{};
#pragma%if MWGCONF_STD_VARIADIC
  template<typename Alloc,typename... Ts>
  struct uses_allocator<tuple<Ts...>,Alloc>:mwg::stdm::true_type{};
#pragma%else
  template<typename Alloc$".for:K:0:ArN:,typename TK:">
  struct uses_allocator<tuple<$".for:K:0:ArN:TK:,">,Alloc>:mwg::stdm::true_type{};
#pragma%end

//fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// get

namespace tuple_detail{
  template<typename T>
  struct element_traits{
    typedef T stored_type;
  };

#if defined(_MSC_VER)
# define MWG_STD_TUPLE_NONVARIADIC_TUPLE_HPP__NonTrivialElementTraits
  //
  // REQUIRE: T* stored_type::operator&() const;
  //
  //   The stored_type should define operator& to get an address.
  //   If T is a reference, operator& returns the address of target.
  //   If T is a non-reference type,
  //   operator& returns the address of the tuple element.
  //
  // REQUIRE: stored_type& stored_type::operator=(...);
  //
  // 2015-09-22 vcbug workaround
  //
  //   VC10 では参照変数を初期化しようとすると
  //   勝手に一時オブジェクトへのアドレスに変わってしまう。
  //

  template<typename T,typename U> struct assign_enabler
    :stdm::integral_constant<bool,
      stdm::is_convertible<U,T>::value&&!stdm::is_const<T>::value
      ||stdm::is_same<stdm::ignore_type,typename stdm::remove_cv<T>::type>::value>{};
  // :stdm::integral_constant<bool,stdm::is_assignable<T,U>::value>{};
  // ■(上記)is_assignable が未だ実装されていないので
  //   is_convertible, is_const で代わりに判定している。

  template<typename T>
  struct lvalue_reference_wrapper{
    T* lvalue;
    lvalue_reference_wrapper(T& _value):lvalue(&_value){}

    // operator T&&() const{return static_cast<T&&>(*this->lvalue);}
    // T&& get() const{return static_cast<T&&>(*this->lvalue);}
    T* operator&() const{return this->lvalue;}

    template<typename U>
    typename stdm::enable_if<assign_enabler<T,U>::value,lvalue_reference_wrapper&>::type
    operator=(U mwg_forward_rvalue value){*this->lvalue=stdm::forward<U>(value);return *this;}
  };
  template<typename T>
  struct element_traits<T&>{
    typedef lvalue_reference_wrapper<T> stored_type;
  };

# if defined(MWGCONF_STD_RVALUE_REFERENCES)
  template<typename T>
  struct rvalue_reference_wrapper{
    T* rvalue;
    rvalue_reference_wrapper(T& _value):rvalue(&_value){}
    rvalue_reference_wrapper(T&& _value):rvalue(&_value){}

    // operator T&&() const{return static_cast<T&&>(*this->rvalue);}
    // T&& get() const{return static_cast<T&&>(*this->rvalue);}
    T* operator&() const{return this->rvalue;}

    template<typename U>
    typename stdm::enable_if<assign_enabler<T,U>::value,rvalue_reference_wrapper&>::type
    operator=(U mwg_forward_rvalue value){*this->rvalue=stdm::forward<U>(value);return *this;}

  private:
    // 右辺値参照はコピー構築できない。その動作を真似る。
    rvalue_reference_wrapper(rvalue_reference_wrapper const&);
  };
  template<typename T>
  struct element_traits<T&&>{
    typedef rvalue_reference_wrapper<T> stored_type;
  };
# endif
#endif
}

namespace detail{

  template<typename ClassType,typename MemberType>
  struct runtime_member_type{
    typedef ClassType class_type;
    typedef MemberType member_type_declared;
    typedef typename stdm::remove_reference<class_type>::type class_type_noref;
    typedef typename stdx::copy_cv<member_type_declared,class_type_noref>::type member_type_cv;
    typedef typename stdx::copy_reference<member_type_cv,class_type>::type type;
  };

  // @tparam R
  //   get による戻り値の方を指定する。
  //   必ず右辺値参照または左辺値参照である。
  // @tparam TT
  //   const/参照 つきの tuple の型を指定する。
  template<std::size_t I,typename R,typename TT>
  struct tuple_get_impl{
    typedef typename stdm::remove_cv<typename stdm::remove_reference<TT>::type>::type tuple_type;
    typedef typename runtime_member_type<TT,typename tuple_type::rest_type>::type rest_argument_type;
    static R _get(TT t){return tuple_get_impl<I-1,R,rest_argument_type>::_get(t.m_rest);}
  };
  template<typename R,typename TT>
  struct tuple_get_impl<0,R,TT>{
    static R _get(TT t){
#ifdef MWG_STD_TUPLE_NONVARIADIC_TUPLE_HPP__NonTrivialElementTraits
      // 2015-09-22 (tuple_get_impl<K,R,TT>::_get): vcbug workaround
      //
      //   Visual Studio 2010 では cv qualifiers の変更などが伴うと、
      //   どうしても右辺値参照を勝手にローカルの一時オブジェクトに move する。
      //   すると戻り値の寿命が既に切れている状態になる。
      //   仕様がないので一旦ポインタに変換してキャストしてから返す。
      //
      // 2015-10-19: vcbug workaround
      //
      //   R が右辺値参照の時、
      //   一旦ローカル変数 (ref) に参照を代入してから右辺値参照にキャストしないと、
      //   その場に新しい一時オブジェクトが生成されてそのアドレスが返されてしまう。
      //
      typedef typename stdm::remove_reference<R>::type value_type;
      value_type& ref(*(value_type*)(&t.m_head));
      return static_cast<R>(ref);
#else
      return mwg::stdm::forward<R>(t.m_head);
#endif
    }
  };

  template<typename R,typename A0,typename A1>
  struct tuple_get_impl<0,R,pair<A0,A1>&>{
    static R _get(pair<A0,A1>& p){return reinterpret_cast<R>(p.first);}
  };
  template<typename R,typename A0,typename A1>
  struct tuple_get_impl<1,R,pair<A0,A1>&>{
    static R _get(pair<A0,A1>& p){return reinterpret_cast<R>(p.second);}
  };
  template<typename R,typename A0,typename A1>
  struct tuple_get_impl<0,R,pair<A0,A1> const&>{
    static R _get(pair<A0,A1> const& p){return reinterpret_cast<R>(p.first);}
  };
  template<typename R,typename A0,typename A1>
  struct tuple_get_impl<1,R,pair<A0,A1> const&>{
    static R _get(pair<A0,A1> const& p){return reinterpret_cast<R>(p.second);}
  };
#if defined(MWGCONF_STD_RVALUE_REFERENCES)
  template<typename R,typename A0,typename A1>
  struct tuple_get_impl<0,R,pair<A0,A1>&&>{
    static R _get(pair<A0,A1>&& p){return mwg::stdm::forward<R>(p.first);}
  };
  template<typename R,typename A0,typename A1>
  struct tuple_get_impl<1,R,pair<A0,A1>&&>{
    static R _get(pair<A0,A1>&& p){return mwg::stdm::forward<R>(p.second);}
  };
#endif
}
  template<std::size_t I,typename TT>
  typename stdx::add_const_reference<typename tuple_element<I,TT>::type>::type
  get(const TT& t){
    typedef const TT& argument_type;
    typedef typename stdx::add_const_reference<typename tuple_element<I,TT>::type>::type return_type;
    return detail::tuple_get_impl<I,return_type,argument_type>::_get(t);
  }
  template<std::size_t I,typename TT>
  typename stdm::enable_if<
    !stdm::is_const<TT>::value,
    typename add_lvalue_reference<typename tuple_element<I,TT>::type>::type>::type
  get(TT& t){
    typedef TT& argument_type;
    typedef typename add_lvalue_reference<typename tuple_element<I,TT>::type>::type return_type;
    return detail::tuple_get_impl<I,return_type,argument_type>::_get(t);
  }
#if defined(MWGCONF_STD_RVALUE_REFERENCES)
  template<std::size_t I,typename TT>
  typename add_rvalue_reference<typename tuple_element<I,TT>::type>::type
  get(TT&& t){
    typedef TT&& argument_type;
    typedef typename add_rvalue_reference<typename tuple_element<I,TT>::type>::type return_type;
    return detail::tuple_get_impl<I,return_type,argument_type>::_get(mwg::stdm::move(t));
  }
#endif

//ttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
// T: class tuple;
//-----------------------------------------------------------------------------
#pragma%if MWGCONF_STD_VARIADIC
# error "実装中: 現在は使用されていない。また、get など他の物も書き換える必要有り。"

  template<>
  class tuple<>{
    template<typename... Us> friend class tuple;
    template<std::size_t I,typename R,typename TT> friend struct detail::tuple_get_impl;
  public:
    explicit tuple(){}
    tuple& operator=(const tuple& other){return *this;}
    void swap(tuple& other){}
  };

  template<typename T0>
  class tuple<T0>{
    template<typename... Us> friend class tuple;
    //template<std::size_t I,typename R,typename TT> friend struct detail::tuple_get_impl;
    T0 m_data;
  public:
    explicit tuple(typename stdx::add_const_reference<T0>::type arg0)
      :m_data(arg0){}
  };

  template<typename T0,typename... T1s>
  class tuple<T0,T1s...>{
    template<typename... Us> friend class tuple;
    //template<std::size_t I,typename R,typename TT> friend struct detail::tuple_get_impl;
    T0 m_data;
    tuple<T1s...> m_list;
  public:
    template<typename U0,typename... U1s>
    explicit tuple(U0 mwg_forward_rvalue a0,U1s mwg_forward_rvalue... a1s)
      :m_data(stdm::forward<U0>(a0)),m_list(stdm::forward<U1s>(a1s)...){}

    // default constructor = default
    // copy constructor = default
    // move constructor = default
    // copy assignment = default
    // move assignment = default

    // TODO
    // explicit tuple(const tuple<other>&)
    // explicit tuple(const tuple<other>&&)
    // operator=(const tuple<other>&)
    // operator=(const tuple<other>&&)

    // template<typename UU>
    // explicit tuple(
    //   UU mwg_forward_rvalue t,
    //   typename stdm::enable_if<stdm::tuple_size<tuple>::value==stdm::tuple_size<UU>::value,int>::type=1
    // )
    //   :m_data(stdm::forward<*****>(t.m_data)),
    //    m_list(stdm::forward<*****>(t.m_list)){}

    // operator=(const tuple<...>&)
    // operator=(const pair&)

  };
#pragma%else

#pragma%m 1

namespace detail{
  struct sfinae_parameter{};
}

  template<>
  class tuple<>{
    template<typename... UK,bool IsDefaultConstructible2>
    friend class tuple;
    //template<std::size_t I,typename R,typename TT> friend struct detail::tuple_get_impl;
  public:
    explicit tuple(){}
    tuple& operator=(const tuple&){return *this;}
    void swap(tuple&){}
  };

  template<typename T0>
  class tuple<T0,void[0...-1],false>{
#pragma%m tupleContent
    template<$".for/K/0/ArN/typename UK/,",bool IsDefaultConstructible2> friend class tuple;
    template<std::size_t I,typename R,typename TT> friend struct detail::tuple_get_impl;

    typedef T0 head_type;
    typename tuple_detail::element_traits<T0>::stored_type m_head;
#pragma%%if has_rest
    typedef tuple<$".for/K/1/ArN/TK/,"> rest_type;
    rest_type m_rest;
#pragma%%end

    // template<std::size_t I,typename R,typename TT> friend struct detail::tuple_get_impl;

  public:
#pragma%%if has_default_ctor
    //
    // default constructor
    //
    tuple(){}
#pragma%%end

    //
    // memberwise construction
    //

    /* ※1 非テンプレートの memberwise construction があると
     *     is_copy_constructible でない場合にクラスごと死ぬ。
     *     従って以下は敢えて宣言しない。
     */
#pragma%%if !has_rest
    // explicit tuple(typename mwg::stdm::add_const_reference<T0>::type);
#pragma%%else
    // explicit tuple($".for/K/0/ArN/typename stdx::add_const_reference<TK>::type argK/,")
    //   :m_head(arg0),m_rest($".for/K/1/ArN/argK/,"){}
#pragma%%end

#pragma%%if !has_rest
    template<typename BK>
    explicit tuple(
      BK mwg_forward_rvalue arg,
      typename mwg::stdm::enable_if<!detail::is_tuple<BK>::value,detail::sfinae_parameter*>::type=0
    ):m_head(mwg::stdm::forward<BK>(arg)){}

#ifndef MWGCONF_STD_RVALUE_REFERENCES
    //
    // C++03 における完全転送が不完全なので参照版も用意する。
    // これがないと std::tie で tuple を構築できない。ただしこれでも完全ではない。
    //
    template<typename BK>
    explicit tuple(
      BK& arg,
      typename mwg::stdm::enable_if<!detail::is_tuple<BK>::value,detail::sfinae_parameter*>::type=0
    ):m_head(arg){}
#endif
#pragma%%else
    // 以下は引数の個数で SFINAE にかけている。
    // ■他の template constructor と曖昧にならない様に、
    // 他の template constructor の条件の否定を含める必要がある。
#pragma%%x
    template<$".for/K/0/%Ar%/typename BK/,">
    explicit tuple(
      $".for/K/0/%Ar%/BK mwg_forward_rvalue argK/,",
      typename mwg::stdm::enable_if<tuple_size<tuple<$".for/K/0/%Ar%/BK/,"> >::value==tuple_size<tuple>::value,detail::sfinae_parameter*>::type=0
    )
      :m_head(mwg::stdm::forward<B0>(arg0)),
       m_rest($".for/K/1/%Ar%/mwg::stdm::forward<BK>(argK)/,"){}
#ifndef MWGCONF_STD_RVALUE_REFERENCES
    template<$".for/K/0/%Ar%/typename BK/,">
    explicit tuple(
      $".for/K/0/%Ar%/BK& argK/,",
      typename mwg::stdm::enable_if<tuple_size<tuple<$".for/K/0/%Ar%/BK/,"> >::value==tuple_size<tuple>::value,detail::sfinae_parameter*>::type=0
    )
      :m_head(arg0),
       m_rest($".for/K/1/%Ar%/argK/,"){}
#endif
#pragma%%end.f/%Ar%/1/ArN+1/
#pragma%%end

    //
    // copy constructors
    //
#ifdef MWGCONF_STD_DEFAULTED_FUNCTIONS
    tuple(const tuple& other) = default;
#else
    tuple(const tuple& other)
      :m_head(other.m_head)
# pragma%%if has_rest
      ,m_rest(other.m_rest)
# pragma%%end
    {}
#endif
    template<typename TT>
    tuple(const TT& other,typename stdm::enable_if<tuple_size<TT>::value==tuple_size<tuple>::value,detail::sfinae_parameter*>::type=0)
      :m_head(get<0>(other))
#pragma%%if has_rest
      ,m_rest(other.m_rest)
#pragma%%end
    {}
#pragma%%if has_rest
    template<typename U0,typename U1>
    tuple(const pair<U0,U1>& other,typename stdm::enable_if<tuple_size<pair<U0,U1> >::value==tuple_size<tuple>::value,detail::sfinae_parameter*>::type=0)
      :m_head(other.first)
      ,m_rest(other.second){}
#pragma%%end

#ifdef MWGCONF_STD_RVALUE_REFERENCES
    //
    // move constructors
    //
# ifdef MWGCONF_STD_DEFAULTED_FUNCTIONS
    tuple(tuple&& other) = default;
# else
    tuple(tuple&& other)
      :m_head(mwg::stdm::move(other.m_head))
#  pragma%%if has_rest
      ,m_rest(mwg::stdm::move(other.m_rest))
#  pragma%%end
    {}
# endif
    template<typename TT>
    tuple(TT&& other,typename stdm::enable_if<tuple_size<TT>::value==tuple_size<tuple>::value,detail::sfinae_parameter*>::type=0)
      :m_head(get<0>(mwg::stdm::move(other)))
# pragma%%if has_rest
      ,m_rest(mwg::stdm::move(other.m_rest))
# pragma%%end
    {}
# pragma%%if has_rest
    template<typename U0,typename U1>
    tuple(pair<U0,U1>&& other,typename stdm::enable_if<tuple_size<pair<U0,U1> >::value==tuple_size<tuple>::value,detail::sfinae_parameter*>::type=0)
      :m_head(mwg::stdm::forward<U0>(other.first))
      ,m_rest(mwg::stdm::forward<U1>(other.second)){}
# pragma%%end
#endif

    // TODO: tuple(std::allocaltor_arg_t,Alloc,--same--)

    //
    // copy assignments
    //

    // operator=(const tuple&)
    tuple& operator=(const tuple& other){
      this->m_head=other.m_head;
#pragma%%if has_rest
      this->m_rest=other.m_rest;
#pragma%%end
      return *this;
    }
    // operator=(const tuple<...>&)
    template<typename TT>
    typename stdm::enable_if<tuple_size<TT>::value==tuple_size<tuple>::value,tuple&>::type
    operator=(const TT& other){
      this->m_head=get<0>(other);
#pragma%%if has_rest
      this->m_rest=other.m_rest;
#pragma%%end
      return *this;
    }
#pragma%%if has_rest
    // operator=(const pair&)
    template<typename A0,typename A1>
    typename stdm::enable_if<tuple_size<pair<A0,A1> >::value==tuple_size<tuple>::value,tuple&>::type
    operator=(const pair<A0,A1>& other){
      this->m_head=other.first;
      this->m_rest.m_head=other.second;
      return *this;
    }
#pragma%%end

#ifdef MWGCONF_STD_RVALUE_REFERENCES
    //
    // move assignments
    //

    // operator=(tuple&&)
    tuple& operator=(tuple&& other){
      this->m_head=stdm::forward<head_type>(other.m_head);
# pragma%%if has_rest
      this->m_rest=stdm::move(other.m_rest);
# pragma%%end
      return *this;
    }

    // operator=(tuple<...>&&)
    template<typename TT>
    typename stdm::enable_if<tuple_size<TT>::value==tuple_size<tuple>::value,tuple&>::type
    operator=(TT&& other){
      this->m_head=get<0>(stdm::move(other));
# pragma%%if has_rest
      this->m_rest=stdm::move(other.m_rest);
# pragma%%end
      return *this;
    }

# pragma%%if has_rest
    // operator=(pair&&)
    template<typename A0,typename A1>
    typename stdm::enable_if<tuple_size<pair<A0,A1> >::value==tuple_size<tuple>::value,tuple&>::type
    operator=(pair<A0,A1>&& other){
      this->m_head=stdm::forward<A0>(other.first);
      this->m_rest.m_head=stdm::forward<A1>(other.second);
      return *this;
    }
# pragma%%end

#endif

    void swap(tuple& other){
      using namespace std;
      swap(this->m_head,other.m_head);
#pragma%%if has_rest
      swap(this->m_rest,other.m_rest);
#pragma%%end
    }

    // 以下の関数群は非標準であるので対応しなくて良い。
    //   swap(tuple<...>&)
    //   swap(pair&)
    //   swap(tuple&&)
    //   swap(tuple<...>&&)
    //   swap(pair&&)
#pragma%end
#pragma%[has_rest=0,has_default_ctor=0]
#pragma%x tupleContent
  };

  // tuple<...,true> の実装:
  //   public tuple<T0,void[0...-1],false> から派生させて using するという手もある。
  //   しかし false 版にキャストできるというのも何か変である。
  template<typename T0>
  class tuple<T0,void[0...-1],true>{
#pragma%[has_rest=0,has_default_ctor=1]
#pragma%x tupleContent
  };

  template<typename... TK,bool IsDefaultConstructible>
  class tuple{
#pragma%[has_rest=1,has_default_ctor=0]
#pragma%x tupleContent
  };
  template<typename... TK>
  class tuple<TK...,true>{
#pragma%[has_rest=1,has_default_ctor=1]
#pragma%x tupleContent
  };
#pragma%end
#pragma%m 1 1.R|\ytypename\.\.\. (.)K\y|$".for/K/0/ArN/typename $1K/,"|
#pragma%m 1 1.R|\y(.)K\.\.\.|$".for/K/0/ArN/$1K/,"|
#pragma%m 1 1.R|void\[0\.\.\.-1\]|$".for/K/0/ArN-1/void/,"|
#pragma%x 1

#pragma%end
//fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// F: tuple<> tie();
//-----------------------------------------------------------------------------
  // make_tuple, forward_as_tuple
  inline tuple<> make_tuple(){return tuple<>();}
  inline tuple<> forward_as_tuple(){return tuple<>();}
#if defined(MWGCONF_STD_RVALUE_REFERENCES)
#pragma%expand
  template<$".for/K/0/%Ar%/typename TK/,">
  tuple<$".for/K/0/%Ar%/typename decay<TK>::type/,">
  make_tuple($".for/K/0/%Ar%/TK&& argK/,"){
    typedef tuple<$".for/K/0/%Ar%/typename decay<TK>::type/,"> return_type;
    // // vc2010bug workaround
    // //   一旦変数に入れてから初期化しないと変な値になる? ■
    // return_type result($".for/K/0/%Ar%/forward<TK>(argK)/,")
    // return result;
    return return_type($".for/K/0/%Ar%/forward<TK>(argK)/,");
  }
#pragma%end.f/%Ar%/1/ArN+1/
#ifdef _MSC_VER
  // vcbug workaround 2015-09-22
  //
  //   return 上で構築すると何故かコンストラクタに渡される右辺値参照が一時オブジェクトを指す。
  //   仕様がないので一旦 ret 上で初期化してその後で move する事にする。
  //
#pragma%expand
  template<$".for/K/0/%Ar%/typename TK/,">
  tuple<$".for/K/0/%Ar%/TK&&/,">
  forward_as_tuple($".for/K/0/%Ar%/TK&& argK/,"){
    typedef tuple<$".for/K/0/%Ar%/TK&&/,"> return_type;
    return_type ret($".for/K/0/%Ar%/forward<TK>(argK)/,");
    return move(ret);
  }
#pragma%end.f/%Ar%/1/ArN+1/
#else
#pragma%expand
  template<$".for/K/0/%Ar%/typename TK/,">
  tuple<$".for/K/0/%Ar%/TK&&/,">
  forward_as_tuple($".for/K/0/%Ar%/TK&& argK/,"){
    typedef tuple<$".for/K/0/%Ar%/TK&&/,"> return_type;
    return return_type($".for/K/0/%Ar%/forward<TK>(argK)/,");
  }
#pragma%end.f/%Ar%/1/ArN+1/
#endif
#else
#pragma%expand
  template<$".for/K/0/%Ar%/typename TK/,">
  tuple<$".for/K/0/%Ar%/typename decay<TK const&>::type/,"> make_tuple($".for/K/0/%Ar%/TK const& argK/,"){
    typedef tuple<$".for/K/0/%Ar%/typename decay<TK const&>::type/,"> return_type;
    return return_type($".for/K/0/%Ar%/argK/,");
  }
  template<$".for/K/0/%Ar%/typename TK/,">
  tuple<$".for/K/0/%Ar%/const TK&/,"> forward_as_tuple($".for/K/0/%Ar%/const TK& argK/,"){
    return tuple<$".for/K/0/%Ar%/const TK&/,">($".for/K/0/%Ar%/argK/,");
  }
#pragma%end.f/%Ar%/1/ArN+1/
#endif

  // tie
  inline tuple<> tie(){return tuple<>();}
#pragma%expand
  template<$".for/K/0/%Ar%/typename TK/,">
  tuple<$".for/K/0/%Ar%/TK&/,"> tie($".for/K/0/%Ar%/TK& argK/,"){
    return tuple<$".for/K/0/%Ar%/TK&/,">($".for/K/0/%Ar%/argK/,");
  }
#pragma%end.f/%Ar%/1/ArN+1/

//fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// operators
namespace detail{
  template<std::size_t I,std::size_t INMin>
  struct tuple_compare_impl{
    template<typename TTL,typename TTR>
    static bool operator_eq(const TTL& left,const TTR& right){
      return get<I>(left)==get<I>(right)&&tuple_compare_impl<I+1,INMin>::operator_eq(left,right);
    }
    template<typename TTL,typename TTR>
    static bool operator_le(const TTL& left,const TTR& right){
      return get<I>(left)<get<I>(right)||(get<I>(left)==get<I>(right)&&tuple_compare_impl<I+1,INMin>::operator_le(left,right));
    }
  };
  template<std::size_t INMin>
  struct tuple_compare_impl<INMin,INMin>{
    template<typename TTL,typename TTR>
    static bool operator_eq(const TTL& left,const TTR& right){
      mwg_unused(left);
      mwg_unused(right);
      return true;
    }
    template<typename TTL,typename TTR>
    static bool operator_le(const TTL& left,const TTR& right){
      mwg_unused(left);
      mwg_unused(right);
      return tuple_size<TTL>::value<tuple_size<TTR>::value;
    }
  };
}

  template<
    $".for/K/0/ArN/typename TLK/,",
    $".for/K/0/ArN/typename TRK/,"
  > bool operator==(const tuple<$".for/K/0/ArN/TLK/,">& left,const tuple<$".for/K/0/ArN/TRK/,">& right){
    typedef tuple_size<tuple<$".for/K/0/ArN/TLK/,"> > left_size;
    typedef tuple_size<tuple<$".for/K/0/ArN/TRK/,"> > right_size;
    typedef detail::tuple_compare_impl<0,left_size::value<right_size::value?left_size::value:right_size::value> comparer_type;
    return left_size::value==right_size::value&&comparer_type::operator_eq(left,right);
  }
  template<
    $".for/K/0/ArN/typename TLK/,",
    $".for/K/0/ArN/typename TRK/,"
  > bool operator!=(const tuple<$".for/K/0/ArN/TLK/,">& left,const tuple<$".for/K/0/ArN/TRK/,">& right){
    return !operator==(left,right);
  }
  template<
    $".for/K/0/ArN/typename TLK/,",
    $".for/K/0/ArN/typename TRK/,"
  > bool operator<(const tuple<$".for/K/0/ArN/TLK/,">& left,const tuple<$".for/K/0/ArN/TRK/,">& right){
    typedef tuple_size<tuple<$".for/K/0/ArN/TLK/,"> > left_size;
    typedef tuple_size<tuple<$".for/K/0/ArN/TRK/,"> > right_size;
    typedef detail::tuple_compare_impl<0,left_size::value<right_size::value?left_size::value:right_size::value> comparer_type;
    return comparer_type::operator_le(left,right);
  }
  template<
    $".for/K/0/ArN/typename TLK/,",
    $".for/K/0/ArN/typename TRK/,"
  > bool operator>(const tuple<$".for/K/0/ArN/TLK/,">& left,const tuple<$".for/K/0/ArN/TRK/,">& right){
    return operator<(right,left);
  }
  template<
    $".for/K/0/ArN/typename TLK/,",
    $".for/K/0/ArN/typename TRK/,"
  > bool operator<=(const tuple<$".for/K/0/ArN/TLK/,">& left,const tuple<$".for/K/0/ArN/TRK/,">& right){
    return !operator<(right,left);
  }
  template<
    $".for/K/0/ArN/typename TLK/,",
    $".for/K/0/ArN/typename TRK/,"
  > bool operator>=(const tuple<$".for/K/0/ArN/TLK/,">& left,const tuple<$".for/K/0/ArN/TRK/,">& right){
    return !operator<(left,right);
  }
//fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// tuple_cat

namespace detail{
  //---------------------------------------------------------------------------
  // tuple_unshift
  template<typename TT>
  struct tuple_unshift_type{};
  template<$".for/K/0/ArN/typename TK/,">
  struct tuple_unshift_type<tuple<$".for/K/0/ArN/TK/,"> >
    :mwg::identity<tuple<$".for/K/1/ArN/TK/,"> >{};
#pragma%expand
  template<$".for/K/0/%Ar%/typename TK/,">
  tuple<$".for/K/1/%Ar%/TK/,"> tuple_unshift(const tuple<$".for/K/0/%Ar%/TK/,">& tuplet){
    mwg_unused(tuplet); // Ar=1 の時には使われない
    return tuple<$".for/K/1/%Ar%/TK/,">($".for/K/1/%Ar%/get<K>(tuplet)/,");
  }
#pragma%end.f/%Ar%/1/ArN+1/
  template<typename A0,typename A1>
  struct tuple_unshift_type<pair<A0,A1> >
    :mwg::identity<tuple<A1> >{};
  template<typename A0,typename A1>
  tuple<A1> tuple_unshift(const pair<A0,A1>& p){return tuple<A1>(get<1>(p));}

  //---------------------------------------------------------------------------
  // tuple_push
  template<typename TT,typename T>
  struct tuple_push_type{};
#ifdef _MSC_VER
  // vc10bug workaround 2015-10-18
  //
#pragma%x
  template<$".for/K/0/%Ar%/typename TK,/"bool B>
  struct tuple_push_type<tuple<$".for/K/0/%Ar%-1/TK,/"$".for/K/%Ar%-1/ArN/void,/"B>,T$".eval:m=%Ar%-1">
    :mwg::identity<tuple<$".for/K/0/%Ar%/TK/,"> >{};
#pragma%end.f/%Ar%/1/ArN+1/
#else
#pragma%x
  template<$".for/K/0/%Ar%/typename TK/,">
  struct tuple_push_type<tuple<$".for/K/0/%Ar%-1/TK/,">,T$".eval:m=%Ar%-1">
    :mwg::identity<tuple<$".for/K/0/%Ar%/TK/,"> >{};
#pragma%end.f/%Ar%/1/ArN+1/
#endif
#pragma%x
  template<typename T$".eval:m=%Ar%-1"$".for/K/0/%Ar%-1/,typename TK/">
  tuple<$".for/K/0/%Ar%/TK/,"> tuple_push(const tuple<$".for/K/0/%Ar%-1/TK/,">& tuplet,T$"m" arg$"m"){
#pragma%%if %Ar%==1
    mwg_unused(tuplet); // Ar=1 の時には使われない
#pragma%%end
    return tuple<$".for/K/0/%Ar%/TK/,">($".for/K/0/%Ar%-1/get<K>(tuplet),/"arg$"m");
  }
#pragma%end.f/%Ar%/1/ArN+1/

  //---------------------------------------------------------------------------
  // tuple_cat2
  template<typename TTL,typename TTR>
  struct tuple_cat2_type
    :tuple_cat2_type<
      typename tuple_push_type<TTL,typename tuple_element<0,TTR>::type>::type,
      typename tuple_unshift_type<TTR>::type
    >{};
  template<typename TTL>
  struct tuple_cat2_type<TTL,tuple<> >:mwg::identity<TTL>{};
  template<typename TTL>
  const TTL& tuple_cat2(const TTL& left,const tuple<>& right){
    mwg_unused(right);
    return left;
  }
  template<typename TTL,typename TTR>
  typename tuple_cat2_type<TTL,TTR>::type tuple_cat2(const TTL& left,const TTR& right){
    return tuple_cat2(tuple_push<typename tuple_element<0,TTR>::type>(left,get<0>(right)),tuple_unshift(right));
  }

  //---------------------------------------------------------------------------
  // tuple_reference
  template<typename T> struct tuple_element_ref:stdx::add_const_reference<T>{};
  template<>           struct tuple_element_ref<void>:mwg::identity<void>{};
  template<$".for/K/0/ArN/typename AK/,">
  tuple<$".for/K/0/ArN/typename tuple_element_ref<AK>::type/,">
  tuple_reference(const tuple<$".for/K/0/ArN/AK/,">& tuplet){
    return tuple<$".for/K/0/ArN/typename tuple_element_ref<AK>::type/,">(tuplet);
  }
  template<typename A0,typename A1>
  tuple<typename tuple_element_ref<A0>::type,typename tuple_element_ref<A1>::type>
  tuple_reference(const pair<A0,A1>& p){
    return tuple<typename tuple_element_ref<A0>::type,typename tuple_element_ref<A1>::type>(p.first,p.second);
  }

  //---------------------------------------------------------------------------
  // tuple_cat
  template<$".for/K/0/ArN/typename TTK=void/,">
  struct tuple_cat_type
    :tuple_cat2_type<TT0,typename tuple_cat_type<$".for/K/1/ArN/TTK/,">::type>{};
  template<>
  struct tuple_cat_type<>:mwg::identity<tuple<> >{};
  inline tuple<> tuple_cat(){return tuple<>();}
#pragma%expand
  template<$".for/K/0/%Ar%/typename TTK/,">
  typename tuple_cat_type<$".for/K/0/%Ar%/TTK/,">::type tuple_cat($".for/K/0/%Ar%/const TTK& tupletK/,"){
    typedef typename tuple_cat_type<$".for/K/0/%Ar%/TTK/,">::type return_type;
    return return_type(tuple_cat2(tuple_cat($".for/K/0/%Ar%-1/tupletK/,"),tuple_reference(tuplet$".eval:%Ar%-1")));
  }
#pragma%end.f/%Ar%/1/ArN+1/
  //---------------------------------------------------------------------------
}
  using mwg::stdm::detail::tuple_cat;

  // TODO: tuple::constructor
  // TODO: tuple::operator=
}
}
namespace std{
  inline void swap(mwg::stdm::tuple<>& left,mwg::stdm::tuple<>& right){
    mwg_unused(left);
    mwg_unused(right);
  }
  template<$".for:K:0:ArN:typename TK:,">
  void swap(mwg::stdm::tuple<$".for:K:0:ArN:TK:,">& left,mwg::stdm::tuple<$".for:K:0:ArN:TK:,">& right){
    left.swap(right);
  }
}
#pragma%end.i
#endif
