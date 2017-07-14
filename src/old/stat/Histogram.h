// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_STAT_HISTOGRAM_H
#define MWG_STAT_HISTOGRAM_H
#include <cmath>
#include <mwg/std/memory>
#include <mwg/std/type_traits>
#include <mwg/std/utility>
#include "Binning.h"
#include "errored_value.h"
namespace mwg{
namespace stat{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  template<typename T=double>
  class CountAccumulator;
  template<typename T=double,bool enable_errored_value=stdm::is_floating_point<T>::value>
  struct AverageAccumulator;
  template<typename TBinning=TBinningWrapper<double>,typename TAcc=CountAccumulator<double> >
  class Histogram;
//-----------------------------------------------------------------------------

  template<typename T>
  class CountAccumulator{
    T sw1;
    T sw2;
  public:
    typedef T value_type;
    CountAccumulator():sw1(0),sw2(0){}
    CountAccumulator& operator()(const T& weight){
      sw1+=weight;
      sw2+=weight*weight;
      return *this;
    }
    CountAccumulator& operator()(){
      return this->operator()(1);
    }
  public:
    T GetValue() const{return sw1;}
    T GetError() const{return std::sqrt(sw2);}
  };

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
namespace detail{
  template<typename T>
  class AverageAccumulatorBase{
    int s0;
    T s1;
    T s2;
  public:
    typedef T value_type;
    typedef T result_type;
  public:
    AverageAccumulatorBase():s0(0),s1(0),s2(0){}
    AverageAccumulatorBase& operator()(const T& value){
      s0++;
      s1+=value;
      s2+=value*value;
      return *this;
    }
  public:
    T GetValue() const{return this->GetAverage();}
    T GetError() const{return this->GetAverageError();}
  public:
    T GetSum() const{return s1;}
    T GetSumError() const{return s0*this->GetAverageError();}
    T GetAverage() const{
      if(s0==0)return 0;
      return s1/s0;
    }
    T GetAverageError() const{
      if(s0<=1)return 0;
      return std::sqrt(this->GetBiasedVariance()/(s0-1));
    }
    T GetBiasedVariance() const{
      if(s0==0)return 0;
      return s2/s0-(s1/s0)*(s1/s0);
    }
    T GetUnbiasedVariance() const{
      if(s0<=1)return 0;
      return s0/T(s0-1)*this->GetBiasedVariance();
    }
    T GetBiasedStandardDeviation() const{
      return std::sqrt(this->GetBiasedVariance());
    }
    T GetUnbiasedStandardDeviation() const{
      return std::sqrt(this->GetUnbiasedVariance());
    }
  };

}

  struct accumulator_tag{};

  template<typename T,bool enable_errored_value>
  class AverageAccumulator:public mwg::stat::detail::AverageAccumulatorBase<T>{
    typedef accumulator_tag* tag;
  };

  template<typename T>
  class AverageAccumulator<T,true>:public mwg::stat::detail::AverageAccumulatorBase<T>{
  public:
    errored<T> GetAverageWithError() const{
      return errored<T>(this->GetAverage(),this->GetAverageError());
    }
    errored<T> GetSumWithError() const{
      return errored<T>(this->GetSum(),this->GetSumError());
    }

    typedef accumulator_tag* tag;
  };

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  template<typename TBinning,typename TAcc>
  class Histogram{
    TBinning binning;
    TAcc dummy_bin;
    mwg::stdm::unique_ptr<TAcc[]> data;
  public:
    typedef typename TBinning::domain_type domain_type;
    typedef TAcc bin_type;
  public:
    template<typename T>
    explicit Histogram(const T& binning)
      :binning(binning),data(new TAcc[binning.GetBinNumber()])
    {}
#if defined(MWGCONF_STD_RVALUE_REFERENCES)
    Histogram& operator=(Histogram&& other){
      this->binning=mwg::stdm::move(other.binning);
      this->data=mwg::stdm::move(other.data);
      return *this;
    }
    Histogram(Histogram&& other)
      :binning(mwg::stdm::move(other.binning))
      ,data(mwg::stdm::move(other.data)){}
#endif
  private:
    Histogram(const Histogram&) mwg_std_deleted;
    Histogram& operator=(const Histogram&) mwg_std_deleted;
  public:
#if defined(MWGCONF_STD_RVALUE_REFERENCES)&&defined(MWGCONF_STD_VARIADIC_TEMPLATES)
    template<typename... Args>
    void Add(const domain_type& domain,Args&&... args){
      int index=binning.GetBinIndex(domain);
      if(index<0)return;
      data[index](mwg::stdm::forward<Args>(args)...);
    }
#else
    void Add(const domain_type& domain){
      int index=binning.GetBinIndex(domain);
      if(index<0)return;
      data[index]();
    }
    template<typename A1>
    void Add(const domain_type& domain,const A1& arg1){
      int index=binning.GetBinIndex(domain);
      if(index<0)return;
      data[index](arg1);
    }
    template<typename A1,typename A2>
    void Add(const domain_type& domain,const A1& arg1,const A2& arg2){
      int index=binning.GetBinIndex(domain);
      if(index<0)return;
      data[index](arg1,arg2);
    }
    template<typename A1,typename A2,typename A3>
    void Add(const domain_type& domain,const A1& arg1,const A2& arg2,const A3& arg3){
      int index=binning.GetBinIndex(domain);
      if(index<0)return;
      data[index](arg1,arg2,arg3);
    }
    template<typename A1,typename A2,typename A3,typename A4>
    void Add(const domain_type& domain,const A1& arg1,const A2& arg2,const A3& arg3,const A4& arg4){
      int index=binning.GetBinIndex(domain);
      if(index<0)return;
      data[index](arg1,arg2,arg3,arg4);
    }
#endif
  public:
    const bin_type& operator[](int index) const{
      return data[index];
    }
    bin_type& operator[](int index){
      return data[index];
    }
    const bin_type& GetBin(int index) const{
      return data[index];
    }
    bin_type& GetBin(int index){
      return data[index];
    }
  public:
#if defined(MWGCONF_STD_RVALUE_REFERENCES)&&defined(MWGCONF_STD_VARIADIC_TEMPLATES)
    template<typename... Args>
    const bin_type& operator()(Args&&... args) const{
      int index=binning.GetBinIndex(mwg::stdm::forward<Args>(args)...);
      return index<0?dummy_bin:data[index];
    }
    template<typename... Args>
    bin_type& operator()(Args&&... args){
      int index=binning.GetBinIndex(mwg::stdm::forward<Args>(args)...);
      return index<0?dummy_bin:data[index];
    }
#else
    const bin_type& operator()(const domain_type& domain) const{
      int index=binning.GetBinIndex(domain);
      return index<0?dummy_bin:data[index];
    }
    bin_type& operator()(const domain_type& domain){
      int index=binning.GetBinIndex(domain);
      return index<0?dummy_bin:data[index];
    }
    template<typename T0,typename T1>
    const bin_type& operator()(const T0& b0,const T1& b1) const{
      int index=binning.GetBinIndex(b0,b1);
      return index<0?dummy_bin:data[index];
    }
    template<typename T0,typename T1>
    bin_type& operator()(const T0& b0,const T1& b1){
      int index=binning.GetBinIndex(b0,b1);
      return index<0?dummy_bin:data[index];
    }
    template<typename T0,typename T1,typename T2>
    const bin_type& operator()(const T0& b0,const T1& b1,const T2& b2) const{
      int index=binning.GetBinIndex(b0,b1,b2);
      return index<0?dummy_bin:data[index];
    }
    template<typename T0,typename T1,typename T2>
    bin_type& operator()(const T0& b0,const T1& b1,const T2& b2){
      int index=binning.GetBinIndex(b0,b1,b2);
      return index<0?dummy_bin:data[index];
    }
    template<typename T0,typename T1,typename T2,typename T3>
    const bin_type& operator()(const T0& b0,const T1& b1,const T2& b2,const T3& b3) const{
      int index=binning.GetBinIndex(b0,b1,b2,b3);
      return index<0?dummy_bin:data[index];
    }
    template<typename T0,typename T1,typename T2,typename T3>
    bin_type& operator()(const T0& b0,const T1& b1,const T2& b2,const T3& b3){
      int index=binning.GetBinIndex(b0,b1,b2,b3);
      return index<0?dummy_bin:data[index];
    }
#endif
  };
//fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
  template<typename TBin>
  Histogram<TBin,CountAccumulator<> > make_histogram(const TBin& binning){
    return Histogram<TBin,CountAccumulator<> >(binning);
  }
  template<typename TAcc,typename TBin>
  Histogram<TBin,TAcc> make_histogram(const TBin& binning){
    return Histogram<TBin,TAcc>(binning);
  }
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
