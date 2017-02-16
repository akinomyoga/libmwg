#include <cassert>
#include <iterator>
#include <cstddef>

namespace case1 {
  struct base {
    int f(int a) const {return a;}
    int f() const {return 1;}
  };

  struct derived: base {
    char f() const {return 2;}
    using base::f; // want to inherit func(int)
  };

  void run() {
    derived instance;

    // compile error in msc17 msc18 msc19:
    //   both "char f() const" and "int f() const" participates in overload resolution
    // works with clang and gcc.
    assert(instance.f() == 2);

    // 解決方法: using base::f を char f() const の宣言よりも前に移動する。
  }
}

int main() {
  case1::run();
  return 0;
}
