// -*- coding: cp932 -*-
#include <cstdio>
#include <type_traits>

// �Č����Ȃ�
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

// �Č����Ȃ�
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
// �ȉ��ōČ�����B
// mwg+functor_h.cpp �̓W�J���ʂ���ŏ����B
//
namespace test0 {
  // (0) original
  // template<typename F>
  // struct functor_traits_1: std::integral_constant<bool, std::is_function<typename std::remove_pointer<F>::type>::value> {};
  // template<typename F, typename S>
  // struct isF: std::integral_constant<bool, functor_traits_1<F>::value> {};

  // (1) ������g���Ă��Ȃ�Ȃ��B
  // template<typename F, typename S>
  // struct isF: std::true_type {};

  // (2) ������Ȃ�Ȃ�
  // template<typename F> struct always_true: std::true_type {};
  // template<typename F, typename S>
  // struct isF: std::integral_constant<bool, always_true<F>::value> {};

  // (3) ���ꂪ�Ȃ�!
  template<typename F>
  struct isF: std::is_function<typename std::remove_pointer<F>::type> {};

  struct C {
    C() {}
    //template<typename F> explicit C(const F& f) {} // ���ꂾ�ƂȂ�Ȃ��B

    // explicit �͍Č��ɕK�v�BisF �̃_�~�[�����ɓ����^���w�肷��K�v������B
    template<typename F> explicit C(const F& f, typename std::enable_if<isF<F>::value>::type* = nullptr) {}

    template<typename F> void operator=(const F&) {} // �߂�l�� void �ł��Č�����B
  };

  int func1() {return 123;}
  char func2() {return 1;}

  void run() {
    //C<int(int, int)> g(&func1);
    //C<int(int, char)> g(&func1); // �Ȃ�� int(int, char) ���ƍČ����Ȃ� �� ����������Č�����
    //C<void()> g; // ���ꂾ�ƂȂ�Ȃ��B
    //C g(&func1);
    C g; // ���̂����ꂾ�ƂȂ�
    std::printf("test0: isF = %d\n", isF<int (*)()>::value? 1: 0);
    g = func1;
    // g.operator=(func1); // �����o�֐��Ƃ��Ă̌Ăяo�����ƍČ����Ȃ��B
    // g = 0; // �Ⴄ�^���ƍČ����Ȃ��B
    // g = func2; // �Ⴄ�֐��^�ł��Č����Ȃ��B
    // tf(func1); // �e���v���[�g�֐��ɓn�������ł͂Ȃ�Ȃ��B
    // a = func1; // �ʂ̃N���X�ɑ�����Ă��Ȃ�Ȃ��B
    std::printf("test0: isF = %d\n", isF<int (*)()>::value? 1: 0);
  }
}

// �P����
namespace test1 {
  // msc17 �ɂ����ĖŒ��ꒃ�ȃo�O�ɓ��������B
  //
  // msc17 �ɂ����� functor.h �̃e�X�g�Ɏ��s����B
  // ������T���Ă����� msc17 ����̋��������鎖�ɋC�����B
  // �R�[�h���ǂ�ǂ����Ă����ƌ��ǈȉ��̗l�Ȍ`���ɂ܂ŗ����������B
  // ������Z�q�Ŋ֐��^�𒼐ڑ������ƁA�Ή�����֐��|�C���^�^�ɂ��Ă� be_functor �̋������ω�����B
  // explicit �ϊ��R���X�g���N�^�� SFINAE �_�~�[����������ꍇ�ɋN����B
  //
  // ���ۂ̃R�[�h�ł� C �� mwg::functor �ŁA
  // std::is_function<std::remove_pointer<F>::type> �� mwg::be_functor �ł������B
  // �Ώ��̕��@�͖����������Ă��Ȃ��B�Ⴆ�� is_function �����삵�Ă��������Ȃ����낤���B
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

// ���P����
namespace test2 {

  // ���� is_function �̑��ł͂Ȃ��� std::remove_pointer �̕��łȂ��Ă���l���B
  //
  // ���O�ŊȒP�� remove_pointer ��p�ӂ��Ă��Č�����B
  // �]���ĕW�����C�u������ std:remove_pointer �̃o�O�Ƃ͍l�����Ȃ��B
  // �֐��|�C���^�ɓ��������������ꉻ�▾���I���ꉻ��ǉ����Ă����͉������Ȃ��B

  template<typename T> struct remove_pointer {typedef T type;};
  template<typename T> struct remove_pointer<T*> {typedef T type;};
  template<typename R> struct remove_pointer<R (*)()> {typedef R (type)();}; // �֐��p�̕������ꉻ
  template<> struct remove_pointer<unsigned (*)()> {typedef unsigned (type)();}; // �����I���ꉻ���Ă��ʖ�

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
