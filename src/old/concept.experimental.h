// -*- mode: c++; coding: utf-8 -*-

#define mwg_concept_is_valid_expression_impl20130318A(NAME,T,X,EXPR) \
  struct NAME{                                                       \
    mwg_concept_sfinae_param{                                        \
      template<int I> struct receiver{};                             \
      mwg_concept_sfinae_param_true(X,(receiver<sizeof(EXPR)>*));    \
      mwg_concept_sfinae_param_false(X,(...));                       \
    };                                                               \
    mwg_concept_sfinae_param_check(T,(0));                           \
  };                                                              /**/
  /*
   * これを用いると gcc3 は Segmentation fault で落ちる
   */
#define mwg_concept_is_valid_expression_impl20130318B(NAME,T,X,EXPR) \
  struct NAME{                                                       \
    mwg_concept_sfinae_param{                                        \
      mwg_concept_sfinae_param_true(X,(int(*)[sizeof(EXPR)]));       \
      mwg_concept_sfinae_param_false(X,(...));                       \
    };                                                               \
    mwg_concept_sfinae_param_check(T,(0));                           \
  };                                                              /**/
  /*
   * これを用いると gcc3 は Segmentation fault で落ちる
   */
