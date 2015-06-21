# -*- mode:sh; mode:sh-bash -*-
# config.mk - compiler settings

CXXFLAGS += -Wall
CXXFLAGS += -Wextra

CXXCFG_FOUND:=
ifeq ($(CXXCFG),default)
  CXXCFG_FOUND:=1
  CXXFLAGS += -g
endif
ifeq ($(CXXCFG),release)
  CXXCFG_FOUND:=1
  CXXFLAGS += -O3
endif
ifeq ($(CXXCFG),cxx11)
  CXXCFG_FOUND:=1
  CXXFLAGS += -g
  CXXFLAGS += -std=c++0x
endif
ifeq ($(CXXCFG),cxx11-release)
  CXXCFG_FOUND:=1
  CXXFLAGS += -O3
  CXXFLAGS += -std=c++0x
endif

ifeq ($(CXXCFG_FOUND),)
include config-$(CXXCFG).mk
endif
