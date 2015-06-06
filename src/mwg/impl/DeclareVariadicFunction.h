// // -*- mode:C++;coding:utf-8 -*-
// #include <mwg/std/utility>

//%[ArN=10]
#if defined(MWGCONF_STD_VARIADIC_TEMPLATES)
#  define mwg_impl_variadic__template_decl     template<typename... Args>
#  define mwg_impl_variadic__template_params   typename... Args
#  define mwg_impl_variadic__template_params_r ,typename... Args
#  define mwg_impl_variadic__template_params_l typename... Args,
#  define mwg_impl_variadic__params            Args... args
#  define mwg_impl_variadic__params_r          ,Args... args
#  define mwg_impl_variadic__params_l          Args... args,
#  define mwg_impl_variadic__forward_params    Args mwg_forward_rvalue... args
#  define mwg_impl_variadic__forward_params_r  ,Args mwg_forward_rvalue... args
#  define mwg_impl_variadic__forward_params_l  Args mwg_forward_rvalue... args,
#  define mwg_impl_variadic__args              args...
#  define mwg_impl_variadic__args_r            ,args...
#  define mwg_impl_variadic__args_l            args...,
#  define mwg_impl_variadic__forward_args      mwg::stdm::forward<Args>(args)...
#  define mwg_impl_variadic__forward_args_r    ,mwg::stdm::forward<Args>(args)...
#  define mwg_impl_variadic__forward_args_l    mwg::stdm::forward<Args>(args)...,
   mwg_impl_variadic__function
#  undef mwg_impl_variadic__template_decl
#  undef mwg_impl_variadic__template_params
#  undef mwg_impl_variadic__template_params_r
#  undef mwg_impl_variadic__template_params_l
#  undef mwg_impl_variadic__params
#  undef mwg_impl_variadic__params_r
#  undef mwg_impl_variadic__params_l
#  undef mwg_impl_variadic__forward_params
#  undef mwg_impl_variadic__forward_params_r
#  undef mwg_impl_variadic__forward_params_l
#  undef mwg_impl_variadic__args
#  undef mwg_impl_variadic__args_r
#  undef mwg_impl_variadic__args_l
#  undef mwg_impl_variadic__forward_args
#  undef mwg_impl_variadic__forward_args_r
#  undef mwg_impl_variadic__forward_args_l
#else
//%x (
//%%if _AR_==0 (
#  define mwg_impl_variadic__template_decl     
//%%else
#  define mwg_impl_variadic__template_decl     template<$".for/K/0/_AR_/typename AK/,">
//%%)
#  define mwg_impl_variadic__template_params   $".for/K/0/_AR_/typename AK/,"
#  define mwg_impl_variadic__template_params_r $".for/K/0/_AR_/,typename AK/"
#  define mwg_impl_variadic__template_params_l $".for/K/0/_AR_/typename AK,/"
#  define mwg_impl_variadic__params            $".for/K/0/_AR_/AK argK/,"
#  define mwg_impl_variadic__params_r          $".for/K/0/_AR_/,AK argK/"
#  define mwg_impl_variadic__params_l          $".for/K/0/_AR_/AK argK,/"
#  define mwg_impl_variadic__forward_params    $".for/K/0/_AR_/AK mwg_forward_rvalue argK/,"
#  define mwg_impl_variadic__forward_params_r  $".for/K/0/_AR_/,AK mwg_forward_rvalue argK/"
#  define mwg_impl_variadic__forward_params_l  $".for/K/0/_AR_/AK mwg_forward_rvalue argK,/"
#  define mwg_impl_variadic__args              $".for/K/0/_AR_/argK/,"
#  define mwg_impl_variadic__args_r            $".for/K/0/_AR_/,argK/"
#  define mwg_impl_variadic__args_l            $".for/K/0/_AR_/argK,/"
#  define mwg_impl_variadic__forward_args      $".for/K/0/_AR_/mwg::stdm::forward<AK>(argK)/,"
#  define mwg_impl_variadic__forward_args_r    $".for/K/0/_AR_/,mwg::stdm::forward<AK>(argK)/"
#  define mwg_impl_variadic__forward_args_l    $".for/K/0/_AR_/mwg::stdm::forward<AK>(argK),/"
   mwg_impl_variadic__function
#  undef mwg_impl_variadic__template_decl
#  undef mwg_impl_variadic__template_params
#  undef mwg_impl_variadic__template_params_r
#  undef mwg_impl_variadic__template_params_l
#  undef mwg_impl_variadic__params
#  undef mwg_impl_variadic__params_r
#  undef mwg_impl_variadic__params_l
#  undef mwg_impl_variadic__forward_params
#  undef mwg_impl_variadic__forward_params_r
#  undef mwg_impl_variadic__forward_params_l
#  undef mwg_impl_variadic__args
#  undef mwg_impl_variadic__args_r
#  undef mwg_impl_variadic__args_l
#  undef mwg_impl_variadic__forward_args
#  undef mwg_impl_variadic__forward_args_r
#  undef mwg_impl_variadic__forward_args_l
//%).f/_AR_/0/ArN/.i
#endif
