# -*- Makefile -*-

.PHONY: all test
all: Binning.h

Binning.h: Binning.hpp mwg_concept.hpp
	mwg_pp.awk $< > $@

test:
	cd test && make
