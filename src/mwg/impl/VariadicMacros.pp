#pragma%# -*- mode: c++; coding: utf-8 -*-
#pragma%begin
//-----------------------------------------------------------------------------

#pragma%[ArN=10]

#pragma%m variadic_expand::with_arity
#pragma%%m _ 1.r/\y__[a]rity__\y/__arity__/
#pragma%%m _ _.r/\ysizeof\.\.\.\([^()]+\)/__arity__/
#pragma%# typename... args
#pragma%%m _ _.R@,[[:space:]]*\y(typename|class)[[:space:]]*\.\.\.[[:space:]]*([_[:alpha:]][_[:alnum:]]*)\y@$".for/%K/0/__arity__/,$1 $2%K/"@
#pragma%%m _ _.R@[[:space:]]*\y(typename|class)[[:space:]]*\.\.\.[[:space:]]*([_[:alpha:]][_[:alnum:]]*)\y,@$".for/%K/0/__arity__/$1 $2%K,/"@
#pragma%%m _ _.R@[[:space:]]*\y(typename|class)[[:space:]]*\.\.\.[[:space:]]*([_[:alpha:]][_[:alnum:]]*)\y@$".for/%K/0/__arity__/$1 $2%K/,"@
#pragma%# hoge<Args>fuga... args
#pragma%%m _ _.R|,[[:space:]]*([:_[:alnum:][:space:]&*]*)<([_[:alnum:]]+,?)>([:_[:alnum:][:space:]&*]*)\.\.\.[[:space:]]*([_[:alnum:]]+)\y|$".for/%K/0/__arity__/,$1<$2%K>$3 $4%K/"|
#pragma%%m _ _.R|[[:space:]]*([:_[:alnum:][:space:]&*]*)<([_[:alnum:]]+,?)>([:_[:alnum:][:space:]&*]*)\.\.\.[[:space:]]*([_[:alnum:]]+)\y,|$".for/%K/0/__arity__/$1<$2%K>$3 $4%K,/"|
#pragma%%m _ _.R|[[:space:]]*([:_[:alnum:][:space:]&*]*)<([_[:alnum:]]+,?)>([:_[:alnum:][:space:]&*]*)\.\.\.[[:space:]]*([_[:alnum:]]+)\y|$".for/%K/0/__arity__/$1<$2%K>$3 $4%K/,"|
#pragma%# Args qualifiers... args
#pragma%%m _ _.R|,[[:space:]]*([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.[[:space:]]*([_[:alnum:]]+)\y|$".for/%K/0/__arity__/,$1%K$2 $3%K/"|
#pragma%%m _ _.R|[[:space:]]*([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.[[:space:]]*([_[:alnum:]]+)\y,|$".for/%K/0/__arity__/$1%K$2 $3%K,/"|
#pragma%%m _ _.R|[[:space:]]*([_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.[[:space:]]*([_[:alnum:]]+)\y|$".for/%K/0/__arity__/$1%K$2 $3%K/,"|
#pragma%# Args qualifiers...
#pragma%%m _ _.R|,[[:space:]]*([&*]*[_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.|$".for/%K/0/__arity__/,$1%K$2/"|
#pragma%%m _ _.R|[[:space:]]*([&*]*[_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.,|$".for/%K/0/__arity__/$1%K$2,/"|
#pragma%%m _ _.R|[[:space:]]*([&*]*[_[:alnum:]]+)([_[:alnum:][:space:]&*]*)\.\.\.|$".for/%K/0/__arity__/$1%K$2/,"|
#pragma%# hoge<Args>fuga(args)... (mwg::stdm::forward<Args>(args)...)
#pragma%%m _ _.R|,[[:space:]]*([&*]*[:_[:alnum:]]+)<([_[:alnum:]]+,?)>([:_[:alnum:]]*)\(([&*]*[_[:alnum:]]+)\)\.\.\.|$".for/%K/0/__arity__/,$1<$2%K>$3($4%K)/"|
#pragma%%m _ _.R|[[:space:]]*([&*]*[:_[:alnum:]]+)<([_[:alnum:]]+,?)>([:_[:alnum:]]*)\(([&*]*[_[:alnum:]]+)\)\.\.\.,|$".for/%K/0/__arity__/$1<$2%K>$3($4%K),/"|
#pragma%%m _ _.R|[[:space:]]*([&*]*[:_[:alnum:]]+)<([_[:alnum:]]+,?)>([:_[:alnum:]]*)\(([&*]*[_[:alnum:]]+)\)\.\.\.|$".for/%K/0/__arity__/$1<$2%K>$3($4%K)/,"|
#pragma%# hoge<Args>fuga()...
#pragma%%m _ _.R|,[[:space:]]*([&*]*[:_[:alnum:]]+)<([_[:alnum:]]+,?)>([:_[:alnum:]]*)\(\)\.\.\.|$".for/%K/0/__arity__/,$1<$2%K>$3()/"|
#pragma%%m _ _.R|[[:space:]]*([&*]*[:_[:alnum:]]+)<([_[:alnum:]]+,?)>([:_[:alnum:]]*)\(\)\.\.\.,|$".for/%K/0/__arity__/$1<$2%K>$3(),/"|
#pragma%%m _ _.R|[[:space:]]*([&*]*[:_[:alnum:]]+)<([_[:alnum:]]+,?)>([:_[:alnum:]]*)\(\)\.\.\.|$".for/%K/0/__arity__/$1<$2%K>$3()/,"|
#pragma%%m _
#pragma%%x _.i.r/(\ytemplate[[:space:]]*)?<##>|##//
#pragma%%end
#pragma%%if (__arity__==0)
#pragma%%%m _ _.R/\ytemplate<>([[:space:][:cntrl:]]*(struct|union|class))/template<--->$1/
#pragma%%%m _ _.r|\ytemplate<>([[:space:]]*typename\y)?|inline|
#pragma%%%m _ _.r|\ytemplate<--->|template<>|
#pragma%%end
#pragma%%x _
#pragma%end

#pragma%m variadic_expand_0toArN
#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
#pragma%%x 1.r/##//
#else
#pragma%%x variadic_expand::with_arity.f/__arity__/0/ArN+1/
#endif
#pragma%end

#pragma%m variadic_expand_0toArNm1
#ifdef MWGCONF_STD_VARIADIC_TEMPLATES
#pragma%%x 1.r/##//
#else
#pragma%%x variadic_expand::with_arity.f/__arity__/0/ArN/
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
