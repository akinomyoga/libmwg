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

all: $(objfiles)
clean:
	-rm -f *.o
