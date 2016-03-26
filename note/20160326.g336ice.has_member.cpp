#include <cstdio>

template<class T,T mem> struct check_type{};

struct _true{char d[1];};
struct _false{char d[2];};

template<typename T>
struct has_print{
  struct Eval{
    template<typename X> static _true eval(check_type<void(X::*)(const char*),&X::print>*);

    // 回避方法
    // ?

    template<typename X> static _false eval(...);
  };
  static const int value=sizeof(Eval::template eval<T>(0))==sizeof(_true);
};

struct MyClass1{};
struct MyClass2{void print(const char*){}};
struct MyClass3{void print(const char*) const{}};

int main(){
  std::printf("%d (0)\n",has_print<int>::value);
  // std::printf("%d (0)\n",has_print<MyClass1>::value);
  // std::printf("%d (1)\n",has_print<MyClass2>::value);
  // std::printf("%d (0)\n",has_print<MyClass3>::value);
  return 0;
}
