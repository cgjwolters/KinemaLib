//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Data Input Stream (Data in network byteorder) ---------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Reader.h"

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup stream_reader Stream Readers
 @{
*/

//---------------------------------------------------------------------------
/** \class DataReader
  Reads basic types (such as chars, ints, doubles) from
  a \ref Ino::Reader Reader.

  The data must have been previously written by a \ref DataWriter.\n
  The data is always read/written in <em>network byte order</em>: the format
  of the data is hardware independent.\n
  \n
  The various read methods come in two flavors:
  \li Methods that \b return the value to be read, these throw an
  exception if there is an error while reading.
  \li Methods that take an argument (to receive the value read) and return
  \c true or \c false to indicate success/failure.

  \see DataWriter
  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------

union DataReaderOverlay {
  bool b;
  char c;
  wchar_t wc;
  short s;
  int   i;
  __int64 l;
  float  f;
  double d;

  char data[16];
};

//---------------------------------------------------------------------------

static bool isBigEndian()
{
  DataReaderOverlay buf;

  buf.data[0] = '\1'; buf.data[1] = '\0';

  return buf.s != 1;
}

//---------------------------------------------------------------------------

static void reverse(char *buf, int len)
{
  --len;
  int beg = 0;

  while (len > beg) {
    char c = buf[beg];
    buf[beg++] = buf[len];
    buf[len--] = c;
  }
}

//---------------------------------------------------------------------------

bool DataReader::readData(char *buf, int itemSz, int len)
{
  if (rdr.isEof() || isAborted()) 
     return false;

  int initLen = len;
  char *bufPtr = buf;
  len *= itemSz;

  while (len > 0) {
    int byteCount = rdr.read(bufPtr,len);

    if (byteCount <= 0) return false;

    bufPtr += byteCount;
    len    -= byteCount;

    bytesRead += byteCount;
    bytesInc  += byteCount;
  }

  if (!bigEndian && itemSz > 1) {
    for (int i=0; i<initLen; ++i) {
      reverse(buf,itemSz);
      buf += itemSz;
    }
  }

  if (!reporter) return true;

  return reportProgress();
}

//---------------------------------------------------------------------------
/** Constructor.
  \param reader The \ref Reader to read the data from.
  \param rep An optional \ref ProgressReporter, may be \c NULL.
*/

DataReader::DataReader(Reader& reader, ProgressReporter *rep)
: Reader(rep), rdr(reader), bigEndian(isBigEndian()),
   dataBuf(new DataReaderOverlay()), utfIdx(0), utfUpb(0)
{
}

//---------------------------------------------------------------------------
/** Destructor.

  Does \b not close the underlying \ref Reader.
*/
DataReader::~DataReader()
{
  if (dataBuf) delete dataBuf;
}

//---------------------------------------------------------------------------
/** End-of-file indicator.
  \return \c true if:
  \li Method \ref isAborted() const "isAborted()" returns \c true or
  \li The underlying reader returns an end-of-file condition.
*/

bool DataReader::isEof() const
{
  return isAborted() || rdr.isEof();
}
  
//---------------------------------------------------------------------------
/** Abort indicator.
  \return \c true if:
  \li The underlying Reader returns an abort condition.
  \li The optional \ref ProgressReporter returns an abort condition.
*/

bool DataReader::isAborted() const
{
  if (rdr.isAborted()) return true;
  if (reporter) return reporter->mustAbort();

  return false;
}

//---------------------------------------------------------------------------
/** Returns the latest system error code regarding this reader.
  \return Zero if all is ok, or else the errorcode of the underlying
  Reader.
*/

long DataReader::getErrorCode() const
{
  return rdr.getErrorCode();
}

//---------------------------------------------------------------------------

bool DataReader::readUtf8Byte(int& utfSz, char& c)
{
  if (utfIdx >= utfUpb) {
    if (utfSz < 1) return false;

    utfIdx = 0;
    utfUpb = utfSz;
    if (utfUpb > UtfCap) utfUpb = UtfCap;

    if (!readData(utfBuf,1,utfUpb)) return false;

    utfSz -= utfUpb;
  }

  c = utfBuf[utfIdx++];

  return true;
}

//---------------------------------------------------------------------------
/** Reads an Utf8 encoded Unicode string.
   \param wc The buffer to receive the decoded Unicode string.
   \param cap The capacity (in Unicode characters) of buffer \c wc.
   \param utfSz The number of \b bytes to read from the stream.
   \return The number of Unicode characters actually written into
   buffer \c wc (possibly zero) or\n
   -1 if:
   \li There was an error while reading from the underlying
   \ref Reader.
   \li If \c utfSz bytes have been read and more
   bytes are needed to form the next Unicode character (so
   probably the value of \c utfSz is wrong).
   \li The optional \ref ProgressReporter wants to abort.
   \throw NullPointerException If <tt>wc == NULL</tt>.
   \throw IllegalArgumentException If <tt>cap < 0 || utfSz < 0</tt>.
   \throw UTFDataFormatException If the data read does not conform
   to the Utf8 coding standard or if parameter \c utfSz is not correct.
   \note Parameter \c utfSz must specify the exact amount of bytes
   to read from the underlying \ref Reader. This value cannot be
   predicted, but must be stored with the stream.\n
   It can be calculated with method
   \ref DataWriter::calcUtf8Len(const wchar_t* wc, int sz) "DataWriter::calcUtf8Len()").
*/

int DataReader::readUtf8(wchar_t* wc, int cap, int utfSz)
{
  if (!wc) throw NullPointerException("DataReader::readUtf8: wc == NULL");
  if (cap < 0) throw IllegalArgumentException("DataReader::readUtf8: cap < 0");
  if (utfSz < 0) throw IllegalArgumentException("DataReader::readUtf8: utfSz < 0");

  if (utfSz < 1) return 0;

  int initCap = cap;
  int initUtfSz = utfSz;

  utfIdx = 0;
  utfUpb = 0;

  char c;
  int idx = 0;
  
  while (idx++ < initUtfSz) {
    if (!readUtf8Byte(utfSz,c)) return -1;

    switch ((c >> 4) & 0x0F) {
      case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
        // 0xxxxxxx
        if (cap > 0) {
          *wc++ = c;
          --cap;
        }
      break;

      case 12: // 110x xxxx   10xx xxxx
      case 13: {
        if (idx >= initUtfSz) throw UTFDataFormatException("");

        char c2;
        if (!readUtf8Byte(utfSz,c2)) return -1;

        ++idx;

        if ((c2 & 0xC0) != 0x80) throw UTFDataFormatException("");

        if (cap > 0) {
          *wc++ = (wchar_t)(((c & 0x1F) << 6) | (c2 & 0x3F));
          --cap;
        }
      }
      break;

      case 14: { // 1110 xxxx  10xx xxxx  10xx xxxx
        if (idx >= initUtfSz-1) throw UTFDataFormatException("");

        char c2;
        if (!readUtf8Byte(utfSz,c2)) return -1;

        char c3;
        if (!readUtf8Byte(utfSz,c3)) return -1;
        
        idx += 2;

        if (((c2 & 0xC0) != 0x80) || ((c3 & 0xC0) != 0x80))
                                           throw UTFDataFormatException("");

        if (cap > 0) {
          *wc++ = (wchar_t)(((c&0x0F) << 12) | ((c2&0x3F) << 6) | ((c3&0x3F) << 0));
          --cap;
        }
      }
      break;

      default: /* 10xx xxxx,  1111 xxxx */
               throw UTFDataFormatException("");
    }
  }

  return initCap - cap;
}

//---------------------------------------------------------------------------
/** Reads a boolean value from the stream.
    \param b Receives the boolean to read.
    \return \c true If the value was read,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
*/

bool DataReader::readBool(bool& b)
{
  if (!readData(dataBuf->data,1,1)) return false;

  b = dataBuf->b;

  return true;
}

//---------------------------------------------------------------------------
/** Reads an 8-bit byte \c (char) value from the stream.
    \param c Receives the byte to read.
    \return \c true If the value was read,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
*/

bool DataReader::readByte(char &c)
{
  if (!readData(dataBuf->data,1,1)) return false;

  c = dataBuf->c;

  return true;
}

//---------------------------------------------------------------------------
/** Reads a 2-byte Unicode character value from the stream.
    \param wc Receives the Unicode character to read.
    \return \c true If the value was read,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
*/

bool DataReader::readWChar(wchar_t& wc)
{
  if (!readData(dataBuf->data,2,1)) return false;

  wc = dataBuf->wc;

  return true;
}

//---------------------------------------------------------------------------
/** Reads a 2-byte short integer value from the stream.
   \param s Receives the short integer to read.
    \return \c true If the value was read,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
*/

bool DataReader::readShort(short& s)
{
  if (!readData(dataBuf->data,2,1)) return false;

  s = dataBuf->s;

  return true;
}

//---------------------------------------------------------------------------
/** Reads a 4-byte long integer value from the stream.
   \param i Receives the integer to read.
    \return \c true If the value was read,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
*/

bool DataReader::readInt(long& i)
{
  if (!readData(dataBuf->data,4,1)) return false;

  i = dataBuf->i;

  return true;
}

//---------------------------------------------------------------------------
/** Reads an 8-byte long long integer value from the stream.
   \param l Receives the long long to read.
    \return \c true If the value was read,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
*/

bool DataReader::readLong(__int64& l)
{
  if (!readData(dataBuf->data,8,1)) return false;

  l = dataBuf->l;

  return true;
}

//---------------------------------------------------------------------------
/** Reads a 4-byte float value from the stream.
   \param f Receives the float to read.
    \return \c true If the value was read,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
*/

bool DataReader::readFloat(float& f)
{
  if (!readData(dataBuf->data,4,1)) return false;

  f = dataBuf->f;

  return true;
}

//---------------------------------------------------------------------------
/** Reads an 8-byte double value from the stream.
   \param d Receives the double to read.
    \return \c true If the value was read,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
*/

bool DataReader::readDouble(double& d)
{
  if (!readData(dataBuf->data,8,1)) return false;

  d = dataBuf->d;

  return true;
}

//---------------------------------------------------------------------------
/** Reads an array of booleans from the stream.
   \param b Pointer to the buffer that will receive the result.
   \param sz The number of booleans to read.
    \return \c true If successfull,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
   \throw NullPointerException If <tt>b == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
*/

bool DataReader::read(bool *b, int sz)
{
  if (!b) throw NullPointerException("DataReader::read(bool): b == NULL");
  if (sz < 1) throw IllegalArgumentException("DataReader::read(bool): sz < 1");

  return readData((char *)b,1,sz);
}

//---------------------------------------------------------------------------
/** Reads an array of bytes from the stream.
   \param c Pointer to the buffer that will receive the result.
   \param sz The number of characters to read.
    \return \c true If successfull,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
   \throw NullPointerException If <tt>c == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
*/

int DataReader::read(char *c, int sz)
{
  if (!c) throw NullPointerException("DataReader::read(char): c == NULL");
  if (sz < 1) throw IllegalArgumentException("DataReader::readCh: sz < 1");

  if (readData(c,1,sz)) return sz;
  else return -1;
}

//---------------------------------------------------------------------------
/** Reads an array of (2-byte) Unicode characters from the stream.
   \param wc Pointer to the buffer that will receive the result.
   \param sz The number of characters to read.
    \return \c true If successfull,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
   \throw NullPointerException If <tt>wc == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
*/

bool DataReader::read(wchar_t *wc, int sz)
{
  if (!wc) throw NullPointerException("DataReader::read(wchar_t): wc == NULL");
  if (sz < 1) throw IllegalArgumentException("DataReader::read(wchar_t): sz < 1");

  return readData((char *)wc,2,sz);
}

//---------------------------------------------------------------------------
/** Reads an array of (2-byte) short integers from the stream.
   \param s Pointer to the buffer that will receive the result.
   \param sz The number of short integers to read.
    \return \c true If successfull,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
   \throw NullPointerException If <tt>s == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
*/

bool DataReader::read(short *s, int sz)
{
  if (!s) throw NullPointerException("DataReader::read(short): s == NULL");
  if (sz < 1) throw IllegalArgumentException("DataReader::read(short): sz < 1");

  return readData((char *)s,2,sz);
}

//---------------------------------------------------------------------------
/** Reads an array of (4-byte) long integers from the stream.
   \param i Pointer to the buffer that will receive the result.
   \param sz The number of long integers to read.
    \return \c true If successfull,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
   \throw NullPointerException If <tt>i == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
*/

bool DataReader::read(long *i, int sz)
{
  if (!i) throw NullPointerException("DataReader::read(long): i == NULL");
  if (sz < 1) throw IllegalArgumentException("DataReader::read(long): sz < 1");

  return readData((char *)i,4,sz);
}

//---------------------------------------------------------------------------
/** Reads an array of (8-byte) long long integers from the stream.
   \param l Pointer to the buffer that will receive the result.
   \param sz The number of long long integers to read.
    \return \c true If successfull,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
   \throw NullPointerException If <tt>l == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
*/

bool DataReader::read(__int64 *l, int sz)
{
  if (!l) throw NullPointerException("DataReader::read(__int64): l == NULL");
  if (sz < 1) throw IllegalArgumentException("DataReader::read(__int64): sz < 1");

  return readData((char *)l,8,sz);
}

//---------------------------------------------------------------------------
/** Reads an array of (4-byte) floats from the stream.
   \param f Pointer to the buffer that will receive the result.
   \param sz The number of floats to read.
    \return \c true If successfull,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
   \throw NullPointerException If <tt>f == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
*/

bool DataReader::read(float *f, int sz)
{
  if (!f) throw NullPointerException("DataReader::read(float): f == NULL");
  if (sz < 1) throw IllegalArgumentException("DataReader::read(float): sz < 1");

  return readData((char *)f,4,sz);
}

//---------------------------------------------------------------------------
/** Reads an array of (8-byte) doubles from the stream.
   \param d Pointer to the buffer that will receive the result.
   \param sz The number of doubles to read.
    \return \c true If successfull,\n
    \c false If:
    \li There was an error in the underlying \ref Reader.
    \li The optional \ref ProgressReporter wants to abort.
   \throw NullPointerException If <tt>d == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
*/

bool DataReader::read(double *d, int sz)
{
  if (!d) throw NullPointerException("DataReader::read(double): d == NULL");
  if (sz < 1) throw IllegalArgumentException("DataReader::read(double): sz < 1");

  return readData((char *)d,8,sz);
}

//---------------------------------------------------------------------------
/** Reads a boolean value from the stream.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \return The boolean read.
   \throw StreamCorruptedException If there is an error.
*/

bool DataReader::readBool(const char *exceptMsg)
{
  bool b;
  if (readBool(b)) return b;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::readBool: Premature End");
}

//---------------------------------------------------------------------------
/** Reads a byte (char) value from the stream.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \return The byte read.
   \throw StreamCorruptedException If there is an error.
*/

char DataReader::readByte(const char *exceptMsg)
{
  char c;
  if (readByte(c)) return c;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::readByte: Premature End");
}

//---------------------------------------------------------------------------
/** Reads a 2-byte Unicode character from the stream.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \return The Unicode character read.
   \throw StreamCorruptedException If there is an error.
*/

wchar_t DataReader::readWChar(const char *exceptMsg)
{
  wchar_t wc;
  if (readWChar(wc)) return wc;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::readWChar: Premature End");
}

//---------------------------------------------------------------------------
/** Reads a 2-byte short integer from the stream.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \return The short integer read.
   \throw StreamCorruptedException If there is an error.
*/

short DataReader::readShort(const char *exceptMsg)
{
  short s;
  if (readShort(s)) return s;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::readShort: Premature End");
}

//---------------------------------------------------------------------------
/** Reads a 4-byte long integer from the stream.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \return The long integer read.
   \throw StreamCorruptedException If there is an error.
*/

long DataReader::readInt(const char *exceptMsg)
{
  long i;
  if (readInt(i)) return i;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::readInt: Premature End");
}

//---------------------------------------------------------------------------
/** Reads an 8-byte long long integer from the stream.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \return The long long integer read.
   \throw StreamCorruptedException If there is an error.
*/

__int64 DataReader::readLong(const char *exceptMsg)
{
  __int64 l;
  if (readLong(l)) return l;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::readLong: Premature End");
}

//---------------------------------------------------------------------------
/** Reads a 4-byte float value from the stream.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \return The float value read.
   \throw StreamCorruptedException If there is an error.
*/

float DataReader::readFloat(const char *exceptMsg)
{
  float f;
  if (readFloat(f)) return f;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::readFloat: Premature End");
}

//---------------------------------------------------------------------------
/** Reads an 8-byte double value from the stream.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \return The double value read.
   \throw StreamCorruptedException If there is an error.
*/

double DataReader::readDouble(const char *exceptMsg)
{
  double d;
  if (readDouble(d)) return d;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::readDouble: Premature End");
}

//---------------------------------------------------------------------------
/** Reads an array of booleans from the stream.
   \param b Pointer to the buffer that will receive the result.
   \param sz The number of booleans to read.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error.
*/

void DataReader::read(bool *b, int sz, const char *exceptMsg)
{
  if (read(b,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::read(bool*): Premature End");
}

//---------------------------------------------------------------------------
/** Reads an array of bytes (char) from the stream.
   \param c Pointer to the buffer that will receive the result.
   \param sz The number of bytes to read.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw NullPointerException If <tt>c == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
   \throw StreamCorruptedException If there is an error.
*/

void DataReader::read(char *c, int sz, const char *exceptMsg)
{
  if (read(c,sz) == sz) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::read(char*): Premature End");
}

//---------------------------------------------------------------------------
/** Reads an array of 2-byte Unicode characters from the stream.
   \param wc Pointer to the buffer that will receive the result.
   \param sz The number of Unicode characters to read.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw NullPointerException If <tt>wc == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
   \throw StreamCorruptedException If there is an error.
*/

void DataReader::read(wchar_t *wc, int sz, const char *exceptMsg)
{
  if (read(wc,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::read(wchar_t*): Premature End");
}

//---------------------------------------------------------------------------
/** Reads an array of 2-byte short integers from the stream.
   \param s Pointer to the buffer that will receive the result.
   \param sz The number of short integers to read.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw NullPointerException If <tt>s == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
   \throw StreamCorruptedException If there is an error.
*/

void DataReader::read(short *s, int sz, const char *exceptMsg)
{
  if (read(s,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::read(short*): Premature End");
}

//---------------------------------------------------------------------------
/** Reads an array of 4-byte long integers from the stream.
   \param i Pointer to the buffer that will receive the result.
   \param sz The number of long integers to read.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw NullPointerException If <tt>i == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
   \throw StreamCorruptedException If there is an error.
*/

void DataReader::read(long *i, int sz, const char *exceptMsg)
{
  if (read(i,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::read(int*): Premature End");
}

//---------------------------------------------------------------------------
/** Reads an array of 8-byte long long integers from the stream.
   \param l Pointer to the buffer that will receive the result.
   \param sz The number of long long integers to read.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw NullPointerException If <tt>l == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
   \throw StreamCorruptedException If there is an error.
*/

void DataReader::read(__int64 *l, int sz, const char *exceptMsg)
{
  if (read(l,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::read(long long*): Premature End");
}

//---------------------------------------------------------------------------
/** Reads an array of 4-byte float values from the stream.
   \param f Pointer to the buffer that will receive the result.
   \param sz The number of float values to read.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw NullPointerException If <tt>f == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
   \throw StreamCorruptedException If there is an error.
*/

void DataReader::read(float *f, int sz, const char *exceptMsg)
{
  if (read(f,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::read(float*): Premature End");
}

//---------------------------------------------------------------------------
/** Reads an array of 8-byte double values from the stream.
   \param d Pointer to the buffer that will receive the result.
   \param sz The number of double values to read.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw NullPointerException If <tt>d == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 1</tt>.
   \throw StreamCorruptedException If there is an error.
*/

void DataReader::read(double *d, int sz, const char *exceptMsg)
{
  if (read(d,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamCorruptedException("DataReader::read(double*): Premature End");
}

} // namespace Ino

//---------------------------------------------------------------------------
