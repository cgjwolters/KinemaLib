CPPFLAGS += -I./inc -I../inc/1.0
CXXFLAGS += -W -Wall

LIB  = ../lib/1.0/libDxf3D.a
LIBD = ../lib/1.0/libDxf3D-d.a

OBJS = BlockTable3D.o ColorTable3D.o DxfBlock3D.o DxfLayer3D.o DxfLineType3D.o \
       DxfNameTable3D.o DxfPattern3D.o DxfRead3D.o DxfReader3D.o HeaderTable3D.o \
       IntNameTable3D.o LayerTable3D.o LineTypeTable3D.o \
       ObjNameTableBase3D.o TextStyleTable3D.o

vpath %.cpp src
vpath %.h  inc ../inc/1.0

.phony : all

all :  $(LIB) $(LIBD)

include ../Makefile.inc


