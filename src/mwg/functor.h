// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_FUNCTOR_H
#define MWG_FUNCTOR_H
#pragma%x begin_check
#include <cstdio>
#include <cstring>
#include <mwg/std/type_traits>
#include <mwg/std/utility>
#include <mwg/except.h>
#include <mwg/concept.h>
#include <mwg/functor.h>

//-----------------------------------------------------------------------------
// Test targets

int func1() {
  return 123;
}

class C {
  int x;
public:
  C(int x): x(x) {}
  void print() const {
    std::printf("C::print(): this->x =%d\n", this->x);
  }

  int getValue() const {return this->x;}
};

class F {
  int x;
public:
  F(int x): x(x) {}
  int operator()(int y) const {
    //std::printf("F::print(int): x+y=%d\n", x + y);
    return x + y;
  }
};

class F2 {
public:
  int operator()(int) const {return 1;}
  int operator()(float) const {return 1;}
  int operator()(int, int) const {return 2;}
  int operator()(int x, int y, int z) const {return x + y + z;}
};

struct Str {
  int x;
  int y;
};

#pragma%x end_check
#include <new>
#include <algorithm>
#include <cstring>
#include <mwg/std/type_traits>
#include <mwg/std/utility>
#include <mwg/concept.h>
#include "funcsig.h"
#include "functor.proto.h"

#define mwg_attribute(X) mwg_attribute_##X
#if MWGCONF_GCC_VER > 30300
# define mwg_attribute_may_alias __attribute__((may_alias))
#else
# define mwg_attribute_may_alias
#endif

#pragma%include "impl/VariadicMacros.pp"
#pragma%include "bits/functor.type_traits.pp"

namespace mwg {
namespace functor_detail {
  namespace sig = mwg::funcsig;
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
//
//    Functor Traits
//
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  concept functor_traits
//-----------------------------------------------------------------------------
  /*?lwiki
   * &pre*(!cpp){
   * template<typename F>
   * struct functor_traits {
   *   static const bool is_functor;
   *   typedef F fct_t;
   *   typedef auto sgn_t;
   *
   *   typedef auto ref_tr; // functor への参照を保持
   *   typedef auto ins_tr; // functor の複製インスタンスを保持
   *   static '''return-type''' invoke(const fct_t& f, ...) {
   *     f(...);
   *   }
   * };
   * template<typename F, typename S>
   * struct functor_traits {
   *   static const bool is_functor;
   *   typedef F fct_t;
   *   typedef S sgn_t;
   *
   *   typedef auto ref_tr; // functor への参照を保持
   *   typedef auto ins_tr; // functor の複製インスタンスを保持
   *   static '''return-type''' invoke(const fct_t& f, ...) {
   *     f(...);
   *   }
   * };
   * }
   */
  template<typename F>
  struct functor_case_traits {
    typedef F fct_t;
    typedef functor_traits<F> fct_tr;
    typedef fct_t case_data;
    static const fct_t& endata(const fct_t& f) {return f;}
    static const fct_t& dedata(const fct_t& f) {return f;}
  };
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  functor_traits implementation helpers
//-----------------------------------------------------------------------------
  struct functor_traits_empty {
    // not functor
    static const bool is_functor = false;

    typedef void fct_t;
    typedef void sgn_t;
    typedef void ref_tr;
    typedef void ins_tr;
  };

  /*?lwiki
   * :@class class functor_traits_signature<S>;
   *  `functor_traits` 特殊化の実装に使うヘルパクラスです。以下の物を定義します。
   *  &pre(!cpp){
   *  static const bool is_functor = true;
   *  typedef S sgn_t;
   *  }
   *  `struct functor_traits_signature` で定義される物の他に以下の物を定義する必要があります。
   *  &pre(!cpp){
   *  typedef auto fct_t;                // raw functor type
   *  typedef auto ref_tr;               // traits of mwg::functor_ref case
   *  typedef auto ins_tr;               // traits of mwg::functor data case
   *  static R invoke(const fct_t&, ...); // invoke functor object
   *  }
   */
  template<typename S>
  struct functor_traits_signature {};
#pragma%m 1
  template<typename R, typename... A>
  struct functor_traits_signature<R(A...)>: functor_traits_empty {
    static const bool is_functor = true;
    typedef R (sgn_t)(A...);
  };
  template<typename R, typename... A>
  struct functor_traits_signature<R(A..., ...)>: functor_traits_empty {
    static const bool is_functor = true;
    typedef R (sgn_t)(A..., ...);
  };
#pragma%end
#pragma%x variadic_expand_0toArN
  template<
    typename S,
    typename F,
    typename RefCaseTr = functor_case_traits<F>,
    typename InsCaseTr = functor_case_traits<F> >
  struct functor_traits_impl: functor_traits_signature<S> {
    typedef F fct_t;
    typedef RefCaseTr ref_tr;
    typedef InsCaseTr ins_tr;
  };
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT

  struct functor_traits_end: functor_traits_empty {};

  /*?lwiki
   * :@class template<int L, typename F, typename = void> struct functor_traits_chain;
   *  指定した型 `T` を関手として取り扱う方法を提供するリストです。
   *  `L` について特殊化を定義することにより新しい方法を追加します。
   *  具体的な `L` の値の決定は <?pp functor_traits_chain::register?>
   *  マクロを使用します。実際の使用例を参照して下さい。
   */
  template<int L, typename F, typename = void>
  struct functor_traits_chain: functor_traits_empty {};

  /*?lwiki
   * @class template<int L, typename F, typename S, typename = void> struct functor_traits_chain2;
   *  指定した型 `T` を関手 `S` として取り扱う方法を提供するリストです。
   *  `L` について特殊化を定義することにより新しい方法を追加します。
   *  具体的な `L` の値の決定は <?pp functor_traits_chain::register?>
   *  マクロを使用します。実際の使用例を参照して下さい。
   */
  template<int L, typename F, typename S, typename = void>
  struct functor_traits_chain2: functor_traits_empty {};

#pragma%[functor_traits_chain_count=0]
#pragma%m functor_traits_chain::register
#pragma%%x 1.r/@/$"functor_traits_chain_count"/.i
#pragma%%[functor_traits_chain_count++]
#pragma%end
#pragma%m functor_traits_chain::terminate
#pragma%%x
  template<typename F>
  struct functor_traits_chain<$"functor_traits_chain_count", F, void>: functor_traits_end {};
  template<typename F, typename S>
  struct functor_traits_chain2<$"functor_traits_chain_count", F, S, void>: functor_traits_end {};
#pragma%%end.i
#pragma%end

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
}
}

#pragma%include "bits/functor.link.function.pp"
#pragma%include "bits/functor.link.member.pp"
#pragma%include "bits/functor.link.operator.pp"
#pragma%include "bits/functor.link.variance.pp"

namespace mwg {
namespace functor_detail {

#pragma%x functor_traits_chain::terminate

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class is_functor/be_functor
//-----------------------------------------------------------------------------
  template<typename F, typename S>
  struct is_functor: stdm::is_same<S, typename mwg::functor_traits<F>::sgn_t> {};
  template<typename F, typename S>
  struct be_functor: stdm::bool_constant<functor_traits<F, S>::is_functor> {};

  template<typename F, typename S, typename CaseTr>
  struct _as_functor_adapter {};
// TODO: struct _as_functor_adapter<F, R (A..., ...)>;
#pragma%m 1
  template<typename F, typename CaseTr, typename R, typename... A>
  struct _as_functor_adapter<F, R (A...), CaseTr> {
    typename CaseTr::case_data m_ref;
    _as_functor_adapter(F const& func): m_ref(CaseTr::endata(func)) {}
    R operator()(A... a) const {
      return mwg::functor_traits<F, R (A...)>::invoke(CaseTr::dedata(m_ref), stdm::forward<A>(a)...);
    }
  };
#pragma%end
#pragma%x variadic_expand_0toArN

  template<typename F, typename S, typename = void>
  struct _as_functor: mwg::stdm::false_type {};
  template<typename F, typename S>
  struct _as_functor<F, S, typename mwg::stdm::enable_if<mwg::be_functor<F, S>::value>::type>: mwg::stdm::true_type {
    typedef _as_functor_adapter<F, S, typename functor_traits<F, S>::ref_tr> adapter;
  };

  template<typename F, typename S>
  struct as_functor: _as_functor<F, S> {};
}
  using functor_detail::as_functor;
}


namespace mwg {
namespace functor_detail {

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
//
//    Functor Classes
//
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  interface functor_case
//-----------------------------------------------------------------------------

  //---------------------------------------------------------------------------
  /*?lwiki
   * @interface template<typename S> class functor_case;
   */
#pragma%m 1
  template<typename R, typename... A>
  struct functor_case<R(A...)> {
    virtual R call(A...) const = 0;
    virtual ~functor_case() {}
    virtual functor_case* placement_clone(void* ptr) const = 0;
    virtual functor_case* placement_move(void* ptr) const = 0;
  };
#pragma%end
#pragma%x variadic_expand_0toArN

  //---------------------------------------------------------------------------
  /*?lwiki
   * :@class template<typename T, std::size_t I> class limited_storage;
   *  上限サイズを定めてデータを保持するクラスです。
   *  `T` を格納するのに必要なサイズが `I` 以下の時はメンバとして内部に保持します。
   *  `T` のサイズが `I` に収まらない時は、`new`/`delete` によってヒープに確保します。
   */
  namespace detail {
    // 参照型の実質サイズを正確に取るため holder に入れてから sizeof する。
    template<typename F>
    struct limited_storage_holder {F value;};

    template<typename F, std::size_t I, bool IsFunction>
    struct limited_storage_is_interior_impl
      :stdm::bool_constant<(sizeof(limited_storage_holder<F>) <= I)> {};
    template<typename F, std::size_t I>
    struct limited_storage_is_interior_impl<F, I, true>
      :stdm::bool_constant<(sizeof(limited_storage_holder<F*>) <= I)> {};
    template<typename F, std::size_t I>
    struct limited_storage_is_interior
      :limited_storage_is_interior_impl<F, I, stdm::is_function<F>::value> {};
  }

  // CHK: sizeof(void*) * 2 の値は妥当か?
  template<
    typename T,
    std::size_t I = sizeof(void*) * 2,
    bool = detail::limited_storage_is_interior<T, I>::value>
  class limited_storage {
    typedef typename stdm::conditional<stdm::is_function<T>::value, T*,T>::type value_type;
    value_type* m_ptr;
  public:
    limited_storage(T const& value): m_ptr(new value_type(value)) {}
    limited_storage(limited_storage const& c): m_ptr(new value_type(*c.m_ptr)) {}
    ~limited_storage() {delete this->m_ptr; this->m_ptr = nullptr;}
    const value_type& ref() const {return *m_ptr;}

    limited_storage& operator=(limited_storage const&) mwg_std_deleted;
  };

  template<typename T, std::size_t I>
  class limited_storage<T, I, true> {
    T m_data;
    typedef typename stdx::add_const_reference<T>::type const_reference;
  public:
    limited_storage(const_reference value): m_data(value) {}
    limited_storage(limited_storage const& c): m_data(c.m_data) {}
    const_reference ref() const {return this->m_data;}

    limited_storage& operator=(limited_storage const&) mwg_std_deleted;
  };

  //---------------------------------------------------------------------------
  /*?lwiki
   * @class template<typename S, typename Tr> class functor_case_impl;
   */
  template<typename S, typename Tr>
  class functor_case_impl;

  template<typename S, typename Tr>
  class functor_case_base: public functor_case<S> {
    typedef functor_case_impl<S, Tr> CRTP;

  public:
    typedef Tr case_tr; // used by others
    typedef typename Tr::case_data case_data;

    virtual functor_case<S>* placement_clone(void* ptr) const {
      return new(ptr) CRTP(this->ref());
    }
    virtual functor_case<S>* placement_move(void* ptr) const {
      return new(ptr) CRTP(mwg::stdm::move(this->ref()));
    }

  protected:
    functor_case_base(case_data const& value): m_data(value) {}
    case_data const& ref() const {return this->m_data.ref();}

  private:
    limited_storage<case_data, sizeof(void*) * 2> m_data;
  };

#pragma%m 1
  template<typename Tr, typename R, typename... A>
  class functor_case_impl<R(A...), Tr>: public functor_case_base<R(A...), Tr> {
    typedef functor_case_base<R(A...), Tr> base;
    typedef typename base::case_data case_data;
  public:
    functor_case_impl(case_data const& f): base(f) {} // used by placement clone
    template<typename F> functor_case_impl(F const& f): base(Tr::endata(f)) {}
    virtual R call(A... arg) const {
      typedef typename Tr::fct_tr fct_tr; // gcc-2.95.3 work around 一旦型に入れる必要有り。
      return R(fct_tr::invoke(Tr::dedata(this->ref()), arg...));
    }
  };
#pragma%end
#pragma%x variadic_expand_0toArN

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  interface functor_base
//-----------------------------------------------------------------------------
#pragma%m 1
  template<typename R, typename... A>
  struct functor_base<R(A...)> {
    functor_case<R(A...)>* h;
    R operator()(A... a) const {
      return this->h->call(a...);
    }
  };
#pragma%end
#pragma%x variadic_expand_0toArN
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class functor
//-----------------------------------------------------------------------------
  // 2016-03-25 gcc-2.95.3 bug work around:
  //   enable_if に複雑な式を指定すると ICE になる。
  template<typename F, typename S>
  struct is_explicit_functor: stdm::bool_constant<(!is_functor<F, S>::value && be_functor<F, S>::value)> {};

#pragma%m 1
  template<typename S>
  class functor_ref: public functor_base<S> {
    union {
      void* forceAlign;
      char buffer[3 * sizeof(void*)];
    };
  protected:
    functor_ref() {}
    template<typename F, typename Case>
    void init(const F& f) {
#if mwg_has_feature(cxx_static_assert)
      static_assert(sizeof(Case) <= sizeof(this->buffer), "sizeof(Case) too large");
#else
      static_assert(sizeof(Case) <= sizeof(mwg::declval<functor_ref>().buffer), "sizeof(Case) too large");
#endif
      static_assert((mwg::stdm::alignment_of<Case>::value <= mwg::stdm::alignment_of<void*>::value), "alignof(Case) too large");
      this->h = new(this->buffer) Case(f);
    }
  public:
    template<typename F>
    functor_ref(const F& f, typename stdm::enable_if<is_functor<F, S>::value, mwg::invalid_type*>::type = nullptr) {
      this->init<F, functor_case_impl<S, typename functor_traits<F>::ref_tr> >(f);
    }
    template<typename F>
    explicit functor_ref(const F& f, typename stdm::enable_if<is_explicit_functor<F, S>::value, mwg::invalid_type*>::type = nullptr) {
      this->init<F, functor_case_impl<S, typename functor_traits<F, S>::ref_tr> >(f);
    }
    template<typename F>
    typename stdm::enable_if<be_functor<F, S>::value, functor_ref&>::type
    operator=(const F& f) {
      this->free();
      this->init<F, functor_case_impl<S, typename functor_traits<F, S>::ref_tr> >(f);
      return *this;
    }
    ~functor_ref() {this->free();}

    functor_ref(const functor_ref& f) {
      this->h = f.h->placement_clone(this->buffer);
    }
    functor_ref& operator=(const functor_ref& f) {
      this->free();
      this->h = f.h->placement_clone(this->buffer);
      return *this;
    }
#if mwg_has_feature(cxx_rvalue_references)
    functor_ref(functor_ref&& f) {
      if (this == &f) return;
      this->h = f.h->placement_move(this->buffer);
    }
    functor_ref& operator=(functor_ref&& f) {
      if (this == &f) return *this;
      this->free();
      this->h = f.h->placement_move(this->buffer);
      return *this;
    }
#endif

    void swap(functor_ref& rhs) {
      functor_ref tmp(mwg::stdm::move(*this));
      *this = mwg::stdm::move(rhs);
      rhs = mwg::stdm::move(tmp);
    }
  private:
    void free() {
      if (this->h) {
        this->h->~functor_case<S>();
        this->h = nullptr;
      }
    }
  };
  template<typename S>
  class vfunctor_ref: public functor_ref<S> {
  public:
    vfunctor_ref() mwg_std_deleted;
    template<typename F>
    vfunctor_ref(const F& f, typename stdm::enable_if<is_functor<F, S>::value, mwg::invalid_type*>::type = nullptr) {
      this->template init<F, functor_case_impl<S, typename functor_traits<F>::ref_tr> >(f);
    }
    template<typename F>
    vfunctor_ref(const F& f, typename stdm::enable_if<!is_functor<F, S>::value && be_functor<F, S>::value, mwg::invalid_type*>::type = nullptr) {
      this->template init<F, functor_case_impl<S, typename functor_traits<F, S>::ref_tr> >(f);
    }
  };
#pragma%end
#pragma%x 1
#pragma%x 1.r#functor_ref#functor#.r#ref_tr#ins_tr#
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
} // end of functor_detail
} // end of mwg
//EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
/*?lwiki
 * &pre(!cpp){
 * template<typename F>
 * typename mwg::stdm::enable_if<mwg::is_functor<bool(), F>::value, void>::type
 * test1(const F& f) {
 *   puts(mwg::functor_traits<F>::invoke(f)? "O": "X");
 * }
 * template<typename F>
 * typename mwg::stdm::enable_if<mwg::be_functor<bool(), F>::value, void>::type
 * test2(const F& f) {
 *   puts(mwg::functor_traits<F, bool()>::invoke(f)? "O": "X");
 * }
 * }
 */
//EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
#endif
#pragma%x begin_check

//-----------------------------------------------------------------------------
// for debug

#ifdef MWG_FUNCTOR_H_VariantFunctorEnabled

#ifdef mwg_concept_is_valid_expression
template<typename F>
mwg_concept_is_valid_expression(has_single_operator_functor, F, F_, ((F_*) 0)->operator());
#elif defined(mwg_concept_is_valid_expression_vc2010A)
template<typename F>
mwg_concept_is_valid_expression_vc2010A(has_single_operator_functor, F, F_, ((F_*) 0)->operator());
#else
template<typename F>
struct has_single_operator_functor: mwg::stdm::false_type {};
#define mwgconf_mwg_functor_nosupport
#endif

template<typename P>
struct is_pointer_to_single_operator_functor {
#ifdef mwg_concept_is_valid_expression
  mwg_concept_is_valid_expression(c1_1, P, P_, ((P_*) 0)->operator*());
#elif defined(mwg_concept_is_valid_expression_vc2010A)
  mwg_concept_is_valid_expression_vc2010A(c1_1, P, P_, ((P_*) 0)->operator*());
#else
  struct c1_1: mwg::stdm::false_type {};
#endif

  struct c1: mwg::stdm::bool_constant<c1_1::value || mwg::stdm::is_pointer<P>::value> {};

  template<typename T> static T expr();

  template<typename P_, bool B> struct c2: mwg::stdm::false_type {
    typedef mwg::unknown_type operator_type;
  };
  template<typename P_> struct c2<P_, true> {
    typedef typename std::remove_reference<decltype(*expr<P_>())>::type functor_type;
    static const bool value = has_single_operator_functor<functor_type>::value;
    typedef decltype(&functor_type::operator()) operator_type;
  };

  mwg_concept_condition(c2<P, c1::value>::value);
  typedef typename c2<P, c1::value>::operator_type operator_type;
};

#endif

//-----------------------------------------------------------------------------
void debug_support_member_function_pointer() {
  mwg_check( mwg::stdm::is_member_function_pointer<void(C::*)() const>::value);
  mwg_check(!mwg::stdm::is_member_function_pointer<void(const C&)>::value);

  // mwg_check(mwg::functor_detail::signature_mfp<void(C::*)()>::is_mfp == true);
  // mwg_check(mwg::functor_detail::signature_mfp<int(C::*)()>::is_mfp == true);
  // mwg_check(mwg::functor_detail::signature_mfp<int(C::*)(int)>::is_mfp == true);

  mwg_check(( mwg::functor_detail::is_memfun_pointer<void(C::*)() const>::value));
  mwg_check((!mwg::functor_detail::is_memfun_pointer<void(const C&)>::value));
  mwg_check(( mwg::stdm::is_same<void(const C&), mwg::functor_detail::is_memfun_pointer<void(C::*)() const>::functor_sgn>::value));
  mwg_check(( mwg::is_functor<void(C::*)() const, void(const C&)>::value));
  mwg_check(( mwg::be_functor<void(C::*)() const, void(const C&)>::value));
}

void debug_support_functor_object() {
#ifdef MWG_FUNCTOR_H_VariantFunctorEnabled
  mwg_check(!(is_pointer_to_single_operator_functor<F>::c1_1::value));
  mwg_check(!(is_pointer_to_single_operator_functor<F>::c1::value));
  mwg_check(!(is_pointer_to_single_operator_functor<F>::value));
  mwg_check(!(mwg::functor_detail::is_pointer_to_single_operator_functor<F>::value));

  mwg_check( (mwg::functor_detail::functor_traits<F>::is_functor));
  mwg_check( (mwg::functor_traits<F>::is_functor));
#else
  mwg_check(!(mwg::functor_detail::is_pointer_to_single_operator_functor<F>::value));

  mwg_check(!(mwg::functor_traits<F>::is_functor));
  mwg_check( (mwg::functor_detail::can_be_called_as<F, int(int)>::value));
  //mwg_check( (mwg::functor_detail::functor_traits_chain2<6, F, int(int)>::is_functor));
  mwg_check( (mwg::functor_detail::functor_traits_signature<int(int)>::is_functor));
  mwg_check( (mwg::functor_traits<F, int(int)>::is_functor));
#endif

#if 0
  // debug
  typedef mwg::functor_detail::functor_case_traits_FunctorRef<F> case_tr;
  typedef mwg::functor_detail::functor_case_impl<void(int), case_tr> case_t;
  mwg_check((mwg::stdm::is_same<case_tr::case_data, const F*>::value));
  //mwg_check((mwg::stdm::is_same<case_t::base, mwg::functor_detail::functor_case_base<void(int), const F*,true> >::value));
  //case_t::base x(reinterpret_cast<const F*>(0));
#endif
}

//-----------------------------------------------------------------------------

void check_function_pointer() {
  // 1. function pointer
  mwg::functor<int()> f1(&func1);
  mwg_check(f1() == func1());
  f1 = func1;
  mwg_check(f1() == func1());
}

void check_member_function_pointer() {
  C instance(123);
  mwg::functor<int(const C&)> f2(&C::getValue);
  mwg_check(f2(instance) == 123);
  mwg::functor<void(C&)> f2_2(&C::getValue);
  f2_2(instance);

  //mwg::functor<void(const C&)> f2(&C::print); f2(C(123));
  //mwg::functor<void(C&)> f2_2(&C::print);
}

void check_functor_object() {
  F functor(5);
  mwg::functor<int(int)> f3(functor);
  mwg_check(f3(4) == functor(4));
  mwg::functor_ref<int(int)> f3_1(functor);
  mwg_check(f3_1(4) == functor(4));
}

void check_member_object_pointer() {
  mwg::functor<int&(Str&)> f4(&Str::x);
  Str hoge = {2011, 2012};
  mwg_check(f4(hoge) == hoge.x);
  mwg::functor<const int&(const Str&)> f4_1(&Str::x);
  mwg_check(f4_1(hoge) == 2011);
  mwg::functor<int(const Str&)> f4_2(&Str::x);
  mwg_check(f4_2(hoge) == 2011);
  mwg_check( (mwg::stdm::is_convertible<const int&, int>::value));

  f4(hoge) = 2016;
  mwg_check(hoge.x == 2016);
}

void check_variance() {
  {
    mwg::functor<int(int, int)> g1(&func1);
    mwg_check(g1(1, 2) == func1());
#if defined(_MSC_VER) && (_MSC_VER == 1700)
    //
    // work around for msc17 (Visual Studio 2012) bug
    //
    //   何故か代入演算子に関数型を指定すると、
    //   変換コンストラクタに指定していた std::remove_reference でその関数型を取れなくなる。
    //   仕方がないので関数ポインタでテストを行う。
    //   cf. note/20170220.msc17bug.is_function.cpp
    //
    g1 = &func1;
#else
    g1 = func1;
#endif
    mwg_check(g1(1, 2) == func1());
  }

  {
    mwg::functor<void(char*,const char*,int)> g2(sprintf);
    char buff[100];
    g2(buff, "hello! %dth world!\n", 4);
    mwg_check(std::strcmp(buff, "hello! 4th world!\n") == 0);
  }

#ifdef MWG_FUNCTOR_H_VariantFunctorEnabled
  mwg_check( (mwg::functor_detail::can_be_called_as<F, int(int)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F, int(short)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F, int(char)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F, int(float)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F, int(double)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F, int*(int)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F, int()>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F, int(int, int)>::value));

  mwg_check(!(mwg::functor_detail::can_be_called_as<F2, void()>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2, void(int)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2, void(int, int)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2, void(int, int, int)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2, void(int, int, int, int)>::value));
#else
  mwg_check( (mwg::functor_detail::can_be_called_as<F, int(int)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F, int(short)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F, int(char)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F, int(float)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F, int(double)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F, int*(int)>::value));
  mwg_check(!(mwg::functor_detail::can_be_called_as<F, void()>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F, int(int, int)>::value));

  mwg_check(!(mwg::functor_detail::can_be_called_as<F2, int()>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2, int(int)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2, int(int, int)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2, int(int, int, int)>::value));
  mwg_check( (mwg::functor_detail::can_be_called_as<F2, int(int, int, int, int)>::value));
#endif

  mwg::functor<int(int, int, int)> g3 = mwg::functor<int(int, int, int)>(F2());
  mwg_check( (g3(1, 2, 3) == 6));
}

int main() {
  typedef mwg::functor_traits<void (C::*)() const> mfp_ftr;

  // sizeof(functor_case)
  std::fprintf(stderr, "  sizeof(functor_case)=%zd sizeof(ins_t)=%zd\n",
    sizeof(mwg::functor_detail::functor_case<int()>),
    sizeof(mwg::functor_detail::functor_case_impl<mfp_ftr::sgn_t, mfp_ftr::ins_tr>)
  );

  check_can_be_called_as();

  //---------------------------------------------------------------------------
  check_function_pointer();

  debug_support_member_function_pointer();
  check_member_function_pointer();

  debug_support_functor_object();
  check_functor_object();

  check_member_object_pointer();

  //---------------------------------------------------------------------------
  check_variance();

  return 0;
}
#pragma%x end_check
