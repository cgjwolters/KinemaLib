CPPFLAGS += -I../../inc/zlib
CFLAGS += -O2 -W -Wall

CC = gcc

LIB  = ../../../lib/1.0/libzlib.a

OBJS = adler32.o compress.o crc32.o deflate.o gzio.o inffast.o inflate.o inftrees.o trees.o uncompr.o zutil.o

vpath %.h ../../inc/zlib

.phony: all clean

all : $(LIB)

$(LIB) : $(OBJS:.o=.d)

$(LIB) : $(OBJS)
	$(AR) -r $@ $^

clean: 
	rm -f $(OBJS) $(OBJS:.o=.d) \
	exit;

distclean: 
	rm -f $(OBJS) $(OBJS:.o=.d) $(LIB) \
	exit;

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) -MT "$@ $(@.d=.o)" $< > $@;

-include $(OBJS:.o=.d)

