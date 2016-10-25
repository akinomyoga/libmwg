// -*- mode:C++;coding:utf-8 -*-
namespace mwg{
namespace functor_detail{
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//
//  (1) 可変長引数を持つ関数
//
//    functor_traits_chain<F>
//      F = R (As...,...)
//      F = R (*)(As...,...)
//    functor_traits_chain2<F,S>
//      F = R (As...,...)
//      F = R (*)(As...,...)
//

  namespace detail{
    template<std::size_t Arity,typename R,template<std::size_t> class Params>
    struct construct_signature:sig::arity_push<
      typename construct_signature<Arity-1,R,Params>::type,
      typename Params<Arity-1>::type>{};
    template<typename R,template<std::size_t> class Params>
    struct construct_signature<0,R,Params>{typedef R(type)();};
  }

  template<typename Sv,typename Sc>
  struct get_vaarg_variance{
  private:
    typedef typename functor_traits<Sv>::sgn_t vararg_signature;
    typedef typename functor_traits<Sc>::sgn_t caller_signature;

    template<typename X,typename Y>
    struct replace_void:stdm::conditional<!stdm::is_same<X,void>::value,X,Y>{};

    template<std::size_t K>
    struct params:replace_void<
      typename sig::parameter<K,vararg_signature>::type,
      typename sig::parameter<K,caller_signature>::type>{};

  public:
    typedef typename detail::construct_signature<
      sig::arity<caller_signature>::value,
      typename sig::returns<vararg_signature>::type,
      params
    >::type sgn_t;
  };

  template<typename S,typename Sv>
  struct functor_invoker_vaarg;
#pragma%m 1
  template<typename Sv,typename R,typename... A>
  struct functor_invoker_vaarg<R(A...),Sv>{
    typedef R (sgn_t)(A...);
    static R invoke(Sv* f,A... a){
      return R(f(a...));
      //return R(reinterpret_cast<sgn_t*>(f)(a...));
    }
  };
#pragma%end
#pragma%x variadic_expand_0toArN

#pragma%m 1
  template<typename S>
  struct functor_traits_chain<@,S*,typename stdm::enable_if<is_vararg_function<S>::value>::type>
    :functor_traits_signature<S>{typedef S* fct_t;};
  template<typename S>
  struct functor_traits_chain<@,S,typename stdm::enable_if<is_vararg_function<S>::value>::type>
    :functor_traits_signature<S>{typedef S fct_t;};

  template<typename F,typename S>
  struct functor_traits_chain2<@,F*,S,typename stdm::enable_if<is_vararg_function<F>::value>::type>
    :functor_traits_signature<S>
    ,functor_invoker_vaarg<S,F>
  {
    typedef F* fct_t;
    typedef struct case_tr:functor_case_traits<F*>{
      typedef functor_traits<F,S> fct_tr;
    } ref_tr,ins_tr;
  };

  // typedef typename get_vaarg_variance<fct_t,S>::sgn_t data_sgn;
  // typedef struct case_tr:functor_case_traits<data_sgn*>{
  //   using functor_case_traits<data_sgn*>::endata;
  //   static data_sgn* endata(mwg_vc_typename functor_traits_chain::fct_t f){
  //     printf("dbg: ***** hello! endata! ***** %s\n",__PRETTY_FUNCTION__);
  //     return reinterpret_cast<data_sgn*>(f);
  //   }
  // } ref_tr,ins_tr;

  template<typename F,typename S>
  struct functor_traits_chain2<@,F,S,typename stdm::enable_if<is_vararg_function<F>::value>::type>:functor_traits_chain2<@,F*,S>{
    typedef F fct_t;
  };
#pragma%end
#pragma%x functor_traits_chain::register

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//
//  (2) (固定長引数の) 関数
//
//  functor_traits_chain<F>
//    F = R (*)(A...)
//    F = R (A...)
//

  template<typename S,typename F>
  struct functor_invoker_function{};
#pragma%m 1
  template<typename F,typename R,typename... A>
  struct functor_invoker_function<R(A...),F>{
    static R invoke(F f,A... arg,...){
      return R(f(arg...));
    }
  };
#pragma%end
#pragma%x variadic_expand_0toArN

#pragma%m 1
  template<typename F>
  struct functor_traits_chain<@,F*,typename stdm::enable_if<stdm::is_function<F>::value>::type>
    :functor_traits_impl<F,F*>,functor_invoker_function<F,F*>{};
  template<typename F>
  struct functor_traits_chain<@,F,typename stdm::enable_if<stdm::is_function<F>::value>::type>
    :functor_traits_impl<F,F*>,functor_invoker_function<F,F*>{};
#pragma%end
#pragma%x functor_traits_chain::register

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
}
}
