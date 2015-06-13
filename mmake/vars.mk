# -*- mode:makefile-gmake -*-

# external tools
MWGCXX:=$(BASE)/mmake/mcxx/cxx
MWGCXXAR:=$(BASE)/mmake/mcxx/cxxar
MWGPP:=$(BASE)/mmake/mwg_pp.awk

# compiler settings
CXXPREFIX:=$(shell $(MWGCXX) +prefix)
CXXENC:=utf-8
CXXCFG:=default
CXXEXT:=.cpp

# project settings
SRCENC=utf-8
CPPDIR:=$(BASE)/out/src.$(CXXENC)
CFGDIR:=$(BASE)/out/$(CXXPREFIX)+$(CXXCFG)
