CPPFLAGS += -I./inc -I../inc/1.0
CXXFLAGS += -W -Wall

LIB  = ../lib/1.0/libCcdOut.a
LIBD = ../lib/1.0/libCcdOut-d.a

OBJS = CcdDb.o CcdEnt.o CcdLayer.o CcdWFile.o CcdOut.o

vpath %.cpp src
vpath %.h  inc ../inc/1.0

.phony : all

all :  $(LIB) $(LIBD)

include ../Makefile.inc


