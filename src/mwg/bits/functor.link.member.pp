// -*- mode: c++; coding: utf-8 -*-
namespace mwg{
namespace functor_detail{
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//
//  (1) メンバ関数ポインタ [traits.mfp]
//
//    functor_traits_chain<F>
//      F = R (C::*)(A...)
//      F = R (C::*)(A...,...) ■TODO
//
//  (2) メンバポインタ [traits.mp]
//
//    functor_traits_chain<F>
//      F = T C::*
//    functor_traits_chain2<F,S>
//      F = T C::*
//

  template<typename S,typename Mfp>
  struct functor_invoker_mfp;
#pragma%m 1
  template<typename Mfp,typename R,typename CRef,typename... A>
  struct functor_invoker_mfp<R(CRef,A...),Mfp>{
    static R invoke(Mfp f,CRef c,A... a){
      return R((c.*f)(a...));
    }
  };
#pragma%end
#pragma%x variadic_expand_0toArNm1

  template<typename S,typename Mop>
  struct functor_invoker_mop;
  template<typename Mop,typename R,typename C>
  struct functor_invoker_mop<R(C),Mop>{
    static typename stdm::add_lvalue_reference<R>::type invoke(Mop f,C& c,...){return c.*f;}
    static typename stdx::add_const_reference<R>::type invoke(Mop f,C const& c,...){return c.*f;}
  };

#pragma%m 1
  template<typename Mfp>
  struct functor_traits_chain<@,Mfp,typename stdm::enable_if<stdm::is_member_function_pointer<Mfp>::value>::type>
    : functor_traits_impl<typename is_memfun_pointer<Mfp>::functor_sgn,Mfp>
    , functor_invoker_mfp<typename is_memfun_pointer<Mfp>::functor_sgn,Mfp>{};
  template<typename T,typename C>
  struct functor_traits_chain<@,T C::*,typename stdm::enable_if<stdm::is_member_object_pointer<T C::*>::value>::type>
    : functor_traits_impl<T&(C&),T C::*>
    , functor_invoker_mop<T(C),T C::*>{};

  template<typename T,typename C,typename S>
  struct functor_traits_chain2<
    @,T C::*,S,
    typename stdm::enable_if<
      stdm::is_member_object_pointer<T C::*>::value&&(
        is_variant_function<typename stdm::add_lvalue_reference<T>::type (C&),S>::value||
        is_variant_function<typename stdx::add_const_reference<T>::type (C const&),S>::value
      )
    >::type >:functor_traits<T C::*>{typedef S sgn_t;};
#pragma%end
#pragma%x functor_traits_chain::register

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
}
}
