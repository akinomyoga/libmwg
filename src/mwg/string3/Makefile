# -*- makefile-gmake -*-

all:
.PHONY: all clean

BASE=../../..
include $(BASE)/mmake/vars.mk

CXX=$(MWGCXX)
CXXFLAGS= -I $(INS_INCDIR) -I $(INS_INCCFG)

objfiles+=i001-bench1.o
i001-bench1.o: i001-bench1.cpp
	$(CXX) $(CXXFLAGS) -c -o$@ $<

objfiles+=i001-bench2.o
i001-bench2.o: i001-bench2.cpp
	$(CXX) $(CXXFLAGS) -c -o$@ $<

files+=i001$(EXEEXT)
i001$(EXEEXT): i001.cpp
	$(CXX) $(CXXFLAGS) -o$@ $<

all: $(files) $(objfiles)
clean:
	-rm -f *.o
