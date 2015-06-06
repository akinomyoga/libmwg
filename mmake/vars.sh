# bash source file
# -*- mode:sh; mode:sh-bash -*-

if [[ ${BASH_SOURCE:-$0} =~ ^(.*)/+mmake/+[^/]+ ]]; then
  BASE="${BASH_REMATCH[1]:-/}"
elif [[ $0 =~ mmake/+[^/]+ ]]; then
  BASE=.
else
  echo "unknown script directory! \$0=$0" >&2
  exit 1
fi

# project settings
MWGCXX=cxx
CXXPP="$BASE/mmake/mwg_pp.awk"
SRCENC=utf-8

# compiler settings
CXXPREFIX="$("$MWGCXX" +prefix)"
CXXENC=utf-8
CXXEXT=.cpp
CXXCFG=default

CPPDIR="$BASE/out/src.$CXXENC"
CFGDIR="$BASE/out/$CXXPREFIX+$CXXCFG"
