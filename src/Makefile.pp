# -*- mode:makefile-gmake -*-

all:
.PHONY: all

# disable implicit rules (they makes the dependency resolution slow)
MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

#%[BASE=".."]
#%include ../mmake/src.mk
#%x prologue

Makefile: Makefile.pp
	$(BASE)/mmake/mwg_pp.awk $< > $@ || mv $@ $@.error

#%x AddCxxHeader.r|%file%|mwg/defs.h|
#%x AddCxxHeader.r|%file%|mwg/concept.h|
#%x AddCxxHeader.r|%file%|mwg/except.h|
#%x AddCxxHeader.r|%file%|mwg/mpl.h|
#%x AddCxxHeader.r|%file%|mwg/range.h|

#%x AddCxxHeader.r|%file%|mwg/std/chrono|
#%x AddCxxHeader.r|%file%|mwg/std/cmath|
#%x AddCxxHeader.r|%file%|mwg/std/cstdint|
#%x AddCxxHeader.r|%file%|mwg/std/initializer_list|
#%x AddCxxHeader.r|%file%|mwg/std/typeinfo|
#%x AddCxxHeader.r|%file%|mwg/std/iterator|
#%x AddCxxHeader.r|%file%|mwg/std/limits|
#%x AddCxxHeader.r|%file%|mwg/std/memory|
#%x AddCxxHeader.r|%file%|mwg/std/memory.unique_ptr.inl|
#%x AddCxxHeader.r|%file%|mwg/std/ratio|

#%x AddCxxHeader.r|%file%|mwg/std/tuple|
#%[ppdeps="mwg/std/tuple.nonvariadic_tuple.hpp"]
#%x AddCxxHeader.r|%file%|mwg/std/tuple.nonvariadic_tuple.inl|
#%[ppdeps="mwg/std/tuple.nonvariadic_tuple.hpp"]
#%x AddCxxHeader.r|%file%|mwg/std/tuple.variadic_tuple.inl|

#%x AddCxxHeader.r|%file%|mwg/std/type_traits|
#%x AddCxxHeader.r|%file%|mwg/std/type_traits.is_constructible.h|
#%x AddCxxHeader.r|%file%|mwg/std/type_traits.is_convertible.inl|
#%x AddCxxHeader.r|%file%|mwg/std/utility|
#%x AddCxxHeader.r|%file%|mwg/impl/warning_push.inl|
#%x AddCxxHeader.r|%file%|mwg/impl/warning_pop.inl|
#%x AddCxxHeader.r|%file%|mwg/impl/DeclareVariadicFunction.inl|

#%x AddCxxHeader.r|%file%|mwg/xprintf.h|
#%x AddCxxSource.r|%file%|mwg/xprintf.cpp|

#%x AddCxxHeader.r|%file%|mwg/char.h|
#%x AddCxxHeader.r|%file%|mwg/cast.h|
#%x AddCxxHeader.r|%file%|mwg/exp/utils.h|
#%x AddCxxHeader.r|%file%|mwg/exp/iprint.h|

#%x AddCxxHeader.r|%file%|mwg/stat/errored.h|
#%x AddCxxHeader.r|%file%|mwg/stat/bindex.h|
#%x AddCxxHeader.r|%file%|mwg/stat/accumulator.h|
#%x AddCxxHeader.r|%file%|mwg/stat/histogram2.h|
#%[ppdeps="mwg/stat/mwg_concept.hpp"]
#%x AddCxxHeader.r|%file%|mwg/stat/binning2.h|
#%[ppdeps="mwg/stat/binning2.ProductBinning.hpp"]
#%x AddCxxHeader.r|%file%|mwg/stat/binning2.ProductBinning.inl|
#%[ppdeps="mwg/stat/binning2.ProductBinning.hpp"]
#%x AddCxxHeader.r|%file%|mwg/stat/binning2.ProductBinning_nonvariadic.inl|

#%x AddCxxHeader.r|%file%|mwg/bio/defs.h|
#%x AddCxxHeader.r|%file%|mwg/bio/tape.h|
#%x AddCxxHeader.r|%file%|mwg/bio/tape.util.inl|
#%x AddCxxHeader.r|%file%|mwg/bio/tape.stdio.inl|
#%x AddCxxHeader.r|%file%|mwg/bio/tape.stream.inl|
#%x AddCxxHeader.r|%file%|mwg/bio/ttx2.h|
#%x AddCxxHeader.r|%file%|mwg/bio/filter.inl|
#%x AddCxxSource.r|%file%|mwg/bio/tape.util.cpp|
#%x AddCxxSource.r|%file%|mwg/bio/ttx2.cpp|

#%x AddCxxHeader.r|%file%|mwg/bio/mwb_header.h|
#%x AddCxxHeader.r|%file%|mwg/bio/mwb_format.h|
#%x AddCxxSource.r|%file%|mwg/bio/mwb_format.cpp|

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
	$(MWGCXX) +config -o "$@" --cache="$(CFGDIR)/cache" --log="$(CFGDIR)/config.log" $< -- $(CXXFLAGS) $(FLAGS) $(LDFLAGS)
$(CFGDIR)/include/mwg_config.stamp: $(CFGDIR)/include/mwg_config.1.h $(CFGDIR)/include/mwg_config.2.h
	mv $(CFGDIR)/include/mwg_config.h $@ || touch $@
	$(MWGPP) $< > $(CFGDIR)/include/mwg_config.h
	touch -r $@ $(CFGDIR)/include/mwg_config.h
	touch $@
$(CFGDIR)/include/mwg_config_common.h: $(CFGDIR)/include/mwg_config.1.h
ifeq ($(CXXENC),$(SRCENC))
$(CPPDIR)/mwg/config.h: mwg_config.mconf | $(CFGDIR)/include/mwg_config_common.h $(CPPDIR)/mwg
	cp $(CFGDIR)/include/mwg_config_common.h $@
else
$(CPPDIR)/mwg/config.h: mwg_config.mconf | $(CFGDIR)/include/mwg_config_common.h $(CPPDIR)/mwg
	iconv -c -f "$(SRCENC)" -t "$(CXXENC)" $(CFGDIR)/include/mwg_config_common.h > $@
endif
source_files+=$(CFGDIR)/include/mwg_config.stamp $(CPPDIR)/mwg/config.h
install_files+=$(INS_INCCFG)/mwg_config.h $(INS_INCCFG)/mwg/config.h
$(INS_INCCFG)/mwg_config.h: $(CFGDIR)/include/mwg_config.stamp
	$(BASE)/mmake/make_command.sh install-header $(CFGDIR)/include/mwg_config.h $@
$(INS_INCCFG)/mwg/config.h: $(CPPDIR)/mwg/config.h
	$(BASE)/mmake/make_command.sh install-header $< $@

$(CFGDIR)/libmwg.a: $(object_files)
	@echo 'AR $@'
	@$(MWGCXXAR) $@ $^
install_files+=$(INS_LIBDIR)/libmwg.a
$(INS_LIBDIR)/libmwg.a: $(CFGDIR)/libmwg.a
	$(BASE)/mmake/make_command.sh install $< $@

all: $(source_files) $(CFGDIR)/libmwg.a
check: all $(check_files)
install: $(install_files)

#%x epilogue
