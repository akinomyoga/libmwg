// -*- mode: c++; coding: utf-8 -*-
namespace mwg{
namespace functor_detail{
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//
//  (1) 関手の共変性・反変性 [is_variant_functor]
//
//    functor_traits_chain2<F,S>
//
//  (2) decltype [can_be_called_as]
//
//    functor_traits_chain2<F,S>
//      F where is_valid_expression(R( declval<F>()(declval<A>()...) ))
//

  /*?lwiki
   * :@var mwg::functor_detail::==is_variant_functor==<typename From,typename To>::value;
   *  関手 `From` を関手 `To` に変換できるかどうかを判定します。
   */
  template<typename F,typename T,bool=(functor_traits<F>::is_functor&&functor_traits<T>::is_functor)>
  struct is_variant_functor:stdm::false_type{};
  template<typename F,typename T>
  struct is_variant_functor<F,T,true>
    :is_variant_function<typename functor_traits<F>::sgn_t,typename functor_traits<T>::sgn_t>{};

#pragma%m 1
  template<typename F,typename S>
  struct functor_traits_chain2<@,F,S,typename stdm::enable_if<is_variant_functor<F,S>::value>::type>
    :functor_traits<F>{typedef S sgn_t;};
#pragma%end
#pragma%x functor_traits_chain::register

#pragma%m 1
  template<typename F,typename S>
  struct functor_traits_chain2<@,F,S,typename stdm::enable_if<can_be_called_as<F,S>::value>::type>
    :functor_traits_signature<typename can_be_called_as<F,S>::signature_type>
    ,functor_invoker_functor<typename can_be_called_as<F,S>::signature_type,F>
  {
    typedef F fct_t;
    struct ref_tr:functor_case_traits_FunctorRef<F>{typedef functor_traits_chain2 fct_tr;};
    struct ins_tr:functor_case_traits           <F>{typedef functor_traits_chain2 fct_tr;};
  };
#pragma%end
#pragma%x functor_traits_chain::register
}
}
