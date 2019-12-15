# -*- makefile-gmake -*-

all: mwb/mwb$(EXEEXT)
.PHONY: all

CXX := g++ -O3
#CXX:= g++ -O3 -march=native

BASE := ../..
MWGPP := $(BASE)/mmake/mwg_pp.awk
include $(BASE)/mmake/vars.mk
CXXFLAGS := -I $(CPPDIR) -I $(CFGDIR)/include
LDFLAGS := -L $(CFGDIR)

Makefile: Makefile.pp | $(MWGPP)
	$(MWGPP) $< > $@

mwb/mwb.o: mwb/mwb.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
mwb/mwb_dump.o: mwb/mwb_dump.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

mwb/mwb$(EXEEXT): mwb/mwb.o mwb/mwb_dump.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ -lmwg
