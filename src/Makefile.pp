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
#%x AddCxxHeader.r|%file%|mwg/impl/header_begin.inl|
#%x AddCxxHeader.r|%file%|mwg/impl/header_end.inl|
#%x AddCxxHeader.r|%file%|mwg/impl/DeclareVariadicFunction.h|

#%x AddConfigHeader.r|%file%|mwg_config.2.h|

$(CPPDIR)/mwg:
	mkdir -p $(CPPDIR)/mwg
$(CFGDIR)/include/mwg_config.1.h: mwg_config.mconf
	$(MWGCXX) +config -o "$@" --cache="$(CFGDIR)/cache" $<
$(CFGDIR)/include/mwg_config.stamp: $(CFGDIR)/include/mwg_config.1.h $(CFGDIR)/include/mwg_config.2.h
	mv $(CFGDIR)/include/mwg_config.h $@
	$(MWGPP) $< > $(CFGDIR)/include/mwg_config.h
	touch -r $@ $(CFGDIR)/include/mwg_config.h
	touch $@
$(CFGDIR)/include/mwg_config_common.h: $(CFGDIR)/include/mwg_config.1.h
$(CPPDIR)/mwg/config.h: mwg_config.mconf | $(CFGDIR)/include/mwg_config_common.h $(CPPDIR)/mwg
	cp $(CFGDIR)/include/mwg_config_common.h $@
source_files+=$(CFGDIR)/include/mwg_config.stamp $(CPPDIR)/mwg/config.h

all: $(source_files)
check: $(check_files)
