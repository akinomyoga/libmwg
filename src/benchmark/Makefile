# -*- makefile-gmake -*-

all:
.PHONY: all clean

BASE=../..
include $(BASE)/mmake/vars.mk

OBJDIR = .obj
$(OBJDIR):
	mkdir -p $@

CXX=$(MWGCXX)
#CXXFLAGS += -I $(INS_INCDIR) -I $(INS_INCCFG) -O3
ifneq ($(subst -vc-,-,$(CXXPREFIX)),$(CXXPREFIX))
  # msvc
  CXXFLAGS += -I $(CPPDIR) -I $(CFGDIR)/include -O3 /arch:AVX2
  LIBS :=
else
  CXXFLAGS += -I $(CPPDIR) -I $(CFGDIR)/include -O3 -march=native -Wall -Wextra
  LIBS := -lrt
endif

ifeq ($(HOSTNAME),laguerre01.ph.sophia.ac.jp)
  CXXFLAGS += -DLAGUERRE
endif

CPPFLAGS = -MD -MF $(@:.o=.dep)

-include $(OBJDIR)/*.dep
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

str.bench1.exe: $(OBJDIR)/str.bench1.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
all: str.bench1.exe

integer.nlz.bench1.exe: $(OBJDIR)/integer.nlz.bench1.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
all: integer.nlz.bench1.exe

clean:
	-rm -f *.o
