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
include ${BASE}/mmake/vars.mk
Makefile: ${BASE}/mmake/src.mk
#%end.i

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
clean-all: clean-src clean-obj clean-lib clean-cache
.PHONY: clean clean-src clean-obj clean-lib clean-cache clean-all
#%end
#%m epilogue
source_files: $(source_files)
config_files: $(config_files)
.PHONY: source_files config_files
#%end

#%m _preprocess_file
source_files+=$(CPPDIR)/${file}
$(CPPDIR)/${file}: ${file}
	$(BASE)/mmake/make_command.sh copy-pp ${file}
$(CPPDIR)/${filex}.mconf: $(CPPDIR)/${file}
$(CPPDIR)/${filex}.lwiki: $(CPPDIR)/${file}
config_files+=$(CFGDIR)/config/${name}.h
$(CFGDIR)/config/${name}.h: $(CPPDIR)/${filex}.mconf | $(CFGDIR)/config
	$(BASE)/mmake/make_command.sh config ${file}
check_files+=$(CFGDIR)/check/${name}.stamp
$(CPPDIR)/check/${name}$(CXXEXT): $(CPPDIR)/${file}
$(CFGDIR)/check/${name}.stamp: $(CPPDIR)/check/${name}$(CXXEXT)
	$(BASE)/mmake/make_command.sh check ${file}
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
$(CFGDIR)/obj/${name}.o: $(CPPDIR)/${file} | source_files
	$(BASE)/mmake/make_command.sh compile ${file}
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
