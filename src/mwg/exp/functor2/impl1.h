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
namespace functor_detail {

  enum invokation_type {
    invokation_function,
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
  template<typename T, int Index>
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

  template<typename S, typename XS, int Arity>
  struct enable_forward: stdm::enable_if<
    (Arity == (is_vararg_function<S>::value? sig::arity<XS>::value: sig::arity<S>::value)),
    typename sig::returns<XS>::type> {};

  template<typename XS, int Index>
  struct param: sig::parameter<Index,XS> {};

#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
  template<typename S, typename CRTP>
  struct functor_invoker_call {};
  template<class CRTP, class R, class... A>
  struct functor_invoker_call<R (A...), CRTP>: CRTP {
    template<class F> functor_invoker_call(F const& func): CRTP(func) {}
    template<class XS>
    typename sig::returns<XS>::type
    forward(A... a, ...) const {
      return (CRTP::get())(fwd<A>(a)...);
    }
  };
  template<class CRTP, class R, class... A>
  struct functor_invoker_call<R (A..., ...), CRTP>: CRTP {
    template<class F> functor_invoker_call(F const& func): CRTP(func) {}
    template<class XS, class... B>
    typename sig::returns<XS>::type
    forward(A... a, B mwg_rfwd... b) const {
      return (CRTP::get())(fwd<A>(a)..., fwd<B>(b)...);
    }
  };
#else
  template<class S, class CRTP>
  struct functor_invoker_call: CRTP {
    template<class F> functor_invoker_call(F const& func): CRTP(func) {}
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

  template<typename F, typename S, typename = void>
  struct _as_functor: stdm::false_type {};
  template<typename F, typename S>
  struct _as_functor<F, S, typename stdm::enable_if<stdm::is_function<F>::value && stdm::is_same<F, S>::value>::type>: stdm::true_type {
    struct base1 {
      F* m_fun;
      template<typename T> base1(T const& fun): m_fun(fun) {}
      F* get() const {return m_fun;}
    };
    typedef functor_interface<S, functor_invoker_call<S, base1> > adapter;
  };

  template<typename F, typename S = void>
  struct as_functor;
  template<typename F, typename S>
  struct as_functor: _as_functor<F, S> {};

#undef mwg_rfwd mwg_forward_rvalue
}
  using functor_detail::as_functor;
}
#pragma%x begin_check

void func1(int, int) {
  std::printf("func1 called\n");
}

int main() {
  mwg::as_functor<void(int, int), void (int, int)>::adapter f1(func1);

  f1(1, 2);

  mwg_printd();
  return 0;
}

#pragma%x end_check
#endif
