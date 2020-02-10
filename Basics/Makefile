CPPFLAGS += -I./inc -I./inc/zlib -I../inc/1.0
CXXFLAGS += -W -Wall

LIB  = ../lib/1.0/libBasics.a
LIBD = ../lib/1.0/libBasics-d.a

OBJS = Base64.o Base64Writer.o Basics.o \
       BufferedReader.o BufferedWriter.o ByteArrayReader.o ByteArrayWriter.o \
       CompressedReader.o CompressedWriter.o Crc.o DataReader.o DataWriter.o \
       DesCipher.o Hex.o EventDispatcher.o Hex.o NonLinLsSolver.o ProgressReporter.o \
       Reader.o StdioReader.o StdioWriter.o Trf.o PTrf.o TrfTrain.o \
       Vec.o PVec.o Rect.o Box3D.o Writer.o Crc32Writer.o ZipOut.o
       
vpath %.cpp src
vpath %.h  inc inc/zlib ../inc/1.0

.phony: all zlib

all :  $(LIB) $(LIBD) zlib

zlib:
	cd src/zlib; $(MAKE)

undefclean = 1 # Prevents Makefile.inc from defining targets "clean" and "distclean"

include ../Makefile.inc

clean:
	rm -f $(OBJS) $(OBJS:.o=d.o) $(OBJS:.o=.d); \
	cd src/zlib; $(MAKE) clean
	exit

distclean:
	rm -f $(OBJS) $(OBJS:.o=d.o) $(OBJS:.o=.d) $(LIB) $(LIBD); \
	cd src/zlib; $(MAKE) distclean
	exit

