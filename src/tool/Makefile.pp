# -*- makefile-gmake -*-

all: mwb/mwb$(EXEEXT)
.PHONY: all

CXX:=cxx -O3
#CXX:=cxx -O3 -march=native

Makefile: Makefile.pp
	mwg_pp.awk $< > $@

mwb/mwb_format.o: mwb/mwb_format.cpp
	$(CXX) -c -o$@ $<
mwb/mwb_dump.o: mwb/mwb_dump.cpp
	$(CXX) -c -o$@ $<

mwb/mwb$(EXEEXT): mwb/mwb.cpp mwb/mwb_format.o mwb/mwb_dump.o
	$(CXX) -o$@ $^
