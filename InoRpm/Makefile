CPPFLAGS += -I./inc -I../inc/1.0
CXXFLAGS += -W -Wall

LIB  = lib/libInoRpm.a
LIBD = lib/libInoRpm-d.a

OBJS = Capability.o CapabilityList.o Package.o PackageList.o FileList.o RpmDataDef.o

vpath %.cpp src
vpath %.h  inc ../inc/1.0

.phony : all

all :  $(LIB) $(LIBD)

include ../Makefile.inc
