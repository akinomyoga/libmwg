# -*- mode:makefile-gmake -*-

all:
.PHONY: all clean dist

CFGSTAMP=out/configure.stamp

configure $(CFGSTAMP):
	test -d mmake/mcxx || (cd mmake && tar xJf mmake/mcxx.tar.xz)
	-rm -rf mmake/mcxx/local
	mmake/mcxx/cxx +prefix auto -q
	stamp=$(CFGSTAMP); mkdir -p "$${stamp%/*}" && touch "$$stamp"
.PHONY: configure

.NOTPARALLEL:
MAKEFLAGS += --no-print-directory -O

all: | $(CFGSTAMP)
	+make -C src

clean:
	+make -C src clean

distclean:
	-rm -rf out

dist-excludes = \
  --exclude=./dist \
	--exclude=./out \
	--exclude=./.git \
	--exclude=*~ \
	--exclude=?*.2[0-9][0-9][0-9][0-1][0-9][0-3][0-9].* \
	--exclude=?*.2[0-9][0-9][0-9][0-1][0-9][0-3][0-9]

dist:
	test -d dist || mkdir dist; tar cJf dist/libmwg-2.0.$$(date +'%Y%m%d').tar.xz ./ $(dist-excludes) --transform='s|^\./|libmwg/|'
