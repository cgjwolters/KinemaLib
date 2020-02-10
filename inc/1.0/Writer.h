//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Generalized Abstract Output Streams--------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INOWRITER_INC
#define INOWRITER_INC

#include "Exceptions.h"
#include "Basics.h"

#include <cstdio>

//---------------------------------------------------------------------------

struct z_stream_s;

namespace Ino
{

//---------------------------------------------------------------------------

class ProgressReporter;

class Writer
{
  Writer(const Writer& cp);             // No Copying
  Writer& operator=(const Writer& src); // No Assignment

protected:
  ProgressReporter *const reporter;

  long bytesWritten, bytesInc;

  virtual bool reportProgress();

public:
  Writer(ProgressReporter *rep);
  virtual ~Writer() {}

  virtual bool isClosed() const = 0;
  virtual bool isAborted() const = 0;
  virtual long getErrorCode() const = 0;

  virtual long getBytesWritten() const { return bytesWritten; }
  virtual void resetBytesWritten();

  virtual bool isBuffered() const = 0;

  virtual bool write(const char *buf, int sz) = 0;
  virtual bool flush() = 0;

  virtual bool close() { return true; }
};

//---------------------------------------------------------------------------

class StdioWriter : public Writer
{
  FILE *fd;

  long sysErrorCode;
  bool autoClose;

  StdioWriter(const StdioWriter& cp);             // No Copying
  StdioWriter& operator=(const StdioWriter& src); // No Assignment

public:
  StdioWriter(FILE *file, bool closeOnDestroy,
                                            ProgressReporter *rep = NULL);
  virtual ~StdioWriter();

  virtual bool isClosed() const;
  virtual bool isAborted() const;
  virtual long getErrorCode() const { return sysErrorCode; }

  virtual bool isBuffered() const { return true; }

  virtual bool write(const char *buf, int sz);
  virtual bool flush();
  virtual bool close();
};

//---------------------------------------------------------------------------

class CompressedWriter : public Writer
{
  Writer& wrt;

  struct z_stream_s *zStr;

  unsigned char *inBuf, *outBuf;
  unsigned int  bufSz;

  bool compressing;
  bool flushed;

  long rawBytesWritten;

  bool writeData(int len);
  bool pumpPlain();
  bool pump();

  virtual bool reportProgress();

  CompressedWriter(const CompressedWriter& cp);             // No Copying
  CompressedWriter& operator=(const CompressedWriter& src); // No Assignment

public:
  CompressedWriter(Writer& writer, int bufCap,
                                              ProgressReporter *rep = NULL);
  virtual ~CompressedWriter();

  bool isCompressing() const { return compressing; }
  void setCompressing(bool newCompress);

  long getRawBytesWritten() const { return rawBytesWritten; }
  virtual void resetBytesWritten();

  virtual bool isClosed() const;
  virtual bool isAborted() const;
  virtual long getErrorCode() const { return wrt.getErrorCode(); }

  virtual bool isBuffered() const { return true; }

  virtual bool write(const char *buf, int sz);
  virtual bool flush();
};

//---------------------------------------------------------------------------

class Crc32Writer : public Writer
{
  Writer& wrt;
  long crc;

  Crc32Writer(const Crc32Writer& cp);             // No Copying
  Crc32Writer& operator=(const Crc32Writer& src); // No Assignment

public:
  Crc32Writer(Writer& writer, ProgressReporter *rep = NULL);
  ~Crc32Writer() {}

  void resetCrc();
  long getCrc32() const { return crc; }

  virtual bool isClosed() const;
  virtual bool isAborted() const;
  virtual long getErrorCode() const { return wrt.getErrorCode(); }

  virtual bool isBuffered() const { return wrt.isBuffered(); }

  virtual bool write(const char *buf, int sz);
  virtual bool flush();
};

//---------------------------------------------------------------------------

class BufferedWriter : public Writer
{
  Writer& wrt;

  char *const outputBuf;
  char *outputEnd;
  char *outputPtr;

  BufferedWriter(const BufferedWriter& cp);             // No Copying
  BufferedWriter& operator=(const BufferedWriter& src); // No Assignment

public:
  BufferedWriter(Writer& writer, int bufCap, ProgressReporter *rep = NULL);
  virtual ~BufferedWriter();

  virtual bool isClosed() const;
  virtual bool isAborted() const;
  virtual long getErrorCode() const { return wrt.getErrorCode(); }

  virtual bool isBuffered() const { return true; }

  virtual bool write(const char *buf, int sz);
  virtual bool flush();
};

//---------------------------------------------------------------------------

union DataWriterOverlay;

class DataWriter : public Writer
{
  Writer& wrt;

  bool bigEndian;
  DataWriterOverlay *const dataBuf;

  bool writeItem(char *buf, int len);

  bool writeDirect(const char *buf, int len);
  bool writeArray(const char *buf, int itemSz, int len);

  DataWriter(const DataWriter& cp);             // No Copying
  DataWriter& operator=(const DataWriter& src); // No Assignment

public:
  DataWriter(Writer& writer, ProgressReporter *rep = NULL);
  virtual ~DataWriter();

  void writeLittleEndian();

  virtual bool isClosed() const;
  virtual bool isAborted() const;
  virtual long getErrorCode() const { return wrt.getErrorCode(); }

  virtual bool isBuffered() const { return wrt.isBuffered(); }

  virtual bool flush();

  static int calcUtf8Len(const wchar_t* wc, int sz);
  bool writeUtf8(const wchar_t* wc, int sz);

  bool writeBool(bool b);
  bool writeByte(char c);
  bool writeWChar(wchar_t wc);
  bool writeShort(short s);
  bool writeInt(long i);
  bool writeLong(__int64 l);
  bool writeFloat(float f);
  bool writeDouble(double d);

  bool write(const bool *b,     int sz);
  virtual bool write(const char *c,   int sz);
  bool write(const wchar_t *wc, int sz);
  bool write(const short *s,    int sz);
  bool write(const long *i,     int sz);
  bool write(const __int64 *l,   int sz);
  bool write(const float *f,    int sz);
  bool write(const double *d,   int sz);

  // These will throw a StreamClosedException on error:

  void writeBool(bool b, const char* exceptMsg);
  void writeByte(char c, const char* exceptMsg);
  void writeWChar(wchar_t wc, const char* exceptMsg);
  void writeShort(short s, const char* exceptMsg);
  void writeInt(long i, const char* exceptMsg);
  void writeLong(__int64 l, const char* exceptMsg);
  void writeFloat(float f, const char* exceptMsg);
  void writeDouble(double d, const char* exceptMsg);

  void write(const bool *b,     int sz, const char* exceptMsg);
  void write(const char *c,   int sz, const char* exceptMsg);
  void write(const wchar_t *wc, int sz, const char* exceptMsg);
  void write(const short *s,    int sz, const char* exceptMsg);
  void write(const long *i,     int sz, const char* exceptMsg);
  void write(const __int64 *l,   int sz, const char* exceptMsg);
  void write(const float *f,    int sz, const char* exceptMsg);
  void write(const double *d,   int sz, const char* exceptMsg);
};

//---------------------------------------------------------------------------

class ByteArrayWriter : public Writer
{
  char *data;
  int cap,capInc;
  int pos,sz;

  ByteArrayWriter(const ByteArrayWriter& cp);             // No Copying
  ByteArrayWriter& operator=(const ByteArrayWriter& src); // No Assignment

public:
  ByteArrayWriter(int initCap, int incCap);
  ~ByteArrayWriter();

  virtual bool isClosed() const { return false; }
  virtual bool isAborted() const { return false; }
  virtual long getErrorCode() const { return 0; }

  virtual bool isBuffered() const { return true; }

  int getCap() const { return cap; }
  void setCap(int newCap);
  void ensureCap(int newCap);

  int getPos() const { return pos; }
  void setPos(int newPos);

  int getSize() const { return sz; }
  void setSize(int newSz);

  char* &getBuffer() { return data; }

  void clearArray(char val = 0);

  virtual bool write(const char *buf, int bSz);
  virtual bool flush() { return true; } // Is a no-op
};

//---------------------------------------------------------------------------

class Base64Writer : public Writer
{
  Writer& wrt;
  char buf[84];
  int bufSz, mod, msk;

  bool writeBuf();

  bool reportProgress();

  Base64Writer(const Base64Writer& cp);             // No Copying
  Base64Writer& operator=(const Base64Writer& src); // No Assignment

public:
  Base64Writer(Writer& writer, ProgressReporter *rep = NULL);
  ~Base64Writer();

  virtual bool isClosed() const;
  virtual bool isAborted() const;
  virtual long getErrorCode() const { return wrt.getErrorCode(); }

  virtual bool isBuffered() const { return true; }

  virtual bool write(const char *msg, int msgLen);
  virtual bool flush();
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
