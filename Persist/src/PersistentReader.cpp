//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library ----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "PersistentIO.h"

#include "Type.h"
#include "InpPools.h"

#include <cstring>

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup persistence Persistence
 @{
*/

//---------------------------------------------------------------------------
/** \file PersistentIO.h
  Contains the definition of all classes related to persistence.
  
  A set of C++ class instances with arbritarly complex mutual relations
  may be \link Ino::PersistentWriter persisted\endlink (serialized) to a
  \link Ino::Writer Writer\endlink and later be
  \link Ino::PersistentReader de-serialized\endlink from a
  \link Ino::Reader Reader\endlink.

  The classes to be persisted must all be derived from abstract base
  class Ino::Persistable.\n
  \n
  In such a graph of related classes one class must be derived from
  abstract class Ino::MainPersistable.\n\n
  Such a \link Ino::MainPersistable MainPersistable\endlink may be
  written to a \link Ino::Writer stream\endlink
  using class \link Ino::PersistentWriter PersistentWriter\endlink
  and then \link Ino::PersistentWriter PersistentWriter\endlink assures
  that every class that is reachable from that 
  \link Ino::MainPersistable MainPersistable\endlink  is written
  exactly once to the stream.

  At a later time the stream may be read back in again using class
  \link Ino::PersistentReader PersistentReader\endlink.\n
  It returns the \link Ino::MainPersistable MainPersistable\endlink
  and ensures that all classes that were originally written to
  the stream are restored exactly as they were before
  and with exactly the same relations.

  \par Version Resilience

  This set of classes was written with the objective to be as insensitive
  (backward compatible) as possible to changes in the classes that
  make up a stream.

  This software is the result of some very disappointing experiences
  with Java serialization.\n
  When using the Java serialization even the smallest change to one of the
  classes made it impossible to read back an earlier stream.

  So an alternive Java version called Persistence was written (1996).\n
  The key idea was that the serialized data format must be only
  loosely coupled to the naming of the classes and their members.

  First of all the classes are written to the stream have a universally unique
  name, which is by convention a reversed DNS name, for example:
  <tt>nl.inofor.geom.vector</tt>.\n
  Every such class must be defined in a class derived from
  \link Ino::PersistentTypeDef PersistentTypeDef\endlink and that class
  must then be passed to \link Ino::PersistentWriter PersistentWriter\endlink
  and later \link Ino::PersistentReader PersistentReader\endlink.

  Secondly for every data member of every class typeinfo and a name are
  written to the stream. The member names are not necessarily the same as
  their name in the class.

  Method \link Ino::Persistable::definePersistentFields()
  definePersistentFields()\endlink
  of class \link Ino::Persistable Persistable\endlink  must be implemented by
  the user in each class that is to be written to a stream.\n
  In that method the fields to be read/written are given a name and type info.

  This method is automatically called by
  \link Ino::PersistentWriter PersistentWriter\endlink when appropriate so that
  the field information is written (just once) to the stream.

  Method
  \link Ino::Persistable::writePersistentObject() writePersistentObject()\endlink
  must also be defined by the user for each class that is to be
  written to the stream.\n
  In that method a series of method calls define what gets written to the
  stream (one call for each defined field).\n
  \n
  The \link Ino::PersistentWriter PersistentWriter\endlink will automatically
  call method
  \link Ino::Persistable::writePersistentObject() writePersistentObject()\endlink
  once for each class in turn.\n\n

  In order to be able to reconstruct the stream the user must implement the
  \link Ino::Persistable::Persistable(PersistentReader&)
  persistence constructor\endlink for each class.\n
  In this constructor a series of read calls initializes the fields from
  the stream.\n
  When the stream is read back by a
  \link Ino::PersistentReader PersistentReader\endlink the
  \link Ino::Persistable::Persistable(PersistentReader&)
  persistence constructor\endlink for each class is called (using the
  \link Ino::PersistentTypeDef PersistentTypeDef\endlink derived class).\n\n

  The version resilience is based on the following:

  \li Fields may have changed names in the class, but will still be the same
  in the stream.

  \li Fields in the class may no longer be present. This is no problem
  because then they will simply be skipped in the stream.

  \li Fields may have been added to the class that are not yet present in the
  field definition of the stream.\n
  The user can anticipate on this fact by detecting this in the persistence
  constructor and then initialize the field with a default value (e.g. \c NULL).

  \li A new class may have been defined derived from an existing class.
  Then the stream is written.\n\n
  Older software that does not know about this class can still read the newer
  stream. When the PeristentReader encounters a class of a type it does not
  know about it will pass a \c NULL pointer for that class.\n\n
  Thus \b forward compatibility can also be achieved quite easily, at least
  when the programmer anticipates on the possibility in his program.

  After about 5 years of experience with the persistence library a matching
  C++ version was implemented in 2005.\n
  Initially the C++ version was fully compatible with the Java version.\n
  The current situation is that the Java version lags somewhat behind the
  C++ version.\n\n

  Over the years the version resilience has proven to work very well.\n
  One particular file format for a measuring machine has gone through about
  25 changes in the persisted classes, yet all the files (streams) of all
  previous versions can be read without problem.\n
  To achieve that only in three cases a bit of conditional code had
  to be written.

  The one drawback is that three methods have to be implemented in every class
  that is to be written to a stream\n\n
  There is no automated tool to do that and the feeling remains that there
  shouldn't be.

  The programmer must carefully define the type and format of each class and
  each field manually, already with future changes in mind.

  Another important aspect is that redundant information must not (and need not)
  be written to a stream.\n
  Such information can easily be reconstructed when the stream in read back in.\n
  This allows for maximum future flexibility.

  \note
  Persistence is not only useful to serialize a set of classes to a disk file.
  It can also be used to pass (complicated) messages over the internet.\n
  (See also the note with: 
  \link Ino::PersistentWriter::writeMainObject() writeMainObject()\endlink).

  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/** \class PersistentReader
  A PersistentReader is used to restore a MainPersistable and all connected
  Persistable objects from a stream read by a Reader.
  
  The constructor must be passed an instance of a user defined data
  definition class derived from PersistentTypeDef.

  Then a call to method readMainObject() is all that is needed to restore
  the MainPersistable class and all related classes in memory (on the heap).

  If more than one MainPersistable has been written to the same stream,
  then a matching number of readMainObject() calls will be required to
  restore all data.

  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/** 
 @}
*/

//---------------------------------------------------------------------------
/** Constructor.
  \param typeDef The data definition class derived from
  PersistentTypeDef to be used.
  \param reader The Reader that is used to get the stream data.

  There must be a \link PersistentTypeDef data definition class\endlink
  for each stream format,\n
  and the same data definition class must be used to both write and
  read the stream.
*/

PersistentReader::
            PersistentReader(PersistentTypeDef& typeDef, Reader& reader)
: tpDef(typeDef),cRdr(reader), dRdr(cRdr), byteRdr(1024),byteDataRdr(byteRdr),
  mmajor(0),  mminor(0), first(true), usrPtr(NULL), errMsg(NULL),
  typePool(*new InTypePool(typeDef)),
  structPool(*new InpStructPool(typePool)),
  stringPool(*new InpStringPool()),
  arrayPool(*new InpArrayPool(typePool)),
  curType(NULL),  curStructId(0),  curArrayId(0)
{
}

//---------------------------------------------------------------------------
/** Destructor.
  Cleans up all internal datastructures related to the stream.
*/

PersistentReader::~PersistentReader()
{
  if (errMsg) delete[] errMsg;

  delete &typePool;
  delete &structPool;
  delete &stringPool;
  delete &arrayPool;
}
                                             
//---------------------------------------------------------------------------

void PersistentReader::readHeader()
{
  cRdr.setDecompressing(false);

  long famMagic = dRdr.readInt();
  long magic    = dRdr.readInt();

  if (famMagic != tpDef.famMagic || magic != tpDef.magic)
      throw StreamCorruptedException("PersistentReader: Wrong Magic");

  mmajor = dRdr.readByte();
  mminor = dRdr.readByte();

  dRdr.readByte(); // FormatMajor and minor: internal version
  dRdr.readByte(); // indicators for this sfw, not used yet

  bool compressed = dRdr.readBool();

  cRdr.setDecompressing(compressed);

  first = false;
}

//---------------------------------------------------------------------------
/** Returns the major version number from the stream.

    The major number defined in the \ref PersistentTypeDef "data definition"
    was originally written to the stream.

    This methed returns that number.

    If method readMainObject() has not yet been called this method
    will read the header of the stream to obtain the major number.

    \return The streams major version number.

    \note Due to the version resilience it is normally not necessary to
    change version numbers if the underlying classes are changed.\n
    In fact this feature has never been used yet in the past 15 years!
*/

short PersistentReader::getMajor()
{
  if (first) readHeader();

  return mmajor & 0xFF;
}

//---------------------------------------------------------------------------
/** Returns the minor version number from the stream.

    The minor number defined in the \ref PersistentTypeDef "data definition"
    was originally written to the stream.

    This methed returns that number.

    If method readMainObject() has not yet been called this method
    will read the header of the stream to obtain the minor number.

    \return The streams minor version number.

    \note Due to the version resilience it is normally not necessary to
    change version numbers if the underlying classes are changed.\n
    In fact this feature has never been used yet in the past 15 years!
*/

short PersistentReader::getMinor()
{
  if (first) readHeader();

  return mminor & 0xFF;
}

//---------------------------------------------------------------------------
/** Returns the End-Of_file condition of the stream.

  \return \c true if all data in the stream has been read,\n
  \c false otherwise.

  This class relies on the supplied Reader class to supply this information.
*/

bool PersistentReader::isEof() const
{
  return cRdr.isEof();
}

//---------------------------------------------------------------------------
/** Returns the abort state of the stream.

   \return \c true if the underlying Reader indicates so,
   \c false otherwise.

   The used Reader object may have a ProgressReporter, and that class allows
   the reading of a stream to be prematurely
   \ref ProgressReporter::mustAbort() "aborted".
*/

bool PersistentReader::isAborted() const
{
  return cRdr.isAborted();
}

//---------------------------------------------------------------------------
/** \fn void *PersistentReader::getUserPtr()
  Returns the value passed to readMainObject().

  May called from within one of the \c read() methods.

  \return The user pointer.
*/

//---------------------------------------------------------------------------
/** \fn const char *PersistentReader::errorMsg() const
  Returns the error message if any.

  \return The error message if readMainObject() returned \c NULL \n
  or returns \c NULL if there was no error.

  \note That all thrown exceptions are caught internally and will result
  in an error message that can be retrieved with this method.
*/

//---------------------------------------------------------------------------

void PersistentReader::processEof(MainPersistable *mps, bool reset)
{
  if (!mps) throw IllegalStateException("PersistentReader::processEof: No main");

  structPool.postProcess(*this);

  structPool.setAllValid();
  stringPool.setAllValid();
  arrayPool.setAllValid();

  mps->readPersistentComplete(*this);

  if (reset) {
    first = true;

    curStructId = 0;
    curArrayId  = 0;

    typePool.clear();
    structPool.clear();
    stringPool.clear();
    arrayPool.clear();
  }
}

//---------------------------------------------------------------------------

Field *PersistentReader::getValField(const char *fldName,
                                          const type_info& inf,
                                          const char *msg, bool nullOk)
{
  if (!curType)
        throw IllegalStateException("PersistentReader::getValField");

  Field *fld = curType->getValField(fldName);

  if (!fld) {
    if (nullOk) return NULL;
    else {
      char msgBuf[512];
      sprintf(msgBuf,"Field: \"%s\" is not defined in class: \"%s\"",
                                                      fldName,inf.name());
      throw NoSuchElementException(msgBuf);
    }
  }

  if (!fld->type) {
    if (nullOk) return NULL;
    else {
      char msgBuf[512];
      sprintf(msgBuf,"Type of field: \"%s\" is not defined in class: \"%s\"",
                                                        fldName,inf.name());
      throw IllegalArgumentException(msgBuf);
    }
  }

  if (fld->type->getInfo() != inf) {
    char msgBuf[512];
    sprintf(msgBuf,"Field \"%s\" is not a %s in class \"%s\"",
                                                   fldName,msg,inf.name());
    throw IllegalArgumentException(msgBuf);
  }

  return fld;
}

//---------------------------------------------------------------------------

Field *PersistentReader::getRefField(const char *fldName, bool nullOk)
{
  if (!curType)
           throw IllegalStateException("PersistentReader::getRefField");

  Field *fld = curType->getRefField(fldName);

  if (!fld) {
    if (nullOk) return NULL;
    else {
      char msg[512];
      sprintf(msg,"Field: \"%s\" is not defined in class: \"%s\"",
                                           fldName,curType->getInfo().name());
      throw NoSuchElementException(msg);
    }
  }

  if (!fld->type) {
    if (nullOk) return NULL;
    else {
      char msgBuf[512];
      sprintf(msgBuf,"Type of field: \"%s\" is not defined in class: \"%s\"",
                                        fldName,curType->getInfo().name());
      throw IllegalArgumentException(msgBuf);
    }
  }

  if (!fld->type->isRefType()) {
    char msg[512];
    sprintf(msg,"Field \"%s\" is not an object type (in class \"%s\"",
                                        fldName,curType->getInfo().name());
    throw IllegalArgumentException(msg);
  }

  return fld;
}

//---------------------------------------------------------------------------

Field *PersistentReader::getArrayField(const char *fldName, bool nullOk)
{
  if (!curType)
      throw IllegalStateException("PersistentReader::getValArrayField");

  Field *fld = curType->getRefField(fldName);

  if (!fld) {
    if (nullOk) return NULL;
    else {
      char msg[512];
      sprintf(msg,"Field: \"%s\" is not defined in class: \"%s\"",
                                           fldName,curType->getInfo().name());
      throw NoSuchElementException(msg);
    }
  }

  if (!fld->type) {
    if (nullOk) return NULL;
    else {
      char msgBuf[512];
      sprintf(msgBuf,"Type of array field: \"%s\" is not defined in class: \"%s\"",
                                        fldName,curType->getInfo().name());
      throw IllegalArgumentException(msgBuf);
    }
  }

  if (fld->type->getCategory() != Type::CatArray) {
    char msg[512];
    sprintf(msg,"Field \"%s\" is not an array (in class \"%s\"",
                                        fldName,curType->getInfo().name());
    throw IllegalArgumentException(msg);
  }

  return fld;
}

//--------------------------------------------------------------------------

static void skipRecord(DataReader& rdr, int bytes)
{
  char buf[1024];

  while (bytes > 0) {
    int bSz = bytes;
    if (bSz > 1024) bSz = 1024;

    int bRd = rdr.read(buf,bSz);
    if (bRd <= 0) return;

    bytes -= bRd;
  }
}

//-------------------------------------------------------------------------

void PersistentReader::readStruct()
{
  curStructId++;

  int recLen = Type::readRecordLen(dRdr,"PersistentReader::readStruct 1");

  curType = structPool.getType(curStructId);

  if (!curType) { // Just skip record
    skipRecord(dRdr,recLen);
    return;
  }

  if (!curType->isDefined())
     throw StreamCorruptedException("PersistentReader::readStruct 3");

  int rSz = curType->getStructSize();

  if (rSz != recLen)
          throw StreamCorruptedException("PersistentReader::readStruct 5");

  byteRdr.ensureCap(rSz);
  char* &byteBuf = byteRdr.getBuffer();

  dRdr.read(byteBuf,rSz,"PersistentReader::readStruct 6");

  byteRdr.setPos(0);
  byteRdr.setSize(rSz);

  Persistable *newP = structPool.get(curStructId);

  curType->baseType.construct(newP,*this);

  curType = NULL;
}

//---------------------------------------------------------------------------
/** Reads and restores a MainPersistable and all related classes.

   Restores a MainPersistable from the stream and then returns
   a pointer to that object.\n
   (A typecast to the original type will be necessary).

   \param userPtr A user defined opaque value that can be obtained
   (using getUserPtr()) from within the Persistable methods while
   each class is being restored.\n
   This may for instance be used to adapt the behavior while the
   classes are being restored.

   \return A pointer to the restored MainPersistable or \c NULL
   of there was an error.\n
   In the latter case method errorMsg() may be called to obtain more
   information.\n\n
   Internally all exceptions thrown are caught and converted to
   an appropriate error message.

  \note In order to be compatible with all types of computer
   hardware all fields of basic types are stored in
   <em>network byte order</em>
*/

MainPersistable *PersistentReader::readMainObject(void *userPtr)
{
  if (errMsg) delete[] errMsg;
  errMsg = NULL;

  usrPtr = userPtr;

  int mainStructId = curStructId+1;

  try {
    if (first) readHeader();

    for (;;) {
      char recType = dRdr.readByte("PersistentReader::readMainObject");

      switch (recType) {
        case Record_Eof: {
          Persistable *ps = structPool.get(mainStructId);
          MainPersistable *mps = dynamic_cast<MainPersistable *>(ps);

          processEof(mps,false);

          return mps;
        }

        case Record_EofReset: {
          Persistable *ps = structPool.get(mainStructId);
          MainPersistable *mps = dynamic_cast<MainPersistable *>(ps);

          processEof(mps,true);
  
          return mps;
        }

        case Record_Type:
          typePool.readType(dRdr);
        break;

        case Record_StructDecl: {
          short tpId = dRdr.readShort("PersistentReader::readMainObject: StructDecl");
          structPool.add(tpId);
        }
        break;

        case Record_TypeDef:
          typePool.readTypeDef(dRdr);
        break;

        case Record_ArrayDecl:
          arrayPool.readDecl(dRdr);
        break;

        case Record_String: 
          stringPool.readString(dRdr);
        break;

        case Record_Struct:
          readStruct();
        break;

        case Record_Array:
          readArray();
        break;

        default: 
          throw StreamCorruptedException(
                    "PersistentReader::readMainObject: Unknown Record Type");
      }
    }
  }
  catch (exception& ex) {
    first = true;

    curStructId = 0;
    curArrayId  = 0;

    typePool.clear();
    structPool.clear();
    stringPool.clear();
    arrayPool.clear();

    if (cRdr.isAborted()) errMsg = dupStr("StreamAbortedException");
    else if (cRdr.isEof()) errMsg = dupStr("StreamClosedException: Premature End");
    else {
      char msgBuf[1024];
      int msgLen = sprintf(msgBuf,"%s: %s",typeid(ex).name(),ex.what());

      errMsg = new char[msgLen+1];
      strcpy(errMsg,msgBuf);
    }

    return NULL;
  }
}

//---------------------------------------------------------------------------
/** Determines if a field in the class currently being de-serialized, exists.

  This method may only be called from within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor"
  of the object being deserialized.\n
  An IllegalStateException is thrown otherwise.

  \param fldName The name of the field to test for existence.
  \return \c true if the field is defined in the stream for this class,
  \c false otherwise.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".
*/

bool PersistentReader::fieldExists(const char *fldName)
{
  if (!curType) throw IllegalStateException(
                        "PersistentReader::fieldExists: Illegal Call");

  return curType->getValField(fldName) != NULL || 
                               curType->getRefField(fldName) != NULL;
}

//---------------------------------------------------------------------------
/** This method is \b seriously deprecated, <b>do not use!</b>
*/

void PersistentReader::callPostProcess()
{
  if (!curType) throw IllegalStateException(
                        "PersistentReader::callPostProcess: Illegal Call");

  structPool.setPostProcess(curStructId);
}

//---------------------------------------------------------------------------
/** Reads a boolean field from the stream.

  \param fldName The name of the field.
  \return The value stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note The PersistentReader will abort if a field of the specified name does
  not exist or if its type is not \c bool.\n\n
  Usually it is better to use readBool(const char *fldName, bool defVal)
  since that will never cause an abort.

  \see errorMsg()
*/

bool PersistentReader::readBool(const char *fldName)
{
  Field *fld = getValField(fldName,typeid(bool),"Boolean",false);

  byteRdr.setPos(fld->getOffset());

  return byteDataRdr.readBool();
}

//---------------------------------------------------------------------------
/** Reads a boolean field from the stream.

  \param fldName The name of the field.
  \param defVal The value to return if the field does not exist or is
  not of type \c bool.

  \return The value stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note To be version resilient always call this method instead of
  readBool(const char *fldName)!\n\n
  An older or newer stream version may (no longer) have this field and this
  method lets you then continue without problem.

  \see errorMsg()
*/

bool PersistentReader::readBool(const char *fldName, bool defVal)
{
  Field *fld = getValField(fldName,typeid(bool),"Boolean",true);
  if (!fld) return defVal;

  byteRdr.setPos(fld->getOffset());

  return byteDataRdr.readBool();
}

//---------------------------------------------------------------------------
/** Reads a \c wchar_t field from the stream.

  \param fldName The name of the field.
  \return The single wide character stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note The PersistentReader will abort if a field of the specified name does
  not exist or if its type is not \c wchar_t.\n\n
  Usually it is better to use readWChar(const char *fldName, wchar_t defVal)
  since that will never cause an abort.

  \see errorMsg()
*/

wchar_t PersistentReader::readWChar(const char *fldName)
{
  Field *fld = getValField(fldName,typeid(wchar_t),"WChar",false);

  byteRdr.setPos(fld->getOffset());
  return byteDataRdr.readWChar();
}

//---------------------------------------------------------------------------
/** Reads a \c wchar_t field from the stream.

  \param fldName The name of the field.
  \param defVal The value to return if the field does not exist or is
  not of type \c wchar_t.

  \return The single wide character stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note To be version resilient always call this method instead of
  readWChar(const char *fldName)!\n\n
  An older or newer stream version may (no longer) have this field and this
  method lets you then continue without problem.

  \see errorMsg()
*/

wchar_t PersistentReader::readWChar(const char *fldName, wchar_t defVal)
{
  Field *fld = getValField(fldName,typeid(wchar_t),"WChar",true);
  if (!fld) return defVal;

  byteRdr.setPos(fld->getOffset());
  return byteDataRdr.readWChar();
}

//---------------------------------------------------------------------------
/** Reads a byte (\c char) field from the stream.

  \param fldName The name of the field.
  \return The single character stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note The PersistentReader will abort if a field of the specified name does
  not exist or if its type is not \c wchar_t.\n\n
  Usually it is better to use readByte(const char *fldName, char defVal)
  since that will never cause an abort.

  \see errorMsg()
*/

char PersistentReader::readByte(const char *fldName)
{
  Field *fld = getValField(fldName,typeid(char),"Byte",false);

  byteRdr.setPos(fld->getOffset());
  return byteDataRdr.readByte();
}

//---------------------------------------------------------------------------
/** Reads a byte (\c char) field from the stream.

  \param fldName The name of the field.
  \param defVal The value to return if the field does not exist or is
  not of type \c char.

  \return The value stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note To be version resilient always call this method instead of
  readByte(const char *fldName)!\n\n
  An older or newer stream version may (no longer) have this field and this
  method lets you then continue without problem.

  \see errorMsg()
*/

char PersistentReader::readByte(const char *fldName, char defVal)
{
  Field *fld = getValField(fldName,typeid(char),"Byte",true);
  if (!fld) return defVal;

  byteRdr.setPos(fld->getOffset());
  return byteDataRdr.readByte();
}

//---------------------------------------------------------------------------
/** Reads a \c short field from the stream.

  \param fldName The name of the field.
  \return The value stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note The PersistentReader will abort if a field of the specified name does
  not exist or if its type is not \c short.\n\n
  Usually it is better to use readShort(const char *fldName, short defVal)
  since that will never cause an abort.

  \see errorMsg()
*/

short PersistentReader::readShort(const char *fldName)
{
  Field *fld = getValField(fldName,typeid(short),"Short",false);

  byteRdr.setPos(fld->getOffset());
  return byteDataRdr.readShort();
}

//---------------------------------------------------------------------------
/** Reads a \c short field from the stream.

  \param fldName The name of the field.
  \param defVal The value to return if the field does not exist or is
  not of type \c short.

  \return The value stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note To be version resilient always call this method instead of
  readShort(const char *fldName)!\n\n
  An older or newer stream version may (no longer) have this field and this
  method lets you then continue without problem.

  \see errorMsg()
*/

short PersistentReader::readShort(const char *fldName, short defVal)
{
  Field *fld = getValField(fldName,typeid(short),"Short",true);
  if (!fld) return defVal;

  byteRdr.setPos(fld->getOffset());
  return byteDataRdr.readShort();
}

//---------------------------------------------------------------------------
/** Reads an \c int or \c long field from the stream.

  \param fldName The name of the field.
  \return The value stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note The PersistentReader will abort if a field of the specified name does
  not exist or if its type is not \c int or \c long.\n\n
  Usually it is better to use readInt(const char *fldName, long defVal)
  since that will never cause an abort.

  \see errorMsg()
*/

long PersistentReader::readInt(const char *fldName)
{
  Field *fld = getValField(fldName,typeid(long),"Int",false);

  byteRdr.setPos(fld->getOffset());
  return byteDataRdr.readInt();
}

//---------------------------------------------------------------------------
/** Reads an \c int or \c long field from the stream.

  \param fldName The name of the field.
  \param defVal The value to return if the field does not exist or is
  not of type \c int or \c long.

  \return The value stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note To be version resilient always call this method instead of
  readInt(const char *fldName)!\n\n
  An older or newer stream version may (no longer) have this field and this
  method lets you then continue without problem.

  \see errorMsg()
*/

long PersistentReader::readInt(const char *fldName, long defVal)
{
  Field *fld = getValField(fldName,typeid(long),"Int",true);
  if (!fld) return defVal;

  byteRdr.setPos(fld->getOffset());
  return byteDataRdr.readInt();
}

//---------------------------------------------------------------------------
/** Reads a <tt>long long</tt> (64 bits) field from the stream (\b Not a \c long!).

  Type <tt>long long</tt> is also known as \c __int64.

  \param fldName The name of the field.
  \return The value stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note The PersistentReader will abort if a field of the specified name does
  not exist or if its type is not <tt>long long</tt>.\n\n
  Usually it is better to use readLong(const char *fldName, __int64 defVal)
  since that will never cause an abort.

  \see errorMsg()
*/

__int64 PersistentReader::readLong(const char *fldName)
{
  Field *fld = getValField(fldName,typeid(__int64),"Long",false);

  byteRdr.setPos(fld->getOffset());
  return byteDataRdr.readLong();
}

//---------------------------------------------------------------------------
/** Reads a <tt>long long</tt> (64 bits) field from the stream (\b Not a \c long!).

  Type <tt>long long</tt> is also known as \c __int64.

  \param fldName The name of the field.
  \param defVal The value to return if the field does not exist or is
  not of type <tt>long long</tt>.

  \return The value stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note To be version resilient always call this method instead of
  readLong(const char *fldName)!\n\n
  An older or newer stream version may (no longer) have this field and this
  method lets you then continue without problem.

  \see errorMsg()
*/

__int64 PersistentReader::readLong(const char *fldName, __int64 defVal)
{
  Field *fld = getValField(fldName,typeid(__int64),"Long",true);
  if (!fld) return defVal;

  byteRdr.setPos(fld->getOffset());
  return byteDataRdr.readLong();
}

//---------------------------------------------------------------------------
/** Reads a \c float (32 bits) field from the stream.

  \param fldName The name of the field.
  \return The value stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note The PersistentReader will abort if a field of the specified name does
  not exist or if its type is not \c float.\n\n
  Usually it is better to use readFloat(const char *fldName, float defVal)
  since that will never cause an abort.

  \see errorMsg()
*/

float PersistentReader::readFloat(const char *fldName)
{
  Field *fld = getValField(fldName,typeid(float),"Float",false);

  byteRdr.setPos(fld->getOffset());
  return byteDataRdr.readFloat();
}

//---------------------------------------------------------------------------
/** Reads a \c float (32 bits) field from the stream.

  \param fldName The name of the field.
  \param defVal The value to return if the field does not exist or is
  not of type \c float.

  \return The value stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note To be version resilient always call this method instead of
  readFloat(const char *fldName)!\n\n
  An older or newer stream version may (no longer) have this field and this
  method lets you then continue without problem.

  \see errorMsg()
*/

float PersistentReader::readFloat(const char *fldName, float defVal)
{
  Field *fld = getValField(fldName,typeid(float),"Float",true);
  if (!fld) return defVal;

  byteRdr.setPos(fld->getOffset());
  return byteDataRdr.readFloat();
}

//---------------------------------------------------------------------------
/** Reads a \c double (64 bits) field from the stream.

  \param fldName The name of the field.
  \return The value stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note The PersistentReader will abort if a field of the specified name does
  not exist or if its type is not \c double.\n\n
  Usually it is better to use readDouble(const char *fldName, double defVal)
  since that will never cause an abort.

  \see errorMsg()
*/

double PersistentReader::readDouble(const char *fldName)
{
  Field *fld = getValField(fldName,typeid(double),"Double",false);

  byteRdr.setPos(fld->getOffset());
  return byteDataRdr.readDouble();
}

//---------------------------------------------------------------------------
/** Reads a \c double (64 bits) field from the stream.

  \param fldName The name of the field.
  \param defVal The value to return if the field does not exist or is
  not of type \c double.

  \return The value stored for this field.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note To be version resilient always call this method instead of
  readDouble(const char *fldName)!\n\n
  An older or newer stream version may (no longer) have this field and this
  method lets you then continue without problem.

  \see errorMsg()
*/

double PersistentReader::readDouble(const char *fldName, double defVal)
{
  Field *fld = getValField(fldName,typeid(double),"Double",true);
  if (!fld) return defVal;

  byteRdr.setPos(fld->getOffset());
  return byteDataRdr.readDouble();
}

//---------------------------------------------------------------------------
/** Reads a \c wchar_t* \b string field from the stream.

  \param fldName The name of the field.
  \return The pointer to a string restored for this field, may be \c NULL.\n
  The string read back includes the trailing null character.\n
  The string will have been allocated on the heap.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note The PersistentReader will abort if a field of the specified name does
  not exist or if its type is not \c wchar_t *.\n\n
  Usually it is better to use readString(const char *fldName, wchar_t *defVal)
  since that will never cause an abort.

  \see errorMsg()
*/

wchar_t *PersistentReader::readString(const char *fldName)
{
  Field *fld = getValField(fldName,typeid(wchar_t *),"String",false);

  byteRdr.setPos(fld->getOffset());
  int strId = byteDataRdr.readInt();

  if (strId == 0) return NULL;
  return stringPool.get(strId);
}

//---------------------------------------------------------------------------
/** Reads a \c wchar_t* \b string field from the stream.

  \param fldName The name of the field.
  \param defVal The value to return if the field does not exist or is
  not of type \c wchar_t*.

  \return The pointer to a string restored for this field, may be \c NULL.\n
  The string will have been allocated on the heap.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note To be version resilient always call this method instead of
  readString(const char *fldName)!\n\n
  An older or newer stream version may (no longer) have this field and this
  method lets you then continue without problem.

  \see errorMsg()
*/

wchar_t *PersistentReader::readString(const char *fldName, wchar_t *defVal)
{
  Field *fld = getValField(fldName,typeid(wchar_t *),"String",true);
  if (!fld) return defVal;

  byteRdr.setPos(fld->getOffset());
  int strId = byteDataRdr.readInt();

  if (strId == 0) return NULL;
  return stringPool.get(strId);
}

//---------------------------------------------------------------------------
/** Reads an object (derived from Persistable) field from the stream.

  \param fldName The name of the field.
  \return The pointer to the object restored, may be \c NULL.\n
  The object will have been allocated on the heap.\n
  A typecast to the correct type will be necessary, preferably use
  \c dynamic_cast<T> over a hard cast.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \warning The object whose pointer is returned will normally not have been
  constructed yet at this point (but only allocated).\n
  So no attempt must be made to derefence the pointer!

  \note The PersistentReader will abort if a field of the specified name does
  not exist or if its type is not \c Persistable *.\n\n
  Usually it is better to use readObject(const char *fldName, Persistable *defVal)
  since that will never cause an abort.

  \see errorMsg()
*/

Persistable *PersistentReader::readObject(const char *fldName)
{
  Field *fld = getRefField(fldName,false);

  byteRdr.setPos(fld->getOffset());
  int pId = byteDataRdr.readInt();

  if (pId == 0) return NULL;

  return structPool.get(pId);
}

//---------------------------------------------------------------------------
/** Reads an object (derived from Persistable) field from the stream.

  \param fldName The name of the field.
  \param defVal The value to return if the field does not exist or is
  not of type \c Persistable *.

  \return The pointer to the object restored, may be \c NULL.\n
  The object will have been allocated on the heap.\n
  A typecast to the correct type will be necessary, preferably use
  \c dynamic_cast<T> over a hard cast.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \warning The object whose pointer is returned will normally not have been
  constructed yet at this point (but only allocated).\n
  So no attempt must be made to derefence the pointer!

  \note To be version resilient always call this method instead of
  readObject(const char *fldName)!\n\n
  An older or newer stream version may (no longer) have this field and this
  method lets you then continue without problem.

  \see errorMsg()
*/

Persistable *PersistentReader::readObject(const char *fldName,
                                                   Persistable *defVal)
{
  Field *fld = getRefField(fldName,true);
  if (!fld) return defVal;

  byteRdr.setPos(fld->getOffset());
  int pId = byteDataRdr.readInt();

  if (pId == 0) return NULL;

  return structPool.get(pId);
}

//---------------------------------------------------------------------------
/** Reads the stored array size for a field.
  
  \param fldName The name of the field.
  \return The size of the array, as a number of elements (\b not bytes).

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note The PersistentReader will abort if a field of the specified name does
  not exist or if it is not an array.\n\n
  Usually it is better to use readArraySize(const char *fldName, int defVal)
  since that will never cause an abort.

  \see errorMsg()
*/

int PersistentReader::readArraySize(const char *fldName)
{
  Field *fld = getArrayField(fldName, false);

  byteRdr.setPos(fld->getOffset());
  int arrId = byteDataRdr.readInt();

  if (arrId == 0) return 0;

  return arrayPool.getSize(arrId);
}

//---------------------------------------------------------------------------
/** Reads the stored array size for a field.
  
  \param fldName The name of the field.
  \param defVal The value to return if the field does not exist or is
  not an array.
  \return The size of the array, as a number of elements (\b not bytes).

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \note To be version resilient always call this method instead of
  readArraySize(const char *fldName)!\n\n
  An older or newer stream version may (no longer) have this field and this
  method lets you then continue without problem.

  \see errorMsg()
*/

int PersistentReader::readArraySize(const char *fldName, int defVal)
{
  Field *fld = getArrayField(fldName, true);
  if (!fld) return defVal;

  byteRdr.setPos(fld->getOffset());
  int arrId = byteDataRdr.readInt();

  if (arrId == 0) return 0;

  return arrayPool.getSize(arrId);
}

//---------------------------------------------------------------------------
/** Reads an array field of type <b>other than</b> Persistable **.

  \param fldName The name of the field.
  \return The array restored, may be \c NULL.\n
  The array will have been allocated on the heap.\n
  A typecast to the correct type will be necessary, preferably use
  \c dynamic_cast<T> over a hard cast.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \warning The array will normally not have been
  populated yet at this point (but only allocated).\n
  So no attempt must be made to derefence the pointer!

  \note The PersistentReader will abort if a field of the specified name does
  not exist or if its type is not an array.\n\n
  Usually it is better to use readValArray(const char *fldName, void *defVal)
  since that will never cause an abort.

  \see errorMsg()
*/

void *PersistentReader::readValArray(const char *fldName)
{
  Field *fld = getArrayField(fldName, false);

  byteRdr.setPos(fld->getOffset());
  int arrId = byteDataRdr.readInt();

  if (arrId == 0) return NULL;

  int arrSz;
  return arrayPool.getPtr(arrId,arrSz);
}

//---------------------------------------------------------------------------
/** Reads an array field of type <b>other than</b> Persistable **.

  \param fldName The name of the field.
  \param defVal The value to return if the field does not exist or is
  not of type \c Persistable **.

  \return The array restored, may be \c NULL.\n
  The array will have been allocated on the heap.\n
  A typecast to the correct type will be necessary, preferably use
  \c dynamic_cast<T> over a hard cast.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \warning The array will normally not have been
  populated yet at this point (but only allocated).\n
  So no attempt must be made to derefence the pointer!

  \note To be version resilient always call this method instead of
  readValArray(const char *fldName)!\n\n
  An older or newer stream version may (no longer) have this field and this
  method lets you then continue without problem.

  \see errorMsg()
*/

void *PersistentReader::readValArray(const char *fldName, void *defVal)
{
  Field *fld = getArrayField(fldName, true);
  if (!fld) return defVal;

  byteRdr.setPos(fld->getOffset());
  int arrId = byteDataRdr.readInt();

  if (arrId == 0) return NULL;

  int arrSz;
  return arrayPool.getPtr(arrId,arrSz);
}

//---------------------------------------------------------------------------
/** Reads an array field of type Persistable **.

  \param fldName The name of the field.

  \return The array restored, may be \c NULL.\n
  The array will have been allocated on the heap.\n
  A typecast to the correct type will be necessary, preferably use
  \c dynamic_cast<T> over a hard cast.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \warning The array will normally not have been
  populated yet at this point (but only allocated).\n
  So no attempt must be made to derefence the pointer!

  \note The PersistentReader will abort if a field of the specified name does
  not exist or if its type is not an array.\n\n
  Usually it is better to use
  readObjArray(const char *fldName, Persistable **defVal)
  since that will never cause an abort.

  \see errorMsg()
*/

Persistable **PersistentReader::readObjArray(const char *fldName)
{
  Field *fld = getArrayField(fldName, false);

  byteRdr.setPos(fld->getOffset());
  int arrId = byteDataRdr.readInt();

  if (arrId == 0) return NULL;

  int arrSz;
  return (Persistable **)arrayPool.getPtr(arrId,arrSz);
}

//---------------------------------------------------------------------------
/** Reads an array field of type Persistable **.

  \param fldName The name of the field.
  \param defVal The value to return if the field does not exist or is
  not of type \c Persistable **.

  \return The array restored, may be \c NULL.\n
  The array will have been allocated on the heap.\n
  A typecast to the correct type will be necessary, preferably use
  \c dynamic_cast<T> over a hard cast.

  \throw IllegalStateException If this method is called other than from
  within the
  \ref Persistable::Persistable(PersistentReader&) "persistence constructor".

  \warning The array will normally not have been
  populated yet at this point (but only allocated).\n
  So no attempt must be made to derefence the pointer!

  \note To be version resilient always call this method instead of
  readObjArray(const char *fldName)!\n\n
  An older or newer stream version may (no longer) have this field and this
  method lets you then continue without problem.

  \see errorMsg()
*/

Persistable **PersistentReader::readObjArray(const char *fldName,
                                             Persistable **defVal)
{
  Field *fld = getArrayField(fldName,true);
  if (!fld) return defVal;

  byteRdr.setPos(fld->getOffset());
  int arrId = byteDataRdr.readInt();

  if (arrId == 0) return NULL;

  int arrSz;
  return (Persistable **)arrayPool.getPtr(arrId,arrSz);
}

//--------------------------------------------------------------------------

void PersistentReader::readArray()
{
  int recLen = Type::readRecordLen(dRdr,"PersistentReader::readArray");

  curArrayId++;

  Array *tp = arrayPool.getType(curArrayId);
  if (!tp) {
    skipRecord(dRdr,recLen);
    return;
  }

  int elSz = tp->elemType.getDataSize();

  if ((recLen % elSz) != 0)
             throw StreamCorruptedException("PersistentReader::readArray 2");

  recLen /= elSz;

  int arrSz;
  void *arrPtr = arrayPool.getPtr(curArrayId,arrSz);

  if (recLen != arrSz)
             throw StreamCorruptedException("PersistentReader::readArray 3");

  switch (tp->elemType.getCategory()) {
    case Type::CatBasic:
        readBasicArray(arrPtr,arrSz,(Basic &)tp->elemType);
      break;

    case Type::CatStruct:
        readStructArray((Persistable **)arrPtr,arrSz);
      break;

    case Type::CatArray:
        readArrayArray(arrPtr,arrSz,(Array &)tp->elemType);
      break;

    default: throw StreamCorruptedException("PersistentReader::readArray 3");
  }
}

//---------------------------------------------------------------------------

void PersistentReader::readBasicArray(void *arr, int items, Basic &elType)
{
  if (items < 1) return;

  switch (elType.dataType) {
    case Type::Boolean: {
      bool *bArr = (bool *)arr;
      dRdr.read(bArr,items,"PersistentReader::readBasicArray (bool)");
    }
    break;

    case Type::WChar: {
      wchar_t *wcArr = (wchar_t *)arr;
      dRdr.read(wcArr,items,"PersistentReader::readBasicArray (wchar_t)");
    }
    break;

    case Type::Byte: {
      char *cArr = (char *)arr;
      dRdr.read(cArr,items,"PersistentReader::readBasicArray (char)");
    }
    break;

    case Type::Short: {
      short *sArr = (short *)arr;
      dRdr.read(sArr,items,"PersistentReader::readBasicArray (short)");
    }
    break;

    case Type::Integer: {
      long *iArr = (long *)arr;
      dRdr.read(iArr,items,"PersistentReader::readBasicArray (long)");
    }
    break;

    case Type::Long: {
      __int64 *lArr = (__int64 *)arr;
      dRdr.read(lArr,items,"PersistentReader::readBasicArray (__int64)");
    }
    break;

    case Type::Float: {
      float *fArr = (float *)arr;
      dRdr.read(fArr,items,"PersistentReader::readBasicArray (float)");
    }
    break;

    case Type::Double: {
      double *dArr = (double *)arr;
      dRdr.read(dArr,items,"PersistentReader::readBasicArray (double)");
    }
    break;

    case Type::LongDouble: {
      throw OperationNotSupportedException(
                       "PersistentReader::readBasicArray: long double");
    }
    break;

    case Type::String: {
      wchar_t **strArr = (wchar_t **)arr;

      for (int i=0; i<items; ++i) {
        int strId = dRdr.readInt("PersistentReader::readBasicArray (string)");
        if (strId == 0) strArr[i] = NULL;
        else strArr[i] = stringPool.get(strId);
      }
    }
    break;

    default: throw StreamCorruptedException("PersistentReader::readBasicArray");
  }
}

//---------------------------------------------------------------------------

void PersistentReader::readStructArray(Persistable **arr, int items)
{
  for (int i=0; i<items; ++i) {
    int sId = dRdr.readInt("PersistentReader::readStructArray");

    if (sId == 0) arr[i] = NULL;
    else arr[i] = structPool.get(sId);
  }
}

//---------------------------------------------------------------------------

void PersistentReader::readArrayArray(void * /*arr*/, int /*items*/,
                                                      Array &/*elType*/)
{
  throw OperationNotSupportedException("PersistentReader::readArrayArray");
}

} // namespace Ino

//---------------------------------------------------------------------------
