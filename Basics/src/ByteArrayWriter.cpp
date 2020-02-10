//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Output Stream to a byte array -------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Writer.h"

#include <cmath>

#include <string.h>
#include <malloc.h>

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup stream_writer Stream Writers
 @{
*/

//---------------------------------------------------------------------------
/** \class ByteArrayWriter
  A ByteArrayWriter writes its data to a character buffer.

  This class always considers itself to be the owner of the character buffer
  and so the destructor will delete the buffer.\n
  Whenever required, the \ref getCap() const "capacity" of the buffer will
  automatically be increased.
  \n
  This class maintains a \ref getSize() const "size", indicating the amount
  of valid data present, and a \ref getPos() const "position", that defines
  the position at which next data will be written.\n
  A trailing zero character will always be automatically appended.

  \author C. Wolters
  \date June 2005
*/
//---------------------------------------------------------------------------
/**
 @}
*/

//---------------------------------------------------------------------------
/** Constructor.
  \param initCap The initial capacity of the byte array, must not be negative.
  \param incCap The capacity by which the byte array will be increased
  whenever more capacity is needed.\n
  \bug \c incCap is not used in the current implementation!
  \throw IllegalArgumentException If <tt>initCap < 0</tt>
*/

ByteArrayWriter::ByteArrayWriter(int initCap, int incCap)
: Writer(0), data(new char[initCap >= 0 ? initCap+1 : 1]),
  cap(initCap), capInc(incCap), pos(0), sz(0)
{
  if (initCap < 0) throw IllegalArgumentException("ByteArrayWriter: initCap < 0");
}

//---------------------------------------------------------------------------
/** Destructor.
   Deletes the byte array.
*/

ByteArrayWriter::~ByteArrayWriter()
{
  if (data) delete[] data;
}

//---------------------------------------------------------------------------
/**  File closed indicator.
  
  \return Always \c false.
*/

//---------------------------------------------------------------------------
/** \fn bool ByteArrayWriter::isBuffered() const
   \return Always \c true.
*/

//---------------------------------------------------------------------------
/** \fn bool ByteArrayWriter::isAborted() const
  Abort Indicator, always retuns \c false.
  \return \c false always.
*/

//---------------------------------------------------------------------------
/** \fn long ByteArrayWriter::getErrorCode() const
  Always returns zero.
  \return Zero always.
*/

//---------------------------------------------------------------------------
/** \fn ByteArrayWriter::getCap() const
    Returns the current capacity (the amount of bytes allocated) of the 
    byte array.
    \return The current capacity.
*/

//---------------------------------------------------------------------------
/** Sets the allocated capacity of the byte array.
  \param newCap The new capacity of the byte array, must be >= 0,\n
  (but this is \b not checked in the current implementation!)
  \throw OutOfMemoryException If the operation failed.

  One extra byte is silently allocated to hold a trailing zero character.

  \attention If the new capacity is less than the current \ref getSize() const
  "size", the \ref setSize(int) "size" will be decreased accordingly and
  data will be lost,\n
  however a zero character \b will be written at the new end.
*/

void ByteArrayWriter::setCap(int newCap)
{
  if (cap == newCap) return;

  char *newData = (char *)realloc(data,newCap+1);
  if (!newData) throw OutOfMemoryException("ByteArrayWriter::setCap");

  data = newData;
  cap  = newCap;

  if (sz > cap) sz  = cap;
  if (pos > sz) pos = sz;

  data[sz] = '\0';
}

//---------------------------------------------------------------------------
/** Ensures the capacity of the byte array has at least a specified value.
  \param newCap The minimum capacity of the byte array.
  \throw OutOfMemoryException If the operation failed.

  An extra byte is silently allocated to hold a trailing zero character.\n
  This is a no-op if the byte array already has the required capacity.\n
  \n
  Use this method to avoid frequent reallocations if you know the capacity
  that will be needed.
  \note
  In the current implementation this method will silently allocate at
  least 128 bytes (why?)
*/
void ByteArrayWriter::ensureCap(int newCap)
{
  if (cap >= newCap) return;

  if (capInc > 0) newCap += capInc;
  else {
    newCap = (int)ceil(newCap * 1.5);
    if (newCap < 128) newCap = 128;
  }

  char *newData = (char *)realloc(data,newCap+1);
  if (!newData) throw OutOfMemoryException("ByteArrayWriter::ensureCap");

  data = newData;
  cap  = newCap;

  if (sz > cap) {
    sz = cap;
    data[sz] = '\0';
  }

  if (pos > sz) pos = sz;
}

//---------------------------------------------------------------------------
/** \fn int ByteArrayWriter::getPos() const
   Gets the current write position.
   \return The write position.

   This is the position where the next byte will be written in the byte
   array.
*/

//---------------------------------------------------------------------------
/** Sets the write position.

    \param newPos The position in the byte array at which the next byte
    will be written.

    The capacity of the buffer will be expanded if required (i.e. if
    \c newPos > \ref getCap() const "getCap()").\n
    (In the current implementation no spare bytes will be allocated however).\n
    \n
    If \c newPos is greater than the \ref getSize() const "current size" then
    the \ref setSize(int newSz) "size" will be set to \c newPos and a zero
    character will be written at \c newPos.
*/

void ByteArrayWriter::setPos(int newPos)
{
  if (newPos < 0) throw IllegalArgumentException("ByteArrayWriter::setPos");

  if (newPos > cap) ensureCap(newPos);

  pos = newPos;
  if (sz < pos) {
    sz = pos;
    data[sz] = '\0';
  }
}

//---------------------------------------------------------------------------
/** \fn int ByteArrayWriter::getSize() const
    Gets the current size, that is the amount of valid data in the byte
    array.
    \return the current size.

    It is guaranteed that there is always a zero character in the byte
    array just after the current size of the data.

    \note
    This is not the \ref getCap() const "capacity" of the byte array.
*/

//---------------------------------------------------------------------------
/** Sets the size (the amount of valid data) of the byte array.
   \param newSz The new size of the byte array.
   \throw IllegalArgumentException If <tt>newSz < 0</tt>.

    The capacity of the buffer will be expanded as required (i.e. if
    \c newSz > \ref getCap() const "getCap()").\n
    In the current implementation no spare bytes will be allocated however.\n
    \n
    If \c newSz is less than the old \ref getSize() const "size" a zero
    character is written at \c newSz (and data will have been lost).\n
    \n
    If \c newSz is less than the \ref getPos() const "current write position",
    the \ref setPos(int newPos) "position" will be set to \c newSz.
*/

void ByteArrayWriter::setSize(int newSz)
{
  if (newSz < 0) throw IllegalArgumentException("ByteArrayWriter::setSize");

  if (newSz > cap) ensureCap(newSz);

  sz = newSz;
  data[sz] = '\0';

  if (pos > sz) pos = sz;
}

//---------------------------------------------------------------------------
/** Initializes the byte array.
   \param val The character value to write to every item in the byte array.

   \note
   The value is written to the entire \ref getCap() const "capacity"
   of the byte array, not just to its current \ref getSize() const "size".
*/

void ByteArrayWriter::clearArray(char val)
{
  memset(data,val,cap);
  data[sz] = '\0';
}

//---------------------------------------------------------------------------
/** \fn char* &ByteArrayWriter::getBuffer()
  Returns a <b> non-const reference</b> to the byte array.
  \return a \b reference to the internal buffer.

  \attention <b>Reference returned.</b>\n
  Since this class may reallocate its internal buffer at any time,
  a pointer returned as a value may become invalid.\n
  \n
  For that reason this method returns a \b reference to the internal buffer.
  By initializing a \b reference pointer variable with the returned result,
  the buffer pointer will never become invalid.\n
  \n
  Example: (with reference)\n
  <tt>char * <b>&</b>bufPtr = wrt.getBuffer();\n
  wrt.write(someData,10000); </tt>Ok: \c bufPtr still valid.\n
  \n
  Example 2: (pointer value)\n
  <tt>char *bufPtr2 = wrt.getBuffer();\n
  wrt.write(someData,10000); </tt> Possible error: \c bufPtr2 is
  \b invalid after this call if the capacity was increased!
  
  \note
  Since a reference is returned it is possible to replace the buffer
  that is being used by one that you have allocated.\n
  Resist the temptation: this class does not expect it and
  disaster is bound to happen!
*/

//---------------------------------------------------------------------------
/** Writes a block of data in the byte array.
  \param buf The data to write, must not be \c NULL.
  \param bSz The number of bytes to write.
  \return \c true always.
  \throw NullPointerException If <tt>buf == NULL</tt>.
  \throw IllegalArgumentException If <tt>bSz < 0</tt>.

  Writing starts at the \ref getPos() const "current position".\n
  \n
  The capacity of the buffer will be expanded if required.\n
  (In the current implementation no spare bytes will be allocated however).\n
  \n
  The \ref getSize() const "size" of the byte array will be increased if bytes
  are written past the current size and a trailing zero character will
  automatically be appended in that case.

  \note
  An extra zero character is \b not written if it was not necessary to increase
  the \ref setSize(int newSz) "size"\n
  Thus it is possible to <em>"fill in the fields"</em> at random
  by using \ref setPos(int) "setPos" and \ref write(const char *buf, int bSz)
  "write()" in an alternating fashion.
*/

bool ByteArrayWriter::write(const char *buf, int bSz)
{
  if (!buf) throw NullPointerException("ByteArrayWriter::write: buf == NULL");
  if (bSz < 0) throw IllegalArgumentException("ByteArrayWriter::write: bSz < 0");

  if (bSz < 1) return true;

  if (bSz > cap - pos) ensureCap(pos+bSz);

  memcpy(data+pos,buf,bSz);

  pos += bSz;

  if (sz < pos) {
    sz = pos;
    data[sz] = '\0';
  }

  return true;
}

//---------------------------------------------------------------------------
/** \fn bool ByteArrayWriter::flush()
   This is a no-op.
*/

} // namespace Ino

//---------------------------------------------------------------------------
