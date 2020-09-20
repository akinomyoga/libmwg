// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_STAT_BINNING2_H
#define MWG_STAT_BINNING2_H
#include <cstdlib>
#include <vector>
#include <mwg/std/type_traits>
#include <mwg/std/initializer_list>
#include <mwg/std/utility>
#include <mwg/concept.h>
#include <mwg/range.h>
#include <mwg/except.h>
#pragma%include "mwg_concept.hpp"
namespace mwg {
namespace stat {
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

  class bindex {
    int m_value;
  public:
    bindex(): m_value(0) {}
    explicit bindex(int value): m_value(value) {}

    mwg_explicit_operator int() const { return this->m_value; }

    bindex& operator++() { ++this->m_value; return *this; }
    bindex& operator--() { --this->m_value; return *this; }
    bindex operator++(int) { return bindex(this->m_value++); }
    bindex operator--(int) { return bindex(this->m_value--); }
    bindex operator+(int delta) const { return bindex(this->m_value+delta); }
    bindex operator-(int delta) const { return bindex(this->m_value-delta); }
    bindex operator+=(int delta) { this->m_value += delta; return *this; }
    bindex operator-=(int delta) { this->m_value -= delta; return *this; }

    int operator-(const bindex& right) const { return this->m_value - right.m_value; }
  };
#pragma%m bindex::relational_operator
  inline bool operator OP(const bindex& left, const bindex& right) { return (int) left OP (int) right; }
  template<typename T>
  inline typename stdm::enable_if<stdm::is_arithmetic<T>::value, bool>::type
  operator OP(const bindex& left, T const& right) { return (int) left OP right; }
  template<typename T>
  inline typename stdm::enable_if<stdm::is_arithmetic<T>::value, bool>::type
  operator OP(T const& left, const bindex& right) { return left OP (int) right; }
#pragma%end
#pragma%x bindex::relational_operator.r/OP/</
#pragma%x bindex::relational_operator.r/OP/>/
#pragma%x bindex::relational_operator.r/OP/<=/
#pragma%x bindex::relational_operator.r/OP/>=/
#pragma%x bindex::relational_operator.r/OP/==/
#pragma%x bindex::relational_operator.r/OP/!=/

  // explicit bindex(int) にする事の利点と欠点
  // ○ int 値と bindex 値が混ざる事を防ぐ事ができる。
  //  例: 以下で f(bindex) が呼び出されるのを防ぐ。
  //    void f(double);
  //    void f(bindex);
  //    f(1);
  // × hist(mwg::stat::bindex(part.GetBinNumber() - 1)) 等と書かなければならない。面倒

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  equal_binning : !TBinning1Dim
//-----------------------------------------------------------------------------
  template<typename T>
  class equal_binning {
    int nbin;
    T m_lower;
    T m_upper;
  public:
    typedef T domain_type;
    equal_binning(): nbin(1), m_lower(0), m_upper(1) {}
    equal_binning(int nbin, const T& m_lower, const T& m_upper): nbin(nbin), m_lower(m_lower), m_upper(m_upper) {}
  public:
    std::size_t size() const {
      return nbin;
    }
    bindex operator()(const T& value) const {
      if (value < m_lower || m_upper <= value) return bindex(-1);
      return bindex(int(nbin * (value - m_lower) / (m_upper - m_lower)));
    }
    T width(bindex index) const {
      return (m_upper - m_lower) / nbin;
    }
    T floor(bindex index) const {
      return m_lower + (m_upper - m_lower) / nbin * (int) index;
    }
    T ceil(bindex index) const {
      return m_lower + (m_upper - m_lower) / nbin * ((int) index + 1);
    }
    T representative(bindex index) const {
      //std::fprintf(stderr, "dbg: GetBinMeanValue: m_lower=%lf m_upper=%lf nbin=%d index=%d\n", m_lower, m_upper, nbin, index);
      return m_lower + (m_upper - m_lower) * ((int) index + (T) 0.5) / nbin;
    }
  };

  template<typename T>
  class list_binning {
    std::vector<T> data;
  public:
    typedef T domain_type;
    list_binning(std::vector<T> const& data): data(data) {}
#if mwg_has_feature(cxx_rvalue_references)
    list_binning(std::vector<T>&& data): data(mwg::stdm::move(data)) {}
#endif
    list_binning(stdm::initializer_list<T> list) {
      for (typename stdm::initializer_list<T>::iterator i = list.begin(); i != list.end(); ++i)
        data.push_back(mwg::stdm::move(*i));
    }
  public:
    std::size_t size() const { return this->data.size() - 1; }
    bindex operator()(const T& value) const {
      typename std::vector<T>::const_iterator i = std::upper_bound(data.begin(), data.end(), value);
      return bindex(i == data.begin() || i == data.end() ? -1 : i - data.begin() - 1);
    }
  public:
    T floor(bindex index) const {
      mwg_assert(0 <= (int) index || (int) index < this->size());
      return data[(int) index];
    }
    T ceil(bindex index) const {
      mwg_assert(0 <= (int) index || (int) index < this->size());
      return data[(int) index+1];
    }
    T width(bindex index) const {
      mwg_assert(0 <= (int) index || (int) index < this->size());
      return data[(int) index+1] - data[(int) index];
    }
    T representative(bindex index) const {
      mwg_assert(0 <= (int) index || (int) index < this->size());
      return (T) 0.5 * (data[(int) index + 1] + data[(int) index]);
    }
  };

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  TBinning
//-----------------------------------------------------------------------------
  template<typename T=double>
  struct binning_wrapper;
  template<typename T=double>
  struct binning1dim_wrapper;

  template<typename T=double>
  class equal_binning;
  template<typename T=double>
  class list_binning;

#pragma%m ARG_FOR_MEMBER (
#pragma%%x t.r/%name%/domain_type/
#pragma%%x f.r/%decl%/bindex operator()(const T\& v) const/.r/%name%/operator()/ .r/%args%/v/
#pragma%%x f.r/%decl%/std::size_t size() const/            .r/%name%/size/       .r/%args%//
#pragma%)
#pragma%[ARG_TMPL_PARAMS="typename T"]
#pragma%[ARG_TMPL_PARAMS_FOR_CONCEPT="typename T=typename T_::domain_type"]
#pragma%[ARG_TMPL_ARGS="T"]
#pragma%m wrapper_content (
public:
  typedef typename mwg::identity<T>::type domain_type;
  binning_wrapper():h(new holder<mwg::stat::equal_binning<> >(mwg::stat::equal_binning<>(T(), T(), T()))) {}
#pragma%)
#pragma%x mwg_concept_create_wrapper.r/%TConcept%Wrapper/binning_wrapper/.r/%TConcept%/binning_concept/
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
#pragma%m ARG_FOR_MEMBER (
//#pragma%%x C.r/%value%/binning_concept<T_, T>::value/
#pragma%%x t.r/%name%/domain_type/
#pragma%%x f.r/%decl%/bindex operator()(const T\& v) const/ .r/%name%/operator()/     .r/%args%/v/
#pragma%%x f.r/%decl%/std::size_t size() const/             .r/%name%/size/           .r/%args%//
#pragma%%x f.r/%decl%/T width(bindex index) const/          .r/%name%/width/          .r/%args%/index/
#pragma%%x f.r/%decl%/T floor(bindex index) const/          .r/%name%/floor/          .r/%args%/index/
#pragma%%x f.r/%decl%/T ceil(bindex index) const/           .r/%name%/ceil/           .r/%args%/index/
#pragma%%x f.r/%decl%/T representative(bindex index) const/ .r/%name%/representative/ .r/%args%/index/
#pragma%)
#pragma%[ARG_TMPL_PARAMS="typename T"]
#pragma%[ARG_TMPL_PARAMS_FOR_CONCEPT="typename T=typename T_::domain_type"]
#pragma%[ARG_TMPL_ARGS="T"]
#pragma%m wrapper_content (
public:
  typedef typename mwg::identity<T>::type domain_type;
  binning1dim_wrapper(): h(new holder<mwg::stat::equal_binning<> >(mwg::stat::equal_binning<>(T(), T(), T()))) {}
#pragma%)
#pragma%x mwg_concept_create_wrapper.r/%TConcept%Wrapper/binning1dim_wrapper/.r/%TConcept%/binning1dim_concept/
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}

  template<typename B>
  typename mwg::stdm::enable_if<mwg::stat::binning1dim_concept<B>::value, typename B::domain_type>::type
  floor(B const& bin, mwg::stat::bindex index) { return bin.floor(index); }

  template<typename B>
  typename mwg::stdm::enable_if<mwg::stat::binning1dim_concept<B>::value, typename B::domain_type>::type
  ceil(B const& bin, mwg::stat::bindex index) { return bin.ceil(index); }

  template<typename B>
  typename mwg::stdm::enable_if<mwg::stat::binning1dim_concept<B>::value, mwg::range<typename B::domain_type> >::type
  make_range(B const& bin, mwg::stat::bindex index) {
    return mwg::range<typename B::domain_type>(mwg::floor(bin, index), mwg::ceil(bin, index));
  }

  template<typename B>
  typename mwg::stdm::enable_if<mwg::stat::binning1dim_concept<B>::value, typename B::domain_type>::type
  floor(B const& bin) { return bin.floor((mwg::stat::bindex) 0); }

  template<typename B>
  typename mwg::stdm::enable_if<mwg::stat::binning1dim_concept<B>::value, typename B::domain_type>::type
  ceil(B const& bin) { return bin.ceil((mwg::stat::bindex) (bin.size() - 1)); }

  template<typename B>
  typename mwg::stdm::enable_if<mwg::stat::binning1dim_concept<B>::value, mwg::range<typename B::domain_type> >::type
  make_range(B const& bin) {
    return mwg::range<typename B::domain_type>(mwg::floor(bin), mwg::ceil(bin));
  }

namespace stat {
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  TBinningIterator
//-----------------------------------------------------------------------------
#if 0 /* TODO */
  template<typename B>
  struct TBinningIterator {
    const B& bin;
    bindex index;
  public:
    TBinningIterator(const B& bin, bindex index): bin(bin), index(index) {}
    T representative() const { return bin.representative(index); }
    T floor() const { return bin.floor(index); }
    T ceil() const { return bin.ceil(index); }
    T width() const { return bin.width(index); }
    mwg::range<T> range() const { return mwg::make_range(bin, index); }
    T operator*() const { return this->representative()}
  };
}

  template<typename B>
  typename mwg::stdm::enable_if<mwg::stat::binning1dim_concept<B>::value, mwg::stat::TBinningIterator<B> >::type
  begin(B const& bin) {
    return mwg::stat::TBinningIterator(bin, (bindex) 0);
  }
  template<typename B>
  typename mwg::stdm::enable_if<mwg::stat::binning1dim_concept<B>::value, mwg::stat::TBinningIterator<B> >::type
  end(B const& bin) {
    return mwg::stat::TBinningIterator(bin, (bindex) bin.size());
  }

namespace stat {
#endif
}
}
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  ProductBinning : !TBinning
//-----------------------------------------------------------------------------
#if mwg_has_feature(cxx_variadic_templates)
# include "binning2.ProductBinning.inl"
#else
# include "binning2.ProductBinning_nonvariadic.inl"
#endif
//-----------------------------------------------------------------------------
#endif
