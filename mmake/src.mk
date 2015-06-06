#%begin definitions
# -*- mode:makefile-gmake -*-

# 使い方の例
#
# |	# -*- mode:makefile-gmake -*-
# |
# |	all:
# |	.PHONY: all
# |
# |	#%[BASE=".."]
# |	#%include ../mmake/src.mk
# |
# |	Makefile: Makefile.pp
# |		$(BASE)/mmake/mwg_pp.awk $< > $@ || mv $@ $@.error
# |
# |	#%x AddCxxSource.r|%file%|file1.cpp|
# |	#%x AddCxxSource.r|%file%|subdir/file2.cpp|
# |
# |	all: $(object_files)

#%m prologue
SHELL:=/bin/bash

#%if BASE==""
#%%error mmake/src.mk: BASE is empty!
#%else
BASE:=${BASE}
Makefile: ${BASE}/mmake/src.mk
#%end.i

SRCENC=utf-8

MWGCXX:=cxx
CXXPREFIX:=$(shell $(MWGCXX) +prefix)
CXXENC:=utf-8
CXXCFG:=default
CXXEXT:=.cpp

CPPDIR:=$(BASE)/out/src.$(CXXENC)
CFGDIR:=$(BASE)/out/$(CXXPREFIX)+$(CXXCFG)

MWGPP:=$(BASE)/mmake/mwg_pp.awk

directories+=$(CFGDIR)
directories+=$(CFGDIR)/include
directories+=$(CFGDIR)/config
directories+=$(CPPDIR)
$(directories):
	mkdir -p $@

clean: clean-obj
clean-src:
	-rm -rf $(CPPDIR)/* $(CFGDIR)/config/* $(CFGDIR)/include/*
clean-obj:
	-rm -rf $(CFGDIR)/obj/*.o
clean-lib:
	-rm -rf $(CFGDIR)/lib $(CFGDIR)/bin
clean-cache:
	-rm -rf $(CFGDIR)/cache
#%end

#%m _preprocess_file
source_files+=$(CPPDIR)/${file}
$(CPPDIR)/${file}: ${file}
	$(BASE)/mmake/make_file.sh copy-pp ${file}
$(CPPDIR)/${filex}.mconf: $(CPPDIR)/${file}
$(CPPDIR)/${filex}.lwiki: $(CPPDIR)/${file}
config_files+=$(CFGDIR)/config/${name}.h
$(CFGDIR)/config/${name}.h: $(CPPDIR)/${filex}.mconf | $(CFGDIR)/config
	$(MWGCXX) +config -o "$@" --cache="$(CFGDIR)/cache" $<
check_files+=$(CFGDIR)/check/${name}.stamp
$(CPPDIR)/check/${name}$(CXXEXT): $(CPPDIR)/${file}
$(CFGDIR)/check/${name}.stamp: $(CPPDIR)/check/${name}$(CXXEXT)
	$(BASE)/mmake/make_file.sh check ${file}
#%end

#%m _check_duplicates
#%%if _registered_files[file.tolower()]
#%%%error src.mk: the spefified file '${file}' has already been registered.
#%%else
#%%%[_registered_files[file.tolower()]=1]
#%%end
#%end

#%m AddCxxSource
# AddCxxSource %file%
#%%[file="%file%"]
#%%[filex=file.replace("\\.","_")]
#%%[name=file.replace("\\.(cpp|c|C|cxx)$","").replace("/","+").replace("\\.","_")]
#%%x _check_duplicates.i
#%%x _preprocess_file.i
#%%x
object_files+=$(CFGDIR)/obj/${name}.o
-include $(CFGDIR)/obj/${name}.dep
$(CFGDIR)/obj/${name}.o: $(CPPDIR)/${file}
	$(BASE)/mmake/make_file.sh compile ${file}
#%%end.i

#%end

#%m AddCxxHeader
# AddCxxHeader %file%
#%%[file="%file%"]
#%%[filex=file.replace("\\.","_")]
#%%[name=file.replace("\\.(cpp|c|C|cxx)$","").replace("/","+").replace("\\.","_")]
#%%x _check_duplicates.i
#%%x _preprocess_file.i

#%end

#%m AddConfigHeader
# AddConfigHeader %file%
source_files+=$(CFGDIR)/include/%file%
$(CFGDIR)/include/%file%: $(config_files) | $(CFGDIR)/include
	cat $^ > $@

#%end

#%end
#%x prologue
