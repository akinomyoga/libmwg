#!/bin/bash

if [[ $0 =~ ^(.*)/+mmake/+[^/]+ ]]; then
  BASE="${BASH_REMATCH[1]:-/}"
elif [[ $0 =~ mmake/+[^/]+ ]]; then
  BASE=.
else
  echo "unknown script directory! \$0=$0" >&2
  exit 1
fi

source "$BASE"/mmake/vars.sh

function mkd { [[ -d "$1" ]] || mkdir -p "$1"; }
function mkdf { mkd "${1%/*}"; }

generate_filenames_vars=(fsrc fsource fcheck fmconf flwiki fconfig fdep fobj name)
function generate_filenames {
  # usage
  #   local fsrc fsource fcheck fmconf flwiki fconfig
  #   local fdep fobj name
  #   generate_filenames <filename>

  # exports
  fsrc="$1"
  fsource="$CPPDIR/$fsrc"

  local C_SL='/' C_PC='+'
  name="$fsrc"
  [[ $name =~ ^(.*)\.(cpp|c|C|cxx)$ ]] && name="${BASH_REMATCH[1]}"
  name="${name//$C_SL/$C_PC}"
  name="${name//./_}"
  local fbase="${fsource##*/}"
  fbase="${fsource%/*}/${fbase//./_}"

  # exports
  fcheck="$CPPDIR/check/$name$CXXEXT"
  fmconf="$fbase.mconf"
  flwiki="$fbase.lwiki"
  fconfig="$CFGDIR/config/$name.h"

  fdep="$CFGDIR/obj/$name.dep"
  fobj="$CFGDIR/obj/$name.o"
}

function proc/copy-pp {
  local "${generate_filenames_vars[@]}"
  generate_filenames "$1"

  mkdf "$fcheck"
  mkdf "$fsource"
  {
    export PPLINENO=1 PPC_PRAGMA=1 PPC_CPP=1
    if [[ "$CXXENC" != "$SRCENC" ]]; then
      "$MWGPP" \
        | sed '/-\*-.\{1,\}-\*-/s/\bcoding:[[:space:]]*'"$SRCENC"'\b/coding: '"$CXXENC"'/' \
        | iconv -c -f "$SRCENC" -t "$CXXENC"
    else
      "$MWGPP"
    fi
  } <<EOF > "$fsource"
#%\$> $fcheck
#%\$>
#%m begin_check
  #%%\$>> $fcheck
  #%%# x
#%end
#%m end_check
  #%%# end.R#(^|\n)[[:space:]]*//[[:space:]]*#\$1#
  #%%\$>
#%end
#%begin
#%m define_expression_test
X '%name%' '%headers%' '%expression%'
#%end
#%end
#%include "$fsrc"
EOF

  touch "$fcheck"
  perl "$BASE/mmake/make_extract.pl" "$fsource"

  # 以下の操作は CXXKEY に依存するので別のフェーズで実行するべき
  # echo $fmconf
  # if [[ -s $fmconf ]]; then
  #   mkdf "$fconfig"
  #   cxx +config -o "$fconfig" --cache="$CFGDIR/cache" "$fmconf"
  # else
  #   rm -rf "$fconfig"
  # fi
}

## @fn proc/compile filename.cpp [options...]
function proc/compile {
  local "${generate_filenames_vars[@]}"
  generate_filenames "$1"
  shift

  mkdf "$fdep"
  mkdf "$fobj"
  "$MWGCXX" -MD -MF "$fdep" -MQ "$fobj" -I "$CFGDIR/include" -I "$CPPDIR" -c -o "$fobj" "$fsource" "$@"
}

function proc/check {
  local "${generate_filenames_vars[@]}"
  generate_filenames "$1"
  shift

  local chkexe="$CFGDIR/check/$name.exe"
  local chkstm="$CFGDIR/check/$name.stamp"
  local chkdep="$CFGDIR/check/$name.dep"
  mkdf "$chkstm"
  if [[ -s "$fcheck" ]]; then
    local CHECK_FLAGS_SPEC=$(gawk '{if(match($0,/^[[:space:]]*\/\/[[:space:]]*mmake_check_flags:[[:space:]]*(.+)$/,_m))print _m[1];}' "$fcheck")
    eval "local FLAGS=($CHECK_FLAGS_SPEC)"
    # echo "dbg: FLAGS=(${FLAGS[*]})"
    "$MWGCXX" -MD -MF "$chkdep" -MQ "$chkstm" -I "$CFGDIR/include" -I "$CPPDIR" -o "$chkexe" "$fcheck" "${FLAGS[@]}" "$@" && "$chkexe"
  fi && touch "$chkstm"
}

function proc/config {
  local "${generate_filenames_vars[@]}"
  generate_filenames "$1"
  shift
  "$MWGCXX" +config -o "$fconfig" --cache="$CFGDIR/cache" --log="$CFGDIR/config.log" "$fmconf" -- "$@"
}

function proc/install {
  if (($#==2)); then
    local src="$1"
    local dst="$2"
  else
    echo 'make_command.sh install: requires two arguments.' >&2
    return 1
  fi

  mkdf "$dst"
  cp -p "$src" "$dst"
}

function proc/install-header {
  if (($#==2)); then
    local src="$1"
    local dst="$2"
  else
    local "${generate_filenames_vars[@]}"
    generate_filenames "$1"
    local src="$CPPDIR/$fsrc"
    local dst="$INS_INCDIR/$fsrc"
  fi
  mkdf "$dst"
  sed '/^[[:space:]]*#[[:space:]]*line[[:space:]]/d' "$src" > "$dst"
}

type="$1"; shift
if ! declare -f "proc/$type" &>/dev/null; then
  echo 'make_file.sh! unknown make type' >&2
  exit 1
fi

"proc/$type" "$@"
