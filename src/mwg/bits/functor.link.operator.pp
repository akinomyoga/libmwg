// -*- mode: c++; coding: utf-8 -*-
namespace mwg{
namespace functor_detail{
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//
//  (1) 関数呼出演算子が定義されているクラス [traits.functor]
//
//    functor_traits_chain<F>
//      F where `declval<F>().operator()(...)` is a valid expression.
//
//  (2) 関手への間接参照が定義されているクラス [traits.pfunctor]
//
//    functor_traits_chain<F>
//      F where `(*declval<F>()).operator()(...)` is a valid expression.
//
// TODO
//
//   * operator() と operator() const の両方が定義されている場合
//     どちらか一方を優先させる■
//
//     decltype(&F::operator()) では望んだ方のポインタを取得出来ない
//

  template<typename C>
  struct functor_case_traits_FunctorRef:functor_case_traits<C>{
    typedef const C* case_data;
    static const C* endata(const C& ins){return &ins;}
    static const C& dedata(const C* p){return *p;}
  };

  template<typename S,typename F>
  struct functor_invoker_functor{};
#pragma%m 1
  template<typename R,typename C,typename... A>
  struct functor_invoker_functor<R(A...),C>{
    static R invoke(C const& f,A... a,...){
      return R(const_cast<C&>(f)(a...));
    }
  };
#pragma%end
#pragma%x variadic_expand_0toArN

  template<typename S,typename P,typename C>
  struct functor_invoker_pfunctor{};
#pragma%m 1
  template<typename P,typename C,typename R,typename... A>
  struct functor_invoker_pfunctor<R(A...),P,C>{
    static R invoke(const P& f,A... a,...){
      return R(const_cast<C&>(*f)(a...));
    }
  };
#pragma%end
#pragma%x variadic_expand_0toArN

#if mwg_has_feature(cxx_decltype)
//-----------------------------------------------------------------------------
// [traits.functor] F with R F::operator()(%types%)
  template<typename Mfp>
  struct functor_traits_ftorF
    :functor_traits_impl<
      typename is_memfun_pointer<Mfp>::member_type,
      typename is_memfun_pointer<Mfp>::object_type,
      functor_case_traits_FunctorRef<typename is_memfun_pointer<Mfp>::object_type>
    >
    ,functor_invoker_functor<
      typename is_memfun_pointer<Mfp>::member_type,
      typename is_memfun_pointer<Mfp>::object_type
    >
  {};

#pragma%m 1
  template<typename F>
  struct functor_traits_chain<@,F,typename stdm::enable_if<has_single_operator_functor<F>::value>::type>
    :functor_traits_ftorF<decltype(&F::operator())>{};
#pragma%end
#pragma%x functor_traits_chain::register

//-----------------------------------------------------------------------------
// [traits.pfunctor] P with F P::operator*() / R F::operator()(%types%)

  template<typename P,typename SOp>
  struct functor_traits_ftorP
    :functor_traits_impl<typename is_memfun_pointer<SOp>::member_type,P>
    ,functor_invoker_pfunctor<
      typename is_memfun_pointer<SOp>::member_type,
      P,
      typename is_memfun_pointer<SOp>::object_type
    >
  {};

#pragma%m 1
  template<typename P>
  struct functor_traits_chain<@,P,typename stdm::enable_if<is_pointer_to_single_operator_functor<P>::value>::type>
    :functor_traits_ftorP<P,typename is_pointer_to_single_operator_functor<P>::operator_type>{};
#pragma%end
#pragma%x functor_traits_chain::register
//-----------------------------------------------------------------------------

#endif
}
}
