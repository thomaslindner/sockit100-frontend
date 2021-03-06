# Makefile
#
# $Id$
#

OSFLAGS  = -DOS_LINUX -Dextname
CFLAGS   = -g -O2 -fPIC -Wall -Wuninitialized -I. -I$(MIDASSYS)/include
CXXFLAGS = $(CFLAGS)

LIBS = -lm -lz -lutil  -lnsl -lpthread -lrt  
LIB_DIR         = $(MIDASSYS)/linux/lib

# MIDAS library
MIDASLIBS = $(MIDASSYS)/linux/lib/libmidas.a

# fix these for MacOS
UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)
MIDASLIBS = $(MIDASSYS)/darwin/lib/libmidas.a
LIB_DIR         = $(MIDASSYS)/darwin/lib
endif

# ROOT library
ifdef ROOTSYS
CXXFLAGS += -DHAVE_ROOT -I$(ROOTSYS)/include -I$(ROOTSYS)/include/root
ROOTGLIBS = $(shell $(ROOTSYS)/bin/root-config --glibs)  -Wl,-rpath,$(ROOTSYS)/lib
LIBS += $(ROOTGLIBS)
endif

all:: fesockit.exe 


fesockit.exe: %.exe:   %.o 
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIB_DIR)/mfe.o $(MIDASLIBS) $(LIBS)

feDTM.exe: %.exe:   %.o 
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIB_DIR)/mfe.o $(MIDASLIBS) $(LIBS)

fev1720sim.exe: %.exe:   %.o 
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIB_DIR)/mfe.o $(MIDASLIBS) $(LIBS)

%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(OSFLAGS) -c $<

%.o: %.c
	$(CXX) $(CXXFLAGS) $(OSFLAGS) -c $<

clean::
	-rm -f *.o *.exe

# end
