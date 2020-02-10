//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Input Stream from a byte array ------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Reader.h"

#include <string.h>
#include <malloc.h>

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup stream_reader Stream Readers
 @{
*/

//---------------------------------------------------------------------------
/** \class ByteArrayReader
  A ByteArrayReader reads its data from a character buffer.

  This class always considers itself to be the owner of the character buffer
  and so the destructor will delete the buffer.\n
  \n
  This class maintains a \ref getSize() const "size", indicating the amount
  of valid data present, and a \ref getPos() const "position", that defines
  the index of the next character to be read.\n
  There is no need for a trailing zero character.
  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------
/** Constructor.
  \param initCap The initial capacity of the character buffer to allocate.\n
  If less than one, the capacity is silently set to one.
*/

ByteArrayReader::ByteArrayReader(int initCap)
: Reader(0), data(new char[initCap >= 0 ? initCap : 1]),
  cap(initCap), pos(0), sz(0)
{
  if (initCap < 0) throw IllegalArgumentException("ByteArrayReader: initCap < 0");
}

//---------------------------------------------------------------------------
/** Destructor.
 
   Also deallocates the character buffer.
*/

ByteArrayReader::~ByteArrayReader()
{
  if (data) delete[] data;
}

//---------------------------------------------------------------------------
/** \fn bool ByteArrayReader::isEof() const
  End-of-file indicator.
  \return \c true if <tt>\ref getPos() const "getPos()" >= 
  \ref getSize() const "getSize()"</tt>, \c false otherwise.
*/

//---------------------------------------------------------------------------
/** \fn bool ByteArrayReader::isAborted() const
  Abort Indicator, always retuns \c false.
  \return \c false always.
*/

//---------------------------------------------------------------------------
/** \fn long ByteArrayReader::getErrorCode() const
  Always returns zero.
  \return Zero always.
*/

//---------------------------------------------------------------------------
/** \fn bool ByteArrayReader::isBuffered() const
  Always returns \c true.
  \return \c true always.
*/

//---------------------------------------------------------------------------
/** \fn int ByteArrayReader::getCap() const
  Returns the current capacity of the character buffer.
  \return The buffer capacity.
*/

//---------------------------------------------------------------------------
/** Sets the capacity of the character buffer.
  \param newCap The new capacity of the buffer.

  Standard function \c realloc() is used to reallocate the buffer,
  so data present in the buffer is preserved as much as possible.

  \note
  This method will always reallocate the buffer unless \c newCap
  happens to be equal to the current capacity.

  \throw OutOfMemoryException If the memory allocation failed.
*/

void ByteArrayReader::setCap(int newCap)
{
  if (cap == newCap) return;

  char *newData = (char *)realloc(data,newCap);
  if (!newData) throw OutOfMemoryException("ByteArrayReader::setCap");

  data = newData;
  cap  = newCap;

  if (pos > cap) pos = cap;
  if (sz > cap)  sz  = cap;
}

//---------------------------------------------------------------------------
/** Increases the capacity of the character buffer if necessary.

   \param newCap The minimum capacity of the buffer.\n
   This is a no-op if the capacity was already at least \c newCap bytes.
  \throw OutOfMemoryException If the memory allocation failed.
*/

void ByteArrayReader::ensureCap(int newCap)
{
  if (cap >= newCap) return;

  char *newData = (char *)realloc(data,newCap);
  if (!newData) throw OutOfMemoryException("ByteArrayReader::ensureCap");

  data = newData;
  cap  = newCap;

  if (pos > cap) pos = cap;
  if (sz > cap)  sz  = cap;
}

//---------------------------------------------------------------------------
/** \fn int ByteArrayReader::getPos() const
  Returns the current size read position in the character buffer.
  \return The index of the next byte to read.
*/

//---------------------------------------------------------------------------
/** Sets the read position in the character buffer.

   The position defines the index of the next character to be read.
   \param newPos The new read position.\n
   Can be greater then the current \ref getSize() const "size"
   of the buffer. In that case the size will be silently increased to
   \c newPos.
   \throw IllegalArgumentException if <tt>newPos < 0 ||
   newPos > \ref getCap() const "getCap()"</tt>.
*/

void ByteArrayReader::setPos(int newPos)
{
  if (newPos < 0 || newPos > cap)
                  throw IllegalArgumentException("ByteArrayReader::setPos");

  pos = newPos;

  if (sz < pos) sz = pos;
}

//---------------------------------------------------------------------------
/** \fn int ByteArrayReader::getSize() const
  Returns the current size of data in the character buffer.
  \return The current size of valid data.
*/

//---------------------------------------------------------------------------
/** Sets the size of (amount of valid data in) the character buffer.
  \param newSz The new size of the data in the buffer.\n
  If the current read \ref getPos() const "position" is greater than
  \c newSz then the read position is set to \c newSz (implying an
  \ref isEof() const "end-of-file" condition).
  \throw IllegalArgumentException if <tt>newSz < 0 ||
  newSz > \ref getCap() const "getCap()"</tt>.
*/

void ByteArrayReader::setSize(int newSz)
{
  if (newSz < 0 || newSz > cap)
                  throw IllegalArgumentException("ByteArrayReader::setSize");

  sz = newSz;

  if (pos > sz) pos = sz;
}

//---------------------------------------------------------------------------
/** Reads a block of data from the character buffer.

    \param buf The buffer to read the data into, must \b not be \c NULL and
    must have a capacity of at least \c bSz bytes.
    \param bSz The number of bytes to read.
    \return The actual number of bytes read, or -1 if there was an error.\n
    The number of bytes read may be less than \c bSz, and may in fact be zero
    (if \c bSz is zero or if the \ref getPos() const "read position" coincides
    with the buffers \ref getSize() const "size").
*/

int ByteArrayReader::read(char *buf, int bSz)
{
  if (!buf) throw NullPointerException("ByteArrayReader::read: buf == NULL");
  if (bSz < 0) throw IllegalArgumentException("ByteArrayReader::read: bSz < 0");

  if (bSz < 1) return 0;
  if (pos >= sz) return -1;

  if (pos+bSz > sz) bSz = sz-pos;

  memcpy(buf,data+pos,bSz);

  pos += bSz;

  return bSz;
}

//---------------------------------------------------------------------------
/** \fn char* &ByteArrayReader::getBuffer()
  Returns a <b> non-const reference</b> to the internal character buffer.
  \return a \b reference to the internal buffer.

  \attention <b>Reference returned.</b>\n
  Since this class may reallocate its internal buffer (when
  \ref setCap(int) "setCap()" or \ref ensureCap(int) "ensureCap()" is called),
  a pointer returned as a value may become invalid.\n
  \n
  For that reason this method returns a \b reference to the internal buffer.
  By initializing a \b reference pointer variable with the returned result,
  the buffer pointer will never become invalid.\n
  \n
  Example: (with reference)\n
  <tt>char * <b>&</b>bufPtr = rdr.getBuffer();\n
  rdr.setCap(5000); </tt>Ok: \c bufPtr still valid.\n
  \n
  Example 2: (pointer value)\n
  <tt>char *bufPtr2 = rdr.getBuffer();\n
  rdr.setCap(5000); </tt>Error: \c bufPtr2 is \b invalid after this call!
  
  \note
  Since a reference is returned it is possible to replace the buffer
  that is being used by one that you have allocated.\n
  Resist the temptation: this class does not expect it and
  disaster is bound to happen!
*/

} // namespace Ino

//---------------------------------------------------------------------------
