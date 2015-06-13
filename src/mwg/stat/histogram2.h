// -*- mode:C++;coding:utf-8 -*-
#pragma once
#ifndef LIBMWG_STAT_HISTOGRAM_IMPL2_H
#define LIBMWG_STAT_HISTOGRAM_IMPL2_H
#include <cmath>
#include <mwg/std/memory>
#include <mwg/std/type_traits>
#include <mwg/std/utility>
#include "errored.h"
#include "accumulator.h"
#include "binning2.h"
namespace mwg{
namespace stat{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

namespace histogram_impl2_detail{

  template<typename TBinning=binning_wrapper<double>,typename TAcc=count_accumulator<double> >
  class histogram;

  template<typename TBinning,typename TAcc>
  class histogram{
    TBinning part;
    mwg::stdm::unique_ptr<TAcc[]> data;
    TAcc* ptr;
  public:
    typedef typename TBinning::domain_type domain_type;
    typedef TAcc bin_type;
  public:
    template<typename T>
    explicit histogram(const T& binning)
      :part(binning),data(new TAcc[binning.size()+1]),ptr(&data[1])
    {}
#if defined(MWGCONF_STD_RVALUE_REFERENCES)
    histogram& operator=(histogram&& other){
      this->part=mwg::stdm::move(other.part);
      this->data   =mwg::stdm::move(other.data);
      this->ptr    =other.ptr;
      return *this;
    }
    histogram(histogram&& other)
      :part(mwg::stdm::move(other.part))
      ,data(mwg::stdm::move(other.data))
      ,ptr(ptr){}
#endif
  private:
    histogram(const histogram&) mwg_std_deleted;
    histogram& operator=(const histogram&) mwg_std_deleted;
  public:
    const bin_type& operator[](bindex index) const{
      return ptr[index];
    }
    bin_type& operator[](bindex index){
      return ptr[index];
    }
  public:
    const bin_type& operator()(const domain_type& domain) const{
      return ptr[(int)part(domain)];
    }
    bin_type& operator()(const domain_type& domain){
      return ptr[(int)part(domain)];
    }
    const bin_type& operator()(bindex index) const{
      return ptr[(int)index];
    }
    bin_type& operator()(bindex index){
      return ptr[(int)index];
    }
#if defined(MWGCONF_STD_RVALUE_REFERENCES)&&defined(MWGCONF_STD_VARIADIC_TEMPLATES)
    template<typename A0,typename... Args>
    const bin_type& operator()(A0&& arg0,Args&&... args) const{
      return ptr[(int)part(mwg::stdm::forward<A0>(arg0),mwg::stdm::forward<Args>(args)...)];
    }
    template<typename A0,typename... Args>
    bin_type& operator()(A0&& arg0,Args&&... args){
      return ptr[(int)part(mwg::stdm::forward<A0>(arg0),mwg::stdm::forward<Args>(args)...)];
    }
#else
    template<typename T0,typename T1>
    const bin_type& operator()(const T0& b0,const T1& b1) const{
      return ptr[(int)part(b0,b1)];
    }
    template<typename T0,typename T1>
    bin_type& operator()(const T0& b0,const T1& b1){
      return ptr[(int)part(b0,b1)];
    }
    template<typename T0,typename T1,typename T2>
    const bin_type& operator()(const T0& b0,const T1& b1,const T2& b2) const{
      return ptr[(int)part(b0,b1,b2)];
    }
    template<typename T0,typename T1,typename T2>
    bin_type& operator()(const T0& b0,const T1& b1,const T2& b2){
      return ptr[(int)part(b0,b1,b2)];
    }
    template<typename T0,typename T1,typename T2,typename T3>
    const bin_type& operator()(const T0& b0,const T1& b1,const T2& b2,const T3& b3) const{
      return ptr[(int)part(b0,b1,b2,b3)];
    }
    template<typename T0,typename T1,typename T2,typename T3>
    bin_type& operator()(const T0& b0,const T1& b1,const T2& b2,const T3& b3){
      return ptr[(int)part(b0,b1,b2,b3)];
    }
#endif
  };
//fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
  template<typename TBin>
  histogram<TBin,count_accumulator<> > make_histogram(const TBin& binning){
    return histogram<TBin,count_accumulator<> >(binning);
  }
  template<typename TAcc,typename TBin>
  histogram<TBin,TAcc> make_histogram(const TBin& binning){
    return histogram<TBin,TAcc>(binning);
  }

}

  using histogram_impl2_detail::histogram;
  using histogram_impl2_detail::make_histogram;

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
