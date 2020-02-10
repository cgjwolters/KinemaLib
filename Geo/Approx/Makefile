CPPFLAGS += -I./inc -I../MsrData/inc -I../Contour/inc -I../../cppstd/inc -I../../inc/1.0 -I../../inc/Geo/1.0
CXXFLAGS += -W -Wall

LIB  = ../../lib/Geo/1.0/libApprox.a
LIBD = ../../lib/Geo/1.0/libApprox-d.a

OBJS = LsGeo.o

vpath %.cpp src
vpath %.h  inc ../MsrData/inc ../Contour/inc ../../cppstd/inc ../../inc/1.0 ../../inc/Geo/1.0

.phony: all

all :  $(LIB) $(LIBD)

include ../../Makefile.inc

