// -*- C++ -*-
#ifndef MWG_STAT_BINNING_H
#define MWG_STAT_BINNING_H
#include <utility>
#include <mwg/std/type_traits>
#include <mwg/std/tuple>
#include <mwg/std/utility>
#include <mwg/concept.h>
#include <mwg/range.h>
#pragma%include "mwg_concept.hpp"
#include "bindex.h"
namespace mwg{
namespace stat{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  template<typename T=double>
  class EqualBinning;
  template<typename T=double>
  struct TBinningWrapper;
  template<typename T=double>
  struct TBinning1DimWrapper;

  template<typename T>
  class EqualBinning{
    int nbin;
    T lower;
    T upper;
  public:
    typedef T domain_type;
    EqualBinning():nbin(1),lower(0),upper(1){}
    EqualBinning(int nbin,const T& lower,const T& upper):nbin(nbin),lower(lower),upper(upper){}
  public:
    int GetBinNumber() const{
      return nbin;
    }
    int GetBinIndex(const T& value) const{
      if(value<lower||upper<=value)return -1;
      return int(nbin*(value-lower)/(upper-lower));
    }
    T GetBinWidth(int index) const{
      return (upper-lower)/nbin;
    }
    T GetBinLowerBound(int index) const{
      return lower+(upper-lower)*index/nbin;
    }
    T GetBinUpperBound(int index) const{
      return lower+(upper-lower)*(index+1)/nbin;
    }
    T GetBinMeanValue(int index) const{
      //std::fprintf(stderr,"dbg: GetBinMeanValue: lower=%lf upper=%lf nbin=%d index=%d\n",lower,upper,nbin,index);
      return lower+(upper-lower)*(index+0.5)/nbin;
    }
    mwg::range<T> GetBinRange(int index) const{
      return mwg::range<T>(GetBinLowerBound(index),GetBinUpperBound(index));
    }
  };

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
#pragma%m ARG_FOR_MEMBER (
#pragma%%x t.r/%name%/domain_type/
#pragma%%x f.r/%decl%/int GetBinIndex(const T\& v) const/.r/%name%/GetBinIndex/  .r/%args%/v/
#pragma%%x f.r/%decl%/int GetBinNumber() const/          .r/%name%/GetBinNumber/ .r/%args%//
#pragma%)
#pragma%[ARG_TMPL_PARAMS="typename T"]
#pragma%[ARG_TMPL_PARAMS_FOR_CONCEPT="typename T=typename T_::domain_type"]
#pragma%[ARG_TMPL_ARGS="T"]
#pragma%m wrapper_content (
public:
  typedef typename mwg::identity<T>::type domain_type;
  TBinningWrapper():h(new holder<mwg::stat::EqualBinning<> >(mwg::stat::EqualBinning<>(T(),T(),T()))){}
#pragma%)
#pragma%x mwg_concept_create_wrapper.r/%TConcept%/TBinning/
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
#pragma%m ARG_FOR_MEMBER (
#pragma%%x t.r/%name%/domain_type/
#pragma%%x f.r/%decl%/int GetBinIndex(const T\& v) const/ .r/%name%/GetBinIndex/      .r/%args%/v/
#pragma%%x f.r/%decl%/int GetBinNumber() const/           .r/%name%/GetBinNumber/     .r/%args%//
#pragma%%x f.r/%decl%/T GetBinWidth(int index) const/     .r/%name%/GetBinWidth/      .r/%args%/index/
#pragma%%x f.r/%decl%/T GetBinLowerBound(int index) const/.r/%name%/GetBinLowerBound/ .r/%args%/index/
#pragma%%x f.r/%decl%/T GetBinUpperBound(int index) const/.r/%name%/GetBinUpperBound/ .r/%args%/index/
#pragma%%x f.r/%decl%/T GetBinMeanValue(int index) const/ .r/%name%/GetBinMeanValue/  .r/%args%/index/
#pragma%)
#pragma%[ARG_TMPL_PARAMS="typename T"]
#pragma%[ARG_TMPL_PARAMS_FOR_CONCEPT="typename T=typename T_::domain_type"]
#pragma%[ARG_TMPL_ARGS="T"]
#pragma%m wrapper_content (
public:
  typedef typename mwg::identity<T>::type domain_type;
  TBinning1DimWrapper():h(new holder<mwg::stat::EqualBinning<> >(mwg::stat::EqualBinning<>(T(),T(),T()))){}
  mwg::range<T> GetBinRange(int index) const{
    return mwg::range<T>(GetBinLowerBound(index),GetBinUpperBound(index));
  }
#pragma%)
#pragma%x mwg_concept_create_wrapper.r/%TConcept%/TBinning1Dim/
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ProductBinning
//-----------------------------------------------------------------------------
  namespace detail{
    template<std::size_t I>
    struct ProductBinningImpl{
      template<typename TT>
      static int GetBinNumber(const TT& bins){
        return ProductBinningImpl<I-1>::GetBinNumber(bins)*mwg::stdm::get<I>(bins).GetBinNumber();
      }
      template<typename TTB,typename TTV>
      typename stdm::enable_if<!stdm::is_same<bindex,typename stdm::tuple_element<I,TTV>::type>::value,int>::type
      static GetBinIndex(const TTB& bins,const TTV& vals){
        return ProductBinningImpl<I-1>::GetBinIndex(bins,vals)
          *mwg::stdm::get<I>(bins).GetBinNumber()
          +mwg::stdm::get<I>(bins).GetBinIndex(mwg::stdm::get<I>(vals));
      }
      template<typename TTB,typename TTV>
      typename stdm::enable_if<stdm::is_same<bindex,typename stdm::tuple_element<I,TTV>::type>::value,int>::type
      static GetBinIndex(const TTB& bins,const TTV& vals){
        return ProductBinningImpl<I-1>::GetBinIndex(bins,vals)
          *mwg::stdm::get<I>(bins).GetBinNumber()
          +int(mwg::stdm::get<I>(vals));
      }
      template<typename TT>
      struct IsBins:mwg::stdm::bool_constant<
        ProductBinningImpl<I-1>::template IsBins<TT>::value&&TBinning<typename mwg::stdm::tuple_element<I,TT>::type>::value
      >{};
    };
    template<>
    struct ProductBinningImpl<0>{
      template<typename TT>
      static int GetBinNumber(const TT& bins){
        return mwg::stdm::get<0>(bins).GetBinNumber();
      }
      template<typename TTB,typename TTV>
      typename stdm::enable_if<!stdm::is_same<bindex,typename stdm::tuple_element<0,TTV>::type>::value,int>::type
      static GetBinIndex(const TTB& bins,const TTV& vals){
        return mwg::stdm::get<0>(bins).GetBinIndex(mwg::stdm::get<0>(vals));
      }
      template<typename TTB,typename TTV>
      typename stdm::enable_if<stdm::is_same<bindex,typename stdm::tuple_element<0,TTV>::type>::value,int>::type
      static GetBinIndex(const TTB& bins,const TTV& vals){
        return int(mwg::stdm::get<0>(vals));
      }
      template<typename TT>
      struct IsBins:mwg::stdm::bool_constant<TBinning<typename mwg::stdm::tuple_element<0,TT>::type>::value>{};
    };

#if defined(MWGCONF_STD_VARIADIC_TEMPLATES)
    template<typename TT1,typename TT2,bool S>
    struct is_bindex_pack_impl:stdm::false_type{};
    template<typename... As,typename... Bs>
    struct is_bindex_pack_impl<stdx::parameter_pack<As...>,stdx::parameter_pack<Bs...>,true>
      :stdm::bool_constant<stdx::ice_and<(stdm::is_same<As,bindex>::value||stdm::is_convertible<As,typename Bs::domain_type>::value)...>::value>{};

    template<typename TT1,typename TT2>
    struct is_bindex_pack:stdm::false_type{};
    template<typename... As,typename... Bs>
    struct is_bindex_pack<stdx::parameter_pack<As...>,stdx::parameter_pack<Bs...> >
      :is_bindex_pack_impl<stdx::parameter_pack<As...>,stdx::parameter_pack<Bs...>,sizeof...(As)==sizeof...(Bs)>{};
#else
    template<typename TT1,typename TT2,bool S=true> struct is_bindex_pack_impl:false_type{};
    template<> struct is_bindex_pack_impl<stdx::parameter_pack<>,stdx::parameter_pack<> >:true_type{};
//%eval ArN=10
//%expand (
    template<
      $".for/K/0/ArN/typename AK/,",
      $".for/K/0/ArN/typename BK/,"
      >
    struct is_bindex_pack_impl<stdx::parameter_pack<$".for/K/0/ArN/AK/,">,stdx::parameter_pack<$".for/K/0/ArN/BK/,"> >
      :stdm::bool_constant<
      (stdm::is_same<A0,bindex>::value||stdm::is_convertible<A0,typename B0::domain_type>::value)&&is_bindex_pack_impl<
        stdx::parameter_pack<$".for/K/1/ArN/AK/,">,
        stdx::parameter_pack<$".for/K/1/ArN/BK/,">
        >::value
      >{};
//%).i

    template<typename TT1,typename TT2>
    struct is_bindex_pack
      :is_bindex_pack_impl<TT1,TT2,TT1::size==TT2::size>
    {};
#endif
  }
#if defined(MWGCONF_STD_VARIADIC_TEMPLATES)
  template<typename... TBs>
  class ProductBinning{
    mwg::stdm::tuple<TBs...> data;
  public:
    typedef mwg::stdm::tuple<typename TBs::domain_type...> domain_type;
    ProductBinning(){}
    template<typename... UBs>
    ProductBinning(UBs mwg_forward_rvalue ... args)
      :data(mwg::stdm::forward<UBs>(args)...){}
    ProductBinning(const ProductBinning& other){
      this->data=other.data;
    }
# if defined(MWGCONF_STD_RVALUE_REFERENCES)
    ProductBinning(ProductBinning&& other){
      this->data=mwg::stdm::move(other.data);
    }
# endif
  public:
    int GetBinNumber() const{
      return detail::ProductBinningImpl<sizeof...(TBs)-1>::GetBinNumber(data);
    }
    int GetBinIndex(const domain_type& value) const{
      return detail::ProductBinningImpl<sizeof...(TBs)-1>::GetBinIndex(data,value);
    }
    template<typename... Vs>
    typename stdm::enable_if<detail::is_bindex_pack<stdx::parameter_pack<Vs...>,stdx::parameter_pack<TBs...> >::value,int>::type
    GetBinIndex(Vs const&... args) const{
      return detail::ProductBinningImpl<sizeof...(TBs)-1>::GetBinIndex(data,stdm::make_tuple(args...));
    }
  };

  template<typename... TBs> typename stdm::enable_if<
    detail::ProductBinningImpl<sizeof...(TBs)-1>::template IsBins<stdm::tuple<typename stdm::decay<TBs>::type...>>::value,
    ProductBinning<typename stdm::decay<TBs>::type...>
  >::type make_binning(TBs mwg_forward_rvalue... bins){
    return ProductBinning<typename stdm::decay<TBs>::type...>(mwg::stdm::forward<TBs>(bins)...);
  }
#else
  namespace detail{
    template<typename B>
    struct binning_get_domain_type:mwg::identity<typename B::domain_type>{};
    template<>
    struct binning_get_domain_type<void>:mwg::identity<void>{};
  }

//%expand (
  template<$".for/K/0/ArN/typename TBK=void/,">
  class ProductBinning{
    typedef mwg::stdm::tuple<$".for/K/0/ArN/TBK/,"> bins_type;
    bins_type data;
  public:
    typedef mwg::stdm::tuple<$".for/K/0/ArN/
      typename detail::binning_get_domain_type<TBK>::type/,"
    > domain_type;

    ProductBinning(){}
//%expand (
    template<$".for/K/0/_AR_/typename UBK/,">
    ProductBinning($".for/K/0/_AR_/UBK mwg_forward_rvalue argK/,")
      :data($".for/K/0/_AR_/mwg::stdm::forward<UBK>(argK)/,"){}
//%).f/_AR_/1/ArN+1/
    ProductBinning(const ProductBinning& other){
      this->data=other.data;
    }
# if defined(MWGCONF_STD_RVALUE_REFERENCES)
    ProductBinning(ProductBinning&& other){
      this->data=mwg::stdm::move(other.data);
    }
# endif
  public:
    int GetBinNumber() const{
      return detail::ProductBinningImpl<sizeof...(TBs)-1>::GetBinNumber(data);
    }
    int GetBinIndex(const domain_type& value) const{
      return detail::ProductBinningImpl<sizeof...(TBs)-1>::GetBinIndex(data,value);
    }
//%expand (
    template<$".for/K/0/_AR_/typename VK/,">
    typename stdm::enable_if<detail::is_bindex_pack<stdx::parameter_pack<$".for/K/0/_AR_/VK/,">,stdx::parameter_pack<$".for/K/0/ArN/TBK/,"> >::value,int>::type
    GetBinIndex($".for/K/0/_AR_/VK const& argK/,") const{
      return detail::ProductBinningImpl<sizeof...(TBs)-1>::GetBinIndex(data,stdm::make_tuple($".for/K/0/_AR_/argK/,"));
    }
//%).f/_AR_/1/ArN+1/
  };
//%).i

#pragma%x (
  template<$".for/K/0/_AR_/typename BK/,"> typename mwg::stdm::enable_if<
    ($".for/K/0/_AR_/TBinning<BK>::value/&&"),
    ProductBinning<$".for/K/0/_AR_/BK/,">
  >::type make_binning($".for/K/0/_AR_/const BK& bK/,"){
    return ProductBinning<$".for/K/0/_AR_/BK/,">($".for/K/0/_AR_/bK/,");
  }
#pragma%).f/_AR_/2/ArN+1/.i
#endif

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}

//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
// mwg::abs,floor,ceil

  template<typename B>
  typename mwg::stdm::enable_if<stat::TBinning1Dim<B>::value,typename B::domain_type>::type
  floor(B const& bin,mwg::stat::bindex index){return bin.GetBinLowerBound((int)index);}

  template<typename B>
  typename mwg::stdm::enable_if<stat::TBinning1Dim<B>::value,typename B::domain_type>::type
  ceil(B const& bin,mwg::stat::bindex index){return bin.GetBinUpperBound((int)index);}

  template<typename B>
  typename mwg::stdm::enable_if<stat::TBinning1Dim<B>::value,mwg::range<typename B::domain_type> >::type
  make_range(B const& bin,mwg::stat::bindex index){
    return mwg::range<typename B::domain_type>(mwg::floor(bin,index),mwg::ceil(bin,index));
  }

  template<typename B>
  typename mwg::stdm::enable_if<stat::TBinning1Dim<B>::value,typename B::domain_type>::type
  floor(B const& bin){return bin.GetBinLowerBound(0);}

  template<typename B>
  typename mwg::stdm::enable_if<stat::TBinning1Dim<B>::value,typename B::domain_type>::type
  ceil(B const& bin){return bin.GetBinUpperBound(bin.GetBinNumber()-1);}

  template<typename B>
  typename mwg::stdm::enable_if<stat::TBinning1Dim<B>::value,mwg::range<typename B::domain_type> >::type
  make_range(B const& bin){
    return mwg::range<typename B::domain_type>(mwg::floor(bin),mwg::ceil(bin));
  }

}
#endif
