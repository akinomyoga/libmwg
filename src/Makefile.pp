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

#%x AddCxxHeader.r|%file%|mwg/std/def.h|
#%x AddCxxHeader.r|%file%|mwg/std/cstdint|
#%x AddCxxHeader.r|%file%|mwg/concept.h|
#%x AddCxxHeader.r|%file%|mwg/std/type_traits.is_constructible.h|
#%x AddCxxHeader.r|%file%|mwg/std/type_traits.is_convertible.inl|
#%x AddCxxHeader.r|%file%|mwg/std/type_traits|
#%x AddCxxHeader.r|%file%|mwg/bits/cxx.inttype.h|
#%x AddCxxHeader.r|%file%|mwg/defs.h|
#%x AddCxxHeader.r|%file%|mwg/except.h|

#%x AddCxxHeader.r|%file%|mwg/bits/mpl.integer.h|
#%x AddCxxHeader.r|%file%|mwg/bits/mpl.util.h|
#%x AddCxxHeader.r|%file%|mwg/range.h|

#%x AddCxxHeader.r|%file%|mwg/impl/warning_push.inl|
#%x AddCxxHeader.r|%file%|mwg/impl/warning_pop.inl|
#%x AddCxxHeader.r|%file%|mwg/impl/DeclareVariadicFunction.inl|
#%x AddCxxHeader.r|%file%|mwg/std/utility|
#%x AddCxxHeader.r|%file%|mwg/std/chrono|
#%x AddCxxHeader.r|%file%|mwg/std/cfenv|
#%x AddCxxHeader.r|%file%|mwg/std/cmath|
#%x AddCxxSource.r|%file%|mwg/std/cmath.impl.cpp|
#%x AddCxxHeader.r|%file%|mwg/std/cinttypes|
#%x AddCxxHeader.r|%file%|mwg/std/cctype|
#%x AddCxxHeader.r|%file%|mwg/std/cwctype|
#%x AddCxxHeader.r|%file%|mwg/std/initializer_list|
#%x AddCxxHeader.r|%file%|mwg/std/typeinfo|
#%x AddCxxHeader.r|%file%|mwg/std/iterator|
#%x AddCxxHeader.r|%file%|mwg/std/limits|
#%x AddCxxHeader.r|%file%|mwg/std/memory|
#%x AddCxxHeader.r|%file%|mwg/std/memory.unique_ptr.inl|
#%x AddCxxHeader.r|%file%|mwg/std/ratio|
#%x AddCxxHeader.r|%file%|mwg/std/tuple|
#%x AddCxxHeader.r|%file%|mwg/std/tuple.nonvariadic_tuple.inl|
#%x AddCxxHeader.r|%file%|mwg/std/tuple.variadic_tuple.inl|
#%x AddCxxHeader.r|%file%|mwg/std/array|
#%x AddCxxHeader.r|%file%|mwg/std/algorithm|
#%x AddCxxHeader.r|%file%|mwg/std/execution|
#%x AddCxxHeader.r|%file%|mwg/bits/integer.nlz.h|

#%x AddCxxHeader.r|%file%|mwg/std/functional|
#%x AddCxxHeader.r|%file%|mwg/std/random|

#%x AddCxxHeader.r|%file%|mwg/bits/autoload.inl|

#%x AddCxxHeader.r|%file%|mwg/funcsig.h|
#%x AddCxxHeader.r|%file%|mwg/functor.h|
#%x AddCxxHeader.r|%file%|mwg/functor.proto.h|
#%x AddCxxHeader.r|%file%|mwg/bits/type_traits.member_pointer.hpp|
#%x AddCxxHeader.r|%file%|mwg/exp/fun/fun.h|
#%x AddCxxHeader.r|%file%|mwg/exp/fun/funsig.h|

#%x AddCxxHeader.r|%file%|mwg/xprintf.h|
#%x AddCxxSource.r|%file%|mwg/xprintf.cpp|
#%x AddCxxHeader.r|%file%|mwg/str.h|
#%x AddCxxHeader.r|%file%|mwg/bits/str.strbuf.h|
#%x AddCxxHeader.r|%file%|mwg/bits/str.support.xprintf.h|

#%x AddCxxHeader.r|%file%|mwg/char.h|
#%x AddCxxHeader.r|%file%|mwg/cast.h|
#%x AddCxxHeader.r|%file%|mwg/exp/utils.h|
#%x AddCxxHeader.r|%file%|mwg/exp/iprint.h|

#%x AddCxxHeader.r|%file%|mwg/stat/errored.h|
#%x AddCxxHeader.r|%file%|mwg/stat/bindex.h|
#%x AddCxxHeader.r|%file%|mwg/stat/accumulator.h|
#%x AddCxxHeader.r|%file%|mwg/stat/histogram2.h|
#%x AddCxxHeader.r|%file%|mwg/stat/binning2.h|
#%x AddCxxHeader.r|%file%|mwg/stat/binning2.ProductBinning.inl|
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
#%x AddCxxHeader.r|%file%|mwg/bio/mwt_i3.h|

#%x AddCxxHeader.r|%file%|mwg/bio/mwb_header.h|
#%x AddCxxHeader.r|%file%|mwg/bio/mwb_format.h|
#%x AddCxxSource.r|%file%|mwg/bio/mwb_format.cpp|

#%x AddCxxHeader.r|%file%|mwg/ext/zlib.h|
#%x AddCxxSource.r|%file%|mwg/ext/zlib.cpp|
#%x AddCxxHeader.r|%file%|mwg/ext/xz.h|
#%x AddCxxSource.r|%file%|mwg/ext/xz.cpp|

#%x AddConfigHeader.r|%file%|mwg_config.2.h|

$(CPPDIR)/mwg:
	mkdir -p $(CPPDIR)/mwg
$(CFGDIR)/include/mwg_config.1.h: mwg_config.mconf | $(CFGDIR)/include
	$(MWGCXX) +config -o "$@" --cache="$(CFGDIR)/cache" --log="$(CFGDIR)/config.log" $< -- $(CXXFLAGS) $(FLAGS) $(LDFLAGS)
	@echo 'TRANSFORM $@'; $(MMAKECMD) transform-source $@
	@echo 'TRANSFORM $(CFGDIR)/include/mwg_config_common.h'; $(MMAKECMD) transform-source $(CFGDIR)/include/mwg_config_common.h
$(CFGDIR)/include/mwg_config.stamp: $(CFGDIR)/include/mwg_config.1.h $(CFGDIR)/include/mwg_config.2.h | $(CFGDIR)/include
	mv $(CFGDIR)/include/mwg_config.h $@ || touch $@
	$(MWGPP) $< > $(CFGDIR)/include/mwg_config.h
	touch -r $@ $(CFGDIR)/include/mwg_config.h
	touch $@
$(CFGDIR)/include/mwg_config_common.h: $(CFGDIR)/include/mwg_config.1.h | $(CFGDIR)/include
$(CPPDIR)/mwg/config.h: mwg_config.mconf | $(CFGDIR)/include/mwg_config_common.h $(CPPDIR)/mwg
	cp $(CFGDIR)/include/mwg_config_common.h $@
source_files+=$(CFGDIR)/include/mwg_config.stamp $(CPPDIR)/mwg/config.h
install_files+=$(INS_INCCFG)/mwg_config.h $(INS_INCDIR)/mwg/config.h
$(INS_INCCFG)/mwg_config.h: $(CFGDIR)/include/mwg_config.stamp
	@echo 'INS header $(CFGDIR)/include/mwg_config.h'; $(MMAKECMD) install-header $(CFGDIR)/include/mwg_config.h $@
$(INS_INCDIR)/mwg/config.h: $(CPPDIR)/mwg/config.h
	@echo 'INS header $<'; $(MMAKECMD) install-header $< $@

$(CFGDIR)/libmwg.a: $(object_files)
	@echo 'AR $@'; $(MWGCXXAR) $@ $^
library_files+=$(CFGDIR)/libmwg.a
install_files+=$(INS_LIBDIR)/libmwg.a
$(INS_LIBDIR)/libmwg.a: $(CFGDIR)/libmwg.a
	$(BASE)/mmake/make_command.sh install $< $@

.PHONY: all check install show-install-message
all: $(source_files) $(library_files)
check: all $(check_files)
install: show-install-message $(install_files)
show-install-message:
	@target='$(INSDIR)'; printf 'install target = %q\n' "$${target:-$(BASE)/out}"

#%x DefineRuleDoc.r/%LANG\y/cpp/

ifneq ($(libmwg_cxxkey_scanlist),)
  define _libmwg_scan_target :=
    for key in $(libmwg_cxxkey_scanlist); do \
      if [[ $$key == *+* ]]; then \
        cxxkey="$${key%%+*}"; \
        cxxcfg="$${key#*+}"; \
        printf '\e[48;5;189;1m%-79s\e[m\n' "$$ CXXKEY=$$cxxkey CXXCFG=$$cxxcfg make TARGET"; \
        CXXKEY="$$cxxkey" CXXCFG="$$cxxcfg" make TARGET; \
      else \
        printf '\e[48;5;189;1m%-79s\e[m\n' "$$ CXXKEY=$$key make TARGET"; \
        CXXKEY="$$key" make TARGET; \
      fi \
    done
  endef
else
  define _libmwg_scan_target :=
    for key in $$($(BASE)/mmake/mcxx/cxx +prefix list|awk '{print $$2}'); do \
      printf '\e[48;5;189;1m%-79s\e[m\n' "$$ CXXKEY=$$key make TARGET"; \
      CXXKEY="$$key" make TARGET; \
    done
  endef
endif
.PHONY: scan-check scan-install scan-all scan-clean scan-clean-all
scan-check:
	+@$(subst TARGET,check,$(_libmwg_scan_target))
scan-install:
	+@$(subst TARGET,install,$(_libmwg_scan_target))
scan-all:
	+@$(subst TARGET,all,$(_libmwg_scan_target))
scan-clean:
	+@$(subst TARGET,clean,$(_libmwg_scan_target))
scan-clean-all:
	+@$(subst TARGET,clean-all,$(_libmwg_scan_target))

#%x epilogue
