# -*- mode:makefile-gmake -*-

# external tools
MWGCXX:=$(BASE)/mmake/mcxx/cxx
MWGCXXAR:=$(BASE)/mmake/mcxx/cxxar
MWGPP:=$(BASE)/mmake/mwg_pp.awk
MMAKECMD:=BASE=$(BASE); source $(BASE)/mmake/make_command.sh

# compiler settings
CXXPREFIX:=$(shell $(MWGCXX) +prefix)
ifeq ($(CXXPREFIX),)
  $(error [1mFailed to determine CXXPREFIX[m: [34mCheck MWGCXX='[4m$(MWGCXX)[24m' / CXXKEY='[4m$(CXXKEY)[24m' / cxx configurations[m)
endif
CXXENC:=$(shell $(MWGCXX) +get input-charset)
CXXEXT:=.cpp

ifeq ($(CXXCFG),)
  CXXCFG:=default
endif
-include $(BASE)/config.mk

# project settings
SRCENC:=utf-8
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
