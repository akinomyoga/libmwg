// -*- coding: cp932 -*-
#include <cstdio>
#include <type_traits>

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

void f() {}
int func1() {return 123;}
char func2() {return 1;}

int main() {
  //C<int(int, int)> g1(&func1);
  //C<int(int, char)> g1(&func1); // �Ȃ�� int(int, char) ���ƍČ����Ȃ� �� ����������Č�����
  //C<void()> g1; // ���ꂾ�ƂȂ�Ȃ��B
  //C g1(&func1);
  C g1; // ���̂����ꂾ�ƂȂ�
  std::printf("isF<ptr, fun> = %d\n", isF<void (*)()>::value? 1: 0);
  g1 = f;
  //g1 = func1;
  // g1.operator=(func1); // �����o�֐��Ƃ��Ă̌Ăяo�����ƍČ����Ȃ��B
  // g1 = 0; // �Ⴄ�^���ƍČ����Ȃ��B
  // g1 = func2; // �Ⴄ�֐��^�ł��Č����Ȃ��B
  // tf(func1); // �e���v���[�g�֐��ɓn�������ł͂Ȃ�Ȃ��B
  // a = func1; // �ʂ̃N���X�ɑ�����Ă��Ȃ�Ȃ��B
  std::printf("isF<ptr, fun> = %d\n", isF<void (*)()>::value? 1: 0);

  return 0;
}
