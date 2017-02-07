// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_FUNCTOR2_H
#define MWG_FUNCTOR2_H
#pragma%include "../../impl/VariadicMacros.pp"
#pragma%[ArN=10]
#pragma%x begin_check
#include <mwg/exp/functor2/impl1.h>
#include <mwg/except.h>
#pragma%x end_check
#include <mwg/std/type_traits>
#include <mwg/std/utility>
#include <mwg/funcsig.h>
#pragma%include "../../bits/functor.type_traits.pp"
namespace mwg {
namespace funcsig {

  namespace detail {
    template<typename S, std::size_t N, template<typename A> class Filter>
    struct filtered_rotate: filtered_rotate<typename filtered_rotate<S, 1, Filter>::type, N - 1, Filter> {};
    template<template<typename A> class Filter, typename S>
    struct filtered_rotate<S, 0, Filter>: mwg::identity<S> {};
#pragma%m 1
    template<template<class A> class Filter, class R, class Head, class... A>
    struct filtered_rotate<R (Head, A...), 1, Filter>: mwg::identity<R(A..., typename Filter<Head>::type)> {};
#pragma%end
#pragma%x variadic_expand_0toArNm1
  }

  template<typename S, std::size_t N = 1>
  struct rotate: detail::filtered_rotate<S, N % arity<S>::value, mwg::identity> {};
  template<typename S, template<typename A> class Filter>
  struct filter: detail::filtered_rotate<S, arity<S>::value, Filter> {};

}
namespace functor_detail {

  enum invokation_type {
    invokation_function_call,
    invokation_member_object,
    invokation_member_function
  };

#if defined(MWGCONF_STD_RVALUE_REFERENCES)
  template<typename T>
  T&& fwd(typename stdm::remove_reference<T>::type& value){
    return static_cast<T&&>(value);
  }
  template<typename T>
  T&& fwd(typename stdm::remove_reference<T>::type&& value){
    return static_cast<T&&>(value);
  }

  template<typename S, int Index>
  typename sig::parameter<Index, S>::type&&
  fwdp(typename stdm::remove_reference<typename sig::parameter<Index, S>::type>::type& value){
    return static_cast<typename sig::parameter<Index, S>::type&&>(value);
  }
  template<typename S, int Index>
  typename sig::parameter<Index, S>::type&&
  fwdp(typename stdm::remove_reference<typename sig::parameter<Index, S>::type>::type&& value){
    return static_cast<typename sig::parameter<Index, S>::type&&>(value);
  }
#else
  template<typename T>
  struct result_of_fwd: stdm::conditional<
    stdm::is_reference<T>::value,
    typename stdm::add_lvalue_reference<T>::type,
    typename stdx::add_const_reference<T>::type
    > {};
  template<typename T>
  typename result_of_fwd<T>::type
  fwd(typename result_of_fwd<T>::type value) {return value;}

  template<typename S, int Index>
  struct result_of_fwdp:
    result_of_fwd<typename sig::parameter<Index, S>::type> {};
  template<typename S, int Index>
  typename result_of_fwdp<S, Index>::type
  fwdp(typename result_of_fwdp<S, Index>::type value) {return value;}
#endif
#define mwg_rfwd mwg_forward_rvalue

  namespace sig = mwg::funcsig;

/*?lwiki
 * 最終的に呼び出す関数のシグニチャは決まっている (既知である)。
 * 複数のパターンが存在する。
 *
 * 1. 最終的に呼び出すのは `R(A...)` だが、余分な引数 `R(A..., B...)` のインターフェイスを持つ。
 *
 *    この場合は最初に受取る時は `(A... a, B... b)` で受け取って `b` は無視することになる。
 *    %%実際には最初に受け取る時点で `(A... a, ...)` で宣言しておけば良いのである?%%
 *    最初から `...` を使ってしまうと変な引数を与えても OK になってしまうので駄目だ。
 *    という訳で最初に受け取る時は矢張り `(A... a, B... b)` でなければならない。
 *
 *    最初に受け取る時に引数の数を減少させる作戦だと、
 *    何個引数を減少させるかで沢山の可能性があるから多重定義が沢山になって現実的でない。
 *    従って、最終的に呼び出される側で適宜引数を削って受け取るのが良い。
 *
 * 2. 最終的に呼び出すのは `R(A...)` で可変長の引数 `R(A..., B..., ...)` を持つ。
 * 3. 最終的に呼び出すのは `R(A..., ...)` で固定の引数 `R(A..., B...)` を持つ。
 * 4. 最終的に呼び出すのは `R(A..., ...)` で可変長の引数 `R(A..., B..., ...)` を持つ。
 *
 * どうも、受け取る時点と実行する時点で問題を分割するべきな気がする。
 * 受け取る側 (インターフェイスを公開) は二種類の可能性がある。
 *
 * A1. 固定長の引数: これは既知の引数の型で呼び出して既知の戻り値の型で返すだけ
 *
 * A2. 可変長の引数:
 *
 *     次の関数に引数をそのまま転送しなければならないので、
 *     可変長部分に関してはテンプレートで可変長長さに応じた多重定義を用意する必要がある。
 *     これが今記述していた部分である。この階層では受け取った引数を全て次の階層へ渡す。
 *
 * 実行する側では矢張り二種類の可能性がある。
 *
 * B1. 固定長の引数
 *
 *     これに関しては、余分の引数は `...` で受け取って無視してしまえば良い。
 *     というか無視する必要がある。
 *
 * B2. 可変長の引数
 *
 *     これは更に実際に実行する処理の場所へ転送しなければならない。
 *     既知の型の部分に関しては既知の型で受け取って、
 *     余分の部分に関しては `const&` または `&&` による転送を行う。
 *
 *     実は受取に用いた型は A1/A2 が知っているので、
 *     其処から貰った情報を使えば `const&` や `&&` による転送の自動判定は不要である。
 *     従って処理を簡略化出来る筈である。
 *
 * この様に考えてみると受け取り側と実行側で処理を統一するのは得策ではない。
 * 受け取り側は `functor_interface` として、
 * 実行側は `functor_invoker` などとする。
 *
 */

  template<typename S, typename CRTP>
  struct functor_interface {};
#pragma%m 1
  template<class CRTP, class R, class... A>
  struct functor_interface<R (A...), CRTP>: CRTP {
    template<class F> functor_interface(F const& func): CRTP(func) {}
    R operator()(A... a) const {
      return CRTP::template forward<R(A...)>(fwd<A>(a)...);
    }
  };
#pragma%end
#pragma%x variadic_expand_0toArN

#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
  template<class CRTP, class R, class... A>
  struct functor_interface<R (A..., ...), CRTP>: functor_interface<R (A...), CRTP> {
    typedef functor_interface<R (A...), CRTP> base;
    template<class F> functor_interface(F const& func): base(func) {}
    template<class... B>
    R operator()(A... a, B mwg_rfwd... b) const {
      return CRTP::template forward<R(A..., B mwg_rfwd...)>(fwd<A>(a)..., fwd<B>(b)...);
    }
  };
#else
#pragma%[PROTECT = "$"]
#pragma%m 1
  template<class CRTP, class R, class... A>
  struct functor_interface<R (A..., ...), CRTP>: functor_interface<R (A...), CRTP> {
    typedef functor_interface<R (A...), CRTP> base;
    template<class F> functor_interface(F const& func): base(func) {}
    using base::operator();
#pragma%%x
    template<$"PROTECT"".for/%K/__arity__/__arity2__/class B%K/,">
    R operator()(A... a,$"PROTECT"".for/%K/__arity__/__arity2__/B%K mwg_rfwd b%K/,") const {
      return CRTP::template forward<R(A...,$"PROTECT"".for/%K/__arity__/__arity2__/B%K mwg_rfwd/,")>
        (fwd<A>(a)...,$"PROTECT"".for/%K/__arity__/__arity2__/fwd<B%K>(b%K)/,");
    }
#pragma%%end.f/__arity2__/__arity__+1/ArN+1/
  };
#pragma%end
#pragma%x
#pragma%x variadic_expand::with_arity.f/__arity__/0/ArN+1/
#pragma%end.i
#endif

  //
  //
  // struct functor_invoker<Invokation, S, CRTP>;
  //
  //

  template<int Invokation, typename S, typename CRTP>
  struct functor_invoker_selector {};

  template<typename S, typename XS, int Arity>
  struct enable_forward: stdm::enable_if<
    (Arity == (is_vararg_function<S>::value? sig::arity<XS>::value: sig::arity<S>::value)),
    typename sig::returns<XS>::type> {};

  template<typename XS, int Index>
  struct param: sig::parameter<Index, XS> {};


#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
  template<typename S, typename CRTP>
  struct functor_invoker_function_call {};
  template<class CRTP, class R, class... A>
  struct functor_invoker_function_call<R (A...), CRTP>: CRTP {
    template<class F> functor_invoker_function_call(F const& func): CRTP(func) {}
    template<class XS>
    typename sig::returns<XS>::type
    forward(typename stdx::add_const_reference<A>::type... a, ...) const {
      return (CRTP::get())(fwd<A>(a)...);
    }
  };
  template<class CRTP, class R, class... A>
  struct functor_invoker_function_call<R (A..., ...), CRTP>: CRTP {
    template<class F> functor_invoker_function_call(F const& func): CRTP(func) {}
    template<class XS, class... B>
    typename sig::returns<XS>::type
    forward(typename stdx::add_const_referece<A>::type... a, B mwg_rfwd... b) const {
      return (CRTP::get())(fwd<A>(a)..., fwd<B>(b)...);
    }
  };
#else
  template<class S, class CRTP>
  struct functor_invoker_function_call: CRTP {
    template<class F> functor_invoker_function_call(F const& func): CRTP(func) {}
#pragma%m 1
    template<class XS>
    typename enable_forward<S, XS, __arity__>::type
    forward(typename param<XS,>::type... a, ...) const {
      return (CRTP::get())(fwdp<XS,>(a)...);
    }
#pragma%end
#pragma%x variadic_expand::with_arity.f/__arity__/0/ArN+1/
  };
#endif

  template<typename S, typename CRTP>
  struct functor_invoker_selector<invokation_function_call, S, CRTP>:
    mwg::identity<functor_invoker_function_call<S, CRTP> > {};


  template<typename S, typename CRTP>
  struct functor_invoker_member_object {};
  template<typename T, typename C, typename CRTP>
  struct functor_invoker_member_object<T&(C&), CRTP>: CRTP {
    template<typename F> functor_invoker_member_object(F const& func): CRTP(func) {}
    template<typename XS>
    typename sig::returns<XS>::type
    forward(C& obj, ...) const {return obj.*CRTP::get();}
  };

  template<typename S, typename CRTP>
  struct functor_invoker_selector<invokation_member_object, S, CRTP>:
    mwg::identity<functor_invoker_member_object<S, CRTP> > {};


  /*?lwiki
   *
   * 次に as_functor の実装について考える。
   * 取り敢えず関数の呼び出しは既にできるという事が分かった。
   * variant_functor についても実は呼び出しができるという事がわかった。
   * 今後の実装で何が必要になるだろうか。
   *
   * - 一つがその関数オブジェクトの自然な関数型である。
   *
   *   実際に関数呼出しを実行することができるかどうかは、
   *   この関数型を経由することによって判定できる。
   *
   *   但し、多重定義のメンバ関数などの場合には、
   *   この自然な関数型という物を取得できないが、
   *   要求する関数型を指定されればそれに適合するかどうか判定できるという種類の物もある。
   *   そう言った物に関しては別に指定を行う必要がある。
   *
   * - また functor に入れる為・adapter を生成する為には、
   *
   *   byref_holder と byval_holder を提供する必要がある。
   *   これは mwg/functor1 の ref_tr と ins_tr に対応する。
   *   但し、そんなに複雑な機能が必要になるかどうかは疑問である。
   *   結局の所、operator()() さえ提供できれば何も問題ない気がする。
   *
   *   `typedef functor_interface<Sfwd, functor_invoker<Ffwd, holder> adapter;` を定義するのは、
   *   外部で自動的にやった方が良い。`Sfwd` 及び `Ffwd` の決定は関数オブジェクトの種類とは独立である。
   *   一方で `functor_invoker` の種類は関数オブジェクトに依存する。
   *   どの種類の関数呼び出しを行うかについては整数か何かで指定できる様にしておくと便利である。
   *
   * これらをまとめた型として functor_traits を定義すれば良い。
   */

  template<typename F, typename S = void>
  struct functor_traits;

  enum traits_priority {
    traits_priority_function = 5,
    traits_priority_member   = 4,
    traits_priority_max = 5,
  };

  template<int Priority, typename F, typename S = void>
  struct functor_traits_rule: stdm::false_type {};

  template<int Pr, typename F, typename S>
  struct functor_traits_check: stdm::conditional<
    functor_traits_rule<Pr, F, S>::value,
    functor_traits_rule<Pr, F, S>,
    functor_traits_check<Pr - 1, F, S> >::type {};
  template<typename F, typename S>
  struct functor_traits_check<0, F, S>:
    functor_traits_rule<0, F, S> {};

  template<typename F, typename S>
  struct functor_traits: functor_traits_check<traits_priority_max, F, S> {};

  template<typename F>
  struct functor_traits_function_base: stdm::true_type {
    static const int invokation = invokation_function_call;
    typedef F intrinsic_signature;
    typedef struct holder {
      F* m_fun;
      template<typename T> holder(T const& fun): m_fun(fun) {}
      F* get() const {return m_fun;}
    } byref_holder, byval_holder;
  };
  template<typename F, typename S = void, typename = void>
  struct functor_traits_function: stdm::false_type {};
  template<typename F>
  struct functor_traits_function<F , void, typename stdm::enable_if<stdm::is_function<F>::value>::type>:
    functor_traits_function_base<F> {};
  template<typename F>
  struct functor_traits_function<F*, void, typename stdm::enable_if<stdm::is_function<F>::value>::type>:
    functor_traits_function_base<F> {};
  template<typename F, typename S>
  struct functor_traits_function<F , S, typename stdm::enable_if<is_variant_function<F, S>::value>::type>:
    functor_traits_function_base<F> {};
  template<typename F, typename S>
  struct functor_traits_function<F*, S, typename stdm::enable_if<is_variant_function<F, S>::value>::type>:
    functor_traits_function_base<F> {};

  template<typename F, typename S>
  struct functor_traits_rule<traits_priority_function, F, S>:
    functor_traits_function<F, S> {};

  template<typename MemberPtr, typename S>
  struct functor_traits_member_object: stdm::false_type {};
  template<typename MemberPtr, typename R, typename A>
  struct functor_traits_member_object<MemberPtr, R(A)>: stdm::true_type {
    static const int invokation = invokation_member_object;

  private:
    typedef typename stdx::add_const_reference<R>::type member_reference;
    typedef typename stdx::add_const_reference<A>::type object_reference;
  public:
    typedef member_reference intrinsic_signature(object_reference);

    typedef struct holder {
      MemberPtr m_memptr;
      template<typename T> holder(T const& fun): m_memptr(fun) {}
      MemberPtr get() const {return m_memptr;}
    } byref_holder, byval_holder;
  };
  template<typename F, typename S = void, typename = void>
  struct functor_traits_member: stdm::false_type {};
  template<typename T, typename C>
  struct functor_traits_member<T C::*, void,
    typename stdm::enable_if<stdm::is_member_object_pointer<T C::*>::value>::type>:
    functor_traits_member_object<T C::*, typename stdx::add_const_reference<T>::type (C const&)> {};
  template<typename T, typename C, typename S>
  struct functor_traits_member<T C::*, S,
    typename stdm::enable_if<
      stdm::is_member_object_pointer<T C::*>::value &&
      is_variant_function<typename stdm::add_lvalue_reference<T>::type (C&), S>::value
      >::type>:
    functor_traits_member_object<T C::*, typename stdm::add_lvalue_reference<T>::type (C&)> {};
  template<typename T, typename C, typename S>
  struct functor_traits_member<T C::*, S,
    typename stdm::enable_if<
      stdm::is_member_object_pointer<T C::*>::value &&
      !is_variant_function<typename stdm::add_lvalue_reference<T>::type (C&), S>::value &&
      is_variant_function<typename stdx::add_const_reference<T>::type (C const&), S>::value
      >::type>:
    functor_traits_member_object<T C::*, typename stdx::add_const_reference<T>::type (C const&)> {};
  // @@ToDo is_member_function_pointer
  template<typename F, typename S>
  struct functor_traits_rule<traits_priority_member, F, S>:
    functor_traits_member<F, S> {};

  template<typename F, typename S, typename = void>
  struct _as_functor: stdm::false_type {};
  template<typename F>
  struct _as_functor<F, void, typename stdm::enable_if<functor_traits<F>::value>::type>: stdm::true_type {
    typedef functor_traits<F> traits_type;

  private:
    typedef typename sig::filter<
      typename traits_type::intrinsic_signature,
      stdx::add_const_reference>::type interface_signature;
  public:
    typedef functor_interface<interface_signature,
      typename functor_invoker_selector<
        traits_type::invokation,
        typename traits_type::intrinsic_signature,
        typename traits_type::byref_holder>::type> adapter;
  };
  template<typename F, typename S>
  struct _as_functor<F, S, typename stdm::enable_if<functor_traits<F, S>::value>::type>: stdm::true_type {
    typedef functor_traits<F, S> traits_type;

  private:
    typedef typename sig::filter<S, stdx::add_const_reference>::type interface_signature;
  public:
    typedef functor_interface<interface_signature,
      typename functor_invoker_selector<
        traits_type::invokation,
        typename traits_type::intrinsic_signature,
        typename traits_type::byref_holder>::type> adapter;
  };


  template<typename F, typename S = void>
  struct as_functor;
  template<typename F, typename S>
  struct as_functor: _as_functor<F, S> {};

  /*?lwiki
   *
   * 課題:
   *
   * メンバポインタに関連して。受け取るオブジェクトに応じて戻り値の型が変わる。
   * つまり、`T C::*` は `T& (C&)` と `T& (C&)` の多重定義を持つ関数オブジェクトと考えられる。
   * より一般化して多重定義を持つ関数オブジェクトをどの様に実現するのかという事である。
   *
   * 例えば、多重定義を持つ関数オブジェクトの場合には `intrinsic_signature` として関数型ではなくて、
   * `std::pair<S1, std::pair<S2, S3> >` の様な物を指定することにするのはどうだろう。
   * そして `functor_interaface` は `std::pair` に対する特殊化を用意して、
   * 何とか `operator` を複数用意する事にするのである。
   *
   * 取り敢えず現状では `T& (C&)` を使用する事にする。
   *
   * 課題:
   *
   * is_functor/be_functor に対応する物は何か。
   * mwg/functor では is_functor は厳密に一致するシグニチャを持つ物であった。
   * 一方で be_functor は (引数の個数が異なる物も含めて) 適合するシグニチャを持つ物であった。
   * is_functor の条件は厳しすぎる一方で be_functor の条件は緩すぎる。
   * 丁度中間になる様なものが存在しても良いのではないだろうか。
   * (例えば const, volatile, const& を外した上での比較を行う等)
   *
   * 課題:
   *
   * 可変長引数の関数オブジェクトの実現方法は謎である。
   * というのも、任意の引数を転送する為にはテンプレートを使わなければならないが、
   * テンプレートは仮想関数にする事ができないからである。
   * ただ、関数オブジェクトにしない範囲では対応可能である。
   *
   */

#undef mwg_rfwd
}
  using functor_detail::as_functor;
}
#pragma%x begin_check

namespace test_funcsig {
  void run() {
    using namespace mwg::funcsig;
    mwg_check((mwg::stdm::is_same<
        filter<int (int, char, short&), mwg::stdx::add_const_reference>::type,
        int (int const&, char const&, short&)>::value));
    mwg_check((mwg::stdm::is_same<
        filter<int (int&, char, short), mwg::stdx::add_const_reference>::type,
        int (int&, char const&, short const&)>::value));
  }
}

namespace test_function {
  int test_var = 0;

  void func1(int a, int b) {
    test_var = 1000 + a + b;
    //std::printf("func1(%d, %d) called\n", a, b);
  }
  void func2(int a) {
    test_var = 2000 + a;
    //std::printf("func2(%d) called\n", a);
  }
  void func3(int a, ...) {
    va_list va;
    va_start(va, a);
    int b = va_arg(va, int);
    va_end(va);

    test_var = 3000 + a + b;
    //std::printf("func3(a=%d, ...=%d) called\n", a, b);
  }
  void run() {
    mwg::as_functor<void(int, int), void (int, int)>::adapter f1(func1);
    f1(1, 2);
    mwg_check(test_var == 1003);

    mwg::as_functor<void(*)(int), void (int, int)>::adapter f2(func2);
    f2(1, 2);
    mwg_check(test_var == 2001);

    mwg::as_functor<void(int, ...), void (int, int)>::adapter f3(func3);
    f3(5, 6);
    mwg_check(test_var == 3011);
  }
}

struct Rect {int x, y, w, h;};
void test_member() {
  Rect rect1;
  int Rect::*hoge = &Rect::x;
  mwg_check((mwg::stdm::is_member_object_pointer<int Rect::*>::value &&
      mwg::functor_detail::is_variant_function<mwg::stdm::add_lvalue_reference<int>::type (Rect&), int& (Rect&)>::value));
  mwg::as_functor<int Rect::*, int& (Rect&)>::adapter f1(&Rect::x);
  f1(rect1) = 12;
  mwg::as_functor<int Rect::*, int (Rect const&)>::adapter f2(&Rect::x);
  mwg_check((rect1.x == 12));
  mwg_check((f2(rect1) == 12));
}

int main() {
  test_funcsig::run();
  test_function::run();
  test_member();
  //make_adapter<S(*)>();

  return 0;
}

#pragma%x end_check
#endif
