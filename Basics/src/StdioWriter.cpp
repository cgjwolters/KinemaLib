//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Stdio Output Stream -----------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Writer.h"

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup stream_writer Stream Writers
 @{
*/

//---------------------------------------------------------------------------
/** \class StdioWriter
  A \ref Ino::Writer Writer that writes to \c stdio (a \c FILE stream).

  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------
/** Constructor.
  \param file The file to write to. The file must already be open for writing.
  This parameter must not be \c NULL.
  \param closeOnDestroy If \c true the \c FILE will be closed by the
  destructor of this class.
  \param rep An optional \ref ProgressReporter, may be \c NULL.
  \throw NullPointerException If parameter \c file is \c NULL.
*/

StdioWriter::StdioWriter(FILE *file, bool closeOnDestroy,
                                                ProgressReporter *rep)
: Writer(rep), fd(file), sysErrorCode(0), autoClose(closeOnDestroy)
{
  if (!fd) throw NullPointerException("StdioWriter file == NULL");
}

//---------------------------------------------------------------------------
/** Destructor.
  Closes the \c stdio output stream if (and only if) parameter
  \c closeOnDestroy passed to the constructor was \c true.
*/

StdioWriter::~StdioWriter()
{
  if (autoClose && fd) fclose(fd);
}

//---------------------------------------------------------------------------
/** File closed indicator.
  \return \c true if:
  \li The file has already been closed.
  \li Method \ref isAborted() returns \c true.
*/

bool StdioWriter::isClosed() const
{
  return fd == NULL || isAborted();
}

//---------------------------------------------------------------------------
/** Abort Indicator.
  \return The abort status of the ProgressReporter, if there is one,\n
  or else \c false.
*/

bool StdioWriter::isAborted() const
{
  if (!reporter) return false;

  return reporter->mustAbort();
}

//---------------------------------------------------------------------------
/** \fn long StdioWriter::getErrorCode() const
  Returns the last \c stdio error code or zero if all is well.

  \return The last error code returned by \c ferror().
*/

//---------------------------------------------------------------------------
/** \fn bool StdioWriter::isBuffered() const
   \return Always \c true.
*/

//---------------------------------------------------------------------------
/** Writes a block of data.
  \param buf The data to write, must not be \c NULL.
  \param sz The number of bytes to write.
  \return \c true if all the bytes were written,\n
  \c false if:
  \li \ref isClosed() returns \c true.
  \li The \c stdio file returns an error,
  see \ref getErrorCode() const "getErrorCode()".
  \li The optional \ref ProgressReporter returns an abort condition.
  \throw NullPointerException If <tt>buf == NULL</tt>.
  \throw IllegalArgumentException If <tt>sz < 1</tt>.
*/

bool StdioWriter::write(const char *buf, int sz)
{
  if (!buf) throw NullPointerException("StdioWriter::write: buf == NULL");
  if (sz < 0) throw IllegalArgumentException("StdioWriter::write: sz < 1");

  if (isClosed()) return false;

  if (sz < 1) return true;

  int byteCount = fwrite(buf,1,sz,fd);

  if (byteCount < sz) {
    sysErrorCode = ferror(fd);

    return false;
  }

  bytesWritten += byteCount;
  bytesInc     += byteCount;

  return reportProgress();
}

//---------------------------------------------------------------------------
/** Flushes the data in this output stream.
    \return \c true if successfull,\n
    \c false if:
    \li \ref isClosed() returns \c true.
    \li The \c stdio file returns an error,
    see \ref getErrorCode() const "getErrorCode()".
*/

bool StdioWriter::flush()
{
  if (isClosed()) return false;

  if (fd && fflush(fd) == EOF) sysErrorCode = ferror(fd);

  return true;
}

//---------------------------------------------------------------------------
/** Closes the \c stdio output stream, if it wasn't closed already.

   Method \ref isClosed() const "isClosed()" will return \c true hereafter.
*/

bool StdioWriter::close()
{
  if (fd) fclose(fd);

  fd = NULL;

  return true;
}

} // namespace Ino

//---------------------------------------------------------------------------
