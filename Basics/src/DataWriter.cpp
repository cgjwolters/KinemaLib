//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Data Output Stream (Data in network byteorder) --------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Writer.h"

#include <cstring>

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup stream_writer Stream Writers
 @{
*/

//---------------------------------------------------------------------------
/** \class DataWriter
  Writes basic types (such as chars, ints, doubles) to a
  \ref Ino::Writer Writer.

  The data is always written in <em>network byte order</em>: the format
  of the data is hardware independent.\n
  \n
  The various write methods come in two flavors:
  \li Methods that return a boolean to indicate success or failure.
  \li Methods that return a \c void and throw an exception if there is a
  problem.

  \see DataReader
  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------

union DataWriterOverlay {
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
  DataWriterOverlay buf;

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

bool DataWriter::writeItem(char *buf, int len)
{
  if (isClosed()) return false;

  if (!bigEndian) reverse(buf,len);

  if (!wrt.write(buf,len)) return false;

  bytesWritten += len;
  bytesInc     += len;

  if (!reporter) return true;

  return reportProgress();
}

//---------------------------------------------------------------------------

bool DataWriter::writeDirect(const char *buf, int len)
{
  if (isClosed()) return false;

  if (!wrt.write(buf,len)) return false;

  bytesWritten += len;
  bytesInc     += len;

  if (!reporter) return true;

  return reportProgress();
}

//---------------------------------------------------------------------------

bool DataWriter::writeArray(const char *buf, int itemSz, int len)
{
  if (bigEndian || itemSz < 2) return writeDirect(buf,itemSz*len);

  char swapBuf[4096];

  while (len > 0) {
    int itemCnt = 512;
    if (len < itemCnt) itemCnt = len;

    int byteCnt = itemCnt*itemSz;

    memmove(swapBuf,buf,byteCnt);

    for (int i=0; i<itemCnt; ++i) reverse(swapBuf+i*itemSz,itemSz);

    if (!writeDirect(swapBuf,byteCnt)) return false;

    len -= itemCnt;
    buf += byteCnt;
  }

  return true;
}

//---------------------------------------------------------------------------
/** Constructor.
   \param writer The Writer to write the data to.
   \param rep An optional \ref ProgressReporter, may be \c NULL.
*/

DataWriter::DataWriter(Writer& writer, ProgressReporter *rep)
: Writer(rep), wrt(writer), bigEndian(isBigEndian()),
  dataBuf(new DataWriterOverlay)
{
}

//---------------------------------------------------------------------------
/** Destructor.
  Does \b not close the underlying \ref Writer.
*/

DataWriter::~DataWriter()
{
  if (dataBuf) delete dataBuf;
}

//---------------------------------------------------------------------------
/** Set Endianness of the output to Little Endian instead of the
    normal Big Endian.
*/

void DataWriter::writeLittleEndian()
{
  bigEndian = !isBigEndian();
}

//---------------------------------------------------------------------------
/**  File closed indicator.
  
  \return \c true If more data can be written, \c false if no more data can
  be written to the stream.
*/

bool DataWriter::isClosed() const
{
  return wrt.isClosed() || isAborted();
}

//---------------------------------------------------------------------------
/**  Abort indicator.

  Indicates whether further writing should be stopped.
  \return \c true if:
  \li Method isAborted() of the underlying output stream returns \c true.
  \li The optional \ref ProgressReporter returns an abort condition.

  \note
  Even if \c true is returned, further data may still be written unless there
  is another reason for failure.
*/

bool DataWriter::isAborted() const
{
  if (wrt.isAborted()) return true;

  if (reporter) return reporter->mustAbort();

  return false;
}

//---------------------------------------------------------------------------
/** \fn long DataWriter::getErrorCode() const
  Calls getErrorCode() of the underlying Writer.
  \return The error code of the underlying Writer.
*/

//---------------------------------------------------------------------------
/** \fn bool DataWriter::isBuffered() const
   Calls method isBuffered() of the underlying output stream.
   \return \c true if the underlying output stream is buffered,\n
   \c false otherwise.
*/

//---------------------------------------------------------------------------
/** Calls method flush() of the underlying output stream.
    \return \c true if successfull,\n
    \c false if:
    \li \ref isClosed() returns \c true.
    \li The flush() method of the underlying Writer returns \c false.
*/

bool DataWriter::flush()
{
  if (isClosed()) return false;

  return wrt.flush();
}

//---------------------------------------------------------------------------
/** Calculates the number of bytes required to represent an Unicode string
    in Utf8 format.

    Use this method to predict how many bytes will be written when
    method \ref writeUtf8(const wchar_t* wc, int sz) "writeUtf8()" is
    called with the same Unicode string\n
    Then first use method \ref writeInt(long i) "writeInt()" to write
    this value to the stream before actually calling writeUtf8().\n
    A DataReader can then later read the size of the Utf8 string that follows
    and subsequently call
    \ref DataReader::readUtf8(wchar_t* wc, int cap, int utfSz) "readUtf8()"
    with the proper \c utfSz argument.

    \param wc The Unicode string to use, must not be \c NULL.
    \param sz The length of the string.
    \return The amount of bytes method
    \ref writeUtf8(const wchar_t* wc, int sz) "writeUtf8()" would write
     to the output stream.
     \throw NullPointerException If <tt>wc == NULL</tt>.
     \throw IllegalArgumentException If <tt>sz < 0</tt>
*/

int DataWriter::calcUtf8Len(const wchar_t* wc, int sz)
{
  if (!wc) throw NullPointerException("DataWriter::calcUtf8Len: wc == NULL");
  if (sz < 0) throw IllegalArgumentException("DataWriter::calcUtf8Len: sz < 0");

  int utfLen = 0;

  for (int i=0; i<sz; ++i) {
    wchar_t c = wc[i];
    if ((c >= 0x0001) && (c <= 0x007F)) utfLen++;
    else if (c > 0x07FF) utfLen += 3;
    else                 utfLen += 2;
  }

  return utfLen;
}

//---------------------------------------------------------------------------
/** Writes a Unicode string in Utf8 format to the output stream.
   \param wc The Unicode string to write.
   \param sz The length of the string.
   \return \c true If the string was written,\n
   \c false if:
   \li This writer or the underlying writer is \ref isClosed() const "closed".
   \li There was an error while writing the Utf8 bytes.
   \li The optional ProgressReporter wants to abort.
   \throw NullPointerException If <tt>wc == NULL</tt>.
   \throw IllegalArgumentException <tt>sz < 0</tt>
*/

bool DataWriter::writeUtf8(const wchar_t* wc, int sz)
{
  if (!wc) throw NullPointerException("DataWriter::writeUtf8: wc == NULL");
  if (sz < 0) throw IllegalArgumentException("DataWriter::writeUtf8: sz < 0");

  if (isClosed()) return false;

  char utfBuf[512*3];

  while (sz > 0) {
    int upb = 512;
    if (upb > sz) upb = sz;

    int utfLen = 0;

    for (int i = 0; i < upb; ++i) {
      wchar_t c = wc[i];

      if ((c >= 0x0001) && (c <= 0x007F)) utfBuf[utfLen++] = (char)c;
      else if (c > 0x07FF) {
        utfBuf[utfLen++] = (char)(0xE0 | ((c >> 12) & 0x0F));
        utfBuf[utfLen++] = (char)(0x80 | ((c >>  6) & 0x3F));
        utfBuf[utfLen++] = (char)(0x80 | ((c >>  0) & 0x3F));
      }
      else {
        utfBuf[utfLen++] = (char)(0xC0 | ((c >>  6) & 0x1F));
        utfBuf[utfLen++] = (char)(0x80 | ((c >>  0) & 0x3F));
      }
    }

    wc += upb;
    sz -= upb;

    if (!writeDirect(utfBuf,utfLen)) return false;
  }

  return true;
}

//---------------------------------------------------------------------------
/** Writes a boolean value to the output stream.
   \param b the boolan value to write.
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
*/

bool DataWriter::writeBool(bool b)
{
  dataBuf->b = b;

  return writeItem(dataBuf->data,1);
}

//---------------------------------------------------------------------------
/** Writes a byte (char) to the output stream.
   \param c the byte to write.
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
*/

bool DataWriter::writeByte(char c)
{
  dataBuf->c = c;

  return writeItem(dataBuf->data,1);
}

//---------------------------------------------------------------------------
/** Writes a 2-byte Unicode character to the output stream.
   \param wc the Unicode character to write.
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
*/

bool DataWriter::writeWChar(wchar_t wc)
{
  dataBuf->wc = wc;

  return writeItem(dataBuf->data,2);
}

//---------------------------------------------------------------------------
/** Writes a 2-byte short integer to the output stream.
   \param s the short integer to write.
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
*/

bool DataWriter::writeShort(short s)
{
  dataBuf->s = s;

  return writeItem(dataBuf->data,2);
}

//---------------------------------------------------------------------------
/** Writes a 4-byte long integer to the output stream.
   \param i the long integer to write.
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
*/

bool DataWriter::writeInt(long i)
{
  dataBuf->i = i;

  return writeItem(dataBuf->data,4);
}

//---------------------------------------------------------------------------
/** Writes an 8-byte long long integer to the output stream.
   \param l the long long integer to write.
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
*/

bool DataWriter::writeLong(__int64 l)
{
  dataBuf->l = l;

  return writeItem(dataBuf->data,8);
}

//---------------------------------------------------------------------------
/** Writes a 4-byte float value to the output stream.
   \param f the float value to write.
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
*/

bool DataWriter::writeFloat(float f)
{
  dataBuf->f = f;

  return writeItem(dataBuf->data,4);
}

//---------------------------------------------------------------------------
/** Writes an 8-byte double value to the output stream.
   \param d the double value to write.
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
*/

bool DataWriter::writeDouble(double d)
{
  dataBuf->d = d;

  return writeItem(dataBuf->data,8);
}

//---------------------------------------------------------------------------
/** Writes an array of boolean values to the output stream.
   \param b The boolean array to write, must not be \c NULL.
   \param sz The length of the array.
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
   \throw NullPointerException If <tt>b == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 0</tt>.
*/

bool DataWriter::write(const bool *b, int sz)
{
  if (!b) throw NullPointerException("DataWriter::write(bool): b == NULL");
  if (sz < 0) throw IllegalArgumentException("DataWriter::write(bool): sz < 0");
  if (sz < 1) return true;

  return writeArray((char *)b,1,sz);
}

//---------------------------------------------------------------------------
/** Writes an array of bytes (characters) to the output stream.
   \param c The byte array to write, must not be \c NULL.
   \param sz The length of the array.
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
   \throw NullPointerException If <tt>c == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 0</tt>.
*/

bool DataWriter::write(const char *c, int sz)
{
  if (!c) throw NullPointerException("DataWriter::writeCh: b == NULL");
  if (sz < 0) throw IllegalArgumentException("DataWriter::writeCh: sz < 0");
  if (sz < 1) return true;

  return writeArray(c,1,sz);
}

//---------------------------------------------------------------------------
/** Writes an array of 2-byte Unicode characters to the output stream
   (no encoding).
   \param wc The Unicode character array to write, must not be \c NULL.
   \param sz The length of the array (in characters).
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
   \throw NullPointerException If <tt>wc == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 0</tt>.
*/

bool DataWriter::write(const wchar_t *wc, int sz)
{
  if (!wc) throw NullPointerException("DataWriter::write(wchar_t): wc == NULL");
  if (sz < 0) throw IllegalArgumentException("DataWriter::write(wchar_t): sz < 0");
  if (sz < 1) return true;

  return writeArray((char *)wc,2,sz);
}

//---------------------------------------------------------------------------
/** Writes an array of 2-byte short integers to the output stream.
   \param s The array to write, must not be \c NULL.
   \param sz The length of the array (in short integers).
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
   \throw NullPointerException If <tt>s == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 0</tt>.
*/

bool DataWriter::write(const short *s, int sz)
{
  if (!s) throw NullPointerException("DataWriter::write(short): s == NULL");
  if (sz < 0) throw IllegalArgumentException("DataWriter::write(short): sz < 0");
  if (sz < 1) return true;

  return writeArray((char *)s,2,sz);
}

//---------------------------------------------------------------------------
/** Writes an array of 4-byte long integers to the output stream.
   \param i The array to write, must not be \c NULL.
   \param sz The length of the array (in long integers).
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
   \throw NullPointerException If <tt>i == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 0</tt>.
*/

bool DataWriter::write(const long *i, int sz)
{
  if (!i) throw NullPointerException("DataWriter::write(long): i == NULL");
  if (sz < 0) throw IllegalArgumentException("DataWriter::write(long): sz < 0");
  if (sz < 1) return true;

  return writeArray((char *)i,4,sz);
}

//---------------------------------------------------------------------------
/** Writes an array of 8-byte long long integers to the output stream.
   \param l The array to write, must not be \c NULL.
   \param sz The length of the array (in long long integers).
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
   \throw NullPointerException If <tt>l == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 0</tt>.
*/

bool DataWriter::write(const __int64 *l, int sz)
{
  if (!l) throw NullPointerException("DataWriter::write(__int64): l == NULL");
  if (sz < 0) throw IllegalArgumentException("DataWriter::write(__int64): sz < 0");
  if (sz < 1) return true;

  return writeArray((char *)l,8,sz);
}

//---------------------------------------------------------------------------
/** Writes an array of 4-byte float values to the output stream.
   \param f The array to write, must not be \c NULL.
   \param sz The length of the array (in floats).
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
   \throw NullPointerException If <tt>f == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 0</tt>.
*/

bool DataWriter::write(const float *f, int sz)
{
  if (!f) throw NullPointerException("DataWriter::write(float): f == NULL");
  if (sz < 0) throw IllegalArgumentException("DataWriter::write(float): sz < 0");
  if (sz < 1) return true;

  return writeArray((char *)f,4,sz);
}

//---------------------------------------------------------------------------
/** Writes an array of 8-byte double values to the output stream.
   \param d The array to write, must not be \c NULL.
   \param sz The length of the array (in doubles).
   \return \c true if the operation was successfull,\n
   \c false if:
   \li This stream or the underlying stream is \ref isClosed() const "closed".
   \li There was an error while writing the output byte.
   \li The optional ProgressReporter wants to abort.
   \throw NullPointerException If <tt>d == NULL</tt>.
   \throw IllegalArgumentException If <tt>sz < 0</tt>.
*/

bool DataWriter::write(const double *d, int sz)
{
  if (!d) throw NullPointerException("DataWriter::write(double): d == NULL");
  if (sz < 0) throw IllegalArgumentException("DataWriter::write(double): sz < 0");
  if (sz < 1) return true;

  return writeArray((char *)d,8,sz);
}

//---------------------------------------------------------------------------
/** Writes a boolean value to the output stream.
   \param b the boolan value to write.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::writeBool(bool b, const char* exceptMsg)
{
  if (writeBool(b)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::writeBool");
}

//---------------------------------------------------------------------------
/** Writes a byte (character) to the output stream.
   \param c the byte value to write.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::writeByte(char c, const char* exceptMsg)
{
  if (writeByte(c)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::writeByte");
}

//---------------------------------------------------------------------------
/** Writes a 2-byte Unicode character to the output stream.
   \param wc the Unicode character to write.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::writeWChar(wchar_t wc, const char* exceptMsg)
{
  if (writeWChar(wc)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::writeWChar");
}

//---------------------------------------------------------------------------
/** Writes a 2-byte short integer to the output stream.
   \param s the short integer to write.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::writeShort(short s, const char* exceptMsg)
{
  if (writeShort(s)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::writeShort");
}

//---------------------------------------------------------------------------
/** Writes a 4-byte long integer to the output stream.
   \param i the long integer to write.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::writeInt(long i, const char* exceptMsg)
{
  if (writeInt(i)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::writeInt");
}

//---------------------------------------------------------------------------
/** Writes an 8-byte long long integer to the output stream.
   \param l the long long integer to write.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::writeLong(__int64 l, const char* exceptMsg)
{
  if (writeLong(l)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::writeLong");
}

//---------------------------------------------------------------------------
/** Writes an 4-byte float value to the output stream.
   \param f the float value to write.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::writeFloat(float f, const char* exceptMsg)
{
  if (writeFloat(f)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::writeFloat");
}

//---------------------------------------------------------------------------
/** Writes an 8-byte double value to the output stream.
   \param d the double value to write.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::writeDouble(double d, const char* exceptMsg)
{
  if (writeDouble(d)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::writeDouble");
}

//---------------------------------------------------------------------------
/** Writes an array of boolean values to the output stream.
   \param b The boolean array to write, must not be \c NULL.
   \param sz The length of the array.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::write(const bool *b, int sz, const char* exceptMsg)
{
  if (write(b,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::write(bool*)");
}

//---------------------------------------------------------------------------
/** Writes an array of bytes (characters) values to the output stream.
   \param c The byte array to write, must not be \c NULL.
   \param sz The length of the array.
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::write(const char *c, int sz, const char* exceptMsg)
{
  if (write(c,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::write(char*)");
}

//---------------------------------------------------------------------------
/** Writes an array of 2-byte Unicode characters values to the output stream
   (no encoding).
   \param wc The array to write, must not be \c NULL.
   \param sz The length of the array (in Unicode characters).
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::write(const wchar_t *wc, int sz, const char* exceptMsg)
{
  if (write(wc,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::write(wchar_t*)");
}

//---------------------------------------------------------------------------
/** Writes an array of 2-byte short integers to the output stream.
   \param s The array to write, must not be \c NULL.
   \param sz The length of the array (in shorts).
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::write(const short *s, int sz, const char* exceptMsg)
{
  if (write(s,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::write(short*)");
}

//---------------------------------------------------------------------------
/** Writes an array of 4-byte long integers to the output stream.
   \param i The array to write, must not be \c NULL.
   \param sz The length of the array (in long integers).
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::write(const long *i, int sz, const char* exceptMsg)
{
  if (write(i,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::write(int*)");
}

//---------------------------------------------------------------------------
/** Writes an array of 8-byte long long integers to the output stream.
   \param l The array to write, must not be \c NULL.
   \param sz The length of the array (in long long integers).
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::write(const __int64 *l, int sz, const char* exceptMsg)
{
  if (write(l,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::write(long long*)");
}

//---------------------------------------------------------------------------
/** Writes an array of 4-byte float values to the output stream.
   \param f The array to write, must not be \c NULL.
   \param sz The length of the array (in floats).
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::write(const float *f, int sz, const char* exceptMsg)
{
  if (write(f,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::write(float*)");
}

//---------------------------------------------------------------------------
/** Writes an array of 8-byte double values to the output stream.
   \param d The array to write, must not be \c NULL.
   \param sz The length of the array (in doubles).
   \param exceptMsg If not \c NULL should point to a user defined error message.\n
   The message will be passed to the \ref StreamCorruptedException that is
   thrown if there is an error.
   \throw StreamCorruptedException If there is an error and <tt>exceptMsg != NULL</tt>.
   \throw StreamClosedException If there is an error otherwise.
*/

void DataWriter::write(const double *d, int sz, const char* exceptMsg)
{
  if (write(d,sz)) return;

  if (exceptMsg) throw StreamCorruptedException(exceptMsg);
  else throw StreamClosedException("DataWriter::write(double*)");
}

} // namespace Ino

//---------------------------------------------------------------------------
