CPPFLAGS += -I./inc -I../inc/1.0
CXXFLAGS += -W -Wall

LIB  = ../lib/1.0/libMatrix.a
LIBD = ../lib/1.0/libMatrix-d.a

OBJS = Matrix.o Svd.o

vpath %.cpp src
vpath %.h  inc ../inc/1.0

.phony : all

all :  $(LIB) $(LIBD)

include ../Makefile.inc


