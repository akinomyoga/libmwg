# -*- makefile-gmake -*-

CXX=CXXKEY=i cxx -fast
CXXFLAGS=-ipo -O3

all:
.PHONY: all clean

files+=i001.strbase_constructor_variadic.inl
i001.strbase_constructor_variadic.inl: i001.strbase_constructor_variadic.hpp
	mwg_pp.awk $< > $@

objfiles+=i001-bench1.o
i001-bench1.o: i001-bench1.cpp
	$(CXX) -c -o$@ $(CXXFLAGS) $<

objfiles+=i001-bench2.o
i001-bench2.o: i001-bench2.cpp
	$(CXX) -c -o$@ $(CXXFLAGS) $<

i001$(EXEEXT): i001.cpp i001.h
	$(CXX) -o$@ $<

all: $(files) $(objfiles)
clean:
	-rm -f *.o
