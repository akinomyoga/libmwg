#pragma%# -*- mode:C++;coding:utf-8 -*-
#pragma%begin
//-----------------------------------------------------------------------------

#pragma%[ArN=10]

#pragma%m variadic_expand::with_arity
#pragma%%m _ 1
#pragma%# // typename... args
#pragma%%m _ _.R|,[[:space:]]*typename[[:space:]]*\.\.\.[[:space:]]*([_[:alpha:]][_[:alnum:]]*)\y|$".for/%K/0/__arity__/,typename $1%K/"|
#pragma%%m _ _.R|[[:space:]]*\ytypename[[:space:]]*\.\.\.[[:space:]]*([_[:alpha:]][_[:alnum:]]*)\y|$".for/%K/0/__arity__/typename $1%K/,"|
#pragma%# // Args qualifiers... args
#pragma%%m _ _.R|,[[:space:]]*([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.[[:space:]]*([_[:alnum:]]+)\y|$".for/%K/0/__arity__/,$1%K$2 $3%K/"|
#pragma%%m _ _.R|[[:space:]]*\y([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.[[:space:]]*([_[:alnum:]]+)\y|$".for/%K/0/__arity__/$1%K$2 $3%K/,"|
#pragma%# // Args qualifiers...
#pragma%%m _ _.R|,[[:space:]]*([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.|$".for/%K/0/__arity__/,$1%K$2/"|
#pragma%%m _ _.R|[[:space:]]*\y([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.|$".for/%K/0/__arity__/$1%K$2/,"|
#pragma%# // mwg::stdm::forward<Args>(args)...
#pragma%%m _ _.R|,[[:space:]]*mwg::stdm::forward<([_[:alnum:]]+)>\(([_[:alnum:]]+)\)...|$".for/%K/0/__arity__/,mwg::stdm::forward<$1%K>($2%K)/"|
#pragma%%m _ _.R|[[:space:]]*\ymwg::stdm::forward<([_[:alnum:]]+)>\(([_[:alnum:]]+)\)...|$".for/%K/0/__arity__/mwg::stdm::forward<$1%K>($2%K)/,"|
#pragma%%x _.i.r/<##>|##//
#pragma%end

#pragma%m variadic_expand_0toArN
#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
#pragma%%x 1.r/##//
#else
#pragma%%m a
#pragma%%x variadic_expand::with_arity.f/__arity__/0/ArN+1/
#pragma%%end
#pragma%%m a a.R/\ytemplate<>([[:space:][:cntrl:]]*(struct|union|class))/template<--->$1/
#pragma%%m a a.r|\ytemplate<>([[:space:]]*typename\y)?|inline|
#pragma%%m a a.r|\ytemplate<--->|template<>|
#pragma%%x a
#endif
#pragma%end

#pragma%m variadic_expand_ArN
#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
#pragma%%x 1.r/##//
#else
#pragma%%x variadic_expand::with_arity.r/__arity__/ArN/
#endif
#pragma%end

#pragma%m variadic_expand_ArNm1
#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
#pragma%%x 1.r/##//
#else
#pragma%%x variadic_expand::with_arity.r/__arity__/ArN-1/
#endif
#pragma%end

//-----------------------------------------------------------------------------
#pragma%end

