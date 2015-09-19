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
}
#pragma%if MWGCONF_STD_VARIADIC
  template<typename... Ts>
  class tuple;
#pragma%else
  template<$".for/K/0/ArN/typename TK=void/,">
  class tuple;
#pragma%end

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
    struct is_tuple_nocv:stdm::false_type{static const int size=0;};
#pragma%if MWGCONF_STD_VARIADIC
    template<typename... Ts>
    struct is_tuple_nocv<tuple<Ts...> >:stdm::true_type{
      static const int size=tuple_size<tuple<Ts...> >::value;
    };
#pragma%else
    template<$".for:K:0:ArN:typename TK:,">
    struct is_tuple_nocv<tuple<$".for:K:0:ArN:TK:,"> >:stdm::true_type{
      static const int size=tuple_size<tuple<$".for:K:0:ArN:TK:,"> >::value;
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
    template<$".for:K:0:ArN:typename TK:,">
    struct tuple_element_nocv<0,tuple<$".for:K:0:ArN:TK:,"> >
      :mwg::identity<T0>{};
    template<std::size_t I$".for:K:0:ArN:,typename TK:">
    struct tuple_element_nocv<I,tuple<$".for:K:0:ArN:TK:,"> >
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

namespace detail{
  // template<std::size_t I,typename R,typename TT> struct tuple_get_impl;
  //   R には右辺値参照または左辺値参照が指定される。
  template<std::size_t I,typename R,typename TT> struct tuple_get_impl{};
#pragma%expand
  template<typename R,typename TT>
  struct tuple_get_impl<K,R,TT>{
    static R _get(TT t){return mwg::stdm::forward<R>(t.m_valueK);}
  };
#pragma%end.f/K/0/ArN/
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
  typename add_lvalue_reference<
    typename add_const<typename tuple_element<I,TT>::type>::type
    >::type
  get(const TT& t){
    typedef const TT& argument_type;
    typedef typename add_lvalue_reference<
      typename add_const<typename tuple_element<I,TT>::type>::type
      >::type return_type;
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
  template<>
  class tuple<>{
    template<typename... Us> friend class tuple;
    //template<std::size_t I,typename R,typename TT> friend struct detail::tuple_get_impl;
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
    explicit tuple(typename stdx::add_cref<T0>::type arg0)
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
  template<>
  class tuple<>{
    template<$".for/K/0/ArN/typename UK/,"> friend class tuple;
    template<std::size_t I,typename R,typename TT> friend struct detail::tuple_get_impl;
  public:
    explicit tuple(){}
    tuple& operator=(const tuple& other){mwg_unused(other);return *this;}
    void swap(tuple& other){mwg_unused(other);}
  };
#pragma%end
#pragma%expand
  template<$".for/K/0/%Ar%/typename TK/,">
  class tuple<$".for/K/0/%Ar%/TK/,">{$".for/K/0/%Ar%/
    TK m_valueK;/"
    template<$".for/K/0/ArN/typename UK/,"> friend class tuple;
    template<std::size_t I,typename R,typename TT> friend struct detail::tuple_get_impl;
  public:
    explicit tuple($".for/K/0/%Ar%/typename stdx::add_cref<TK>::type argK/,")
      :$".for/K/0/%Ar%/m_valueK(argK)/,"{}
    template<$".for/K/0/%Ar%/typename BK/,">
    explicit tuple($".for/K/0/%Ar%/BK mwg_forward_rvalue argK/,")
      :$".for/K/0/%Ar%/m_valueK(mwg::stdm::forward<BK>(argK))/,"{}
    // tuple(const tuple& other) mwg_std_default;
    template<$".for/K/0/%Ar%/typename BK/,">
    tuple(const tuple<$".for/K/0/%Ar%/BK/,">& tuplet)
      :$".for/K/0/%Ar%/m_valueK(get<K>(tuplet))/,"{}
#ifdef MWGCONF_STD_RVALUE_REFERENCES
    // tuple(tuple&& other) mwg_std_default;
    template<$".for/K/0/%Ar%/typename BK/,">
    tuple(tuple<$".for/K/0/%Ar%/BK/,">&& tuplet)
      :$".for/K/0/%Ar%/m_valueK(get<K>(mwg::stdm::move(tuplet)))/,"{}
#endif
    // TODO: tuple()
    // TODO: tuple(const pair&)
    // TODO: tuple(pair&&)
    // TODO: tuple(std::allocaltor_arg_t,Alloc,--same--)

    // operator=(const tuple&)
    tuple& operator=(const tuple& other){$".for/K/0/%Ar%/
      this->m_valueK=other.m_valueK;/"
      return *this;
    }
    // operator=(const tuple<...>&)
    template<typename Tpl2>
    typename stdm::enable_if<tuple_size<Tpl2>::value==tuple_size<tuple>::value,tuple&>::type
    operator=(const Tpl2& other){$".for/K/0/%Ar%/
      this->m_valueK=other.m_valueK;/"
      return *this;
    }
    // operator=(const pair&)
    template<typename A0,typename A1>
    typename stdm::enable_if<tuple_size<pair<A0,A1> >::value==tuple_size<tuple>::value,tuple&>::type
    operator=(const pair<A0,A1>& other){
      this->m_value0=other.first;
      this->m_value1=other.second;
      return *this;
    }
#ifdef MWGCONF_STD_RVALUE_REFERENCES
    // operator=(tuple&&)
    tuple& operator=(tuple&& other){$".for/K/0/%Ar%/
      this->m_valueK=stdm::move(other.m_valueK);/"
      return *this;
    }
    // operator=(tuple<...>&&)
    template<typename Tpl2>
    typename stdm::enable_if<tuple_size<Tpl2>::value==tuple_size<tuple>::value,tuple&>::type
    operator=(Tpl2&& other){$".for/K/0/%Ar%/
      this->m_valueK=stdm::move(other.m_valueK);/"
      return *this;
    }
    // operator=(pair&&)
    template<typename A0,typename A1>
    typename stdm::enable_if<tuple_size<pair<A0,A1> >::value==tuple_size<tuple>::value,tuple&>::type
    operator=(pair<A0,A1>&& other){
      this->m_value0=stdm::move(other.first);
      this->m_value1=stdm::move(other.second);
      return *this;
    }
#endif

    void swap(tuple& other){$".for/K/0/%Ar%/
      std::swap(this->m_valueK,other.m_valueK);/"
    }
    // TODO: swap(tuple<...>&)
    // TODO: swap(pair&)
    // TODO: swap(tuple&&)
    // TODO: swap(tuple<...>&&)
    // TODO: swap(pair&&)
  };
#pragma%end.f/%Ar%/1/ArN/
  // TODO: copy above and modify
  template<$".for/K/0/ArN/typename TK/,">
  class tuple{$".for/K/0/ArN/
    TK m_valueK;/"
    template<$".for/K/0/ArN/typename UK/,"> friend class tuple;
    template<std::size_t I,typename R,typename TT> friend struct detail::tuple_get_impl;
  public:
    tuple($".for/K/0/ArN/const TK& argK/,")
      :$".for/K/0/ArN/m_valueK(argK)/,"{}
    tuple& operator=(const tuple& other){$".for/K/0/ArN/
      this->m_valueK=other.m_valueK;/"
      return *this;
    }
    void swap(tuple& other){
      using namespace std;$".for/K/0/ArN/
      swap(this->m_valueK,other.m_valueK);/"
    }
  };
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
    return return_type($".for/K/0/%Ar%/forward<TK>(argK)/,");
  }
  template<$".for/K/0/%Ar%/typename TK/,">
  tuple<$".for/K/0/%Ar%/TK&&/,">
  forward_as_tuple($".for/K/0/%Ar%/TK&& argK/,"){
    typedef tuple<$".for/K/0/%Ar%/TK&&/,"> return_type;
    return return_type($".for/K/0/%Ar%/forward<TK>(argK)/,");
  }
#pragma%end.f/%Ar%/1/ArN+1/
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

  static struct ignore_type{
    ignore_type(){}
    template<typename T> void operator=(T) const{}
  } const ignore;

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
#pragma%expand
  template<$".for/K/0/%Ar%/typename TK/,">
  struct tuple_push_type<tuple<$".for/K/0/%Ar%-1/TK/,">,T$".eval:m=%Ar%-1">
    :mwg::identity<tuple<$".for/K/0/%Ar%/TK/,"> >{};
  template<typename T$"m"$".for/K/0/%Ar%-1/,typename TK/">
  tuple<$".for/K/0/%Ar%/TK/,"> tuple_push(const tuple<$".for/K/0/%Ar%-1/TK/,">& tuplet,T$"m" arg$"m"){
    mwg_unused(tuplet); // Ar=1 の時には使われない
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
  template<typename T> struct tuple_element_ref:stdx::add_cref<T>{};
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
