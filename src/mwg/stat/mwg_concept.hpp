#pragma%m mwg_concept_create_wrapper
#ifdef MWGCONF_STD_CONCEPT
auto concept %TConcept%<typename T_>{
#pragma%%m C
  /* condition %value%; */
#pragma%%end
#pragma%%m f
  %decl%;
#pragma%%end
#pragma%%m t
  typename %name%;
#pragma%%end
#pragma%%x ARG_FOR_MEMBER
}
#else
#pragma%%if ARG_TMPL_PARAMS!=""
#pragma%%%x
template<typename T_,$"ARG_TMPL_PARAMS_FOR_CONCEPT">
#pragma%%%end.i
#pragma%%else
template<typename T_>
#pragma%%end
struct %TConcept%{
  typedef typename mwg::stdm::remove_cv<typename mwg::stdm::remove_reference<T_>::type >::type target_type;
  typedef target_type& adapter;
  typedef const target_type& const_adapter;

#pragma%%[i=0,x="",cond=""]
#pragma%%m C
#pragma%%%[cond=cond+"&&(%value%)"]
#pragma%%end
#pragma%%m f
#pragma%%%[methodname="%name%"]
#pragma%%%[rex_methodname=methodname.slice(0,8)=="operator"?"\\<operator"+methodname.slice(8).Replace("([][+*()^|.?])","\\\\$1"):"\\<"+methodname+"\\>"]
#pragma%%%[decl="%decl%".replace(rex_methodname+"\\s*\\(","(X::*)(")]
#pragma%%%[cond=cond+"&&c"+(++i)+"::value"]
#pragma%%%x
  mwg_concept_has_member(c$"i",target_type,X,%name%,$"decl");
#pragma%%%end.i
#pragma%%end
#pragma%%m t
#pragma%%%[cond=cond+"&&c"+(++i)+"::value"]
#pragma%%%x
  mwg_concept_has_type_member(c$"i",target_type,%name%);
#pragma%%%end.i
#pragma%%end
#pragma%%x ARG_FOR_MEMBER
#pragma%%x
  mwg_concept_condition($".eval#slice(cond,2)");
#pragma%%end.i
};
#endif

#pragma%%if ARG_TMPL_PARAMS!="" (
template<$"ARG_TMPL_PARAMS">
#pragma%%).i
struct %TConcept%Wrapper{
  struct holder_base{
    virtual ~holder_base(){}
    virtual holder_base* clone() const=0;

#pragma%%m f
    virtual %decl%=0;
#pragma%%end
#pragma%%[t=""]
#pragma%%[C=""]
#pragma%%x ARG_FOR_MEMBER
  };

  template<typename T_>
  struct holder:holder_base{
    T_ inst;
    holder(const T_& inst):inst(inst){}
    virtual holder_base* clone() const{
      return new holder(this->inst);
    }

#pragma%%m f
    virtual %decl%{
      return inst.%name%(%args%);
    }
#pragma%%end
#pragma%%x ARG_FOR_MEMBER
  };

  holder_base* h;

  %TConcept%Wrapper(const %TConcept%Wrapper& bin):h(bin.h->clone()){}
  %TConcept%Wrapper& operator=(const %TConcept%Wrapper& bin){
    delete h;
    h=bin.h->clone();
    return *this;
  }

#pragma%%x
  template<typename T_>
  %TConcept%Wrapper(T_ mwg_forward_rvalue bin,typename mwg::stdm::enable_if<(%TConcept%<typename mwg::stdm::remove_reference<T_>::type ${ARG_TMPL_ARGS:+,}${ARG_TMPL_ARGS}>::value),void*>::type d=nullptr)
    :h(new holder<typename mwg::stdm::remove_reference<T_>::type>(mwg::stdm::forward<T_>(bin))){}
#pragma%%end.i
  ~%TConcept%Wrapper(){delete h;h=nullptr;}
#pragma%%m f
  %decl%{
    return h->%name%(%args%);
  }
#pragma%%end
#pragma%%x ARG_FOR_MEMBER
#pragma%%x wrapper_content
};
#pragma%end
