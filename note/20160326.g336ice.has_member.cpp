#include <cstdio>

struct _true{char d[1];};
struct _false{char d[2];};
template<class T,T mem> struct check_type{typedef _true type;};

struct _dummy {void foo() {}};

namespace impl1 {
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
}

// 色々試した結果分かった事。
//
// 1. Eval 構造体に入れ子にしないと invalid use of undefined type と表示される。
// 2. enum {value = ... }; とするか static const bool value = とするかは関係ない。
// 3. どうやら check_type<..., ...> があるかないかで ICE かどうかが決まる様だ。
//
//
template<typename T>
struct has_print {
  struct Eval{
    // (1)
    // template<typename X>
    // static typename check_type<void (X::*)(const char*), &X::print>::type
    // eval(int) {return _true();}

    // (2)
    // typedef void (T::*memfun_t)(const char*); // T がクラスでないと此処で失敗する
    // template<memfun_t> struct accept {typedef _true type;};
    // template<typename X>
    // static typename accept<&X::print>::type // T がクラスでも此処で失敗する。
    // eval(int) {return _true();}

    // (3)
    // template<int> struct accept1 {typedef _true type;};
    // template<typename X>
    // static typename accept1<sizeof(X)>::type // invalid application of sizeof to an incomplete type
    // eval(int) {return _true();}

    // (4)
    // // invalid application of sizeof to an incomplete type
    // template<int> struct accept1 {typedef _true type;};
    // template<typename X>
    // static _true eval(typename accept1<sizeof(X)>::type*) {return _true();}

    // (5)
    // template<typename X> struct accept_type {typedef _true type; static const int size = sizeof(X);};
    // template<typename X> static _true eval(int (*)[accept_type<X>::size]) {return _true();}

    // // (6) void (X::*)(const char*) が存在し得るかどうかの判定は大丈夫の様だ。
    // template<typename X> static _true eval(void (X::*)(const char*)) {return _true();}

    // // (7) メンバ関数型をテンプレート実引数に指定するのも問題ない。
    // // 結局 &X::print という並びが現れた時点でもう駄目ということなのか。
    // template<void (_dummy::*)()> struct accept {typedef _true type;};
    // template<typename X> static typename accept<&_dummy::foo>::type eval(void (X::*)(const char*)) {return _true();}

    template<typename X> static _false eval(...) {return _false();}
  };
  static const bool value = (sizeof(Eval::template eval<T>(0)) == sizeof(_true));
  //enum { value = (sizeof(Eval::template eval<T>(0)) == sizeof(_true)) };
};

struct MyClass1{};
struct MyClass2{void print(const char*){}};
struct MyClass3{void print(const char*) const{}};

int main(){
  //std::printf("%d (0)\n",has_print<int>::value);
  std::printf("%d (0)\n",has_print<MyClass1>::value);
  // std::printf("%d (1)\n",has_print<MyClass2>::value);
  // std::printf("%d (0)\n",has_print<MyClass3>::value);
  return 0;
}
