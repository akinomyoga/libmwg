// -*- coding: cp932 -*-
#include <cstdio>
#include <type_traits>

// 再現しない
namespace case1 {
  template<typename T>
  typename std::enable_if<std::is_function<T>::value>::type
  f(const T&) {}

  template<typename T>
  typename std::enable_if<!std::is_function<T>::value>::type
  f(const T&) {}

  void g() {}
  void run() {
    f(&g);
    f(g);
  }
}

// 再現しない
namespace case2 {

  template<typename F, typename S> struct always_true {static const bool value = true;};
  template<typename F, typename S> struct always_false {static const bool value = false;};

  template<typename F, typename S> struct is_func: std::false_type {};
  template<typename R, typename S> struct is_func<R (), S>: std::true_type {};
  template<typename R, typename S> struct is_func<R (*)(), S>: std::true_type {};
  template<typename F, typename S> struct is_nofunc: std::integral_constant<bool, !is_func<F, S>::value> {};

  struct invalid_type {};

  template<typename S>
  struct functor {
  protected:
    functor() {}

  public:
    template<typename F>
    functor(const F& f, typename std::enable_if<is_func<F, S>::value, invalid_type*>::type = nullptr) {}

    template<typename F>
    explicit functor(const F& f, typename std::enable_if<is_nofunc<F, S>::value, invalid_type*>::type = nullptr) {}

    template<typename F>
    //typename std::enable_if<is_func<F, S>::value, functor&>::type
    functor&
    operator=(const F& f) {return *this;}

    ~functor() {}

    functor(const functor& f) {}
    functor& operator=(const functor& f) {return *this;}
    functor(functor&& f) {}
    functor& operator=(functor&& f) {return *this;}

    void swap(functor& rhs) {}
  };

  int g() {return 0;}

  void run() {
    functor<int (int, int)> f1(&g);
    f1 = g;
  }
}

//
// 以下で再現する。
// mwg+functor_h.cpp の展開結果から最小化。
//
namespace test0 {
  // (0) original
  // template<typename F>
  // struct functor_traits_1: std::integral_constant<bool, std::is_function<typename std::remove_pointer<F>::type>::value> {};
  // template<typename F, typename S>
  // struct isF: std::integral_constant<bool, functor_traits_1<F>::value> {};

  // (1) これを使ってもならない。
  // template<typename F, typename S>
  // struct isF: std::true_type {};

  // (2) これもならない
  // template<typename F> struct always_true: std::true_type {};
  // template<typename F, typename S>
  // struct isF: std::integral_constant<bool, always_true<F>::value> {};

  // (3) これがなる!
  template<typename F>
  struct isF: std::is_function<typename std::remove_pointer<F>::type> {};

  struct C {
    C() {}
    //template<typename F> explicit C(const F& f) {} // これだとならない。

    // explicit は再現に必要。isF のダミー引数に同じ型を指定する必要がある。
    template<typename F> explicit C(const F& f, typename std::enable_if<isF<F>::value>::type* = nullptr) {}

    template<typename F> void operator=(const F&) {} // 戻り値は void でも再現する。
  };

  int func1() {return 123;}
  char func2() {return 1;}

  void run() {
    //C<int(int, int)> g(&func1);
    //C<int(int, char)> g(&func1); // なんと int(int, char) だと再現しない → 今試したら再現する
    //C<void()> g; // これだとならない。
    //C g(&func1);
    C g; // 何故かこれだとなる
    std::printf("test0: isF = %d\n", isF<int (*)()>::value? 1: 0);
    g = func1;
    // g.operator=(func1); // メンバ関数としての呼び出しだと再現しない。
    // g = 0; // 違う型だと再現しない。
    // g = func2; // 違う関数型でも再現しない。
    // tf(func1); // テンプレート関数に渡すだけではならない。
    // a = func1; // 別のクラスに代入してもならない。
    std::printf("test0: isF = %d\n", isF<int (*)()>::value? 1: 0);
  }
}

// 単純化
namespace test1 {
  // msc17 において滅茶苦茶なバグに当たった。
  //
  // msc17 において functor.h のテストに失敗する。
  // 原因を探っていくと msc17 が謎の挙動をする事に気がつく。
  // コードをどんどん削っていくと結局以下の様な形式にまで落ち着いた。
  // 代入演算子で関数型を直接代入すると、対応する関数ポインタ型についての be_functor の挙動が変化する。
  // explicit 変換コンストラクタの SFINAE ダミー引数がある場合に起こる。
  //
  // 実際のコードでは C が mwg::functor で、
  // std::is_function<std::remove_pointer<F>::type> は mwg::be_functor であった。
  // 対処の方法は未だ分かっていない。例えば is_function を自作しても解決しないだろうか。
  //

  struct C {
    C() {}
    template<typename F> explicit C(const F& f, typename std::enable_if<std::is_function<typename std::remove_pointer<F>::type>::value>::type* = nullptr) {}
    template<typename F> void operator=(const F&) {}
  };

  void f() {}

  void run() {
    std::printf("test1: is_function = %d\n", std::is_function<std::remove_pointer<void (*)()>::type>::value? 1: 0); // is_functino = 1 (ok)
    C g;
    g = f;
    std::printf("test1: is_function = %d\n", std::is_function<std::remove_pointer<void (*)()>::type>::value? 1: 0); // is_function = 0 @ msc17 (VS2012)
    std::printf("test1:  \\_ std::is_function -> %s\n", std::is_function<void ()>::value? "ok": "ng"); // ok
    std::printf("test1:  \\_ std::remove_pointer -> %s\n", std::is_same<std::remove_pointer<void (*)()>::type, void ()>::value? "ok": "ng"); // ng
  }
}

// より単純化
namespace test2 {

  // 実は is_function の側ではなくて std::remove_pointer の方でなっている様だ。
  //
  // 自前で簡単な remove_pointer を用意しても再現する。
  // 従って標準ライブラリの std:remove_pointer のバグとは考えられない。
  // 関数ポインタに特化した部分特殊化や明示的特殊化を追加しても問題は解消しない。

  template<typename T> struct remove_pointer {typedef T type;};
  template<typename T> struct remove_pointer<T*> {typedef T type;};
  template<typename R> struct remove_pointer<R (*)()> {typedef R (type)();}; // 関数用の部分特殊化
  template<> struct remove_pointer<unsigned (*)()> {typedef unsigned (type)();}; // 明示的特殊化しても駄目

  struct C {
    C() {}
    template<typename F> explicit C(const F& f, typename remove_pointer<F>::type* = nullptr) {}
    template<typename F> void operator=(const F&) {}
  };

  unsigned f() {return 0;}

  template<typename> void check_type() {
    std::printf("test2: remove_pointer -> %s\n", __FUNCSIG__);
  }

  void run() {
    check_type<remove_pointer<unsigned (*)()>::type>(); // void __cdecl test2::check_type<unsigned int(void)>(void)
    C g;
    g = f;
    check_type<remove_pointer<unsigned (*)()>::type>(); // void __cdecl test2::check_type<unsigned int(__cdecl *)(void)>(void)
  }

}

int main() {
  case1::run();
  case2::run();
  test0::run();
  test1::run();
  test2::run();
  return 0;
}
