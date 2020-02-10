CPPFLAGS += -I./inc -I../inc/1.0
CXXFLAGS += -W -Wall -frtti

LIB  = ../lib/1.0/libPersist.a
LIBD = ../lib/1.0/libPersist-d.a

OBJS = Array.o Basic.o Field.o FieldList.o InTypePool.o InpPools.o OutPools.o OutTypePool.o \
       PersistentReader.o PersistentTypeDef.o PersistentWriter.o Struct.o Type.o

vpath %.cpp src
vpath %.h  inc ../inc/1.0

.phony: all

all :  $(LIB) $(LIBD)

include ../Makefile.inc

