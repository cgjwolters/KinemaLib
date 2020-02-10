DIR_ROOT       = /home/sfw/InoforLibs
DIR_InoQt       = $$DIR_ROOT/InoQt

DIR_UI         = UI
DIR_INC        = inc ../inc/1.0 ../Gsasl32/include
DIR_SRC        = src
DIR_RSC        = Resources

CONFIG(debug,debug|release) {
  DIR_GEN        = GeneratedFiles-d
}
else {
  DIR_GEN        = GeneratedFiles
}

DEPENDPATH     = $$DIR_RSC $$DIR_INC $$DIR_UI $$DIR_SRC
 
CONFIG        += qt debug_and_release build_all staticlib
MOC_DIR        = $$DIR_GEN
OBJECTS_DIR    = $$DIR_GEN
RCC_DIR        = $$DIR_GEN
UI_DIR         = $$DIR_GEN
INCLUDEPATH    = $$DIR_INC

TEMPLATE       = lib

DESTDIR        = ../lib/1.0
VERSION        = 1.10.0

SOURCES        = Smtp.cpp \
                 SmtpAttachmentWriter.cpp \
                 SmtpZipWriter.cpp
                 
FORMS          = 

HEADERS        = Smtp.h \
                 SmtpAttachmentWriter.h \
                 SmtpZipWriter.h

# RESOURCES      = InoQt.qrc
 
# LIBS           = -L../lib/1.0

CONFIG(debug,debug|release) {
  # LIBS += -lBasics-d -lTsharc12Driver-d -lMatrix-d
  TARGET = InoQt-d
}
else {
  # LIBS += -lBasics 
  TARGET = InoQt
}

unix {
  crlf.target = crlf
  crlf.commands = @find . \( -name Makefile -o -name '*.cpp' -o -name '*.c' -o -name '*.h' \) \
                -exec /bin/sh -c "mv {} {}.$$$$; sed 's/\r//g' {}.$$$$ > {}; rm -f {}.$$$$; chmod 664 {}" \;

  distclean.target = distclean
  distclean.depends = clean
  distclean.commands  = -$(DEL_FILE) $(DESTDIR)/$(TARGET); $(DEL_FILE) Makefile
                    
  QMAKE_EXTRA_TARGETS += crlf distclean
}
