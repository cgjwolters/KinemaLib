//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Generalized Compressing Output Stream -----------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Writer.h"

#include "zlib.h"

#include <string.h>

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup stream_writer Stream Writers
 @{
*/

//---------------------------------------------------------------------------
/*! \class CompressedWriter
    \brief CompressedWriter writes data in compressed or uncompressed format
    to a stream.
    
    The data written can be read back with an instance of
    class \ref CompressedReader.

    \attention <b>Compression Mode</b>\n
    Compression can be \ref setCompressing(bool newCompress) "on or off"
    while writing.\n
    A subsequent \ref CompressedReader must
    \ref CompressedReader::setDecompressing(bool newDecompress) "switch"
    at exactly the same point when reading the data back.
    \n
    The compression algorithm employed is zlib, see
    <a href="http://www.zlib.org"> the zlib site</a>.

    \author C. Wolters
    \date Feb 2005
*/

//---------------------------------------------------------------------------
/**
 @}
*/

bool CompressedWriter::writeData(int len)
{
  if (!wrt.write((char *)outBuf,len)) return false;

  bytesWritten += len;

  return true;
}

//---------------------------------------------------------------------------

bool CompressedWriter::pumpPlain()
{
  if (!writeData(bufSz - zStr->avail_out)) return false;

  zStr->avail_out = bufSz;
  zStr->next_out  = outBuf;

  return true;
}

//---------------------------------------------------------------------------

bool CompressedWriter::pump()
{
  for (;;) {
    if (zStr->avail_out < 1) {
      if (!writeData(bufSz)) return false;

      zStr->avail_out = bufSz;
      zStr->next_out  = outBuf;
    }

    if (zStr->avail_in < 1) {
      zStr->next_in = inBuf;
      return true;
    }

    int ret = deflate(zStr,0);
    if (ret == Z_STREAM_ERROR) return false;
  }
}

//---------------------------------------------------------------------------
/** Constructor.
    \param writer The underlying Writer to write (compressed) data to.
    \param bufCap The size of the input and output buffers to be used by
    the compressor. If a value < 128 is supplied, the buffer size will be
    silently set to 128.
    \param rep The optional \ref ProgressReporter, may be \c NULL.

    Data compression is initially switched \b on.
*/

CompressedWriter::CompressedWriter(Writer& writer, int bufCap,
                                                    ProgressReporter *rep)
: Writer(rep), wrt(writer),
  zStr(0), inBuf(0), outBuf(0),
  compressing(true), flushed(true), rawBytesWritten(0)
{
  if (bufCap < 128) bufCap = 128;

  inBuf  = new unsigned char[bufCap];
  outBuf = new unsigned char[bufCap];

  bufSz = bufCap;

  zStr = (z_streamp)new char[sizeof(z_stream)];

  zStr->next_in  = inBuf;
  zStr->avail_in = 0;

  zStr->next_out  = outBuf;
  zStr->avail_out = bufSz;

  zStr->zalloc = Z_NULL;
  zStr->zfree  = Z_NULL;
  zStr->opaque = NULL;

  deflateInit(zStr,Z_DEFAULT_COMPRESSION);
}

//---------------------------------------------------------------------------
/** Destructor.

  Any pending data in the compression buffers will be flushed and then
  the underlying Writer will be flushed as well.\n
  The underlying Writer will \b not be closed. 
*/

CompressedWriter::~CompressedWriter()
{
  if (!flushed) flush();

  deflateEnd(zStr);

  if (inBuf)  delete[] inBuf;
  if (outBuf) delete[] outBuf;
  if (zStr)   delete[] (char *)zStr;
}

//---------------------------------------------------------------------------
/** \fn bool CompressedWriter::isCompressing() const
  Returns the current compression mode.

  \return \c true If the stream is currently compressing,\n
  \c false otherwise.
*/

//---------------------------------------------------------------------------
/** Sets the compression mode.
   \param newCompress \c true to turn compression on false to turn it off.

   When switching from compressed to uncompressed mode method \ref flush()
   will be called first.
*/
void CompressedWriter::setCompressing(bool newCompress)
{
  if (newCompress == compressing) return;

  flush();

  compressing = newCompress;
}

//---------------------------------------------------------------------------
/** \fn long CompressedWriter::getRawBytesWritten() const
    Returns the number of raw (i.e. \b uncompressed) bytes written.

    The number returned is the number of raw bytes written since this Writer
    was constructed or since the last call to resetBytesWritten().
*/

//---------------------------------------------------------------------------
/** Resets the count for the number of bytes written so far.

    This method resets the count for both getRawBytesWritten() and
    getBytesWritten().
*/

void CompressedWriter::resetBytesWritten()
{
  rawBytesWritten = 0;
  Writer::resetBytesWritten();
}

//---------------------------------------------------------------------------

bool CompressedWriter::reportProgress()
{
  if (!reporter || bytesInc < reporter->getReportInc()) return true;

  bool ok = reporter->setProgress(rawBytesWritten);

  bytesInc = 0;

  return ok;
}

//---------------------------------------------------------------------------
/** File closed indicator.
  \return \c true if:
  \li The underlying Writer reports a closed condition.
  \li isAborted() would return \c true.
*/

bool CompressedWriter::isClosed() const
{
  return wrt.isClosed() || isAborted();
}

//---------------------------------------------------------------------------
/** Abort Indicator.
  \return The abort status of the underlying writer or else, if there is
  a ProgressReporter, the abort status of the ProgressReporter.

  \note
  Even if \c true is returned, further data may still be written unless there
  is another reason for failure.
*/

bool CompressedWriter::isAborted() const
{
  if (wrt.isAborted()) return true;

  if (reporter) return reporter->mustAbort();

  return false;
}

//---------------------------------------------------------------------------
/** \fn CompressedWriter::getErrorCode() const
    Error code indicator.
    \return The (system) error code of the underlying writer.
*/

//---------------------------------------------------------------------------
/** \fn bool CompressedWriter::isBuffered() const
   \return Always \c true.
*/

//---------------------------------------------------------------------------
/** Flushes the data in this output stream.
    If in compression mode the compression engine will compress any pending
    data and flush its output buffer.\n
    The method flush() of the underlying Writer will be called.
    \return \c true if successfull,\n
    \c false if:
    \li \ref isClosed() returns \c true.
    \li The compression engine reports an error.
    \li The flush() method of the underlying Writer returns \c false.
*/

bool CompressedWriter::flush()
{
  if (isClosed()) return false;

  if (flushed) return true;

  if (compressing) {
    if (!pump()) return false;

    // Input buffer is now empty;

    for (;;) {
      if (zStr->avail_out < 1) {
        if (!writeData(bufSz)) return false;

        zStr->avail_out = bufSz;
        zStr->next_out  = outBuf;
      }

      int ret = deflate(zStr,Z_FINISH);

      if (ret == Z_STREAM_END) {
        deflateReset(zStr);
        break;
      }
      else if (ret == Z_STREAM_ERROR) return false;
    }
  }

  if (!writeData(bufSz-zStr->avail_out)) return false;

  zStr->avail_out = bufSz;
  zStr->next_out  = outBuf;

  flushed = true;

  return wrt.flush();
}

//---------------------------------------------------------------------------
/** Writes (and optionally
  \link Ino::CompressedWriter::setCompressing(bool) compresses\endlink) a
  block of data.
  \param v The data to write, must not be \c NULL.
  \param len The number of bytes to write.
  \return \c true if <tt>len == 0</tt> of if all the bytes were written,\n
  \c false if:
  \li \ref isClosed() returns \c true.
  \li The compression engine reports an error.
  \throw NullPointerException If <tt>v == NULL</tt>.
  \throw IllegalArgumentException If <tt>len < 0</tt>.
*/

bool CompressedWriter::write(const char *v, int len)
{
  if (!v) throw NullPointerException("CompressedWriter::write: v == NULL");
  if (len < 0) throw IllegalArgumentException("CompressedWriter::write: len < 0");

  if (isClosed()) return false;

  if (len < 1) return true;

  int orgLen = len;

  flushed = false;

  if (compressing) {
    while (len) {
      int space = bufSz - zStr->avail_in;

      if (space < 1) {
        if (!pump()) return false;

        continue;
      }

      if (space > len) space = len;

      memcpy(inBuf+zStr->avail_in,v,space);

      zStr->avail_in += space;

      v   += space;
      len -= space;
    }
  }
  else {
    while (len) {
      if (zStr->avail_out < 1 && !pumpPlain()) return false;

      int space = zStr->avail_out;

      if (space > len) space = len;

      memcpy(zStr->next_out,v,space);

      zStr->avail_out -= space;
      zStr->next_out  += space;

      v   += space;
      len -= space;
    }
  }

  rawBytesWritten += orgLen;
  bytesInc        += orgLen;

  bool ok = reportProgress();
  if (!ok) flush();

  return ok;
}

} // namespace Ino

//---------------------------------------------------------------------------

