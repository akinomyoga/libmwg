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
CXXFLAGS += -I $(CPPDIR) -I $(CFGDIR)/include -O3 -march=native -Wall -Wextra
CPPFLAGS = -MD -MF $(@:.o=.dep)
LIBS = -lrt

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
