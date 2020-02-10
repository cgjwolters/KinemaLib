CPPFLAGS += -I./inc -IVExp/inc -I../Contour/inc -I../../cppstd/inc -I../Approx/inc -I../../inc/1.0 -I../../inc/Geo/1.0
CXXFLAGS += -W -Wall

LIB   = ../../lib/Geo/1.0/libMsrData.a
LIBD  = ../../lib/Geo/1.0/libMsrData-d.a

OBJS = MsrCont.o

vpath %.cpp src
vpath %.h  inc VExp/inc ../Contour/inc ../../cppstd/inc ../Approx/inc ../../inc/1.0 ../../inc/Geo/1.0

all :  $(LIB) $(LIBD)

.phony: all

include ../../Makefile.inc

