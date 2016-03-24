# -*- mode:sh; mode:sh-bash -*-
# bash source file

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
if [[ ! ( $CXXPREFIX && $CXXENC && $CXXEXT ) ]]; then
  # if sourced from src/Makefile, the following variables are already defined.
  CXXPREFIX=($(source "$MWGCXX" +get --eval '$CXXPREFIX $CXX_ENCODING')) || exit 1
  CXXENC="${CXXPREFIX[1]}"
  CXXEXT=.cpp
fi

# read config.mk
: ${CXXCFG:=default}
[[ -s $BASE/consig.src ]] && source $BASE/config.src

# project settings
SRCENC=utf-8
CFGDIR="$BASE/out/$CXXPREFIX+$CXXCFG"
if [[ $MMAKE_SOURCE_FILTER ]]; then
  CPPDIR="$CFGDIR/src"
else
  CPPDIR="$BASE/out/src.$CXXENC"
fi

if [[ $INSDIR ]]; then
  INS_INCDIR="$INSDIR/include"
  INS_INCCFG="$INSDIR/include/$CXXPREFIX+$CXXCFG"
  INS_LIBDIR="$INSDIR/lib/$CXXPREFIX+$CXXCFG"
else
  INS_INCDIR="$BASE/out/include.$CXXENC"
  INS_INCCFG="$BASE/out/include.$CXXENC/$CXXPREFIX+$CXXCFG"
  INS_LIBDIR="$BASE/out/lib/$CXXPREFIX+$CXXCFG"
fi
