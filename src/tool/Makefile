# -*- makefile-gmake -*-

all: mwb/mwb$(EXEEXT)
.PHONY: all

CXX:=cxx -O3
#CXX:=cxx -O3 -march=native

Makefile: Makefile.pp
	mwg_pp.awk $< > $@

#------------------------------------------------------------------------------
# MWB

mwb/mwb_dump.o: mwb/mwb_dump.cpp
	$(CXX) -c -o$@ $<
mwb/mwb.o: mwb/mwb.cpp
	$(CXX) -c -o$@ $<
mwb/mwb$(EXEEXT): mwb/mwb.o mwb/mwb_dump.o
	$(CXX) -o$@ $^ -lmwg

#------------------------------------------------------------------------------
