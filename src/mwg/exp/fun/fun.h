// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_FUN_H
#define MWG_FUN_H
#pragma%include "../../impl/VariadicMacros.pp"
#pragma%include "../../impl/ManagedTest.pp"
#pragma%x begin_check
#include <mwg/exp/fun/fun.h>
#include <mwg/except.h>
#pragma%x end_check
#include <mwg/std/type_traits>
#include <mwg/std/utility>
#include <mwg/exp/fun/funsig.h>
namespace mwg{
namespace functor_detail {
  namespace sig = mwg::funsig;

  namespace type_traits {
    /*?lwiki
     * @typedef typename ==reference_parameter==<T>::type;
     *
     * 非参照型に `const&` を付加します。
     * 参照はそのまま返します。
     * それ以外の場合に `T const&` を返します。
     */
    template<typename T> struct reference_parameter: stdm::conditional<
      stdm::is_reference<T>::value, T,
      typename stdx::add_const_reference<T>::type> {};

    /*?lwiki
     * @typedef typename ==canonical_parameter==<T>::type;
     */
    template<typename T> struct canonical_parameter: stdm::conditional<
      (stdm::is_lvalue_reference<T>::type &&
        stdm::is_const<typename stdm::remove_reference<T>::type>::type &&
        !stdm::is_volatile<typename stdm::remove_reference<T>::type>::type &&
        stdm::is_scalar<typename stdm::remove_reference<T>::type>::type),
      typename stdm::remove_const<typename stdm::remove_reference<T>::type>::type,
      typename stdm::conditional<
        stdm::is_object<T>::value && !stdm::is_scalar<T>::type, typename stdx::add_const_reference<T>::type,
        T>::type> {};

    /*?lwiki
     * @var bool ==is_contravariant==<typename From, typename To>::value;
     *
     * `From` 型仮引数を `To` 型仮引数に置き換えたインターフェイスを持てるかどうかを判定する。
     * `To` で受け取れる実引数が全て元々 `From` で受け取れる場合に `true` となる。
     * `To const&` で受け取った実引数を `From` に変換できることが条件になる。
     *
     * ※`To` が非参照型である場合、コンストラクタが対応する限り
     *   あらゆる value category の値を受け取れてしまう事に注意する。
     *   `reference_parameter` で `const&` を付加するのはこれによる誤判定を除くためである。
     *
     * @var bool ==is_variant==<typename From, typename To>::value;
     *
     * 戻り値型 `From` を戻り値型 `To` に置き換えたインターフェイスを持てるかどうかを判定する。
     *
     */
    namespace detail {
      template<
        typename From, typename To,

        bool const value = (stdm::is_void<From>::value ||
          stdm::is_convertible<typename reference_parameter<To>::type, From>::value)>
      struct is_contravariant: stdm::integral_constant<bool, value> {};

      template<
        typename From, typename To,
        bool const value = (stdm::is_void<To>::value ||
          (stdm::is_convertible<From, To>::value &&
            (!stdm::is_reference<To>::value || stdm::is_reference<From>::value)))>
      struct is_covariant: stdm::integral_constant<bool, value> {};
    }

    template<typename From, typename To>
    struct is_contravariant: detail::is_contravariant<From, To> {};
    template<typename From, typename To>
    struct is_covariant: detail::is_covariant<From, To> {};

    namespace detail {
      template<
        typename FromSignature, typename ToSignature,
        bool cond = sig::arity<FromSignature>::value || sig::arity<ToSignature>::value>
      struct has_contravariant_parameters: stdm::integral_constant<bool,
        is_contravariant<
          typename sig::param<FromSignature, 0>::type,
          typename sig::param<ToSignature  , 0>::type>::value &&
        has_contravariant_parameters<
          typename sig::unshift<FromSignature>::type,
          typename sig::unshift<ToSignature>::type >::value> {};

      template<typename FromSignature, typename ToSignature>
      struct has_contravariant_parameters<FromSignature, ToSignature, false>: stdm::true_type {};

      template<
        typename FromSignature, typename ToSignature,
        bool = stdm::is_function<FromSignature>::value && stdm::is_function<ToSignature>::value>
      struct is_variant_function: stdm::integral_constant<bool,
        (is_covariant<
          typename sig::result<FromSignature>::type,
          typename sig::result<ToSignature>::type>::value &&
          has_contravariant_parameters<FromSignature,ToSignature>::value)>{};
      template<typename FromSignature, typename ToSignature>
      struct is_variant_function<FromSignature, ToSignature, false>: stdm::false_type {};
    }
    template<typename From, typename To>
    struct is_variant_function: detail::is_variant_function<From, To> {};

#pragma%x begin_test
    struct Class {};
    void test() {
      using namespace mwg::functor_detail::type_traits;

      mwg_check((is_contravariant<Class, Class        >::value));
      mwg_check((is_contravariant<Class, Class const  >::value));
      mwg_check((is_contravariant<Class, Class      & >::value));
      mwg_check((is_contravariant<Class, Class const& >::value));
#ifdef MWGCONF_STD_RVALUE_REFERENCES
      mwg_check((is_contravariant<Class, Class      &&>::value));
      mwg_check((is_contravariant<Class, Class const&&>::value));
#endif

      mwg_check((is_contravariant<const Class, Class        >::value));
      mwg_check((is_contravariant<const Class, Class const  >::value));
      mwg_check((is_contravariant<const Class, Class      & >::value));
      mwg_check((is_contravariant<const Class, Class const& >::value));
#ifdef MWGCONF_STD_RVALUE_REFERENCES
      mwg_check((is_contravariant<const Class, Class      &&>::value));
      mwg_check((is_contravariant<const Class, Class const&&>::value));
#endif

      mwg_check((!is_contravariant<Class&, Class        >::value));
      mwg_check((!is_contravariant<Class&, Class const  >::value));
      mwg_check(( is_contravariant<Class&, Class      & >::value));
      mwg_check((!is_contravariant<Class&, Class const& >::value));
#ifdef MWGCONF_STD_RVALUE_REFERENCES
      mwg_check((!is_contravariant<Class&, Class      &&>::value));
      mwg_check((!is_contravariant<Class&, Class const&&>::value));
#endif

      mwg_check((is_contravariant<const Class&, Class        >::value));
      mwg_check((is_contravariant<const Class&, Class const  >::value));
      mwg_check((is_contravariant<const Class&, Class      & >::value));
      mwg_check((is_contravariant<const Class&, Class const& >::value));
#ifdef MWGCONF_STD_RVALUE_REFERENCES
      mwg_check((is_contravariant<const Class&, Class      &&>::value));
      mwg_check((is_contravariant<const Class&, Class const&&>::value));
#endif

#ifdef MWGCONF_STD_RVALUE_REFERENCES
      mwg_check((!is_contravariant<Class&&, Class        >::value));
      mwg_check((!is_contravariant<Class&&, Class const  >::value));
      mwg_check((!is_contravariant<Class&&, Class      & >::value));
      mwg_check((!is_contravariant<Class&&, Class const& >::value));
      mwg_check(( is_contravariant<Class&&, Class      &&>::value));
      mwg_check((!is_contravariant<Class&&, Class const&&>::value));

      mwg_check((!is_contravariant<const Class&&, Class        >::value));
      mwg_check((!is_contravariant<const Class&&, Class const  >::value));
      mwg_check((!is_contravariant<const Class&&, Class      & >::value));
      mwg_check((!is_contravariant<const Class&&, Class const& >::value));
      mwg_check(( is_contravariant<const Class&&, Class      &&>::value));
      mwg_check(( is_contravariant<const Class&&, Class const&&>::value));
#endif
    }
#pragma%x end_test

    /*?lwiki
     * :@class class mwg::functor_detail::==is_member_pointer==<Mfp>;
     *  :@var static const bool ==value==;
     *   `Mfp` がメンバ関数へのポインタかどうかを判定します。
     *  :@typedef typedef '''member-pointer-type''' ==member_type==;
     *   `Mfp` がメンバ関数へのポインタの時、関数型を取得します。
     *  :@typedef typedef '''class-type''' ==object_type==;
     *   `Mfp` がメンバ関数へのポインタの時、メンバが定義されるクラスを取得します。
     *   第一引数に `object_type` を受け取り、第二引数以降に本来の引数を受け取ります。
     *
     * ToDo: 実はこれは <mwg/std/type_traits> の is_member_function_pointer と統合できるのではないか。
     *   funsig.h か bits/is_member_pointer か何処かに分離して詳しくして include する。
     *
     */
    namespace detail {
      template<typename Mfp>
      struct is_member_pointer: stdm::false_type {
        typedef void member_type;
        typedef void object_type;
      };
      template<typename C, typename T>
      struct is_member_pointer_def: stdm::true_type {
        typedef C object_type;
        typedef T member_type;
      };
      template<typename T, typename C>
      struct is_member_pointer<T C::*>: is_member_pointer_def<C, T> {};
#pragma%m 1
      template<typename R, typename C, typename... A>
      struct is_member_pointer<R (C::*)(A...) QUALIFIER>:
        is_member_pointer_def<C QUALIFIER, R (A...)> {};
      template<typename R, typename C, typename... A>
      struct is_member_pointer<R (C::*)(A..., ...) QUALIFIER>:
        is_member_pointer_def<C QUALIFIER, R (A..., ...)> {};
#pragma%end
#pragma%m 1
#pragma%x 1.r/QUALIFIER//
#pragma%x 1.r/QUALIFIER/const/
#pragma%x 1.r/QUALIFIER/volatile/
#pragma%x 1.r/QUALIFIER/const volatile/
#pragma%end
#pragma%x variadic_expand_0toArN
    }
    template<typename MemPtr>
    struct is_member_pointer: detail::is_member_pointer<MemPtr> {};

  }

  enum invokation_type {
    invokation_function_call,
    invokation_member_object,
    invokation_member_function,
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
#else
  template<typename T>
  typename stdx::add_const_reference<T>::type
  fwd(typename stdx::add_const_reference<T>::type value) {return value;}
#endif
#define mwg_rfwd mwg_forward_rvalue

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

  //
  //
  // struct functor_interface<S, CRTP>;
  //
  //

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
    (Arity == (sig::has_va_args<S>::value? sig::arity<XS>::value: sig::arity<S>::value)),
    typename sig::result<XS>::type> {};

  namespace function_invoker {
#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
    template<typename T> struct p: type_traits::reference_parameter<T> {};

    template<typename S, typename CRTP>
    struct invoker {};
    template<class CRTP, class R, class... A>
    struct invoker<R (A...), CRTP>: CRTP {
      template<class F> invoker(F const& func): CRTP(func) {}
      template<class XS>
      typename sig::result<XS>::type
      forward(typename p<A>::type... a, ...) const {
        return (CRTP::get())(fwd<typename p<A>::type>(a)...);
      }
    };
    template<class CRTP, class R, class... A>
    struct invoker<R (A..., ...), CRTP>: CRTP {
      template<class F> invoker(F const& func): CRTP(func) {}
      template<class XS, class... B>
      typename sig::result<XS>::type
      forward(typename p<A>::type... a, B mwg_rfwd... b) const {
        return (CRTP::get())(fwd<typename p<A>::type>(a)..., fwd<B>(b)...);
      }
    };
#else
    template<typename XS, int Index> struct p: sig::param<XS, Index> {};
#if defined(MWGCONF_STD_RVALUE_REFERENCES)
    template<typename S, int Index, typename X>
    typename stdm::add_rvalue_reference<typename sig::param<S, Index>::type>::type f(X&& value) {
      return fwd<typename sig::param<S, Index>::type>(value);
    }
#else
    template<typename S, int Index>
    typename stdx::add_const_reference<typename sig::param<S, Index>::type>::type
    f(typename stdx::add_const_reference<typename sig::param<S, Index>::type>::type value) {return value;}
#endif

    template<class S, class CRTP>
    struct invoker: CRTP {
      template<class F> invoker(F const& func): CRTP(func) {}
#pragma%m 1
      template<class XS>
      typename enable_forward<S, XS, __arity__>::type
      forward(typename p<XS,>::type... a, ...) const {
        return (CRTP::get())(f<XS,>(a)...);
      }
#pragma%end
#pragma%x variadic_expand::with_arity.f/__arity__/0/ArN+1/
    };
#endif
  }

  template<typename S, typename CRTP>
  struct functor_invoker_selector<invokation_function_call, S, CRTP>:
    mwg::identity<function_invoker::invoker<S, CRTP> > {};

  namespace member_object_invoker {
    template<typename S, typename CRTP>
    struct invoker {};
    template<typename R, typename A0, typename CRTP>
    struct invoker<R(A0), CRTP>: CRTP {
      template<typename F> invoker(F const& func): CRTP(func) {}
      template<typename XS>
      typename sig::result<XS>::type
      forward(A0 a0, ...) const {return CRTP::getobj(a0).*CRTP::get();}
    };
  }

  template<typename S, typename CRTP>
  struct functor_invoker_selector<invokation_member_object, S, CRTP>:
    mwg::identity<member_object_invoker::invoker<S, CRTP> > {};

  namespace member_function_invoker {
#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
    template<typename T> struct p: type_traits::reference_parameter<T> {};

    template<typename S, typename CRTP>
    struct invoker {};
    template<class CRTP, class R, class C, class... A>
    struct invoker<R (C, A...), CRTP>: CRTP {
      template<class F> invoker(F const& func): CRTP(func) {}
      template<class XS>
      typename sig::result<XS>::type
      forward(typename p<C>::type obj, typename p<A>::type... a, ...) const {
        return (CRTP::getobj(obj).*CRTP::get())(fwd<typename p<A>::type>(a)...);
      }
    };
    template<class CRTP, class R, class C, class... A>
    struct invoker<R (C, A..., ...), CRTP>: CRTP {
      template<class F> invoker(F const& func): CRTP(func) {}
      template<class XS, class... B>
      typename sig::result<XS>::type
      forward(typename p<C>::type obj, typename p<A>::type... a, B mwg_rfwd... b) const {
        return (CRTP::getobj(obj).*CRTP::get())(fwd<typename p<A>::type>(a)..., fwd<B>(b)...);
      }
    };
#else
    template<typename XS> struct p0: sig::param<XS, 0> {};
    template<typename XS, std::size_t I> struct pr: sig::param<XS, 1 + I> {};

#ifdef MWGCONF_STD_RVALUE_REFERENCES
    template<typename S, int Index, typename X>
    typename stdm::add_rvalue_reference<typename sig::param<S, 1 + Index>::type>::type f(X&& value) {
      return fwd<typename sig::param<S, 1 + Index>::type>(value);
    }
#else
    template<typename S, int Index>
    typename stdx::add_const_reference<typename sig::param<S, 1 + Index>::type>::type
    f(typename stdx::add_const_reference<typename sig::param<S, 1 + Index>::type>::type value) {return value;}
#endif

    template<class S, class CRTP>
    struct invoker: CRTP {
      template<class F> invoker(F const& func): CRTP(func) {}
#pragma%m 1
      template<class XS>
      typename enable_forward<S, XS, 1+__arity__>::type
      forward(typename p0<XS>::type obj, typename pr<XS,>::type... a, ...) const {
        return (CRTP::getobj(obj).*CRTP::get())(f<XS,>(a)...);
      }
#pragma%end
#pragma%x variadic_expand::with_arity.f/__arity__/0/ArN/
    };
#endif
  }

  template<typename S, typename CRTP>
  struct functor_invoker_selector<invokation_member_function, S, CRTP>:
    mwg::identity<member_function_invoker::invoker<S, CRTP> > {};


  /*?lwiki
   *
   * 次に as_fun の実装について考える。
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

  //
  //
  // struct functor_traits<F, S = void>;
  //
  //

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

  namespace function_traits {
    template<typename F>
    struct functor_traits_impl: stdm::true_type {
      static const int invokation = invokation_function_call;
      typedef F intrinsic_signature;
      typedef struct holder {
        F* m_fun;
        template<typename T> holder(T const& fun): m_fun(fun) {}
        F* get() const {return m_fun;}
      } byref_holder, byval_holder;
    };
    template<typename F, typename S = void, typename = void>
    struct _switch_nocv: stdm::false_type {};
    template<typename F>
    struct _switch_nocv<F , void, typename stdm::enable_if<stdm::is_function<F>::value>::type>:
      functor_traits_impl<F> {};
    template<typename F>
    struct _switch_nocv<F*, void, typename stdm::enable_if<stdm::is_function<F>::value>::type>:
      functor_traits_impl<F> {};
    template<typename F, typename S>
    struct _switch_nocv<F , S, typename stdm::enable_if<type_traits::is_variant_function<F, S>::value>::type>:
      functor_traits_impl<F> {};
    template<typename F, typename S>
    struct _switch_nocv<F*, S, typename stdm::enable_if<type_traits::is_variant_function<F, S>::value>::type>:
      functor_traits_impl<F> {};

    template<typename F, typename S>
    struct _switch: _switch_nocv<typename stdm::remove_cv<typename stdm::remove_reference<F>::type>::type, S> {};
  }

  template<typename F, typename S>
  struct functor_traits_rule<traits_priority_function, F, S>:
    function_traits::_switch<F, S> {};

  template<typename F, typename S = void, typename = void>
  struct functor_traits_member: stdm::false_type {};


  namespace member_object_pointer_traits {
    template<typename MemberPtr, typename S>
    struct functor_traits_impl: stdm::false_type {};
    template<typename MemberPtr, typename R, typename A>
    struct functor_traits_impl<MemberPtr, R(A)>: stdm::true_type {
      static const int invokation = invokation_member_object;
      typedef R intrinsic_signature(A);
      typedef struct holder {
        MemberPtr m_memptr;
        template<typename T> holder(T const& fun): m_memptr(fun) {}

      public:
        MemberPtr get() const {return m_memptr;}

        template<typename C> static C               & getobj(C               * ptr) {return *ptr;}
        template<typename C> static C const         & getobj(C const         * ptr) {return *ptr;}
        template<typename C> static C       volatile& getobj(C       volatile* ptr) {return *ptr;}
        template<typename C> static C const volatile& getobj(C const volatile* ptr) {return *ptr;}
#ifdef MWGCONF_STD_RVALUE_REFERENCES
        template<typename C> static C&& getobj(C&& obj) {return obj;}
#else
        template<typename C> static C               & getobj(C               & obj) {return obj;}
        template<typename C> static C const         & getobj(C const         & obj) {return obj;}
        template<typename C> static C       volatile& getobj(C       volatile& obj) {return obj;}
        template<typename C> static C const volatile& getobj(C const volatile& obj) {return obj;}
#endif
      } byref_holder, byval_holder;
    };

    enum {
      ACCEPTS_LREF = 0x01,
      ACCEPTS_RREF = 0x02,
      ACCEPTS_PTR  = 0x04,
      IS_CONST     = 0x10,
      IS_VOLATILE  = 0x20,
    };


    template<
      typename S,
      std::size_t arity = sig::arity<S>::value,
      typename param_t = typename sig::param<S, 0>::type>
    struct find_first: stdm::conditional<
      param_t::value, param_t,
      typename find_first<typename sig::unshift<S>::type>::type> {};
    template<typename S> struct find_first<S, 0>: mwg::identity<void> {};
    template<typename S> struct find_first<S, 1>: sig::param<S, 0> {};

    template<
      typename T, typename C, typename S, int Flags,

      typename obj_const_t = typename stdm::conditional<
        (Flags & IS_CONST) != 0, typename stdm::add_const<C>::type, C>::type,
      typename obj_cv_t = typename stdm::conditional<
        (Flags & IS_VOLATILE) != 0, typename stdm::add_volatile<obj_const_t>::type, obj_const_t>::type,
      typename obj_ref_t = typename stdm::conditional<(Flags & ACCEPTS_LREF) != 0,
        typename stdm::add_lvalue_reference<obj_cv_t>::type,
#ifdef MWGCONF_STD_RVALUE_REFERENCES
        typename stdm::conditional<(Flags & ACCEPTS_RREF) != 0,
          typename stdm::add_rvalue_reference<obj_cv_t>::type,
          typename stdm::add_pointer<obj_cv_t>::type>::type
#else
        typename stdm::add_pointer<obj_cv_t>::type
#endif
        >::type,

      typename mem_const_t = typename stdm::conditional<
        (Flags & IS_CONST) != 0, typename stdm::add_const<T>::type, T>::type,
      typename mem_cv_t = typename stdm::conditional<
        (Flags & IS_VOLATILE) != 0, typename stdm::add_volatile<mem_const_t>::type, mem_const_t>::type,
#ifdef MWGCONF_STD_RVALUE_REFERENCES
      typename mem_ref_t = typename stdm::conditional<(Flags & ACCEPTS_RREF) != 0,
        typename stdm::add_rvalue_reference<mem_cv_t>::type,
        typename stdm::add_lvalue_reference<mem_cv_t>::type>::type,
#else
      typename mem_ref_t = typename stdm::add_lvalue_reference<mem_cv_t>::type,
#endif

      typename sig_t = mem_ref_t (obj_ref_t),
      bool _value = type_traits::is_variant_function<sig_t, S>::value>
    struct check_signature: stdm::integral_constant<bool, _value> {typedef sig_t type;};

    template<typename T, typename C, typename S, int Flags>
    struct check_signature_cv: find_first<void(
      check_signature<T, C, S, Flags>,
      check_signature<T, C, S, Flags | IS_CONST>,
      check_signature<T, C, S, Flags | IS_VOLATILE>,
      check_signature<T, C, S, Flags | IS_VOLATILE | IS_CONST>
    )>::type {};

    template<
      typename MemObj, typename S,

      typename mem_t = typename type_traits::is_member_pointer<MemObj>::member_type,
      typename obj_t = typename type_traits::is_member_pointer<MemObj>::object_type,

      // ToDo @intrinsic_overload
      typename mem_lref_t = typename stdm::add_lvalue_reference<mem_t>::type,
      typename mem_cref_t = typename stdm::conditional<
        stdm::is_reference<mem_t>::value, mem_t,
        typename stdx::add_const_reference<mem_t>::type>::type,

      typename sig_t = typename stdm::conditional<
        stdm::is_void<S>::value,
        check_signature<mem_t, obj_t, S, ACCEPTS_RREF | IS_CONST>,
        typename find_first<
          void(
            check_signature_cv<mem_t, obj_t, S, ACCEPTS_RREF>,
            check_signature_cv<mem_t, obj_t, S, ACCEPTS_LREF>,
            check_signature_cv<mem_t, obj_t, S, ACCEPTS_PTR >,
            mwg::identity<void>)>::type>::type::type,

      typename base = functor_traits_impl<MemObj, sig_t> >
    struct _switch_nocv: base {};

    template<
      typename MemObj, typename S,

      typename memobj_ptr = typename stdm::remove_cv<typename stdm::remove_reference<MemObj>::type>::type,
      bool = stdm::is_member_object_pointer<memobj_ptr>::value>
    struct _switch: _switch_nocv<memobj_ptr, S> {};
    template<typename MemObj, typename S, typename memobj_ptr>
    struct _switch<MemObj, S, memobj_ptr, false>: stdm::false_type {};
  }

  template<typename MemObj, typename S>
  struct functor_traits_member<MemObj, S, typename stdm::enable_if<member_object_pointer_traits::_switch<MemObj, S>::value>::type>:
    member_object_pointer_traits::_switch<MemObj, S> {};

  namespace member_function_pointer_traits {

    template<typename MemFun, typename S>
    struct functor_traits_impl: stdm::true_type {
      static const int invokation = invokation_member_function;
      typedef S intrinsic_signature;
      typedef struct holder {
        MemFun m_memptr;
        template<typename T> holder(T const& fun): m_memptr(fun) {}

      public:
        MemFun get() const {return m_memptr;}

        template<typename C> static C               & getobj(C               * ptr) {return *ptr;}
        template<typename C> static C const         & getobj(C const         * ptr) {return *ptr;}
        template<typename C> static C       volatile& getobj(C       volatile* ptr) {return *ptr;}
        template<typename C> static C const volatile& getobj(C const volatile* ptr) {return *ptr;}
#ifdef MWGCONF_STD_RVALUE_REFERENCES
        template<typename C> static C&& getobj(C&& obj) {return stdm::forward<C>(obj);}
#else
        template<typename C> static C               & getobj(C               & obj) {return obj;}
        template<typename C> static C const         & getobj(C const         & obj) {return obj;}
        template<typename C> static C       volatile& getobj(C       volatile& obj) {return obj;}
        template<typename C> static C const volatile& getobj(C const volatile& obj) {return obj;}
#endif
      } byref_holder, byval_holder;
    };
    template<typename MemFun>
    struct functor_traits_impl<MemFun, void>: stdm::false_type {};

    template<
      typename MemFun, typename S,

      typename mem_t = typename type_traits::is_member_pointer<MemFun>::member_type,
      typename obj_t = typename type_traits::is_member_pointer<MemFun>::object_type,

      // obj_cref_t: obj parameter type for intrinsic signature
      //
      // + ref-qualifier がついている場合にはそのまま obj_t が自然な引数の型である。
      // + ref-qualifier がない場合は lvalue/rvalue のどちらからでも呼び出せることを表す。
      //   つまり、同じ cv またはより少ない cv を持つ lvalue/rvalue から呼び出せる。
      //   + 右辺値参照のない環境では関数の引数では値の cv に応じた引数受取の選択はできないので実現方法はない。
      //     この場合には permissive に C const& で値を受け取る様にして内部で const_cast をするしかない。
      //   + 右辺値参照のある環境では obj_t& 及び obj_t&& の多重定義を作ることに対応するが、
      //     現状では未だ intrinsic_signature として多重定義を許す様になっていないので、
      //     取り敢えずのところは const& で修飾して内部で const_cast する様にして置く。
      //     ToDo @intrinsic_overload
      //
      typename obj_cref_t = typename type_traits::reference_parameter<obj_t>::type,

#ifdef MWGCONF_STD_RVALUE_REFERENCES
      typename obj_lref_t = typename stdm::conditional<
        stdm::is_reference<obj_t>::value, obj_t,
        typename stdm::add_lvalue_reference<obj_t>::type>::type,
      typename obj_rref_t = typename stdm::conditional<
        stdm::is_reference<obj_t>::value, obj_t,
        typename stdm::add_rvalue_reference<obj_t>::type>::type,
#else
      typename obj_lref_t = typename type_traits::reference_parameter<obj_t>::type,
      typename obj_rref_t = obj_lref_t,
#endif

      typename sig0_t = typename sig::shift<mem_t, obj_cref_t>::type,
      typename sig1_t = typename sig::shift<mem_t, obj_lref_t>::type,
      typename sig2_t = typename sig::shift<mem_t, obj_rref_t>::type,
      typename sig3_t = typename stdm::conditional<
        !stdm::is_rvalue_reference<obj_t>::value,
        typename sig::shift<mem_t, typename stdm::remove_reference<obj_t>::type*>::type,
        void>::type,

      bool test0 = stdm::is_void<S>::value,
      bool test1 = type_traits::is_variant_function<sig1_t, S>::value,
      bool test2 = type_traits::is_variant_function<sig2_t, S>::value,
      bool test3 = type_traits::is_variant_function<sig3_t, S>::value,

      typename sig_t = typename stdm::conditional<
        test1, sig1_t,
        typename stdm::conditional<
          test2, sig2_t,
          typename stdm::conditional<
            test3, sig3_t, void>::type>::type>::type,

      typename base = functor_traits_impl<MemFun, sig_t> >
    struct _switch_nocv: base {};

    template<
      typename MemFun, typename S,

      typename memfun_ptr = typename stdm::remove_cv<typename stdm::remove_reference<MemFun>::type>::type,
      bool = stdm::is_member_function_pointer<memfun_ptr>::value>
    struct _switch: _switch_nocv<memfun_ptr, S> {};
    template<typename MemFun, typename S, typename memfun_ptr>
    struct _switch<MemFun, S, memfun_ptr, false>: std::false_type {};
  }

  template<typename MemFun, typename S>
  struct functor_traits_member<MemFun, S, typename stdm::enable_if<member_function_pointer_traits::_switch<MemFun, S>::value>::type>:
    member_function_pointer_traits::_switch<MemFun, S> {};

  template<typename F, typename S>
  struct functor_traits_rule<traits_priority_member, F, S>:
    functor_traits_member<F, S> {};

  //
  //
  // struct as_fun<F, S = void>;
  //
  //

  template<typename F, typename S, typename = void>
  struct as_fun: stdm::false_type {};
  template<typename F>
  struct as_fun<F, void, typename stdm::enable_if<functor_traits<F>::value>::type>: stdm::true_type {
    typedef functor_traits<F> traits_type;

  private:
    typedef typename sig::filter<
      typename traits_type::intrinsic_signature,
      type_traits::reference_parameter>::type interface_signature;
  public:
    typedef functor_interface<interface_signature,
      typename functor_invoker_selector<
        traits_type::invokation,
        typename traits_type::intrinsic_signature,
        typename traits_type::byref_holder>::type> adapter;
  };
  template<typename F, typename S>
  struct as_fun<F, S, typename stdm::enable_if<functor_traits<F, S>::value>::type>: stdm::true_type {
    typedef functor_traits<F, S> traits_type;

  private:
    typedef typename sig::filter<S, type_traits::reference_parameter>::type interface_signature;
  public:
    typedef functor_interface<interface_signature,
      typename functor_invoker_selector<
        traits_type::invokation,
        typename traits_type::intrinsic_signature,
        typename traits_type::byref_holder>::type> adapter;
  };

  /*?lwiki
   *
   * 課題:
   *
   * 実は既に関数型だったり関数オブジェクトだったりする物については、
   * adapter は自分自身の型への参照にすれば良いのでは。
   * そちらの方が多重定義を失う事もないし良い。
   *
   * と思ったがその為には現在の様に as_fun の内部で adapter を定義するという方法は使えない。
   * 自分で特別な adapter を生成する場合には functor_traits に `intrinsic_adapter` という名前で、
   * 型メンバーを提供する様にすれば良い。
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

  template<typename F, typename S = void>
  struct as_fun;

  template<typename F, typename S>
  struct as_fun: functor_detail::as_fun<F, S> {};

#ifdef MWGCONF_STD_RVALUE_REFERENCES
  template<typename S, typename F>
  typename as_fun<F&&, S>::adapter
  fun(F&& value) {return typename as_fun<F&&, S>::adapter(stdm::forward<F>(value));}
#else
  template<typename S, typename F>
  typename as_fun<F&, S>::adapter
  fun(F& value) {return typename as_fun<F&, S>::adapter(value);}
  template<typename S, typename F>
  typename as_fun<F const, S>::adapter
  fun(F const& value) {return typename as_fun<F, S>::adapter(value);}
#endif

}
#pragma%x begin_check

namespace test_funcsig {
  void run() {
    using namespace mwg::funsig;
    mwg_check((mwg::stdm::is_same<
        filter<int (int, char, short&), mwg::functor_detail::type_traits::reference_parameter>::type,
        int (int const&, char const&, short&)>::value));
    mwg_check((mwg::stdm::is_same<
        filter<int (int&, char, short), mwg::functor_detail::type_traits::reference_parameter>::type,
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
    mwg::as_fun<void(int, int), void (int, int)>::adapter f1(func1);
    f1(1, 2);
    mwg_check(test_var == 1003);

    mwg::as_fun<void(*)(int), void (int, int)>::adapter f2(func2);
    f2(1, 2);
    mwg_check(test_var == 2001);

    mwg::as_fun<void(int, ...), void (int, int)>::adapter f3(func3);
    f3(5, 6);
    mwg_check(test_var == 3011);
  }
}

namespace test_member {
  struct Rect {
    int x, y, w, h;

    int right() const {
      return x + w;
    }
    int bottom() const {
      return y + h;
    }
    void translate(int dx, int dy) {
      x += dx;
      y += dy;
    }
  };

  void run() {
    namespace type_traits = mwg::functor_detail::type_traits;
    mwg_check((mwg::stdm::is_member_object_pointer<int Rect::*>::value));
    mwg_check((type_traits::is_variant_function<mwg::stdm::add_lvalue_reference<int>::type (Rect&), int& (Rect&)>::value));

    Rect rect1;
    rect1.x = 1;
    rect1.y = 2;
    rect1.w = 3;
    rect1.h = 4;

    {
      namespace ns = mwg::functor_detail::member_object_pointer_traits;
      mwg_check((mwg::stdm::is_same<ns::check_signature<int, Rect, int& (Rect&), ns::ACCEPTS_LREF>::type, int& (Rect&)>::value));
      mwg_check((ns::check_signature<int, Rect, int& (Rect&), ns::ACCEPTS_LREF>::value));
      mwg_check((ns::check_signature_cv<int, Rect, int& (Rect&), ns::ACCEPTS_LREF>::value));

      // For 2017-02-10 -Wc++11-narrowing bug
      //   clang++ -std=c++11 conditional 第一引数に int を渡すと SFINAE で候補から外れる。
      //   しかしこれは SFINAE 的に正しい動作なのだろうか。
      mwg_check((ns::functor_traits_impl<int Rect::*, int const& (Rect const&)>::value));
      mwg_check((ns::_switch<int Rect::*, int (Rect const&)>::value));
      mwg_check((mwg::functor_detail::functor_traits_member<int Rect::*, int (Rect const&)>::value));
      mwg_check((mwg::as_fun<int Rect::*, int (Rect const&)>::value));

      // 2017-02-12 fix value of ACCEPTS_PTR flag
      mwg_check((ns::functor_traits_impl<int Rect::*, int& (Rect&)>::value));
      mwg_check((ns::functor_traits_impl<int Rect::*, int& (Rect*)>::value));
      mwg_check((mwg::stdm::is_same<ns::check_signature<int, Rect, int& (Rect*), ns::ACCEPTS_PTR>::type, int& (Rect*)>::value));
      mwg_check((ns::check_signature<int, Rect, int& (Rect*), ns::ACCEPTS_PTR>::value));
      mwg_check((ns::check_signature_cv<int, Rect, int& (Rect*), ns::ACCEPTS_PTR>::value));
      mwg_check((ns::_switch<int Rect::*, int& (Rect*)>::value));
    }
#ifdef MWGCONF_STD_AUTO_TYPE
    auto f1 = mwg::fun<int& (Rect&)>(&Rect::x);
    auto f2 = mwg::fun<int (Rect const&)>(&Rect::x);
    auto f3 = mwg::fun<int& (Rect*)>(&Rect::x);
    auto f4 = mwg::fun<int (Rect const*)>(&Rect::x);
#else
    mwg::as_fun<int Rect::*, int& (Rect&)>::adapter f1(&Rect::x);
    mwg::as_fun<int Rect::*, int (Rect const&)>::adapter f2(&Rect::x);
    mwg::as_fun<int Rect::*, int& (Rect*)>::adapter f3(&Rect::x);
    mwg::as_fun<int Rect::*, int (Rect const*)>::adapter f4(&Rect::x);
#endif
    f1(rect1) = 12;
    mwg_check((rect1.x == 12));
    mwg_check((f2(rect1) == 12));
    f3(&rect1) = 321;
    mwg_check((rect1.x == 321));
    mwg_check((f4(&rect1) == 321));

    {
      namespace ns = mwg::functor_detail::member_function_pointer_traits;
      mwg_check((mwg::stdm::is_same<type_traits::is_member_pointer<int (Rect::*)() const>::member_type, int()>::value));
      mwg_check((mwg::stdm::is_same<type_traits::is_member_pointer<int (Rect::*)() const>::object_type, Rect const>::value));
      mwg_check((mwg::stdm::is_same<mwg::funsig::shift<int(), Rect const&>::type, int (Rect const&)>::value));
      mwg_check((type_traits::is_variant_function<int (Rect const&), int (Rect const&)>::value));

      mwg_check((mwg::stdm::is_same<type_traits::is_member_pointer<void (Rect::*)(int, int)>::member_type, void(int, int)>::value));
      mwg_check((mwg::stdm::is_same<type_traits::is_member_pointer<void (Rect::*)(int, int)>::object_type, Rect>::value));
      mwg_check((mwg::stdm::is_same<mwg::funsig::shift<void (int, int), Rect const&>::type, void (Rect const&, int, int)>::value));

      mwg_check((ns::_switch<int (Rect::*)() const, int (Rect const&)>::value));
    }
#ifdef MWGCONF_STD_AUTO_TYPE
    auto g1 = mwg::fun<int (Rect const&)>(&Rect::right);
    auto g2 = mwg::fun<int (Rect const*)>(&Rect::right);
    auto g3 = mwg::fun<void (Rect&, int const&, int const&)>(&Rect::translate);
#else
    mwg::as_fun<int (Rect::*)() const, int (Rect const&)>::adapter g1(&Rect::right);
    mwg::as_fun<int (Rect::*)() const, int (Rect const*)>::adapter g2(&Rect::right);
    mwg::as_fun<void (Rect::*)(int, int), void (Rect&, int const&, int const&)>::adapter g3(&Rect::translate);
#endif
    mwg_check((g1(rect1) == rect1.x + rect1.w));
    mwg_check((g2(&rect1) == rect1.x + rect1.w));

    rect1.x = 123;
    rect1.y = 321;
    g3(rect1, 4, 1);
    mwg_check((rect1.x == 127 && rect1.y == 322));
  }
}

int main() {
  test_funcsig::run();
  test_function::run();
  test_member::run();

  managed_test::run_tests();
  return 0;
}

#pragma%x end_check
#endif
