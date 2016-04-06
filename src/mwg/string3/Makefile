# -*- makefile-gmake -*-

all:
.PHONY: all clean

BASE=../../..
include $(BASE)/mmake/vars.mk

CXX=$(MWGCXX)
CXXFLAGS+= -I $(INS_INCDIR) -I $(INS_INCCFG) -O3

objfiles+=str.bench1.o
str.bench1.o: str.bench1.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
str.bench1.exe: $(objfiles)
	$(CXX) $(CXXFLAGS) -o $@ $^
all: str.bench1.exe

clean:
	-rm -f *.o
