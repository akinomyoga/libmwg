#include <mwg/string.h>

namespace string_bench{
  int test_compare1(){
    int r=0;
    if(std::string("hello")==std::string("hello"))r++;
    if(std::string("hello")==std::string("world"))r++;
    if(std::string("world")==std::string("hello"))r++;
    // if(std::string("hello")=="hello")r++;
    // if(std::string("hello")=="world")r++;
    // if(std::string("world")=="hello")r++;
    return r;
  }
}
