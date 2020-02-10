//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Buffered Output Stream --------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Writer.h"

#include <string.h>

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup stream_writer Stream Writers
 @{
*/

//---------------------------------------------------------------------------
/** \class BufferedWriter
  Enhances performance by buffering the data that is written to another
  underlying \ref Ino::Writer Writer.

  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------
/** Constructor.
  \param writer The \ref Writer to write the buffered data to.
  \param bufCap The capacity of the databuffer to use.
  \param rep The optional \ref ProgressReporter.

  Data will be written from the Writer in chunks of \c bufCap bytes
  where possible.\n
  Use this class if only small amounts of bytes are written with each
  call to method \ref write(const char *buf, int sz) "write()"
  to reduce overhead.
*/

BufferedWriter::BufferedWriter(Writer& writer, int bufCap,
                                                      ProgressReporter *rep)
: Writer(rep), wrt(writer),
  outputBuf(new char[bufCap > 1 ? bufCap : 1]),
  outputEnd(outputBuf), outputPtr(outputBuf)
{
  if (bufCap < 1) throw IllegalArgumentException("BufferedWriter: bufCap < 1");
}

//---------------------------------------------------------------------------
/** Destructor.

    Calls method \ref flush().
*/

BufferedWriter::~BufferedWriter()
{
  flush();

  if (outputBuf) delete[] (char *)outputBuf;
}

//---------------------------------------------------------------------------
/**  File closed indicator.
  
  \return \c true If more data can be written, \c false if no more data can
  be written to the stream.
*/

bool BufferedWriter::isClosed() const
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

bool BufferedWriter::isAborted() const
{
  if (wrt.isAborted()) return true;

  if (reporter) return reporter->mustAbort();

  return false;
}

//---------------------------------------------------------------------------
/** \fn bool BufferedWriter::isBuffered() const
   \return Always \c true.
*/

//---------------------------------------------------------------------------
/** \fn long ByteArrayWriter::getErrorCode() const
  Currently always returns zero.
  \return Zero always.
  \bug Should return the error code of the underlying stream.
*/

//---------------------------------------------------------------------------
/** Writes a block of data.
  \param buf The data to write, must not be \c NULL.
  \param sz The number of bytes to write.
  \return \c true if the bytes were written,\n
  or \c false if \ref isClosed() returns \c true.
  \throw NullPointerException If <tt>buf == NULL</tt>.
  \throw IllegalArgumentException If <tt>sz < 1</tt>.
*/

bool BufferedWriter::write(const char *buf, int sz)
{
  if (!buf) throw NullPointerException("BufferedWriter::write: buf == NULL");
  if (sz < 1) throw IllegalArgumentException("BufferedWriter::write: sz < 1");

  if (isClosed()) return false;

  int initSz = sz;

  for (;;) {
    int bytesToCopy = sz;
    if (outputEnd - outputPtr < sz) sz = outputEnd - outputPtr;

    if (bytesToCopy > 0) {
      memmove(outputPtr,buf,bytesToCopy);

      buf += bytesToCopy;
      sz  -= bytesToCopy;

      outputPtr += bytesToCopy;
    }

    if (sz < 1) {
      bytesWritten += initSz;

      if (!reporter) return true; // Quick shortcut

      bytesInc +=  initSz;

      return reportProgress();
    }

    if (!flush()) return false;
  }
}

//---------------------------------------------------------------------------
/** Flushes the data in this output stream.
    First the internally buffered data is written.\n
    Then method flush() of the underlying Writer is called.
    \return \c true if successfull,\n
    \c false if:
    \li \ref isClosed() returns \c true.
    \li The \c flush() method of the underlying Writer returns \c false.
*/

bool BufferedWriter::flush()
{
  if (isClosed()) return false;

  if (outputPtr == outputBuf) return wrt.flush();

  if (!wrt.write(outputBuf,outputPtr-outputBuf)) return false;

  outputPtr = outputBuf;

  return wrt.flush();
}

} // namespace Ino

//---------------------------------------------------------------------------

