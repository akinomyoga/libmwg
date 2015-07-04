//%# -*- mode:C++;coding:utf-8 -*-
//%# to be included from binning2.ProductBinning_{variadic,nonvariadic}.h
#pragma once
#pragma%x
#ifndef $"header_name"
#define $"header_name"
#pragma%end.i
#include <mwg/std/tuple>
#include <mwg/std/utility>
namespace mwg{
namespace stat{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

  namespace detail{
    template<std::size_t I>
    struct ProductBinningImpl{
      template<typename TT>
      static int GetBinNumber(const TT& bins){
        return ProductBinningImpl<I-1>::GetBinNumber(bins)*mwg::stdm::get<I>(bins).size();
      }
      template<typename TTB,typename TTV>
      typename stdm::enable_if<!stdm::is_same<bindex,typename stdm::tuple_element<I,TTV>::type>::value,int>::type
      static GetBinIndex(const TTB& bins,const TTV& vals){
        int const ithis=(int)mwg::stdm::get<I>(bins)(mwg::stdm::get<I>(vals));if(ithis<0)return -1;
        int const irest=ProductBinningImpl<I-1>::GetBinIndex(bins,vals);if(irest<0)return -1;
        return irest*mwg::stdm::get<I>(bins).size()+ithis;
      }
      template<typename TTB,typename TTV>
      typename stdm::enable_if<stdm::is_same<bindex,typename stdm::tuple_element<I,TTV>::type>::value,int>::type
      static GetBinIndex(const TTB& bins,const TTV& vals){
        int const ithis=(int)mwg::stdm::get<I>(vals);if(ithis<0)return -1;
        int const irest=ProductBinningImpl<I-1>::GetBinIndex(bins,vals);if(irest<0)return -1;
        return irest*mwg::stdm::get<I>(bins).size()+ithis;
      }
      template<typename TT>
      struct IsBins:mwg::stdm::integral_constant<
        bool,
        ProductBinningImpl<I-1>::template IsBins<TT>::value&&binning_concept<typename mwg::stdm::tuple_element<I,TT>::type>::value
      >{};
    };
    template<>
    struct ProductBinningImpl<0>{
      template<typename TT>
      static int GetBinNumber(const TT& bins){
        return mwg::stdm::get<0>(bins).size();
      }
      template<typename TTB,typename TTV>
      typename stdm::enable_if<!stdm::is_same<bindex,typename stdm::tuple_element<0,TTV>::type>::value,int>::type
      static GetBinIndex(const TTB& bins,const TTV& vals){
        return (int)mwg::stdm::get<0>(bins)(mwg::stdm::get<0>(vals));
      }
      template<typename TTB,typename TTV>
      typename stdm::enable_if<stdm::is_same<bindex,typename stdm::tuple_element<0,TTV>::type>::value,int>::type
      static GetBinIndex(const TTB& bins,const TTV& vals){
        return (int)mwg::stdm::get<0>(vals);
      }
      template<typename TT>
      struct IsBins:mwg::stdm::integral_constant<bool,binning_concept<typename mwg::stdm::tuple_element<0,TT>::type>::value>{};
    };

#pragma%if variadic
    template<typename TT1,typename TT2,bool S>
    struct is_bindex_pack_impl:stdm::false_type{};
    template<typename... As,typename... Bs>
    struct is_bindex_pack_impl<stdx::parameter_pack<As...>,stdx::parameter_pack<Bs...>,true>
      :stdm::integral_constant<bool,stdx::ice_and<(stdm::is_same<As,bindex>::value||stdm::is_convertible<As,typename Bs::domain_type>::value)...>::value>{};

    template<typename TT1,typename TT2>
    struct is_bindex_pack:stdm::false_type{};
    template<typename... As,typename... Bs>
    struct is_bindex_pack<stdx::parameter_pack<As...>,stdx::parameter_pack<Bs...> >
      :is_bindex_pack_impl<stdx::parameter_pack<As...>,stdx::parameter_pack<Bs...>,sizeof...(As)==sizeof...(Bs)>{};
#pragma%else /* end of if variadic */
    template<typename TT1,typename TT2,bool S=true> struct is_bindex_pack_impl:stdm::false_type{};
    template<> struct is_bindex_pack_impl<stdx::parameter_pack<>,stdx::parameter_pack<> >:stdm::true_type{};
#pragma%%[ArN=10]
#pragma%%x
    template<
      $".for/K/0/ArN/typename AK/,",
      $".for/K/0/ArN/typename BK/,"
      >
    struct is_bindex_pack_impl<stdx::parameter_pack<$".for/K/0/ArN/AK/,">,stdx::parameter_pack<$".for/K/0/ArN/BK/,"> >
      :stdm::integral_constant<
      bool,
      (stdm::is_same<A0,bindex>::value||stdm::is_convertible<A0,typename B0::domain_type>::value)&&is_bindex_pack_impl<
        stdx::parameter_pack<$".for/K/1/ArN/AK/,">,
        stdx::parameter_pack<$".for/K/1/ArN/BK/,">
        >::value
      >{};
#pragma%%end.i

    template<typename TT1,typename TT2>
    struct is_bindex_pack
      :is_bindex_pack_impl<TT1,TT2,TT1::size==TT2::size>
    {};
#pragma%end /* end of if !variadic */
  }

#pragma%if variadic
  template<typename... TBs>
  class product_binning{
    mwg::stdm::tuple<TBs...> data;
  public:
    typedef mwg::stdm::tuple<typename TBs::domain_type...> domain_type;
    product_binning(){}
    template<typename... UBs>
    product_binning(UBs mwg_forward_rvalue ... args)
      :data(mwg::stdm::forward<UBs>(args)...){}
    product_binning(const product_binning& other){
      this->data=other.data;
    }
# if defined(MWGCONF_STD_RVALUE_REFERENCES)
    product_binning(product_binning&& other){
      this->data=mwg::stdm::move(other.data);
    }
# endif
  public:
    std::size_t size() const{
      return detail::ProductBinningImpl<sizeof...(TBs)-1>::GetBinNumber(data);
    }
    bindex operator()(const domain_type& value) const{
      return (bindex)detail::ProductBinningImpl<sizeof...(TBs)-1>::GetBinIndex(data,value);
    }
    template<typename... Vs>
    typename stdm::enable_if<detail::is_bindex_pack<stdx::parameter_pack<Vs...>,stdx::parameter_pack<TBs...> >::value,bindex>::type
    operator()(Vs const&... args) const{
      return (bindex)detail::ProductBinningImpl<sizeof...(TBs)-1>::GetBinIndex(data,stdm::make_tuple(args...));
    }
  };

  template<typename... TBs> typename stdm::enable_if<
    detail::ProductBinningImpl<sizeof...(TBs)-1>::template IsBins<stdm::tuple<typename stdm::decay<TBs>::type...>>::value,
    product_binning<typename stdm::decay<TBs>::type...>
  >::type make_binning(TBs mwg_forward_rvalue... bins){
    return product_binning<typename stdm::decay<TBs>::type...>(mwg::stdm::forward<TBs>(bins)...);
  }
#pragma%else /* end of if variadic */
  namespace detail{
    template<typename B>
    struct binning_get_domain_type:mwg::identity<typename B::domain_type>{};
    template<>
    struct binning_get_domain_type<void>:mwg::identity<void>{};
  }

#pragma%%x
  template<$".for/K/0/ArN/typename TBK=void/,">
  class product_binning{
    typedef mwg::stdm::tuple<$".for/K/0/ArN/TBK/,"> bins_type;
    bins_type data;
  public:
    typedef mwg::stdm::tuple<$".for/K/0/ArN/
      typename detail::binning_get_domain_type<TBK>::type/,"
    > domain_type;

    product_binning(){}
#pragma%%x
    template<$".for/K/0/_AR_/typename UBK/,">
    product_binning($".for/K/0/_AR_/UBK mwg_forward_rvalue argK/,")
      :data($".for/K/0/_AR_/mwg::stdm::forward<UBK>(argK)/,"){}
#pragma%%end.f/_AR_/1/ArN+1/
    product_binning(const product_binning& other)
      :data(other.data)
    {}
# if defined(MWGCONF_STD_RVALUE_REFERENCES)
    product_binning(product_binning&& other)
      :data(mwg::stdm::move(other.data))
    {}
# endif
  public:
    std::size_t size() const{
      return detail::ProductBinningImpl<stdm::tuple_size<bins_type>::value-1>::GetBinNumber(data);
    }
    bindex operator()(const domain_type& value) const{
      return (bindex)detail::ProductBinningImpl<stdm::tuple_size<bins_type>::value-1>::GetBinIndex(data,value);
    }
#pragma%%x
    template<$".for/K/0/_AR_/typename VK/,">
    typename stdm::enable_if<detail::is_bindex_pack<stdx::parameter_pack<$".for/K/0/_AR_/VK/,">,stdx::parameter_pack<$".for/K/0/ArN/TBK/,"> >::value,bindex>::type
    operator()($".for/K/0/_AR_/VK const& argK/,") const{
      return (bindex)detail::ProductBinningImpl<stdm::tuple_size<bins_type>::value-1>::GetBinIndex(data,stdm::make_tuple($".for/K/0/_AR_/argK/,"));
    }
#pragma%%end.f/_AR_/1/ArN+1/
  };
#pragma%%end.i

#pragma%%x
  template<$".for/K/0/_AR_/typename BK/,"> typename mwg::stdm::enable_if<
    ($".for/K/0/_AR_/binning_concept<BK>::value/&&"),
    product_binning<$".for/K/0/_AR_/BK/,">
  >::type make_binning($".for/K/0/_AR_/const BK& bK/,"){
    return product_binning<$".for/K/0/_AR_/BK/,">($".for/K/0/_AR_/bK/,");
  }
#pragma%%end.f/_AR_/2/ArN+1/.i
#pragma%end /* end of if !variadic */

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
