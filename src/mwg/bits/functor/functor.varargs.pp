// -*- mode:C++;coding:utf-8 -*-
#%(
//******************************************************************************
//  可変長引数関数
//------------------------------------------------------------------------------
  template<typename F>
  struct functor_traits_switch<F*,void,5>;
  template<typename F>
  struct functor_traits_switch<F,void,5>;
  template<int AR,typename R,typename A1...>
  struct construct_signature;
  template<typename Sv,typename Sc>
  struct get_vaarg_variance;
  template<typename Sv,typename S>
  struct functor_invoker_vaarg;
  template<typename S,typename F>
  struct functor_traits_switch<F*,S,5>;
  template<typename S,typename F>
  struct functor_traits_switch<F,S,5>;
//******************************************************************************
#%)
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
#%expand (
#define MWG_TMP_TYPENAMES typename R,$".for|@|1|ARITY_MAX+1|typename A@|,"
#define MWG_TMP_TYPES     R,$".for|@|1|ARITY_MAX+1|A@|,"
  template<int AR,MWG_TMP_TYPENAMES> struct construct_signature;
  #%expand (
  template<MWG_TMP_TYPENAMES> struct construct_signature<@,MWG_TMP_TYPES>{typedef R (type)($".for|#|1|@+1|A#|,");};
  #%).f|@|0|ARITY_MAX+1|
#undef MWG_TMP_TYPENAMES
#undef MWG_TMP_TYPES
#%).i
  template<typename Sv,typename Sc>
  struct get_vaarg_variance{
    typedef functor_traits<Sv> SvTr;
    typedef functor_traits<Sc> ScTr;
    typedef typename SvTr::ret_t ret_t;
#%expand (
    typedef typename mwg::stdm::conditional<
      stdm::is_same<typename SvTr::arg@_t,void>::value,
      typename ScTr::arg@_t,typename SvTr::arg@_t
    >::type arg@_t;
#%).f|@|1|ARITY_MAX+1|
#%expand (
    typedef typename construct_signature<ScTr::arity,ret_t,${.for|@|1|ARITY_MAX+1|arg@_t|,}>::type sgn_t;
#%).i
  };
//------------------------------------------------------------------------------
  template<typename Sv,typename S>
  struct functor_invoker_vaarg;
#%define 1
  template<typename Sv,typename R %s_typenames%>
  struct functor_invoker_vaarg<Sv,R(%types%)>{
    typedef R(sgn_t)(%types%);
    static R invoke(Sv* f %s_params%){
      return R(f(%args%));
      //return R(reinterpret_cast<sgn_t*>(f)(%args%));
    }
  };
#%define end
#%expand mwg::functor::arities
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
