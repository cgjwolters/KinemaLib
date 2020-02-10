//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Buffered Input Stream ---------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Reader.h"

#include <string.h>

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup stream_reader Stream Readers
 @{
*/

//---------------------------------------------------------------------------
/** \class BufferedReader
  Enhances performance by buffering the data that is read from another
  underlying \ref Ino::Reader Reader.

  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------
/** Constructor.
  \param reader The \ref Reader to read the buffered data from.
  \param bufCap The capacity of the databuffer to use.
  \param rep The optional \ref ProgressReporter.

  Data will be read from the \c Reader in chunks of \c bufCap bytes
  where possible.\n
  Use this class if only small amounts of bytes are read with each
  call to method \ref read(char *buf, int cap) "read()"
  to reduce overhead.
*/

BufferedReader::BufferedReader(Reader& reader, int bufCap,
                                                 ProgressReporter *rep)
: Reader(rep), rdr(reader),
  inputBuf(new char[bufCap > 1 ? bufCap : 1]), inputCap(bufCap),
  inputEnd(inputBuf), inputPtr(inputBuf)
{
  if (bufCap < 1) throw IllegalArgumentException("BufferedReader:bufCap < 1 ");
}

//---------------------------------------------------------------------------
/** Destructor
  The destructor does \b not close the underlying \ref Ino::Reader Reader.
*/

BufferedReader::~BufferedReader()
{
  if (inputBuf) delete[] (char *)inputBuf;
}

//---------------------------------------------------------------------------
/** End-of-file indicator.
  \return \c true if:
  \li The internal buffer is exhausted and the underlying reader returns
  an end-of-file condition.
  \li Method \ref isAborted() const "isAborted()" returns \c true.
*/

bool BufferedReader::isEof() const
{
  return isAborted() || (inputPtr >= inputEnd && rdr.isEof());
}

//---------------------------------------------------------------------------
/** Abort indicator.
  \return \c true if:
  \li The underlying Reader returns an abort condition.
  \li The optional \ref ProgressReporter returns an abort condition.

  \note
  Even if \c true is returned, further data may still be read unless there
  is another reason for failure.
*/

bool BufferedReader::isAborted() const
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

long BufferedReader::getErrorCode() const
{
  return rdr.getErrorCode();
}

//---------------------------------------------------------------------------
/** \fn bool BufferedReader::isBuffered() const
   Always returns \c true.
   \return \c true always.
*/

//---------------------------------------------------------------------------
/** Reads a block of data from this reader.

  \param buf The buffer to receive the data, must not be \c NULL and
  must have a capacity of at least \c cap bytes.
  \param cap The capacity of the buffer.
  \return The actual number of bytes read, or -1 if there was an error.
  \throw NullPointerException <tt>if buf == NULL</tt>
  \throw IllegalArgumentException <tt>if cap &lt; 1</tt>

  This method will read at most \c cap bytes of data. Less bytes may be
  returned however, particularly when end-of-file is reached while reading.\n
  \n
  This method will return -1, indicating an error if:
  \li Method isEof() would have returned \c true just before this method call.
  \li If the optional \ref ProgressReporter tells this stream to
  \ref isAborted() "abort".
*/

int BufferedReader::read(char *buf, int cap)
{
  if (!buf) throw NullPointerException("BufferedReader::read: buf == NULL");
  if (cap < 1) throw IllegalArgumentException("BufferedReader::read: cap < 1");

  if (isEof()) return -1;

  int initCap = cap;

  for (;;) {
    int bytesToCopy = inputEnd-inputPtr;
    if (cap < bytesToCopy) bytesToCopy = cap;

    if (bytesToCopy > 0) {
      memmove(buf,inputPtr,bytesToCopy);

      buf += bytesToCopy;
      cap -= bytesToCopy;

      inputPtr += bytesToCopy;
    }

    if (cap <= 0) {
      bytesRead += bytesToCopy;

      if (!reporter) return initCap; // Quick shortcut

      bytesInc += bytesToCopy;
      if (!reportProgress()) return -1;
 
      return initCap;
    }

    int byteCount = rdr.read(inputBuf,inputCap);

    if (byteCount > 0) {
      inputEnd = inputBuf + byteCount;
      inputPtr = inputBuf;
    }
    else { // Some problem
      if (rdr.isAborted()) return -1;

      if (initCap != cap) return initCap - cap; // Deliver data present first

      return -1;
    }
  }
}

} // namespace Ino

//---------------------------------------------------------------------------
