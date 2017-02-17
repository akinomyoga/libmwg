// -*- mode: c++; coding: utf-8 -*-
// #include <mwg/std/utility>

#pragma%[ArN=10]
#if defined(MWGCONF_STD_VARIADIC_TEMPLATES)
#  define mwg_impl_variadic_template_decl     template<typename... Args>
#  define mwg_impl_variadic_template_params   typename... Args
#  define mwg_impl_variadic_template_params_r ,typename... Args
#  define mwg_impl_variadic_template_params_l typename... Args,
#  define mwg_impl_variadic_params            Args... args
#  define mwg_impl_variadic_params_r          ,Args... args
#  define mwg_impl_variadic_params_l          Args... args,
#  define mwg_impl_variadic_forward_params    Args mwg_forward_rvalue... args
#  define mwg_impl_variadic_forward_params_r  ,Args mwg_forward_rvalue... args
#  define mwg_impl_variadic_forward_params_l  Args mwg_forward_rvalue... args,
#  define mwg_impl_variadic_args              args...
#  define mwg_impl_variadic_args_r            ,args...
#  define mwg_impl_variadic_args_l            args...,
#  define mwg_impl_variadic_forward_args      mwg::stdm::forward<Args>(args)...
#  define mwg_impl_variadic_forward_args_r    ,mwg::stdm::forward<Args>(args)...
#  define mwg_impl_variadic_forward_args_l    mwg::stdm::forward<Args>(args)...,
   mwg_impl_variadic_function
#  undef mwg_impl_variadic_template_decl
#  undef mwg_impl_variadic_template_params
#  undef mwg_impl_variadic_template_params_r
#  undef mwg_impl_variadic_template_params_l
#  undef mwg_impl_variadic_params
#  undef mwg_impl_variadic_params_r
#  undef mwg_impl_variadic_params_l
#  undef mwg_impl_variadic_forward_params
#  undef mwg_impl_variadic_forward_params_r
#  undef mwg_impl_variadic_forward_params_l
#  undef mwg_impl_variadic_args
#  undef mwg_impl_variadic_args_r
#  undef mwg_impl_variadic_args_l
#  undef mwg_impl_variadic_forward_args
#  undef mwg_impl_variadic_forward_args_r
#  undef mwg_impl_variadic_forward_args_l
#else
#pragma%x
#pragma%%if _AR_==0
#  define mwg_impl_variadic_template_decl
#pragma%%else
#  define mwg_impl_variadic_template_decl     template<$".for/K/0/_AR_/typename AK/,">
#pragma%%end
#  define mwg_impl_variadic_template_params   $".for/K/0/_AR_/typename AK/,"
#  define mwg_impl_variadic_template_params_r $".for/K/0/_AR_/,typename AK/"
#  define mwg_impl_variadic_template_params_l $".for/K/0/_AR_/typename AK,/"
#  define mwg_impl_variadic_params            $".for/K/0/_AR_/AK argK/,"
#  define mwg_impl_variadic_params_r          $".for/K/0/_AR_/,AK argK/"
#  define mwg_impl_variadic_params_l          $".for/K/0/_AR_/AK argK,/"
#  define mwg_impl_variadic_forward_params    $".for/K/0/_AR_/AK mwg_forward_rvalue argK/,"
#  define mwg_impl_variadic_forward_params_r  $".for/K/0/_AR_/,AK mwg_forward_rvalue argK/"
#  define mwg_impl_variadic_forward_params_l  $".for/K/0/_AR_/AK mwg_forward_rvalue argK,/"
#  define mwg_impl_variadic_args              $".for/K/0/_AR_/argK/,"
#  define mwg_impl_variadic_args_r            $".for/K/0/_AR_/,argK/"
#  define mwg_impl_variadic_args_l            $".for/K/0/_AR_/argK,/"
#  define mwg_impl_variadic_forward_args      $".for/K/0/_AR_/mwg::stdm::forward<AK>(argK)/,"
#  define mwg_impl_variadic_forward_args_r    $".for/K/0/_AR_/,mwg::stdm::forward<AK>(argK)/"
#  define mwg_impl_variadic_forward_args_l    $".for/K/0/_AR_/mwg::stdm::forward<AK>(argK),/"
   mwg_impl_variadic_function
#  undef mwg_impl_variadic_template_decl
#  undef mwg_impl_variadic_template_params
#  undef mwg_impl_variadic_template_params_r
#  undef mwg_impl_variadic_template_params_l
#  undef mwg_impl_variadic_params
#  undef mwg_impl_variadic_params_r
#  undef mwg_impl_variadic_params_l
#  undef mwg_impl_variadic_forward_params
#  undef mwg_impl_variadic_forward_params_r
#  undef mwg_impl_variadic_forward_params_l
#  undef mwg_impl_variadic_args
#  undef mwg_impl_variadic_args_r
#  undef mwg_impl_variadic_args_l
#  undef mwg_impl_variadic_forward_args
#  undef mwg_impl_variadic_forward_args_r
#  undef mwg_impl_variadic_forward_args_l
#pragma%end.f/_AR_/0/ArN/.i
#endif
