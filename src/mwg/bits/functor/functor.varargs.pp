// -*- mode:C++;coding:utf-8 -*-
#pragma%begin
//******************************************************************************
//  可変長引数関数
//------------------------------------------------------------------------------
  template<typename F>
  struct functor_traits_switch<F*,void,5>;
  template<typename F>
  struct functor_traits_switch<F,void,5>;
  template<typename Sv,typename Sc>
  struct get_vaarg_variance;
  template<typename Sv,typename S>
  struct functor_invoker_vaarg;
  template<typename S,typename F>
  struct functor_traits_switch<F*,S,5>;
  template<typename S,typename F>
  struct functor_traits_switch<F,S,5>;
//******************************************************************************
#pragma%end
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class functor_traits<R (*)(As...)>
//------------------------------------------------------------------------------
  template<typename S>
  struct functor_traits_switch<S*,void,5>:functor_traits_signature<S>{typedef S* fct_t;};
  template<typename S>
  struct functor_traits_switch<S,void,5>:functor_traits_signature<S>{typedef S fct_t;};
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  class functor_traits<R (*)(As...),S>
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
  template<typename Sv,typename S>
  struct functor_invoker_vaarg;
#pragma%m 1
  template<typename Sv,typename R %s_typenames%>
  struct functor_invoker_vaarg<Sv,R(%types%)>{
    typedef R(sgn_t)(%types%);
    static R invoke(Sv* f %s_params%){
      return R(f(%args%));
      //return R(reinterpret_cast<sgn_t*>(f)(%args%));
    }
  };
#pragma%end
#pragma%x mwg::functor::arities
  template<typename S,typename F>
  struct functor_traits_switch<F*,S,5>
    :functor_traits_signature<S>
    ,functor_invoker_vaarg<F,S>
  {
    typedef F* fct_t;
    typedef struct case_tr:functor_case_traits<F*>{
      typedef functor_traits<F,S> fct_tr;
    } ref_tr,ins_tr;

    // typedef typename get_vaarg_variance<fct_t,S>::sgn_t data_sgn;
    // typedef struct case_tr:functor_case_traits<data_sgn*>{
    //   using functor_case_traits<data_sgn*>::endata;
    //   static data_sgn* endata(mwg_vc_typename functor_traits_switch::fct_t f){
    //     printf("dbg: ***** hello! endata! ***** %s\n",__PRETTY_FUNCTION__);
    //     return reinterpret_cast<data_sgn*>(f);
    //   }
    // } ref_tr,ins_tr;
  };
  template<typename S,typename F>
  struct functor_traits_switch<F,S,5>:functor_traits_switch<F*,S,5>{
    typedef F fct_t;
  };
