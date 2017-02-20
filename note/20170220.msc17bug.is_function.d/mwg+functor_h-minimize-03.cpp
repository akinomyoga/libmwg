// -*- coding: cp932 -*-
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <new>
#include <mwg/concept.h>
#include <mwg/except.h>
#include <mwg/funcsig.h>
#include <mwg/functor.proto.h>
#include <mwg/std/type_traits>
#include <mwg/std/utility>

namespace mwg{
namespace functor_detail{
  namespace sig=mwg::funcsig;

  template<typename F,typename T>
  struct is_covariant:stdm::integral_constant<bool,(stdm::is_void<T>::value||stdm::is_convertible<F,T>::value)>{};

  template<typename FSgn, typename TSgn, typename = void>
  struct is_variant_function: stdm::false_type {};

  namespace detail{
    template<
      typename FromSignature,typename ToSignature,
      std::size_t K      = 0,
      typename FromParam = typename sig::parameter<K,FromSignature>::type,
      typename ToParam   = typename sig::parameter<K,ToSignature  >::type,
      bool               = is_covariant<ToParam,FromParam>::value>
    struct has_contravariant_parameters:stdm::false_type{};

    template<
      typename FromSignature,typename ToSignature,std::size_t K,
      typename FromParam,typename ToParam>
    struct has_contravariant_parameters<FromSignature,ToSignature,K,FromParam,ToParam,true>
      :has_contravariant_parameters<FromSignature,ToSignature,K+1>{};

    template<typename FromSignature,typename ToSignature,std::size_t K>
    struct has_contravariant_parameters<FromSignature,ToSignature,K,void,void,true>:stdm::true_type{};
  }

  template<typename FSgn, typename TSgn>
  struct is_variant_function<FSgn, TSgn, typename stdm::enable_if<stdm::is_function<FSgn>::value && stdm::is_function<TSgn>::value>::type>:
    stdm::integral_constant<bool,
    (is_covariant<typename sig::returns<FSgn>::type, typename sig::returns<TSgn>::type>::value &&
      detail::has_contravariant_parameters<FSgn,TSgn>::value)>{};

  struct functor_traits_empty {
    // not functor
    static const bool is_functor = false;

    typedef void fct_t;
    typedef void sgn_t;
    typedef void ref_tr;
    typedef void ins_tr;
  };

  template<typename S>
  struct functor_traits_signature {};

  template<typename R>
  struct functor_traits_signature<R()>: functor_traits_empty {
    static const bool is_functor = true;
    typedef R (sgn_t)();
  };
  template<typename R,typename A0,typename A1>
  struct functor_traits_signature<R(A0,A1)>: functor_traits_empty {
    static const bool is_functor = true;
    typedef R (sgn_t)(A0,A1);
  };

  template<
    typename S,
    typename F,
    typename RefCaseTr = void,
    typename InsCaseTr = void >
  struct functor_traits_impl: functor_traits_signature<S> {
    typedef F fct_t;
    typedef RefCaseTr ref_tr;
    typedef InsCaseTr ins_tr;
  };

  struct functor_traits_end: functor_traits_empty {};

  template<int L, typename F, typename = void>
  struct functor_traits_chain: functor_traits_empty {};

  template<int L, typename F, typename S, typename = void>
  struct functor_traits_chain2: functor_traits_empty {};


  namespace detail {
    template<
      typename F, int L = 0,
      bool = functor_traits_chain<L, F>::is_functor,
      bool = !stdm::is_base_of<functor_traits_end, functor_traits_chain<L, F> >::value>
    struct functor_traits_selector: functor_traits_empty {};
    template<typename F, int L, bool B>
    struct functor_traits_selector<F, L, true ,B>: functor_traits_chain<L, F> {};
    template<typename F, int L>
    struct functor_traits_selector<F, L, false, true>: functor_traits_selector<F, L + 1> {};
    template<typename F, int L>
    struct functor_traits_selector<F, L, false, false>: functor_traits_empty {};

    template<
      typename F, typename S = void, int L = 0,
      bool = functor_traits_chain2<L, F, S>::is_functor,
      bool = !stdm::is_base_of<functor_traits_end, functor_traits_chain2<L, F, S> >::value>
    struct functor_traits_selector2: functor_traits_empty {};
    template<typename F, typename S, int L, bool B>
    struct functor_traits_selector2<F, S, L, true ,B>: functor_traits_chain2<L, F, S> {};
    template<typename F, typename S, int L>
    struct functor_traits_selector2<F, S, L, false, true>: functor_traits_selector2<F, S, L + 1> {};
    template<typename F, typename S, int L>
    struct functor_traits_selector2<F, S, L, false, false>: functor_traits_empty {};
  }

  template<typename F, typename S>
  struct functor_traits: detail::functor_traits_selector2<F, S> {};
  template<typename F>
  struct functor_traits<F, void>: detail::functor_traits_selector<F> {};


  template<typename F>
  struct functor_traits_chain<1,F*,typename std::enable_if<std::is_function<F>::value>::type>:functor_traits_impl<F,F*> {};
  template<typename F>
  struct functor_traits_chain<1,F,typename std::enable_if<std::is_function<F>::value>::type>:functor_traits_impl<F,F*> {};

  template<typename F,typename T,bool=(functor_traits<F>::is_functor&&functor_traits<T>::is_functor)>
  struct is_variant_functor:stdm::false_type{};
  template<typename F,typename T>
  struct is_variant_functor<F,T,true>
    :is_variant_function<typename functor_traits<F>::sgn_t,typename functor_traits<T>::sgn_t>{};

  template<typename F,typename S>
  struct functor_traits_chain2<5,F,S,typename stdm::enable_if<is_variant_functor<F,S>::value>::type>
    :functor_traits<F>{typedef S sgn_t;};

  template<typename F>
  struct functor_traits_chain<7, F, void>: functor_traits_end {};
  template<typename F, typename S>
  struct functor_traits_chain2<7, F, S, void>: functor_traits_end {};

  template<typename F, typename S>
  struct is_functor: stdm::is_same<S, typename mwg::functor_traits<F>::sgn_t> {};
  template<typename F, typename S>
  struct be_functor: stdm::integral_constant<bool, functor_traits<F, S>::is_functor> {};

  template<typename F, typename S> struct be_functor_dummy: stdm::true_type {}; // これを使ってもならない

  template<typename S>
  struct functor {
    //template<typename F> explicit functor(const F& f) {} // これだとならない。

    template<typename F> explicit functor(const F& f, typename stdm::enable_if<be_functor<F, S>::value, void*>::type = nullptr) {}

    functor(functor const&) {}

    //template<typename F>
    //typename stdm::enable_if<be_functor<F, S>::value, functor&>::type
    //typename stdm::enable_if<is_functor<F, S>::value, functor&>::type
    //functor& operator=(const F& f) {return *this;}

    template<typename F> void operator=(const F&) {} // 戻り値は void でも再現する。
  };

} // end of functor_detail
} // end of mwg

int func1() {
  return 123;
}

char func2() {return 1;}

int main() {
  mwg::functor<int(int, int)> g1(&func1);
  //mwg::functor<int(int, char)> g1(&func1); // なんと int(int, char) だと再現しない。
  mwg_printd("be_functor<ptr, fun> = %d", mwg::functor_detail::be_functor<int (*)(), int(int, int)>::value? 1: 0);
  mwg_printd("be_functor<ptr, fun> = %d", mwg::functor_detail::be_functor<int (*)(), int(int, int)>::value? 1: 0);
  g1 = func1;
  //g1.operator=(func1); // メンバ関数としての呼び出しだと再現しない。
  // g1 = 0; // 違う型だと再現しない。
  // g1 = func2; // 違う関数型でも再現しない。
  // tf(func1); // テンプレート関数に渡すだけではならない。
  // a = func1; // 別のクラスに代入してもならない。
  mwg_printd("be_functor<ptr, fun> = %d", mwg::functor_detail::be_functor<int (*)(), int(int, int)>::value? 1: 0);
  mwg_printd("be_functor<ptr, fun> = %d", mwg::functor_detail::be_functor<int (*)(), int(int, int)>::value? 1: 0);

  return 0;
}
