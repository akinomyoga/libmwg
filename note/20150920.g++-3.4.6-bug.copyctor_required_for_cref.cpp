#include <cstdio>

//-----------------------------------------------------------------------------
// g++-3.4.6 では template friend が無効なのではないかと疑ったが、そうではない様だ。
// 以下は期待通りにコンパイルできる。

// namespace test1{
//   template<typename T>
//   struct A{
//   private:
//     void memberFunction(){}

//     template<typename X>
//     friend void run();
//   };

//   template<typename X>
//   void run(){
//     A<X> a;
//     a.memberFunction();
//   }
// }

//-----------------------------------------------------------------------------
// 戻り値に A<X> を指定するとエラーが出るのかと思ったけれどそうでもない様だ。
// 以下は期待通りにコンパイルできる。

// namespace test2{
//   template<typename T>
//   class A{
//     A(){}
//     A(A const&){}

//     template<typename X> friend void run1();
//     template<typename X> friend A<X> run2();
//   public:
//     void memberFunction() const{}
//   };

//   template<typename X>
//   void run1(){A<X> ret;return;}

//   template<typename X>
//   A<X> run2(){return A<X>();}

//   void test(){
//     test2::run1<int>();
//     test2::run2<int>().memberFunction();
//   }
// }

//-----------------------------------------------------------------------------
// 同じ構造に近づける。以下も動く。

// namespace test3{
//   template<typename X> struct _tuple{};

//   template<typename T>
//   class A{
//   private:
//     A(){}
//     A(A const& c){}
//     A& operator=(A const& c){}

//     template<typename X>
//     friend A<_tuple<X> > run();
//   public:
//     void memberFunction() const{}
//   };

//   template<typename X>
//   A<_tuple<X> >
//   run(){
//     typedef A<_tuple<X> > return_type;
//     return return_type();
//   }
// }

//-----------------------------------------------------------------------------
// もしかして別の所にあるのでは? と思って改めて元のコードを見ると
// operator<< を呼び出している。operator<< は A const& で引数を受け取る。

// namespace test4{
//   template<typename T>
//   class A{
//     A(){}
//     A(A const&){}

//     template<typename X> friend A<X> create();
//   public:
//     void memberFunction() const{}
//   };

//   template<typename X>
//   A<X> create(){return A<X>();}

//   void proc(A<int> const& value){
//     value.memberFunction();
//   }

//   void test(){
//     create<int>().memberFunction();
//     proc(create<int>());
//   }
// }

//-----------------------------------------------------------------------------
// 単純化
//   g++-3.4.6 で駄目。g++-3.3.6, g++-2.95, g++-4.5.4 などで OK。

namespace test5{
  class A{
    A(A const&){}
  public:
    A(){}
  };
  void proc(A const&){}
  void test(){proc(A());}
}

int main(){
  // test1::run<int>();
  // test2::test();
  // test3::run<int>().memberFunction();
  // test4::test();

  test5::test();
  return 0;
}
