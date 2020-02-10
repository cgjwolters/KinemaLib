//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Generalized Abstract Output Stream --------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Writer.h"

#include "Basics.h"

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup stream_writer Stream Writers
 @{
*/

//---------------------------------------------------------------------------
/** \file Writer.h
    \brief Contains definitions of the <em>Writer</em> classes:
    
    \li Class \ref Ino::Writer A pure virtual class from which real writers are
    derived.
    \li Class \ref Ino::StdioWriter Writes data to a stdio \c (FILE) stream.
    \li Class \ref Ino::CompressedWriter can be used to write compressed or
    uncompresses data.
    This class uses zlib to compress the data to be written.
    \li Class \ref Ino::BufferedWriter Employs an internal buffer to minimize the
    number of actual (block) writes to the underlying \ref Ino::Writer.
    \li Class \ref Ino::DataWriter Writes native quantities such as numbers and
    strings to an underlying binary outputstream in a hardware independent
    format.
    \li Class Ino::ByteArrayWriter Writes to a byte array.

    \author C. Wolters
    \date Jul 2005
*/

//---------------------------------------------------------------------------
/** \class Writer
  Pure virtual base class for various derived stream writer classes.

  \see StdioWriter, CompressedWriter, BufferedWriter, DataWriter,
  ByteArrayWriter
  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------
/** Reset the "bytes written" count to zero.
*/

void Writer::resetBytesWritten()
{
  bytesWritten = 0;
  bytesInc     = 0;
}

//---------------------------------------------------------------------------
/** Derived classes call this method to inform the
    \ref Ino::ProgressReporter ProgressReporter if any.
*/

bool Writer::reportProgress()
{
  if (!reporter || bytesInc < reporter->getReportInc()) return true;

  bool ok = reporter->setProgress(bytesWritten);

  bytesInc = 0;

  return ok;
}

//---------------------------------------------------------------------------
/** Constructor.

  \param rep An optional progress reporter, may be \c NULL.\n
  If not \c NULL its \ref Ino::ProgressReporter::setProgress(int) "setProgress"
  method will be called regularly, while data is being written.
*/

Writer::Writer(ProgressReporter *rep)
: reporter(rep), bytesWritten(0), bytesInc(0)
{
}

//---------------------------------------------------------------------------
/** \var ProgressReporter *const Writer::reporter
  Holds a pointer to the optional ProgressReporter.
*/

//---------------------------------------------------------------------------
/** \var long Writer::bytesWritten
   Holds the number of bytes written sofar.
*/

//---------------------------------------------------------------------------
/** \var long Writer::bytesInc
  Holds the byte increment after which the optional
  \ref Ino::ProgressReporter ProgressReporter should be called.
*/

//---------------------------------------------------------------------------
/** \fn Writer::~Writer()
   Destructor.
*/

//---------------------------------------------------------------------------
/** \fn bool Writer::isClosed() const
  File closed indicator.
  
  \return \c true If more data can be written, \c false If no more data can
  be written to the stream.
*/

//---------------------------------------------------------------------------
/** \fn bool Writer::isAborted() const
  Abort indicator.

  A derived class can use this method to indicate a premature abort.\n
  It is up to the derived class to determine if more data can or cannot
  be written after this method has returned \c true.
  \return \c true if writing should be aborted.
*/

//---------------------------------------------------------------------------
/** \fn long Writer::getErrorCode() const
  Error status return.

  A derived class must implement this method and may use it to indicate
  an error or status.\n Zero should be returned if all is well.\n
  Often the code returned will be an operating system dependent value.

  \return An errorcode or zero if all is well.
*/

//---------------------------------------------------------------------------
/** \fn long Writer::getBytesWritten() const
   Returns the number of bytes written.

   The number returned is the number of bytes written since this Writer
   was constructed or since the last call to resetBytesWritten().
*/

//---------------------------------------------------------------------------
/** \fn bool Writer::isBuffered() const
  Indicates whether this writer or an underlying writer has an internal
  data buffer.

  \return \c true if the outputstream is buffered, \c false otherwise.
*/

//---------------------------------------------------------------------------
/** \fn bool Writer::write(const char *buf, int sz)
  Writes a block of data.

  \param buf The data to write, must not be \c NULL.
  \param sz The number of bytes to write.
  \return \c true if the bytes were written, or \c false if there was an error.
*/

//---------------------------------------------------------------------------
/** \fn bool Writer::flush()
    Flushes any buffered data to the ouput.

    A derived class is expected to flush its buffer(s), if any and then
    flush the underlying writer or operating system stream.
    \return \c true if successfull, \c false otherwise.
*/

//---------------------------------------------------------------------------
/** \fn bool Writer::close()
    This is a no-op, always returns \c true.

    Derived classes may or may not implement this method.
*/

} // namespace Ino

//---------------------------------------------------------------------------
