//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Generalized Decompressing Input Stream ----------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Reader.h"

#include "zlib.h"

#include <string.h>

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup stream_reader Stream Readers
 @{
*/

//---------------------------------------------------------------------------
/** \class CompressedReader
    A CompressedReader can read a compressed or uncompressed datastream.
    
    It can be used to read back data that was stored earlier
    with an instance of class \ref CompressedWriter.\n
    \n
    The decompression algorithm employed is zlib, see
    <a href="http://www.zlib.org"> the zlib site</a>.

    \attention  Because data compression can be
    \ref CompressedWriter::setCompressing(bool) "switched on and off" while
    writing, a reader must
    \ref setDecompressing(bool) "switch" at exactly the same point(s).
    \author C. Wolters
    \date Jul 2005
*/

//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------
/** Reads more data into the inputbuffer of the decompressor.

    Called by method pump() below, this method will try to completely fill
    the input buffer of the decompressor. It does this by calling method
    read() one or more times.\n
    This method will return immediately if the return value of read()
    is zero or negative.
*/

bool CompressedReader::readData()
{
  if (isEof()) return false;

  if (zStr->avail_in < (unsigned int)bufSz) {
    int ret = rdr.read((char*)inBuf + zStr->avail_in,bufSz - zStr->avail_in);

    if (ret <= 0) return false;

    zStr->avail_in += ret;
  }

  return true;
}

//---------------------------------------------------------------------------

bool CompressedReader::pumpPlain()
{
  if (zStr->avail_in > 0) return true;

  zStr->next_in  = inBuf;

  if (!readData()) {
    eob = true;
    return false;
  }

  return true;
}

//---------------------------------------------------------------------------
/*! Reads and decompresses more data when and as required.

    This method is called by the read() method if more data is
    required.\n
    It will repeatedly call the decompressor and method readData() above
    until the buffer is completely filled with uncompressed data or until
    there is no more data to decompress.
*/

bool CompressedReader::pump()
{
  eob = false; // Start with next block

  zStr->next_out  = outBuf;
  zStr->avail_out = bufSz;
  outp = outBuf;

  for (;;) {
    if (zStr->avail_in < 1) {
      zStr->next_in  = inBuf;

      if (!readData()) return false;
    }

    if (zStr->avail_out < 1) break;

    int ret = inflate(zStr,0);

    if (ret == Z_STREAM_END) {
      inflateReset(zStr);
      eob = true;
      decompressing = mustDecompress;
      return true;
    }
    else if (ret != Z_BUF_ERROR && ret != Z_OK) return false;
  }

  // Try once more to detect end-of-stream in time

  int ret = inflate(zStr,0);

  if (ret == Z_STREAM_END) {
    inflateReset(zStr);
    eob = true;
    decompressing = mustDecompress;
  }
  else if (ret != Z_BUF_ERROR && ret != Z_OK) return false;

  return true;
}

//---------------------------------------------------------------------------
/** Constructor.
    \param reader The underlying Reader to read data from.
    \param bufCap The size of the input and output buffers to be used by
    the decompressor. If a value < 128 is supplied, the buffer size will be
    silently set to 128.
    \param rep The optional \ref ProgressReporter, may be \c NULL.
*/

CompressedReader::CompressedReader(Reader& reader, int bufCap,
                                                  ProgressReporter *rep)
: Reader(rep), rdr(reader),
  decompressing(true), mustDecompress(true), 
  zStr(0), inBuf(0), outBuf(0), outp(0), eob(true)
{
  if (bufCap < 128) bufCap = 128;

  inBuf  = new unsigned char[bufCap];
  outBuf = new unsigned char[bufCap];

  bufSz = bufCap;

  outp   = outBuf;

  zStr = (z_streamp)new char[sizeof(z_stream)];

  zStr->next_in  = inBuf;
  zStr->avail_in = 0;

  zStr->next_out  = outBuf;
  zStr->avail_out = 0;

  zStr->zalloc = Z_NULL;
  zStr->zfree  = Z_NULL;
  zStr->opaque = NULL;

  inflateInit(zStr);
}

//---------------------------------------------------------------------------
/** Reinitializes this input stream at the current point.
  \param newDecompress If \c true the start of a compressed stream (as
  written by a \ref CompressedWriter) is expected.\n
  If \c false data is read as is.
*/

void CompressedReader::reInit(bool newDecompress)
{
  mustDecompress = newDecompress;
  decompressing  = mustDecompress;

  eob = true;

  zStr->next_in  = inBuf;
  zStr->avail_in = 0;

  outp = outBuf;

  zStr->next_out  = outBuf;
  zStr->avail_out = 0;

  inflateReset(zStr);
}

//---------------------------------------------------------------------------
/** Destructor, closes the decompressor and deallocates buffers as required.

    The underlying Reader (as supplied to the
    \ref CompressedReader(Reader& reader, int bufCap, ProgressReporter *rep)
    "constructor" is \b not closed.
*/

CompressedReader::~CompressedReader()
{
  inflateEnd(zStr);

  if (inBuf)  delete[] inBuf;
  if (outBuf) delete[] outBuf;
  if (zStr)   delete[] (char *)zStr;
}

//---------------------------------------------------------------------------
/** End-of-file indicator.
  \return \c true if:
  \li isAborted() would return \c true.
  \li The input buffer is empty and the underlying Reader reports
  an end-of-file condition.
*/

bool CompressedReader::isEof() const
{
  if (isAborted()) return true;

  if (decompressing) {
    if (zStr->next_out - outp > 0 ) return false;
  }

  bool ef = zStr->avail_in < 1 && rdr.isEof();
  return ef;
}

//---------------------------------------------------------------------------
/** Abort Indicator.
  \return The abort status of the underlying reader or else, if there is
  a ProgressReporter, the abort status of the ProgressReporter.

  \note
  Even if \c true is returned, further data may still be read unless there
  is another reason for failure.
*/

bool CompressedReader::isAborted() const
{
  if (rdr.isAborted()) return true;

  if (reporter) return reporter->mustAbort();

  return false;
}

//---------------------------------------------------------------------------
/** Error code indicator.
  \return The (system) error code of the underlying reader.
*/

long CompressedReader::getErrorCode() const
{
  return rdr.getErrorCode();
}

//---------------------------------------------------------------------------
/** Sets the decompression mode.
  \param newDecompress The new decompression status.

  \note
  If reader is currently decompressing and \c newDecompress is \c false,
  then decompression will be switched off as soon as the end of the
  current decompression block has been reached.\n
  If the reader is not currently decompressing and \c newDecompress is
  \c true then decompression will start immediately.
*/

void CompressedReader::setDecompressing(bool newDecompress)
{
  mustDecompress = newDecompress;

  if (eob) decompressing = mustDecompress;
  else if (mustDecompress) decompressing = true;
}

//---------------------------------------------------------------------------
/** End-Of-Block indicator.

    \return \c true if the end of a compression block has been
    reached (while in \ref getDecompressing() const "compressed" mode).

    An end-of-block is written by class ComrpessedWriter when its
    \ref CompressedWriter::flush() "flush" method is called.
*/

bool CompressedReader::atEob() const
{
  return outp >= zStr->next_out && eob;
}

//---------------------------------------------------------------------------
/** \fn bool CompressedReader::getDecompressing() const
  Returns whether the input stream is currently being decompressed.
  \returns The decompression status.
*/

//---------------------------------------------------------------------------
/** \fn bool CompressedReader::isBuffered() const
  This method always returns \c true.
  \return \c true always.
*/

//---------------------------------------------------------------------------
/** Reads a block of data.
    \param v The buffer to read the data into, must NOT be NULL and
    must have a capacity of at least \c len bytes.
    \param len The number of bytes to read.
    \return The actual number of bytes read, or -1 if there was an error.\n

    This method will return -1, indicating an error if:
    \li Method isEof() would have returned \c true just before this method call.
    \li If the optional \ref ProgressReporter tells this stream to
    \ref isAborted() "abort".
*/

int CompressedReader::read(char *v, int len)
{
  if (!v) throw NullPointerException("CompressedReader::read: v == NULL");
  if (len <= 0) throw IllegalArgumentException("CompressedReader::read: len <= 0");

  if (isEof()) return -1;

  int initLen = len;

  if (decompressing) {
    while (len) {
      int avail = zStr->next_out - outp;

      if (avail < 1) {
        if (!pump()) return -1;

        continue;
      }

      if (avail > len) avail = len;
      memcpy(v,outp,avail);

      outp += avail;
      v    += avail;
      len  -= avail;
    }
  }
  else {
    while (len) {
      if (zStr->avail_in < 1) {
        if (!pumpPlain()) return -1;

        continue;
      }

      int avail = zStr->avail_in;
      if (avail > len) avail = len;
      memcpy(v,zStr->next_in,avail);

      zStr->next_in  += avail;
      zStr->avail_in -= avail;

      v += avail;
      len -= avail;
    }
  }

  int byteCount = initLen - len;
  if (byteCount < 1) return -1;

  bytesRead += byteCount;

  if (!reporter) return byteCount;

  bytesInc += byteCount;

  if (!reportProgress()) return -1;

  return byteCount;
}

} // namespace Ino

//---------------------------------------------------------------------------
