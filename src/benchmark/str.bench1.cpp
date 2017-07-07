#include <mwg/str.h>
#include <string>
#include <iostream>
#include "measure.h"

namespace string_bench {
  int test_compare1a() {
    int r = 0;
    if (mwg::str("hello") == "hello") r++;
    if (mwg::str("hello") == "world") r++;
    if (mwg::str("world") == "hello") r++;
    return r;
  }

  int test_compare1b() {
    int r = 0;
    if (std::string("hello") == "hello") r++;
    if (std::string("hello") == "world") r++;
    if (std::string("world") == "hello") r++;
    // if (std::string("hello") == "hello") r++;
    // if (std::string("hello") == "world") r++;
    // if (std::string("world") == "hello") r++;
    return r;
  }
}

int main() {
  int a = 0;

  {
    libmwg::scope_stopwatch sw("test_compare1a");
    for(int i = 0; i < 10000; i++)
      a += string_bench::test_compare1a();
  }

  {
    libmwg::scope_stopwatch sw("test_compare1b");
    for(int i = 0; i < 10000; i++)
      a -= string_bench::test_compare1b();
  }

  return a == 0? 0: 1;
}
