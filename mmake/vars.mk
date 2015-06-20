# -*- mode:makefile-gmake -*-

# external tools
MWGCXX:=$(BASE)/mmake/mcxx/cxx
MWGCXXAR:=$(BASE)/mmake/mcxx/cxxar
MWGPP:=$(BASE)/mmake/mwg_pp.awk

# compiler settings
CXXPREFIX:=$(shell $(MWGCXX) +prefix)
CXXENC:=utf-8
ifeq ($(CXXCFG),)
  CXXCFG:=default
endif
CXXEXT:=.cpp

# project settings
SRCENC=utf-8
CPPDIR:=$(BASE)/out/src.$(CXXENC)
CFGDIR:=$(BASE)/out/$(CXXPREFIX)+$(CXXCFG)
ifneq ($(INSDIR),)
 INS_INCDIR:=$(INSDIR)/include
 INS_INCCFG:=$(INSDIR)/include/$(CXXPREFIX)+$(CXXCFG)
 INS_LIBDIR:=$(INSDIR)/lib/$(CXXPREFIX)+$(CXXCFG)
else
 INS_INCDIR:=$(BASE)/out/include.$(CXXENC)
 INS_INCCFG:=$(BASE)/out/include.$(CXXENC)/$(CXXPREFIX)+$(CXXCFG)
 INS_LIBDIR:=$(BASE)/out/lib/$(CXXPREFIX)+$(CXXCFG)
endif
