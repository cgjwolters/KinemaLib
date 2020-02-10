CPPFLAGS += -I./inc -I../inc/1.0 -I../inc/Geo/1.0 -I../cppstd/inc
CXXFLAGS += -W -Wall

LIB  = ../lib/1.0/libDxfOut.a
LIBD = ../lib/1.0/libDxfOut-d.a

OBJS = DxfOut.o

vpath %.cpp src
vpath %.h  inc ../inc/1.0 ../inc/Geo/1.0 ../cppstd/inc

.phony : all

all :  $(LIB) $(LIBD)

include ../Makefile.inc


