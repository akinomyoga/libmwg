# -*- mode:makefile-gmake -*-

all:
.PHONY: all

#%[BASE=".."]
#%include ../mmake/src.mk

Makefile: Makefile.pp
	$(BASE)/mmake/mwg_pp.awk $< > $@ || mv $@ $@.error

#%x AddCxxHeader.r|%file%|mwg/defs.h|
#%x AddCxxHeader.r|%file%|mwg/concept.h|
#%x AddCxxHeader.r|%file%|mwg/except.h|
#%x AddCxxHeader.r|%file%|mwg/mpl.h|
#%x AddCxxHeader.r|%file%|mwg/std/chrono|
#%x AddCxxHeader.r|%file%|mwg/std/cmath|
#%x AddCxxHeader.r|%file%|mwg/std/cstdint|
#%x AddCxxHeader.r|%file%|mwg/std/initializer_list|
#%x AddCxxHeader.r|%file%|mwg/std/iterator|
#%x AddCxxHeader.r|%file%|mwg/std/limits|
#%x AddCxxHeader.r|%file%|mwg/std/memory|
#%x AddCxxHeader.r|%file%|mwg/std/memory.unique_ptr.inl|
#%x AddCxxHeader.r|%file%|mwg/std/ratio|
#%x AddCxxHeader.r|%file%|mwg/std/tuple|
#%x AddCxxHeader.r|%file%|mwg/std/tuple.nonvariadic_tuple.h|
#%x AddCxxHeader.r|%file%|mwg/std/type_traits|
#%x AddCxxHeader.r|%file%|mwg/std/type_traits.is_constructible.h|
#%x AddCxxHeader.r|%file%|mwg/std/type_traits.is_convertible.inl|
#%x AddCxxHeader.r|%file%|mwg/std/utility|
#%x AddCxxHeader.r|%file%|mwg/impl/warning_push.inl|
#%x AddCxxHeader.r|%file%|mwg/impl/warning_pop.inl|
#%x AddCxxHeader.r|%file%|mwg/impl/DeclareVariadicFunction.inl|

#%x AddCxxHeader.r|%file%|mwg/char.h|
#%x AddCxxHeader.r|%file%|mwg/cast.h|
#%x AddCxxHeader.r|%file%|mwg/exp/utils.h|
#%x AddCxxHeader.r|%file%|mwg/exp/iprint.h|

#%x AddCxxHeader.r|%file%|mwg/stat/errored.h|
#%x AddCxxHeader.r|%file%|mwg/stat/bindex.h|
#%x AddCxxHeader.r|%file%|mwg/stat/accumulator.h|
#%x AddCxxHeader.r|%file%|mwg/stat/histogram2.h|
#%x AddCxxHeader.r|%file%|mwg/stat/binning2.h|

#%x AddCxxHeader.r|%file%|mwg/bio/defs.h|
#%x AddCxxHeader.r|%file%|mwg/bio/tape.h|
#%x AddCxxHeader.r|%file%|mwg/bio/tape.util.inl|
#%x AddCxxHeader.r|%file%|mwg/bio/tape.stdio.inl|
#%x AddCxxHeader.r|%file%|mwg/bio/tape.stream.inl|
#%x AddCxxHeader.r|%file%|mwg/bio/ttx2.h|
#%x AddCxxHeader.r|%file%|mwg/bio/filter.inl|
#%x AddCxxSource.r|%file%|mwg/bio/tape.util.cpp|
#%x AddCxxSource.r|%file%|mwg/bio/ttx2.cpp|

#%x AddCxxHeader.r|%file%|mwg/funcsig.h|
#%x AddCxxHeader.r|%file%|mwg/functor.h|
#%x AddCxxHeader.r|%file%|mwg/functor.proto.h|

#%x AddCxxHeader.r|%file%|mwg/ext/zlib.h|
#%x AddCxxSource.r|%file%|mwg/ext/zlib.cpp|
#%x AddCxxHeader.r|%file%|mwg/ext/xz.h|
#%x AddCxxSource.r|%file%|mwg/ext/xz.cpp|

#%x AddConfigHeader.r|%file%|mwg_config.2.h|

$(CPPDIR)/mwg:
	mkdir -p $(CPPDIR)/mwg
$(CFGDIR)/include/mwg_config.1.h: mwg_config.mconf
	$(MWGCXX) +config -o "$@" --cache="$(CFGDIR)/cache" $<
$(CFGDIR)/include/mwg_config.stamp: $(CFGDIR)/include/mwg_config.1.h $(CFGDIR)/include/mwg_config.2.h
	mv $(CFGDIR)/include/mwg_config.h $@ || touch $@
	$(MWGPP) $< > $(CFGDIR)/include/mwg_config.h
	touch -r $@ $(CFGDIR)/include/mwg_config.h
	touch $@
$(CFGDIR)/include/mwg_config_common.h: $(CFGDIR)/include/mwg_config.1.h
$(CPPDIR)/mwg/config.h: mwg_config.mconf | $(CFGDIR)/include/mwg_config_common.h $(CPPDIR)/mwg
	cp $(CFGDIR)/include/mwg_config_common.h $@
source_files+=$(CFGDIR)/include/mwg_config.stamp $(CPPDIR)/mwg/config.h

$(CFGDIR)/libmwg.a: $(object_files)
	$(MWGCXXAR) $@ $^

all: $(source_files) $(CFGDIR)/libmwg.a
check: $(check_files)

#%x epilogue
