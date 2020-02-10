//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Generalized Abstract Input Streams --------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INOREADER_INC
#define INOREADER_INC

#include "Exceptions.h"
#include "Basics.h"

#include <cstdio>

struct z_stream_s;

namespace Ino
{

//---------------------------------------------------------------------------

class ProgressReporter;

class Reader
{
  Reader(const Reader& cp);             // No Copying
  Reader& operator=(const Reader& src); // No Assignment

protected:
  ProgressReporter *const reporter;

  long bytesRead, bytesInc;

  bool reportProgress();

public:
  Reader(ProgressReporter *rep);
  virtual ~Reader() {}

  virtual bool isEof() const = 0;
  virtual bool isAborted()    const = 0;
  virtual long getErrorCode() const = 0;

  virtual bool isBuffered() const = 0;

  int getBytesRead() const { return bytesRead; }

  virtual int read(char *buf, int cap) = 0;
};

//---------------------------------------------------------------------------

class StdioReader : public Reader
{
  FILE *fd;

  long sysErrorCode;
  bool autoClose;

  StdioReader(const StdioReader& cp);             // No Copying
  StdioReader& operator=(const StdioReader& src); // No Assignment

public:
  StdioReader(FILE *file, bool closeOnDestroy,
                                             ProgressReporter *rep = 0);
  virtual ~StdioReader();

  virtual bool isEof() const;
  virtual bool isAborted() const;
  virtual long getErrorCode() const;
 
  bool isClosed() const;

  virtual bool isBuffered() const { return true; };

  virtual int read(char *buf, int cap);

  void close();
};

//---------------------------------------------------------------------------

class CompressedReader : public Reader
{
  Reader& rdr;

  bool decompressing, mustDecompress;

  struct z_stream_s *zStr;

  unsigned char *inBuf, *outBuf, *outp;
  int bufSz;

  bool eob;
  
  bool readData();
  bool pumpPlain();
  bool pump();

  CompressedReader(const CompressedReader& cp);
  CompressedReader& operator=(const CompressedReader& src);

public:
  CompressedReader(Reader& reader, int bufCap=4096,
                                             ProgressReporter *rep = 0);
  virtual ~CompressedReader();

  virtual bool isEof() const;
  virtual bool isAborted() const;
  virtual long getErrorCode() const;
  
  bool atEob() const;

  void reInit(bool newDecompress);

  void setDecompressing(bool newDecompress);
  bool getDecompressing() const { return mustDecompress; }

  virtual bool isBuffered() const { return true; };

  virtual int read(char *buf, int cap);
};

//---------------------------------------------------------------------------

class BufferedReader : public Reader
{
  Reader& rdr;

  char *const inputBuf;
  const int inputCap;
  char *inputEnd;
  char *inputPtr;

  BufferedReader(const BufferedReader& cp);
  BufferedReader& operator=(const BufferedReader& src);

public:
  BufferedReader(Reader& reader, int bufCap=4096, ProgressReporter *rep = 0);
  virtual ~BufferedReader();

  virtual bool isEof() const;
  virtual bool isAborted() const;
  virtual long getErrorCode() const;
  
  virtual bool isBuffered() const { return true; };

  virtual int read(char *buf, int cap);
};

//---------------------------------------------------------------------------

class UTFDataFormatException : public FileFormatException
{
  public:
    explicit UTFDataFormatException(const std::string& s)
                                           : FileFormatException(s) {}
};

//---------------------------------------------------------------------------

union DataReaderOverlay;

class DataReader : public Reader
{
  enum { UtfCap = 512 };

  Reader& rdr;

  const bool bigEndian;
  DataReaderOverlay *const dataBuf;

  char utfBuf[UtfCap];
  int utfIdx,utfUpb;

  DataReader(const DataReader& cp);
  DataReader& operator=(const DataReader& src);

  bool readData(char *buf, int itemSz, int len);
  bool readUtf8Byte(int& utfSz, char& c);

public:
  DataReader(Reader& reader, ProgressReporter *rep = 0);
  virtual ~DataReader();

  virtual bool isEof() const;
  virtual bool isAborted() const;
  virtual long getErrorCode() const;
  
  virtual bool isBuffered() const { return rdr.isBuffered(); };

  int readUtf8(wchar_t* wc, int cap, int utfSz);

  bool readBool(bool& b);
  bool readByte(char &c);
  bool readWChar(wchar_t& wc);
  bool readShort(short& s);
  bool readInt(long& i);
  bool readLong(__int64& l);
  bool readFloat(float& f);
  bool readDouble(double& d);

  bool read(bool *b,     int sz);
  virtual int read(char *c, int sz);
  bool read(wchar_t *wc, int sz);
  bool read(short *s,    int sz);
  bool read(long *i,     int sz);
  bool read(__int64 *l,   int sz);
  bool read(float *f,    int sz);
  bool read(double *d,   int sz);

  // These will throw a StreamCorruptedException on error:

  bool    readBool(const char *exceptMsg=NULL);
  char    readByte(const char *exceptMsg=NULL);
  wchar_t readWChar(const char *exceptMsg=NULL);
  short   readShort(const char *exceptMsg=NULL);
  long    readInt(const char *exceptMsg=NULL);
  __int64  readLong(const char *exceptMsg=NULL);
  float   readFloat(const char *exceptMsg=NULL);
  double  readDouble(const char *exceptMsg=NULL);

  void read(bool *b,     int sz, const char *exceptMsg);
  void read(char *c,     int sz, const char *exceptMsg);
  void read(wchar_t *wc, int sz, const char *exceptMsg);
  void read(short *s,    int sz, const char *exceptMsg);
  void read(long *i,     int sz, const char *exceptMsg);
  void read(__int64 *l,   int sz, const char *exceptMsg);
  void read(float *f,    int sz, const char *exceptMsg);
  void read(double *d,   int sz, const char *exceptMsg);
}; 

//---------------------------------------------------------------------------

class ByteArrayReader : public Reader
{
  char *data;
  int cap,pos,sz;

  ByteArrayReader(const ByteArrayReader& cp);             // No Copying
  ByteArrayReader& operator=(const ByteArrayReader& src); // No Assignment

public:
  ByteArrayReader(int initCap);
  virtual ~ByteArrayReader();

  virtual bool isEof() const { return pos >= sz; }
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

  virtual int read(char *buf, int bSz);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
