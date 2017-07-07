#ifndef LIBMWG_BENCHMARK_MEASURE_H
#define LIBMWG_BENCHMARK_MEASURE_H
#include <iostream>
#include <mwg/std/chrono>

namespace libmwg {

namespace chrono = mwg::stdm::chrono;

struct scope_stopwatch {
  struct set_base {};
  const char* message;
  chrono::high_resolution_clock::time_point start;
  scope_stopwatch(set_base const&    ): message(nullptr), start(chrono::high_resolution_clock::now()) {}
  scope_stopwatch(const char* message): message(message), start(chrono::high_resolution_clock::now()) {}
  ~scope_stopwatch() {
    static chrono::microseconds::rep base = 0;
    chrono::high_resolution_clock::time_point const finish = chrono::high_resolution_clock::now();
    chrono::microseconds const us = chrono::duration_cast<chrono::microseconds>(finish-start);
    if (message)
      std::cout << message << ": " << us.count() - base << "us\n";
    else
      base = us.count();
  }
  operator bool() const {return true;}
};
}
#endif
