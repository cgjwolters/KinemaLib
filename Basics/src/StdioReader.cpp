//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Stdio Input Stream ------------------------------------------------
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
/** \class StdioReader
  A \ref Ino::Reader Reader that reads from \c stdio (a \c FILE stream).

  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------
/** Constructor.
  \param file The file to read from. The file must already be open.
  This parameter must not be \c NULL.
  \param closeOnDestroy If \c true the \c FILE will be closed by the
  destructor of this class.
  \param rep An optional \ref ProgressReporter, may be \c NULL.
  \throw NullPointerException If parameter \c file is \c NULL.
*/

StdioReader::StdioReader(FILE *file, bool closeOnDestroy,
                                                 ProgressReporter *rep)
: Reader(rep), fd(file), sysErrorCode(0), autoClose(closeOnDestroy)
{
  if (!fd) throw NullPointerException("StdioReader file == NULL");
}

//---------------------------------------------------------------------------
/** Destructor.
  The \c FILE supplied to the constructor will be closed, if it was
  not already.
*/

StdioReader::~StdioReader()
{
  if (autoClose && fd) fclose(fd);
}

//---------------------------------------------------------------------------
/** End-of-file indicator.
  \return \c true if:
  \li The file has already been closed.
  \li Method \ref isAborted() return \c true.
  \li \c feof() or \c ferror() on the underlying stdio stream return
  a non-zero value.
*/

bool StdioReader::isEof() const
{
  if (!fd || isAborted()) return true;

  return feof(fd) || ferror(fd);
}

//---------------------------------------------------------------------------
/** Abort indicator.
   This method delegates the decision to the optional
   \ref ProgressReporter.
   \return \c false if there is no ProgressReporter or else the result
   of a call to \ref ProgressReporter::mustAbort.
*/

bool StdioReader::isAborted() const
{
  if (!reporter) return false;

  return reporter->mustAbort();
}

//---------------------------------------------------------------------------
/** Returns the latest system error code regarding this reader.
  \return Zero if all is ok, or else the latest value that was
  return by a call to \c ferror().
*/

long StdioReader::getErrorCode() const
{
  return sysErrorCode;
}

//---------------------------------------------------------------------------
/** Closed indication.
  \return \c true if this input stream is closed.

  If \c true is returned no further reading is possible.
*/

bool StdioReader::isClosed() const
{
  return fd == 0;
}

//---------------------------------------------------------------------------
/** \fn bool StdioReader::isBuffered() const
  \return \c true always.
*/

//---------------------------------------------------------------------------
/** Reads a block of data from this reader.
  \param buf The buffer to receive the data, must not be \c NULL.
  \param cap The capacity of the buffer (in bytes).
  \return The actual number of bytes read or -1 if there was an error.
  \throw NullPointerException <tt>if buf == NULL</tt>
  \throw IllegalArgumentException <tt>if cap &lt; 1</tt>

  This method will read at most \c cap bytes of data. Less bytes may be
  returned however, particularly when end-of-file is reached while reading.\n
  \n
  This method will return -1, indicating an error if:
  \li If this stream is \ref isClosed() "closed".
  \li Method isEof() would have returned \c true just before this method call.
  \li If the underlying call to \c fread() returns an error.
  \li If the optional \ref ProgressReporter tells this stream to
  \ref isAborted() "abort".
*/

int StdioReader::read(char *buf, int cap)
{
  if (!buf) throw NullPointerException("StdioReader::read: buf == NULL");
  if (cap < 1) throw IllegalArgumentException("StdioReader::read: cap < 1");

  if (isEof()) return -1;

  long byteCount = fread(buf,1,cap,fd);

  if (byteCount <= 0) {
    sysErrorCode = ferror(fd);

    return -1;
  }

  bytesRead += byteCount;

  if (!reporter) return byteCount; // Prevent unnecessary call of reportProgress

  bytesInc += byteCount;

  if (!reportProgress()) return -1;

  return byteCount;
}

//---------------------------------------------------------------------------
/** Closes this stream (once and for all).
*/

void StdioReader::close()
{
  if (fd) fclose(fd);

  fd = 0;
}

} // namespace Ino

//---------------------------------------------------------------------------
