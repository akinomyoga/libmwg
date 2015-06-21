#%begin definitions
# -*- mode:makefile-gmake -*-

# 使い方の例
#
# | # -*- mode:makefile-gmake -*-
# |
# | all:
# | .PHONY: all
# |
# | #%[BASE=".."]
# | #%include ../mmake/src.mk
# |
# | Makefile: Makefile.pp
# | 	$(BASE)/mmake/mwg_pp.awk $< > $@ || mv $@ $@.error
# |
# | #%x AddCxxSource.r|%file%|file1.cpp|
# | #%x AddCxxSource.r|%file%|subdir/file2.cpp|
# |
# | all: $(object_files)

#------------------------------------------------------------------------------
# Prologue and Epilogue

#%m prologue
SHELL:=/bin/bash

# initialize
FLAGS:=
CFLAGS:=
FFLAGS:=
CXXFLAGS:=
LDFLAGS:=

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

clean-src:
	-rm -rf $(CPPDIR)/*

clean-cxx-source:
	-rm -rf $(CFGDIR)/config $(CFGDIR)/include
clean-cxx-cache:
	-rm -rf $(CFGDIR)/cache
clean-cxx-obj:
	-rm -rf $(CFGDIR)/obj/*.o $(CFGDIR)/obj/*.dep
clean-cxx-lib:
	-rm -rf $(CFGDIR)/lib $(CFGDIR)/bin
clean-cxx-check:
	-rm -rf $(CFGDIR)/check
clean-cxx: clean-cxx-source clean-cxx-cache clean-cxx-obj clean-cxx-lib clean-cxx-check
.PHONY: clean-cxx clean-cxx-source clean-cxx-cache clean-cxx-obj clean-cxx-lib clean-cxx-check

clean: clean-cxx-obj
clean-all: clean-src clean-cxx
.PHONY: clean clean-src clean-all

directories+=$(INS_INCDIR)
directories+=$(INS_INCCFG)
directories+=$(INS_LIBDIR)
#%end

#%m epilogue
source_files: $(source_files)
config_files: $(config_files)
.PHONY: source_files config_files

$(directories):
	mkdir -p $@
#%end

#------------------------------------------------------------------------------
# macros

#%m _preprocess_file
source_files+=$(CPPDIR)/${file}
$(CPPDIR)/${file}: ${file} ${ppdeps}
	$(BASE)/mmake/make_command.sh copy-pp ${file}
$(CPPDIR)/${filex}.mconf: $(CPPDIR)/${file}
$(CPPDIR)/${filex}.lwiki: $(CPPDIR)/${file}
config_files+=$(CFGDIR)/config/${name}.h
$(CFGDIR)/config/${name}.h: $(CPPDIR)/${filex}.mconf | $(CFGDIR)/config
	$(BASE)/mmake/make_command.sh config ${file} $(FLAGS) $(CXXFLAGS) $(LDFLAGS)
check_files+=$(CFGDIR)/check/${name}.stamp
$(CPPDIR)/check/${name}$(CXXEXT): $(CPPDIR)/${file}
-include $(CFGDIR)/check/${name}.dep
$(CFGDIR)/check/${name}.stamp: $(CPPDIR)/check/${name}$(CXXEXT)
	$(BASE)/mmake/make_command.sh check ${file} $(FLAGS) $(CXXFLAGS) $(LDFLAGS)
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
	$(BASE)/mmake/make_command.sh compile ${file} $(FLAGS) $(CXXFLAGS)
#%%end.i
#%%[ppdeps=""]

#%end

#%m AddCxxHeader
# AddCxxHeader %file%
#%%[file="%file%"]
#%%[filex=file.replace("\\.","_")]
#%%[name=file.replace("\\.(cpp|c|C|cxx)$","").replace("/","+").replace("\\.","_")]
#%%x _check_duplicates.i
#%%x _preprocess_file.i
#%%x
install_files+=$(INS_INCDIR)/${file}
$(INS_INCDIR)/${file}: $(CPPDIR)/${file} | $(INS_INCDIR)
	$(BASE)/mmake/make_command.sh install-header ${file}
#%%end.i
#%%[ppdeps=""]

#%end


#%m AddConfigHeader
# AddConfigHeader %file%
source_files+=$(CFGDIR)/include/%file%
$(CFGDIR)/include/%file%: $(config_files) | $(CFGDIR)/include
	cat $^ > $@

#%end

#%end
