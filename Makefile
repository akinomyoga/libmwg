# -*- mode:makefile-gmake -*-

all:
.PHONY: all clean dist check

CFGSTAMP=out/configure.stamp

configure $(CFGSTAMP):
	test -s src/Makefile || (cd src; ../mmake/mwg_pp.awk < Makefile.pp > Makefile)
	mmake/mcxx/cxx +prefix auto -q
	stamp=$(CFGSTAMP); mkdir -p "$${stamp%/*}" && touch "$$stamp"
.PHONY: configure

.NOTPARALLEL:
MAKEFLAGS += --no-print-directory -O

all check: | $(CFGSTAMP)

clean all check:
	+make -C src $@

distclean:
	-rm -rf out
	-rm -rf mmake/mcxx/local

dist-excludes = \
  --exclude=./dist \
	--exclude=./out \
	--exclude=./.git \
	--exclude=*~ \
	--exclude=?*.2[0-9][0-9][0-9][0-1][0-9][0-3][0-9].* \
	--exclude=?*.2[0-9][0-9][0-9][0-1][0-9][0-3][0-9]

dist:
	test -d dist || mkdir dist; tar cJf dist/libmwg-2.0.$$(date +'%Y%m%d').tar.xz ./ $(dist-excludes) --transform='s|^\./|libmwg/|'
