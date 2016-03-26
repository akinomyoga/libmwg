#include <cstdio>
#include <mwg/std/type_traits>

template<typename T,typename U>
typename mwg::stdm::enable_if<!(mwg::stdm::is_same<T,U>::value),void>::type
read(U& value){}

// 回避方法
//
// template<typename T,typename U>
// typename mwg::stdm::enable_if<mwg::stdx::ice_not<mwg::stdm::is_same<T,U>::value>::value,void>::type
// read(U& value){}

int main(){
  short j;
  read<int>(j);
  return 0;
}
