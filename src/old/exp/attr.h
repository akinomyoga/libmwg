// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_ATTR_ATTR_H
#define MWG_ATTR_ATTR_H
#include <mwg/std/type_traits>
namespace mwg{
namespace attr{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

//=============================================================================
//  :integral_attr<MyAttribute,int,value>
//-----------------------------------------------------------------------------
  namespace integral_attr_detail{
    template<typename Tag,typename T> struct integral_attr_base{};

    template<typename Tag,typename T,T Value>
    struct integral_attr:integral_attr_base<Tag,T>{};

    template<typename C,typename Tag,typename T>
    struct has_integral_attr:mwg::stdm::is_base_of<integral_attr_base<Tag,T>,C>{};

    char attr_value_sgn(...);
    char attr_value_abs(...);
    template<mwg::u8t I> struct sized_type{char data[I];};

    // Value==0 の時に data[0] になってまずいから符号だけ分離
    template<typename T,T Value> struct ice_neg{static const T value=-Value;}; // for VC2008, VC2010
    template<typename Tag,typename T,T Value> sized_type<(Value==0?2:Value<0?1:3)> const&
    attr_value_sgn(integral_attr<Tag,T,Value> const&);
    template<typename Tag,typename T,T Value> sized_type<(Value==0?1:Value<0?ice_neg<T,Value>::value:Value)> const&
    attr_value_abs(integral_attr<Tag,T,Value> const&);

    template<typename T,T Default,bool B,std::size_t VS,std::size_t VA>
    struct integral_attr_get_impl:mwg::stdm::integral_constant<T,Default>{};
    template<typename T,T Default,std::size_t VS,std::size_t VA>
    struct integral_attr_get_impl<T,Default,true,VS,VA>
      :mwg::stdm::integral_constant<T,(T)((VS-2)*VA)>{};

    template<typename C,typename Tag,typename T,T Default=0>
    struct integral_attr_get:integral_attr_get_impl<
      T,Default,
      has_integral_attr<C,Tag,T>::value,
      sizeof(attr_value_sgn(mwg::declval<C>())),
      sizeof(attr_value_abs(mwg::declval<C>()))
    >{};
  }
  using integral_attr_detail::integral_attr;
  using integral_attr_detail::has_integral_attr;
  using integral_attr_detail::integral_attr_get;

//-----------------------------------------------------------------------------
// example
#if 0
  struct my_tag1{};
  struct my_tag2{};
  struct my_tag3{};
  class Hoge:
    public mwg::attr::integral_attr<my_tag1,int,-321>,
    public mwg::attr::integral_attr<my_tag2,unsigned,123>
  {};

  static_assert( mwg::attr::has_integral_attr<Hoge,my_tag1,int     >::value,"has");
  static_assert( mwg::attr::has_integral_attr<Hoge,my_tag2,unsigned>::value,"has");
  static_assert(!mwg::attr::has_integral_attr<Hoge,my_tag3,char    >::value,"does not have");

  static_assert(-321==mwg::attr::integral_attr_get<Hoge,my_tag1,int     >::value,"-321");
  static_assert( 123==mwg::attr::integral_attr_get<Hoge,my_tag2,unsigned>::value,"123");

  // 特定の用途に対して使う為には以下の様にするのが良い。
  template<int Flags>
  struct MyCustomAttribute:mwg::attr::integral_attr<MyCustomAttribute,int,Flags>{};
#endif
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class_attributes
//-----------------------------------------------------------------------------
  namespace noncopyable_detail{
    // ADL で変な関数を拾うのを避ける
    //   http://www.boost.org/doc/libs/1_48_0/boost/noncopyable.hpp
    //   http://ml.tietew.jp/cppll/cppll_novice/thread_articles/1652
    // 一般に本来の意味での派生ではない使われ方をする基本クラスの場合は、独立し
    // た名前空間に閉じ込めるべきだそうだ
    class noncopyable{
    protected:
      noncopyable(){}
      ~noncopyable(){}
    private:
      noncopyable(const noncopyable&) mwg_std_deleted;
      const noncopyable& operator=(const noncopyable&) mwg_std_deleted;
    };
  }
  using noncopyable_detail::noncopyable;

  namespace sealed_detail{
    template<typename T> class sealed;

    template<typename NonDerivableClass>
    class ERROR_this_class_cannot_be_derived{
    public:
      friend class sealed<NonDerivableClass>;
      friend class mwg::identity<NonDerivableClass>::type;
    private:
      ERROR_this_class_cannot_be_derived(){}
      ~ERROR_this_class_cannot_be_derived(){}
    };

    template<typename T>
    class sealed:virtual private ERROR_this_class_cannot_be_derived<T>{};
  }
  using sealed_detail::sealed;

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
//=============================================================================
//  mwg_attr_define_flags(className,UnderlyingType,enum{});
//-----------------------------------------------------------------------------
namespace mwg{
namespace attr{
namespace flags_detail{
  typedef mwg::i1t yes_type;
  typedef mwg::i2t no_type;
  struct empty_type{};

  struct flags_attr{};

  template<
    typename T,typename U=void,
    bool isE=sizeof(mwg::declval<T&>(),(no_type)0)==sizeof(yes_type),
    bool isC=mwg::stdm::is_base_of<flags_attr,T>::value
  > struct flags_enum_type:mwg::identity<U>{};
  template<typename T,typename U> struct flags_enum_type<T,U,true,false>:mwg::identity<T>{};
  template<typename T,typename U> struct flags_enum_type<T,U,false,true>:mwg::identity<typename T::enum_t>{};

  template<typename E1,typename E2=void>
  struct return_type_as_flags:mwg::stdm::conditional<
    mwg::stdm::is_same<typename flags_enum_type<E1,yes_type>::type,typename flags_enum_type<E2,no_type>::type>::value,
    mwg::identity<typename flags_enum_type<E1>::type>,
    empty_type
  >::type{};

  template<typename E>
  struct return_type_as_flags<E,void>:mwg::stdm::conditional<
    !mwg::stdm::is_same<typename flags_enum_type<E>::type,void>::value,
    mwg::identity<typename flags_enum_type<E>::type>,
    empty_type
  >::type{};

#define mwg_attr_define_flags(className,UnderlyingType,...)            \
  class className:mwg::attr::flags_detail::flags_attr{                 \
    typedef mwg::identity<UnderlyingType>::type underlying_type;       \
    underlying_type value;                                             \
  public:                                                              \
    typedef __VA_ARGS__ enum_t;                                        \
                                                                       \
    className(enum_t value):value(value){}                             \
    explicit className(underlying_type value):value(value){}           \
    operator enum_t() const{return enum_t(this->value);}               \
  };                                                                   \
  mwg::attr::flags_detail::yes_type operator,(className::enum_t&,mwg::attr::flags_detail::no_type)

  // MSC だと operator しか ADL が働かないので、operator,() を使用
}
  using flags_detail::flags_attr;
}
}

// (GCC だとこれで OK みたいだが…)
// template<typename E> typename mwg::attr::flags_detail::return_type_as_flags<E>::type
// operator~(const E& l){return ~l;}
// template<typename E1,typename E2> typename mwg::attr::flags_detail::return_type_as_flags<E1,E2>::type
// operator&(const E1& l,const E2& r){return l&r;}
// template<typename E1,typename E2> typename mwg::attr::flags_detail::return_type_as_flags<E1,E2>::type
// operator|(const E1& l,const E2& r){return l|r;}
// template<typename E1,typename E2> typename mwg::attr::flags_detail::return_type_as_flags<E1,E2>::type
// operator^(const E1& l,const E2& r){return l^r;}

template<typename E> typename mwg::attr::flags_detail::return_type_as_flags<E>::type
operator~(const E& l){
  typedef typename mwg::attr::flags_detail::return_type_as_flags<E>::type return_type;
  return return_type(~(int)l);
}
template<typename E1,typename E2> typename mwg::attr::flags_detail::return_type_as_flags<E1,E2>::type
operator&(const E1& l,const E2& r){
  typedef typename mwg::attr::flags_detail::return_type_as_flags<E1,E2>::type return_type;
  return return_type((int)l&(int)r);
}
template<typename E1,typename E2> typename mwg::attr::flags_detail::return_type_as_flags<E1,E2>::type
operator|(const E1& l,const E2& r){
  typedef typename mwg::attr::flags_detail::return_type_as_flags<E1,E2>::type return_type;
  return return_type((int)l|(int)r);
}
template<typename E1,typename E2> typename mwg::attr::flags_detail::return_type_as_flags<E1,E2>::type
operator^(const E1& l,const E2& r){
  typedef typename mwg::attr::flags_detail::return_type_as_flags<E1,E2>::type return_type;
  return return_type((int)l^(int)r);
}
//-----------------------------------------------------------------------------
// example
#if 0
  mwg_attr_define_flags(MyFlags,unsigned,enum{
    Flag1,
    Flag2,
    Flag3,
  });
#endif
//=============================================================================
#endif
