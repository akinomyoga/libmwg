// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_STAT_ACCUMULATOR_H
#define MWG_STAT_ACCUMULATOR_H
#include <cmath>
#include <mwg/std/type_traits>
#include "errored.h"
namespace mwg{
namespace stat{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  template<typename T = double>
  class count_accumulator;
  template<typename T = double>
  class average_accumulator;

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  template<typename T>
  class count_accumulator {
    T sw1;
    T sw2;
  public:
    typedef T value_type;
    count_accumulator() { this->clear(); }
    void clear() {
      this->sw1 = 0.0;
      this->sw2 = 0.0;
    }
    count_accumulator& operator()(const T& weight) {
      sw1 += weight;
      sw2 += weight * weight;
      return *this;
    }
    count_accumulator& operator()() {
      return this->operator()(1);
    }
  public:
    T count() const { return sw1; }
    T error() const { return std::sqrt(sw2); }
    mwg_explicit_operator mwg::stat::errored<T>() const {
      return mwg::stat::errored<T>(sw1, sw2, mwg::stat::variance_tag);
    }
  };
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT

  template<typename T>
  class average_accumulator {
    std::size_t s0;
    T s1;
    T s2;
  public:
    typedef T value_type;
    typedef T result_type;
  public:
    average_accumulator() { this->clear(); }
    void clear() {
      this->s0 = 0;
      this->s1 = 0.0;
      this->s2 = 0.0;
    }
    average_accumulator& operator()(const T& value) {
      s0++;
      s1 += value;
      s2 += value * value;
      return *this;
    }
  public:
    T biased_variance() const {
      if (s0 == 0) return 0;
      return s2 / s0 - (s1 / s0) * (s1 / s0);
    }
    T unbiased_variance() const {
      if (s0 <= 1) return 0;
      return T(s0) / (s0 - 1) * this->biased_variance();
    }
    T biased_standard_deviation() const {
      return std::sqrt(this->biased_variance());
    }
    T unbiased_standard_deviation() const {
      return std::sqrt(this->unbiased_variance());
    }
  public:
    errored<T> average_errored() const {
      return errored<T>(s0 == 0 ? 0 : s1 / s0, s0 <= 1 ? 0 : biased_variance() / (s0 - 1), mwg::stat::variance_tag);
    }
    errored<T> sum_errored() const {
      return errored<T>(s1, s0 <= 1 ? 0 : s0 * s0 * biased_variance() / (s0 - 1), mwg::stat::variance_tag);
    }
    T average()       const { return s0 == 0 ? 0 : s1 / s0; }
    T average_error() const { return s0 <= 1 ? 0 : std::sqrt(this->biased_variance() / (s0 - 1)); }
    T sum()           const { return s1; }
    T sum_error()     const { return s0 * this->average_error(); }
  public:
    mwg_explicit_operator mwg::stat::errored<T>() const {
      return this->average_errored();
    }
  };

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
