//%define mwg_concept_create_wrapper (
#ifdef MWGCONF_STD_CONCEPT
auto concept %TConcept%<typename T_>{
//%%m C (
  /* condition %value%; */
//%%)
//%%m f (
  %decl%;
//%%)
//%%m t (
  typename %name%;
//%%)
//%%expand ARG_FOR_MEMBER
}
#else
//%%if ARG_TMPL_PARAMS!="" (
//%%%expand (
template<typename T_,$"ARG_TMPL_PARAMS_FOR_CONCEPT">
//%%%).i
//%%else
template<typename T_>
//%%)
struct %TConcept%{
  typedef typename mwg::stdm::remove_cv<typename mwg::stdm::remove_reference<T_>::type >::type target_type;
  typedef target_type& adapter;
  typedef const target_type& const_adapter;

//%%[i=0,x="",cond=""]
//%%m C (
//%%%[cond=cond+"&&(%value%)"]
//%%)
//%%m f (
//%%%[rex_methodname="%name%".slice(0,8)=="operator"?"\\<operator"+"%name%".slice(8).Replace("(.)","\\$1"):"\\<%name%\\>"]
//%%%[decl="%decl%".replace(rex_methodname+"\\s*\\(","(X::*)(")]
//%%%[cond=cond+"&&c"+(++i)+"::value"]
//%%%x (
  mwg_concept_has_member(c$"i",target_type,X,%name%,$"decl");
//%%%).i
//%%)
//%%m t (
//%%%[cond=cond+"&&c"+(++i)+"::value"]
//%%%x (
  mwg_concept_has_type_member(c$"i",target_type,%name%);
//%%%).i
//%%)
//%%x ARG_FOR_MEMBER
//%%x (
  mwg_concept_condition($".eval#slice(cond,2)");
//%%).i
};
#endif

//%%if ARG_TMPL_PARAMS!="" (
template<$"ARG_TMPL_PARAMS">
//%%).i
struct %TConcept%Wrapper{
  struct holder_base{
    virtual ~holder_base(){}
    virtual holder_base* clone() const=0;
    
//%%define f (
    virtual %decl%=0;
//%%)
//%%[t=""]
//%%[C=""]
//%%expand ARG_FOR_MEMBER
  };
  
  template<typename T_>
  struct holder:holder_base{
    T_ inst;
    holder(const T_& inst):inst(inst){}
    virtual holder_base* clone() const{
      return new holder(this->inst);
    }

//%%define f (
    virtual %decl%{
      return inst.%name%(%args%);
    }
//%%)
//%%expand ARG_FOR_MEMBER
  };
  
  holder_base* h;
  
  %TConcept%Wrapper(const %TConcept%Wrapper& bin):h(bin.h->clone()){}
  %TConcept%Wrapper& operator=(const %TConcept%Wrapper& bin){
    delete h;
    h=bin.h->clone();
    return *this;
  }

//%%expand (
  template<typename T_>
  %TConcept%Wrapper(T_ mwg_forward_rvalue bin,typename mwg::stdm::enable_if<(%TConcept%<typename mwg::stdm::remove_reference<T_>::type ${ARG_TMPL_ARGS:+,}${ARG_TMPL_ARGS}>::value),void*>::type d=nullptr)
    :h(new holder<typename mwg::stdm::remove_reference<T_>::type>(mwg::stdm::forward<T_>(bin))){}
//%%).i
  ~%TConcept%Wrapper(){delete h;h=nullptr;}
//%%define f (
  %decl%{
    return h->%name%(%args%);
  }
//%%)
//%%expand ARG_FOR_MEMBER
//%%x wrapper_content
};
//%)
