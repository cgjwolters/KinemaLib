//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Generalized Abstract Input Stream ---------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Reader.h"

#include "Basics.h"


namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup stream_reader Stream Readers
 @{
*/

//---------------------------------------------------------------------------
/** \file Reader.h
    \brief Contains definitions of the <em>Reader</em> classes:
    
    \li Class \ref Ino::Reader A pure virtual class from which real readers are
    derived.
    \li Class \ref Ino::StdioReader Reads data from a stdio \c (FILE) stream.
    \li Class \ref Ino::CompressedReader can be used to read back compressed data that
    was written earlier with an instance of class \ref Ino::CompressedWriter.
    This class uses zlib to decompress the data read.
    \li Class \ref Ino::BufferedReader Employs an internal buffer to minimize the
    number of actual (block) reads from the underlying \ref Ino::Reader.
    \li Class \ref Ino::DataReader Reads native quantities such as numbers and
    strings from an underlying binary inputstream, that were written by
    a \ref Ino::DataWriter in a hardware independent format.
    \li Class Ino::ByteArrayReader Reads from a byte array.

    \author C. Wolters
    \date Jul 2005
*/

//---------------------------------------------------------------------------
/** \class Reader
  Pure virtual base class for various derived stream reader classes.

  \see StdioReader, CompressedReader, BufferedReader, DataReader,
  ByteArrayReader
  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------
/** \var ProgressReporter *const Reader::reporter
  Holds a pointer to the optional ProgressReporter.
*/

//---------------------------------------------------------------------------
/** \var long Reader::bytesRead
   Holds the number of bytes read sofar.
*/

//---------------------------------------------------------------------------
/** \var long Reader::bytesInc
  Holds the byte increment after which the optional
  \ref Ino::ProgressReporter should be called.
*/

//---------------------------------------------------------------------------
/** \fn Reader::~Reader()
   Destructor.
*/

//---------------------------------------------------------------------------
/** \fn bool Reader::isEof() const
  End-of-file indicator.
  
  \return \c false If more data can be read, \c true If no more data is
  available to be read.
*/

//---------------------------------------------------------------------------
/** \fn bool Reader::isAborted() const
  Abort indicator.

  A derived class can use this method to indicate a premature abort.\n
  It is up to the derived class to determine if more data can or cannot
  be read after this method has returned \c true.
  \return \c true if reading should be aborted.
*/

//---------------------------------------------------------------------------
/** \fn long Reader::getErrorCode() const
  Error status return.

  A derived class must implement this method and may use it to indicate
  an error or status.\n Zero should be returned if all is well.\n
  Often the code returned will be an operating system dependent value.

  \return An errorcode or zero if all is well.
*/

//---------------------------------------------------------------------------
/** \fn bool Reader::isBuffered() const
  Indicates whether this reader or an underlying reader has an internal
  data buffer.

  \return \c true if the inputstream is buffered, \c false otherwise.
*/

//---------------------------------------------------------------------------
/** Derived classes call this method to inform the \ref Ino::ProgressReporter
  if any.
*/

bool Reader::reportProgress()
{
  if (!reporter || bytesInc < reporter->getReportInc()) return true;

  return reporter->setProgress(bytesRead);
}

//---------------------------------------------------------------------------
/** Constructor.

  \param rep An optional progress reporter, may be \c NULL.\n
  If not \c NULL its \ref ProgressReporter::setProgress(int) setProgress
  method will be called regularly, while data is being read.
*/

Reader::Reader(ProgressReporter *rep)
: reporter(rep), bytesRead(0), bytesInc(0)
{
}

//---------------------------------------------------------------------------
/** \fn int Reader::getBytesRead() const
  Returnes the number of bytes read so far.
*/

//---------------------------------------------------------------------------
/** \fn int Reader::read(char *buf, int cap)
  Reads a block of data from this reader.

  \param buf The buffer to receive the data, must not be \c NULL.
  \param cap The capacity of the buffer.
  \return The actual number of bytes read, or -1 if there was an error.
*/

} // namespace Ino

//---------------------------------------------------------------------------
