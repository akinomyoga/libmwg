
namespace test1{
  struct A{
    typedef int my_type;
  };

  struct B:A{
    my_type x;
  };

  void test(){
    B obj;
  }
}

namespace test2{
  template<typename T>
  struct A{
    typedef int my_type;
  };

  struct B:A<int>{
    my_type x;
  };

  void test(){
    B obj;
  }
}

namespace test3{
  template<typename T>
  struct A{
    typedef int my_type;
  };

  template<typename T>
  struct B:A<T>{
    // A<T>::my_type は依存名なので、using なしでは中からは使えない。
    using typename A<T>::my_type;

    my_type x;
  };

  void test(){
    B<int> obj;
  }
}
