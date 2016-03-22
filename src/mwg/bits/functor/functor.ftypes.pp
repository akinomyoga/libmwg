// -*- mode:C++;coding:utf-8 -*-
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
//
//    Function Types
//
#%(
  template<typename F>
  struct is_vararg_function{
    static const bool value;
    typedef void without_vararg_sgn;
  };
  template<typename F>
  struct is_vararg_function_pointer{
    static const bool value;
    typedef void signature_type;
    typedef void without_vararg_sgn;
  };
  template<typename F>
  struct is_function_pointer{
    static const bool value;
    typedef void signature_type;
  };
  template<typename Mfp>
  struct is_memfun_pointer{
    static const bool value;
    typedef void member_type;
    typedef void object_type;
    typedef void functor_sgn;
  };
#%)
//------------------------------------------------------------------------------
  template<typename F>
  struct is_vararg_function:stdm::false_type{typedef void without_vararg_sgn;};
#%define 1 (
  template<typename R %s_typenames%>
  struct is_vararg_function<R(%types...%)>:stdm::true_type{
    typedef R(without_vararg_sgn)(%types%);
  };
#%)
#%expand mwg::functor::arities
  template<typename T>
  struct is_vararg_function_pointer:stdm::false_type{typedef void signature_type;};
  template<typename F>
  struct is_vararg_function_pointer<F*>:is_vararg_function<F>{typedef F signature_type;};
  template<typename T>
  struct is_function_pointer:stdm::false_type{typedef void signature_type;};
  template<typename F>
  struct is_function_pointer<F*>:stdm::is_function<F>{typedef F signature_type;};
//------------------------------------------------------------------------------
  template<typename Mfp>
  struct is_memfun_pointer:stdm::false_type{
    typedef void member_type;
    typedef void object_type;
    typedef void functor_sgn;
  };
#%define 1
  template<typename R,typename C %s_typenames%>
  struct is_memfun_pointer<R(C::*)(%types%)>:stdm::true_type{
    typedef C object_type;
    typedef R(member_type)(%types%);
    typedef R(functor_sgn)(C& $".sep_for|K|1|%AR%+1|AK|,");
  };
  template<typename R,typename C %s_typenames%>
  struct is_memfun_pointer<R(C::*)(%types%) const>:stdm::true_type{
    typedef const C object_type;
    typedef R(member_type)(%types%);
    typedef R(functor_sgn)(const C& $".sep_for|K|1|%AR%+1|AK|,");
  };
#%define end
#%expand mwg::functor::arities
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  判定クラス
//------------------------------------------------------------------------------
#%(
  template<typename F,typename T>
  struct is_variant_argument;
  template<typename TrSF,typename TrST>
  struct is_variant_signature;
  template<typename F>
  struct has_single_operator_functor;
  template<typename P>
  struct is_pointer_to_single_operator_functor;
//------------------------------------------------------------------------------
#%)
  template<typename T> T expr();
  template<typename F,typename T>
  struct is_variant_argument:stdm::integral_constant<bool,
    stdm::is_void<T>::value||stdm::is_convertible<F,T>::value
  >{};
  template<typename TrSF,typename TrST>
  struct is_variant_signature:stdm::integral_constant<bool,
    TrSF::is_functor&&TrST::is_functor
    &&is_variant_argument<typename TrSF::ret_t,typename TrST::ret_t>::value
#%define 1
    &&is_variant_argument<typename TrST::argK_t,typename TrSF::argK_t>::value
#%define end
#%expand 1.f|K|1|ARITY_MAX+1|
  >{};
//------------------------------------------------------------------------------
#if (\
    defined(mwg_concept_is_valid_expression)\
    ||defined(_MSC_VER)&&defined(mwg_concept_is_valid_expression_vc2010A)\
  )
#   define MWG_FUNCTOR_H__VariantFunctorEnabled
#endif
  /// <summary>
  /// F f: f.operator() が有効な型を判定します。
  /// </summary>
#ifdef mwg_concept_is_valid_expression
  template<typename F>
  mwg_concept_is_valid_expression(has_single_operator_functor,F,F_,&F_::operator());
#elif defined(_MSC_VER)&&defined(mwg_concept_is_valid_expression_vc2010A)
  template<typename F>
  mwg_concept_is_valid_expression_vc2010A(has_single_operator_functor,F,F_,((F_*)0)->operator());
#else
  template<typename F>
  struct has_single_operator_functor:mwg::stdm::false_type{};
#endif
  /// <summary>
  /// P p: p->operator() が有効な型を判定します。
  /// </summary>
  template<typename P>
  struct is_pointer_to_single_operator_functor{
#if defined(MWGCONF_STD_DECLTYPE)

# ifdef mwg_concept_is_valid_expression
    mwg_concept_is_valid_expression(c1,P,P_,*expr<P_>());
# elif defined(_MSC_VER)&&defined(mwg_concept_is_valid_expression_vc2010A)
    mwg_concept_is_valid_expression_vc2010A(c1_1,P,P_,expr<P_>().operator*());
    struct c1:stdm::integral_constant<bool,c1_1::value||stdm::is_pointer<P>::value>{};
# else
  struct c1:stdm::false_type{};
# endif

    template<typename F_,bool B=has_single_operator_functor<F_>::value>
    struct get_operator_functor:stdm::false_type{
      typedef mwg::unknown_type operator_type;
    };
    template<typename F_> struct get_operator_functor<F_,true>:stdm::true_type{
      typedef decltype(&F_::operator()) operator_type;
    };

    template<typename P_,bool B> struct c2:stdm::false_type{
      typedef mwg::unknown_type operator_type;
    };
# ifdef _MSC_VER
    template<typename P_> struct c2<P_,true>{
      typedef typename stdm::remove_reference<decltype(*expr<P_>())>::type functor_type;
      typedef typename get_operator_functor<functor_type>::type operator_type;
      static const bool value=get_operator_functor<functor_type>::value;
    };
# else
    template<typename P_> struct c2<P_,true>
      :get_operator_functor< typename stdm::remove_reference<decltype(*expr<P_>())>::type >
    {};
# endif

    mwg_concept_condition(c2<P,c1::value>::value);
    typedef typename c2<P,c1::value>::operator_type operator_type;
#else
    mwg_concept_condition(false);
    typedef mwg::unknown_type operator_type;
#endif
  };

//------------------------------------------------------------------------------
  /// <summary>
  /// F f: f.operator()() を指定した引数で呼出可能かどうかを判定します。
  /// </summary>
  template<typename F,typename S>
  struct can_be_called_as;
  template<typename F,typename S>
  struct can_be_called_as_impl1;
  template<typename F,typename S,bool CANBE1>
  struct can_be_called_as_impl2;

  template<typename F,typename S>
  struct can_be_called_as_impl1:stdm::false_type{};
#%define 1 (
  template<typename F,typename R %s_typenames%>
  struct can_be_called_as_impl1<F,R(%types%)>{

#ifdef mwg_concept_is_valid_expression
    mwg_concept_is_valid_expression(c1,F,F_,expr<F_>().operator()($".for:K:1:%AR%+1:expr<AK>():,"));
#elif defined(_MSC_VER)&&defined(mwg_concept_is_valid_expression_vc2010A)
    mwg_concept_is_valid_expression_vc2010A(c1,F,F_,expr<F_>().operator()($".for:K:1:%AR%+1:expr<AK>():,"));
#else
    mwg_concept_has_member(c1_1,F,X,operator(),R(X::*)(%types%));
    mwg_concept_has_member(c1_2,F,X,operator(),R(X::*)(%types%) const);
    struct c1:stdm::integral_constant<bool,c1_1::value||c1_2::value>{};

    // // permissive: overload 選択などで問題あり。
    // struct c1:stdm::true_type{};
#endif

#if defined(MWGCONF_STD_DECLTYPE)
    template<typename F_,bool B> struct s1{typedef void type;};
    template<typename F_> struct s1<F_,true>{
      typedef decltype(expr<F_>().operator()($".for:K:1:%AR%+1:expr<AK>():,")) type;
    };
    typedef typename s1<F,c1::value>::type OpR;
#else
    typedef R OpR;
#endif
    mwg_concept_condition(c1::value&&is_variant_argument<OpR,R>::value);
  };
#%)
#%expand mwg::functor::arities

  template<typename F,typename S>
  struct can_be_called_as
    :can_be_called_as_impl2<F,S,can_be_called_as_impl1<F,S>::value>{};

  template<typename F,typename S,bool CANBE1>
  struct can_be_called_as_impl2:stdm::false_type{};
  template<typename F,typename S>
  struct can_be_called_as_impl2<F,S,true>:stdm::true_type{
    typedef S signature_type;
  };
  template<typename F,typename S>
  struct can_be_called_as_impl2<F,S,false>
    :can_be_called_as<F,typename mwg::funcsig::decrease_arity<S>::type>{};
  template<typename F,typename R>
  struct can_be_called_as_impl2<F,R(),false>
    :stdm::false_type{};

#pragma%x begin_check
void check_can_be_called_as(){
  using namespace mwg::functor_detail;
  mwg_check( (is_variant_signature<functor_traits<int(int)>,functor_traits<int(int)> >::value));
  mwg_check( (is_variant_signature<functor_traits<void(int)>,functor_traits<void(int)> >::value));
  mwg_check( (is_variant_signature<functor_traits<int(int)>,functor_traits<void(int)> >::value));
  mwg_check(!(is_variant_signature<functor_traits<void(int)>,functor_traits<int(int)> >::value));

  mwg_check( (is_variant_argument<int,int>::value));
  mwg_check( (is_variant_argument<int,void>::value));
  mwg_check(!(is_variant_argument<void,int>::value));
}
#pragma%x end_check

//------------------------------------------------------------------------------
