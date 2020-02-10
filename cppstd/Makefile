CPPFLAGS += -I./inc -I../inc/1.0
CXXFLAGS += -W -Wall

LIB  = ../lib/1.0/libcppstd.a
LIBD = ../lib/1.0/libcppstd-d.a

OBJS = Base_Arr.o dlstbase.o

vpath %.cpp src
vpath %.h  inc ../inc/1.0

all :  $(LIB) $(LIBD)

.phony: all

include ../Makefile.inc

