# -*- mode:makefile-gmake -*-

export CXXKEY
export CXXCFG

# external tools
MWGCXX:=$(BASE)/mmake/mcxx/cxx
MWGCXXAR:=$(BASE)/mmake/mcxx/cxxar
MWGPP:=$(BASE)/mmake/mwg_pp.awk
MMAKECMD:=BASE=$(BASE); source $(BASE)/mmake/make_command.sh

# use bash for makefile shell
ifneq ($(wildcard /bin/bash),)
  SHELL := /bin/bash
else ifneq ($(wildcard /usr/bin/bash),)
  SHELL := /usr/bin/bash
else ifneq ($(wildcard /usr/local/bin/bash),)
  SHELL := /usr/local/bin/bash
else
  SHELL := $(shell which bash 2>/dev/null)
  ifeq ($(SHELL),)
    $(error [1mBash not found. Bash is required to make libmwg[m)
  endif
endif

# compiler settings
_cxx_params:=$(shell source $(MWGCXX) +get --eval '$$CXXPREFIX $$CXX_ENCODING')
ifeq ($(_cxx_params),)
  $(error [1mFailed to determine CXXPREFIX[m: [34mCheck MWGCXX='[4m$(MWGCXX)[24m' / CXXKEY='[4m$(CXXKEY)[24m' / cxx configurations[m)
endif
export CXXPREFIX:=$(word 1,$(_cxx_params))
export CXXENC:=$(word 2,$(_cxx_params))
export CXXEXT:=.cpp

ifeq ($(CXXCFG),)
  CXXCFG:=default
endif
-include $(BASE)/config.mk
$(BASE)/config.mk:
	cp $(BASE)/config.mk.new $(BASE)/config.mk

# project settings
SRCENC:=utf-8
CFGDIR:=$(BASE)/out/$(CXXPREFIX)+$(CXXCFG)
ifneq ($(MMAKE_SOURCE_FILTER),)
  CPPDIR:=$(CFGDIR)/src
else
  CPPDIR:=$(BASE)/out/src.$(CXXENC)
endif
ifneq ($(INSDIR),)
  INS_INCDIR:=$(INSDIR)/include
  INS_INCCFG:=$(INSDIR)/include/$(CXXPREFIX)+$(CXXCFG)
  INS_LIBDIR:=$(INSDIR)/lib/$(CXXPREFIX)+$(CXXCFG)
else
  INS_INCDIR:=$(BASE)/out/include.$(CXXENC)
  INS_INCCFG:=$(BASE)/out/include.$(CXXENC)/$(CXXPREFIX)+$(CXXCFG)
  INS_LIBDIR:=$(BASE)/out/lib/$(CXXPREFIX)+$(CXXCFG)
endif
