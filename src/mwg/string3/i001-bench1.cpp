#include "i001.h"

namespace string_bench{
  int test_compare1(){
    int r=0;
    if(mwg::stradp<char>("hello")==mwg::stradp<char>("hello"))r++;
    if(mwg::stradp<char>("hello")==mwg::stradp<char>("world"))r++;
    if(mwg::stradp<char>("world")==mwg::stradp<char>("hello"))r++;
    return r;
  }
}
