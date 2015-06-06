# -*- mode:makefile-gmake -*-

all:
.PHONY: all clean dist


all:
	make -C src

clean:
	make -C clean

dist-excludes = \
  --exclude=./dist \
	--exclude=./out \
	--exclude=./.git \
	--exclude=*~ \
	--exclude=?*.2[0-9][0-9][0-9][0-1][0-9][0-3][0-9].* \
	--exclude=?*.2[0-9][0-9][0-9][0-1][0-9][0-3][0-9]

dist:
	test -d dist || mkdir dist; tar cJf dist/libmwg-$(date +%Y%m%d).tar.xz ./ $(dist-excludes) --transform='s|^\./|libmwg/|'
