CPPFLAGS += -I./inc -I../inc/1.0
CXXFLAGS += -W -Wall

LIB  = ../lib/1.0/libDxf.a
LIBD = ../lib/1.0/libDxf-d.a

OBJS = BlockTable.o ColorTable.o DxfBlock.o DxfLayer.o DxfLineType.o DxfNameTable.o DxfPattern.o \
       DxfRead.o DxfReader.o HeaderTable.o IntNameTable.o LayerTable.o LineTypeTable.o \
       ObjNameTableBase.o TextStyleTable.o

vpath %.cpp src
vpath %.h  inc ../inc/1.0

.phony : all

all :  $(LIB) $(LIBD)

include ../Makefile.inc


