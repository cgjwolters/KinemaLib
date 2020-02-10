CPPFLAGS += -I./inc -I../../cppstd/inc -I../../inc/1.0 -I../../inc/Geo/1.0
CXXFLAGS += -W -Wall

LIB  = ../../lib/Geo/1.0/libContour.a
LIBD = ../../lib/Geo/1.0/libContour-d.a

OBJS = Contisct2.o contisct1.o contour.o contouri.o el_arc.o el_cir.o el_line.o elem.o \
       geo.o isect.o sub_rect.o

vpath %.cpp src
vpath %.h  inc ../../cppstd/inc ../../inc/1.0 ../../inc/Geo/1.0

.phony: all

all :  $(LIB) $(LIBD)

include ../../Makefile.inc

