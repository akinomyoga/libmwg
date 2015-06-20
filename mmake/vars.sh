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

# external tools
MWGCXX="$BASE/mmake/mcxx/cxx"
MWGPP="$BASE/mmake/mwg_pp.awk"

# compiler settings
CXXPREFIX="$("$MWGCXX" +prefix)"
CXXENC=utf-8
: ${CXXCFG:=default}
CXXEXT=.cpp

# project settings
SRCENC=utf-8
CPPDIR="$BASE/out/src.$CXXENC"
CFGDIR="$BASE/out/$CXXPREFIX+$CXXCFG"
if [[ $INSDIR ]]; then
  INS_INCDIR="$INSDIR/include"
  INS_INCCFG="$INSDIR/include/$CXXPREFIX+$CXXCFG"
  INS_LIBDIR="$INSDIR/lib/$CXXPREFIX+$CXXCFG"
else
  INS_INCDIR="$BASE/out/include.$CXXENC"
  INS_INCCFG="$BASE/out/include.$CXXENC/$CXXPREFIX+$CXXCFG"
  INS_LIBDIR="$BASE/out/lib/$CXXPREFIX+$CXXCFG"
fi
