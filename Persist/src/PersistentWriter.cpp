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
#include "OutPools.h"

#include <cstring>

namespace Ino
{

//---------------------------------------------------------------------------
/** \addtogroup persistence Persistence
 @{
*/

/** \class PersistentWriter
  A PersistentWriter is used to write a MainPersistable and all connected
  Persistable objects to a stream written by a Writer.
  
  The constructor must be passed an instance of a user defined data
  definition class derived from PersistentTypeDef.

  Then a call to method writeMainObject() is all that is needed to write
  (serialize) the MainPersistable class and all related classes to the
  stream.

  More than one MainPersistable may be written to the same stream,
  then a matching number of PersistentReader::readMainObject() calls
  will be required to restore all data.

  \author C. Wolters
  \date June 2005
*/

//---------------------------------------------------------------------------
/** 
 @}
*/

//---------------------------------------------------------------------------

void PersistentWriter::writeHeader()
{
  cWrt.setCompressing(false);

  dWrt.writeInt(tpDef.famMagic);
  dWrt.writeInt(tpDef.magic);
  dWrt.writeByte(tpDef.mmajor);
  dWrt.writeByte(tpDef.mminor);
  dWrt.writeByte(FormatMajor);  // Internal version indicator for this sfw
  dWrt.writeByte(FormatMinor);  // Internal version indicator for this sfw
  dWrt.writeBool(compressed);

  cWrt.setCompressing(compressed);

  first = false;
}

//---------------------------------------------------------------------------

void PersistentWriter::addArrayField(const char *fldName,
                                     const type_info& arrInf,
                                     const type_info& elInf)
{
  if (!curType)
         throw IllegalStateException("PersistentWriter::addArrayField");

  if (!typePool.isBasicType(elInf) && !tpDef.get(elInf))
      throw OperationNotSupportedException("Not a valid array element type");

  Type& elTp  = typePool.add(elInf);
  Type& arrTp = typePool.addArray(arrInf,elTp);

  curType->addField(fldName,arrTp);
}

//---------------------------------------------------------------------------

Field& PersistentWriter::getValField(const char *fldName,
                                           const type_info& inf,
                                           const char* msg)
{
  if (!curType )
        throw IllegalStateException("PersistentWriter::getValField");

  const char *clsName = curType->baseType.name;

  Field *fld = curType->getValField(fldName);

  if (!fld) {
    char msgBuf[512];
    sprintf(msgBuf,"No such field: \"%s\" (in class \"%s\"",fldName,clsName);
    throw IllegalArgumentException(msgBuf);
  }

  if (!fld->type || fld->type->getInfo() != inf) {
    char msgBuf[512];
    sprintf(msgBuf,"Field \"%s\" is not a %s in class \"%s\"",
                                                        fldName,msg,clsName);
    throw IllegalArgumentException(msgBuf);
  }

  return *fld;
}

//---------------------------------------------------------------------------

Field& PersistentWriter::getRefField(const char *fldName)
{
  if (!curType)
     throw IllegalStateException("PersistentWriter::getRefField");

  Field *fld = curType->getRefField(fldName);

  if (!fld) {
    const char *clsName = curType->baseType.name;

    char msg[512];
    sprintf(msg,"No such field: \"%s\" (in class \"%s\"",fldName,clsName);
    throw IllegalArgumentException(msg);
  }

  return *fld;
}

//---------------------------------------------------------------------------

Field& PersistentWriter::getArrayField(const char *fldName,
                                             const type_info& inf)
{
  if (!curType)
     throw IllegalStateException("PersistentWriter::getArrayField");

  Field *fld = curType->getRefField(fldName);

  if (!fld) {
    char msg[512];
    sprintf(msg,"No such field: \"%s\" (in class \"%s\"",fldName,inf.name());
    throw IllegalArgumentException(msg);
  }

  if (fld->type) {
    if (fld->type->getCategory() != Type::CatArray ||
                                            fld->type->getInfo() != inf) {
      char msg[512];
      sprintf(msg,"Field \"%s\" is not an array in class \"%s\"",
                                                    fldName,inf.name());
      throw IllegalArgumentException(msg);
    }
  }

  return *fld;
}

//---------------------------------------------------------------------------

bool PersistentWriter::writeNextStruct()
{
  int structId = 0;
  const Persistable *p = dynamic_cast<const Persistable *>(structPool.getNext(structId));
  if (!p) return false;

  Type& tp = typePool.add(typeid(*p));
  if (tp.getCategory() != Type::CatStruct)
       throw IllegalStateException("PersistentWriter::writeNextStruct");

  curType = dynamic_cast<Struct *>(&tp);

  if (!curType->isDefined()) {
    p->definePersistentFields(*this);

    curType->writeDef(byteWrt);

    dWrt.writeByte(Record_TypeDef);
    Type::writeRecordLen(dWrt,byteWrt.getSize(),
                               "PersistentWriter::writeNextStruct");
    dWrt.write(byteWrt.getBuffer(),byteWrt.getSize(), 
                               "PersistentWriter::writeNextStruct");
  }

  int sSz = curType->getStructSize();
  byteWrt.ensureCap(sSz);
  byteWrt.clearArray();

  byteWrt.setSize(0);

  curObject = p;
  p->writePersistentObject(*this);
  curObject = NULL;

  dWrt.writeByte(Record_Struct);
  Type::writeRecordLen(dWrt,sSz);
  dWrt.write(byteWrt.getBuffer(),sSz,"PersistentWriter::writeStruct");

  curType = NULL;
  
  return true;
}

//---------------------------------------------------------------------------

bool PersistentWriter::writeNextArray()
{
  const OutArray *arrObj = arrayPool.getNext();
  if (!arrObj) return false;

  switch (arrObj->type.elemType.getCategory()) {
    case Type::CatBasic:
      emitBasicArray(*arrObj);
      break;

    case Type::CatStruct:
      emitObjectArray(*arrObj);
      break;

    case Type::CatArray:
      emitArrayArray(*arrObj);
      break;

    default: throw IllegalStateException("writeNextArray");
  }

  return true;
}

//---------------------------------------------------------------------------
/** Constructor.
  \param typeDef The data definition class derived from
  PersistentTypeDef to be used.
  \param wrt The Writer that is used to write the stream data.
  \param compress It \c true (the default) the data will be compressed
  with \c zlib.

  There must be a \link PersistentTypeDef data definition class\endlink
  for each stream format,\n
  and the same data definition class must be used to both write and
  read the stream.
*/

PersistentWriter::PersistentWriter(PersistentTypeDef& typeDef,
                                               Writer& wrt, bool compress)
: tpDef(typeDef), cWrt(wrt,4096), dWrt(cWrt),
  byteWrt(1024,1024), byteDataWrt(byteWrt),
  first(true), compressed(compress),
  usrPtr(NULL), errMsg(NULL),
  typePool(*new OutTypePool(typeDef,dWrt)),
  structPool(*new OutStructPool(typeDef,dWrt)),
  stringPool(*new OutStringPool(dWrt)),
  arrayPool(*new OutArrayPool(dWrt)),
  curType(NULL), curObject(NULL)
{
}

//---------------------------------------------------------------------------
/** Destructor.
  Cleans up all internal datastructures related to the stream.
*/

PersistentWriter::~PersistentWriter()
{
  if (errMsg) delete[] errMsg;

  delete &arrayPool;
  delete &stringPool;
  delete &structPool;
  delete &typePool;
}

//---------------------------------------------------------------------------
/** Returns open/closed status of the underlying Writer.

  \return \c true If the underlying Writer reports it is closed,\n
  \c false otherwise.
*/

bool PersistentWriter::isClosed() const
{
  return cWrt.isClosed();
}

//---------------------------------------------------------------------------
/** Returns the abort state of the stream.

   \return \c true if the underlying Writer indicates so,
   \c false otherwise.

   The used Writer object may have a ProgressReporter, and that class allows
   the writing of a stream to be prematurely
   \ref ProgressReporter::mustAbort() "aborted".
*/

bool PersistentWriter::isAborted() const
{
  return cWrt.isAborted();
}

//---------------------------------------------------------------------------
/** Writes (serializes) a MainPersistable and all related classes.

   Writes a MainPersistable and all classes reachable from it
   to the stream.\n
   Every class instance is written to the stream exactly once.

   \param mps The MainPersistable to serialize to the stream.

   \param resetWhenDone If \c true this PersistentWriter will
   erase all information about which class instances have been
   written to the stream just before return from this method,\n
   if \c false this information is kept and will be used
   for a next call to this method.\n\n
   In most cases cases \c true should be specified.\n
   For more detail see below.

   \param userPtr A user defined opaque value that can be obtained
   (using getUserPtr()) from within the Persistable methods while
   each class is being written.\n
   This may for instance be used to adapt the behavior while the
   classes are being written.

   \return \c true If the MainPersistable was successfully
   written to the stream,\n
   \c false otherwise.\n\n
   In the latter case method errorMsg() may be called to obtain more
   information.\n\n
   Internally all exceptions thrown are caught and converted to
   an appropriate error message.

   \note In order to be compatible with all types of computer
   hardware all fields of basic types are written in
   <em>network byte order</em>

   \par <b>Reset When Done</b>

   This methods builds up a list of class instances and arrays
   that have been written to the stream as the graph of
   class instances and array elements is traversed.

   Before each instance of a Persistable class or array element
   is written to the stream the list is checked, and if the
   instance was already written, it will not be written again.

   If flag \p resetWhenDone is \c true this list is cleared
   just before the return from this method.\n
   A subsequent call to this method will then start afresh.

   The list is not cleared if flag \c resetWhenDone is \c false.\n
   As a consequence, if a subsequent call to this method writes
   another MainPersistable that shares reachable class instances
   with the previous object written, then those shared instances
   will not be written again.

   This feature can for instance be used as follows:

   A MainPersistable derived class called \c Drawing is the main
   class of quite a large CAD drawing.\n
   The \c Drawing class has a field that points to a much
   smaller class called \c SummaryInfo (a Persistable).

   Now before calling writeMainObject() on the \c Drawing
   instance first a call to writeMainObject() is made on the
   \c SummaryInfo class instance with flag \c resetWhenDone
   set to \c false.\n
   Then writeMainObject() is called on the \c Drawing objects.\n
   (This will \b not cause the \c SummaryInfo instance to be
   written again).

   A PersistentReader can now be used to
   restore only the \c SummaryInfo object by calling
   PersistentReader::readMainObject() just once.\n
   Thus a listing of a set of such files (holding the streams)
   can quickly be generated without having to read the
   entire contents of each file.

   If the \c Drawing object is wanted a subsequent call
   to PersistentReader::readMainObject() will restore the
   \c Drawing object and that will also hold the
   very same \c SummaryInfo instance that was read earlier.

   Since this method also writes the state of the \c resetWhenDone
   flag to the stream, the PersistentReader will know
   what to do at the end of each call
   to PersistentReader::readMainObject().

   \note
   Since classes are persisted to a stream this library may
   also be used to pass (complicated) messages back and forth
   between two network applications.\n\n
   Be aware that in that case the flag \c resetWhenDone must be
   set to \c true.\n
   Otherwise the same instance of a class  will not be written
   again (not even if it was modified to hold a different message).\n\n
   Also the list of instances kept by this class could grow
   without bound if \c resetWhenDone were not set to \c true.
*/

bool PersistentWriter::writeMainObject(const MainPersistable& mps,
                                         bool resetWhenDone, void *userPtr)
{
  if (errMsg) {
    delete[] errMsg;
    errMsg = NULL;
  }

  usrPtr  = userPtr;

  curType = NULL;

  bool ok = true;

  try {
    if (first) writeHeader(); // Write uncompressed header

    // The rest is written with compression (if required)

    Type& mainTp = typePool.add(typeid(mps));
    structPool.get(&mps,mainTp.id);

    while (writeNextStruct()) {}
    while (writeNextArray()) {}

    structPool.postProcess(*this);

    if (resetWhenDone) {
      dWrt.writeByte(Record_EofReset);

      first = true;

      arrayPool.clear();
      stringPool.clear();
      structPool.clear();
      typePool.clear();
    }
    else dWrt.writeByte(Record_Eof);
  }
  catch (exception& ex) {
    arrayPool.clear();
    stringPool.clear();
    structPool.clear();
    typePool.clear();

    if (cWrt.isAborted()) errMsg = dupStr("StreamAbortedException");
    else if (cWrt.isClosed()) errMsg = dupStr("StreamClosedException");
    else {
      char msgBuf[1024];
      int msgLen = sprintf(msgBuf,"%s: %s",typeid(ex).name(),ex.what());

      errMsg = new char[msgLen+1];
      strcpy(errMsg,msgBuf);
    }

    ok = false;
  }

  bool fOk = cWrt.flush();
  ok = ok && fOk;

  return ok;
}

//---------------------------------------------------------------------------
/** \fn void *PersistentWriter::getUserPtr() const
  Returns the value passed to readMainObject().

  May called from within one of the \c write() methods.

  \return The user pointer.
*/

//---------------------------------------------------------------------------
/** \fn const char *PersistentWriter::errorMsg() const
  Returns the error message if any.

  \return The error message if writeMainObject() returned <tt>false</tt>\n
  or \c NULL if there was no error.

  \note That all thrown exceptions are caught internally and will result
  in an error message that can be retrieved with this method.
*/

//---------------------------------------------------------------------------
/** Defines one field to be persisted.

  \param fldName The name of the field.
  \param inf The \c typeid() of the field.

  Use this method to define a fields of any type except an array.

  Examples:\n
  <tt>addField("IntFld",typeid(int))</tt> Defines an \c int field.\n
  <tt>addField("StrFld",typeid(wchar_t *))</tt> Defines a unicode string field.\n
  <tt>addField("ObjFld",typeid(MyClass))</tt> Defines a Persistable
  field.\n

  \note That a Persistable field must be a reference or pointer
  to a class derived from Persistable (it cannot be an embedded field).\n\n
  Note also however that it is <tt>typeid(MyClass))</tt> and \b not
  <tt>typeid(MyClass *))</tt>

  \see Persistable::definePersistentFields()

  \throw IllegalStateException If this method is called other than from
  within method Persistable::definePersistentFields().

  \note That exceptions are internally caught, see errorMsg().
*/

void PersistentWriter::addField(const char *fldName, const type_info& inf)
{
  if (!curType)
         throw IllegalStateException("PersistentWriter::addField");

  if (inf == typeid(int)) {
    Type &tp = typePool.add(typeid(long));
    curType->addField(fldName,tp);
  }
  else {
    Type &tp = typePool.add(inf);
    curType->addField(fldName,tp);
  }
}

//---------------------------------------------------------------------------
/** \fn void PersistentWriter::addArrayField(const char *fldName, T *a);
  Defines one array field to be persisted,
  used for all array element types except Persistable.

  \param fldName The name of the field.
  \param a Must be of the correct type, but may be <tt>NULL</tt>.

  Examples:\n
  <tt>addArrayField("IntFld",(int *)NULL)</tt> Defines an array of <tt>int</tt>.\n
  <tt>addArrayField("StrFld",(wchar_t **)NULL)</tt> Defines an
  array of strings (<tt>wchar_t *</tt>)\n

  An attempt to declare an array of Persistable will fail,
  use addObjectArrayField() to do that.

  \see Persistable::definePersistentFields()

  \throw IllegalStateException If this method is called other than from
  within method Persistable::definePersistentFields().

  \note That exceptions are internally caught, see errorMsg().
*/

//---------------------------------------------------------------------------
/** \fn void PersistentWriter::addObjectArrayField(const char *fldName, T **a);
  Defines one array field to be persisted,
  used for array element types derived from Persistable.

  \param fldName The name of the field.
  \param a Must be of the correct type, but may be <tt>NULL</tt>.

  Example:\n
  <tt>addObjectArrayField("ObjFld",(MyClass **)NULL)</tt>
  Defines an array of <tt>MyClass **</tt>\n\n

  \b Note that is \b not possible to persist an array of objects, \b only an
  array of object \b pointers.\n
  I.e. <tt>po.addObjectArrayField("ObjArr",(MyClass *)NULL);</tt> will \b not work.\n\n
  One reason is that an array of objects is not supported in languages such as Java,
  .NET and Delphi and being compatible with those is a goal.

  \see Persistable::definePersistentFields()

  \throw IllegalStateException If this method is called other than from
  within method Persistable::definePersistentFields().

  \note That exceptions are internally caught, see errorMsg().
*/

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/** This method is \b seriously deprecated, <b>do not use!</b>
*/

void PersistentWriter::callPostProcess()
{
  if (!curType || !curObject)
         throw IllegalStateException("PersistentWriter::callPostProcess");

  structPool.setPostProcess(curObject);
}

//---------------------------------------------------------------------------
/** Writes a boolean field to the stream.

  \param fldName The name of the field.
  \param v The value to be written for this field.

  \throw IllegalStateException If this method is called other than from
  within Persistable::writePersistentObject().

  \throw IllegalArgumentException If a field of the specified name does
  not exist or if its type is not \c bool.

  \note That exceptions are internally caught, see errorMsg().
*/

void PersistentWriter::writeBool(const char *fldName, bool v)
{
  Field& fld = getValField(fldName,typeid(bool),"Boolean");

  byteWrt.setPos(fld.getOffset());
  byteDataWrt.writeBool(v);
}

//---------------------------------------------------------------------------
/** Writes a single \c wchar_t to the stream.

  \param fldName The name of the field.
  \param wc The value to be written for this field.

  \throw IllegalStateException If this method is called other than from
  within Persistable::writePersistentObject().


  \throw IllegalArgumentException If a field of the specified name does
  not exist or if its type is not \c wchar_t.

  \note That exceptions are internally caught, see errorMsg().
*/

void PersistentWriter::writeWChar(const char *fldName, wchar_t wc)
{
  Field& fld = getValField(fldName,typeid(wchar_t),"WChar");

  byteWrt.setPos(fld.getOffset());
  byteDataWrt.writeWChar(wc);
}

//---------------------------------------------------------------------------
/** Writes a single \c char (byte) to the stream.

  \param fldName The name of the field.
  \param v The value to be written for this field.

  \throw IllegalStateException If this method is called other than from
  within Persistable::writePersistentObject().

  \throw IllegalArgumentException If a field of the specified name does
  not exist or if its type is not \c char.

  \note That exceptions are internally caught, see errorMsg().
*/

void PersistentWriter::writeByte(const char *fldName, char v)
{
  Field& fld = getValField(fldName,typeid(char),"Byte");

  byteWrt.setPos(fld.getOffset());
  byteDataWrt.writeByte(v);
}

//---------------------------------------------------------------------------
/** Writes a \c short to the stream.

  \param fldName The name of the field.
  \param v The value to be written for this field.

  \throw IllegalStateException If this method is called other than from
  within Persistable::writePersistentObject().

  \throw IllegalArgumentException If a field of the specified name does
  not exist or if its type is not \c short.

  \note That exceptions are internally caught, see errorMsg().
*/

void PersistentWriter::writeShort(const char *fldName, short v)
{
  Field& fld = getValField(fldName,typeid(short),"Short");

  byteWrt.setPos(fld.getOffset());
  byteDataWrt.writeShort(v);
}

//---------------------------------------------------------------------------
/** Writes an \c int or \c long (32 bit) to the stream.

  \param fldName The name of the field.
  \param v The value to be written for this field.

  \throw IllegalStateException If this method is called other than from
  within Persistable::writePersistentObject().

  \throw IllegalArgumentException If a field of the specified name does
  not exist or if its type is not \c int or \c long.

  \note That exceptions are internally caught, see errorMsg().
*/

void PersistentWriter::writeInt(const char *fldName, long v)
{
  Field& fld = getValField(fldName,typeid(long),"Integer");

  byteWrt.setPos(fld.getOffset());
  byteDataWrt.writeInt(v);
}

//---------------------------------------------------------------------------
/** Writes a <tt>long long</tt> (64 bit) to the stream.

  Type <tt>long long</tt> is also known as \c __int64.

  \param fldName The name of the field.
  \param v The value to be written for this field.

  \throw IllegalStateException If this method is called other than from
  within Persistable::writePersistentObject().

  \throw IllegalArgumentException If a field of the specified name does
  not exist or if its type is not <tt>long long</tt>.

  \note That exceptions are internally caught, see errorMsg().
*/

void PersistentWriter::writeLong(const char *fldName, __int64 v)
{
  Field& fld = getValField(fldName,typeid(__int64),"Long");

  byteWrt.setPos(fld.getOffset());
  byteDataWrt.writeLong(v);
}

//---------------------------------------------------------------------------
/** Writes a \c float (32 bit) to the stream.

  \param fldName The name of the field.
  \param v The value to be written for this field.

  \throw IllegalStateException If this method is called other than from
  within Persistable::writePersistentObject().

  \throw IllegalArgumentException If a field of the specified name does
  not exist or if its type is not \c float.

  \note That exceptions are internally caught, see errorMsg().
*/

void PersistentWriter::writeFloat(const char *fldName, float v)
{
  Field& fld = getValField(fldName,typeid(float),"Float");

  byteWrt.setPos(fld.getOffset());
  byteDataWrt.writeFloat(v);
}

//---------------------------------------------------------------------------
/** Writes a \c double (64 bit) to the stream.

  \param fldName The name of the field.
  \param v The value to be written for this field.

  \throw IllegalStateException If this method is called other than from
  within Persistable::writePersistentObject().

  \throw IllegalArgumentException If a field of the specified name does
  not exist or if its type is not \c double.

  \note That exceptions are internally caught, see errorMsg().
*/

void PersistentWriter::writeDouble(const char *fldName, double v)
{
  Field& fld = getValField(fldName,typeid(double),"Double");

  byteWrt.setPos(fld.getOffset());
  byteDataWrt.writeDouble(v);
}

//---------------------------------------------------------------------------
/** Writes a string (a <tt>wchar_t *</tt>) to the stream.

  \param fldName The name of the field.
  \param wc The string to be written for this field, may be \c NULL.\n
  The string is written including the trailing null character,\b
  but see parameter \p wcSz.
  \param wcSz The number of characters to write or the entire string
  if \p wcSz is -1 (the default).

  \throw IllegalStateException If this method is called other than from
  within Persistable::writePersistentObject().

  \throw IllegalArgumentException If a field of the specified name does
  not exist or if its type is not <tt>wchar_t *</tt>.

  \note That exceptions are internally caught, see errorMsg().

  \attention Pointer \c wc \b must remain valid until method
  writeMainObject() returns!\n
*/

void PersistentWriter::writeString(const char *fldName,
                                           const wchar_t *wc, int wcSz)
{
  if (!wc) wcSz = 0;
  else if (wcSz < 0) wcSz = wcslen(wc);

  Field& fld = getValField(fldName,typeid(wchar_t *),"String");

  byteWrt.setPos(fld.getOffset());

  int id = stringPool.get(wc,wcSz);
  byteDataWrt.writeInt(id);
}

//---------------------------------------------------------------------------
/** Writes an object instance (derived from Persistable) to the stream.

  \param fldName The name of the field.
  \param p A pointer to the Persistable to be written for this field,
  may be \c NULL.

  \throw IllegalStateException If this method is called other than from
  within Persistable::writePersistentObject().

  \throw IllegalArgumentException If a field of the specified name does
  not exist or if its type is not the type defined in method
  addField().

  \note That exceptions are internally caught, see errorMsg().

  \attention Pointer \c p \b must remain valid until method
  writeMainObject() returns!\n
*/

void PersistentWriter::writeObject(const char *fldName, const Persistable *p)
{
  Field& fld = getRefField(fldName);

  if (p) {
    const char *clsName = curType->getInfo().name();

    if (!fld.type) {
      char msg[512];
      sprintf(msg,"Type of field \"%s\" is not defined in class \"%s\"",
                                                          fldName,clsName);
      throw IllegalArgumentException(msg);
    }

    const Struct *fldType = dynamic_cast<const Struct *>(fld.type);

    if (!fldType) {
      char msg[512];
      sprintf(msg,"Type of field \"%s\" is not a struct type in class \"%s\"",
                                                           fldName,clsName);
      throw IllegalArgumentException(msg);
    }

    if (!fldType->baseType.canCast(p)) {
      char msg[512];
      sprintf(msg,"Field \"%s\" is not a \"%s\" in class \"%s\"",
                                    fldName,fldType->baseType.name,clsName);
      throw IllegalArgumentException(msg);
    }
  }

  byteWrt.setPos(fld.getOffset());

  if (!p) byteDataWrt.writeInt(0);
  else {
    if (fld.type->getInfo() == typeid(*p)) {
      int id = structPool.get(p,fld.type->id);
      byteDataWrt.writeInt(id);
    }
    else {
      Type& tp = typePool.add(typeid(*p));

      if (tp.getCategory() != Type::CatStruct)
         throw IllegalStateException("PersistentWriter::writeObject");

      int id = structPool.get(p,tp.id);
      byteDataWrt.writeInt(id);
    }
  }
}

//---------------------------------------------------------------------------
/** \fn void PersistentWriter::writeArray(const char *fldName, const T *arr, int len)
  Writes an array to the stream.

  \param fldName The name of the field.
  \param arr The array to be written, may be \c NULL.
  \param len The number of array elements to write.

  \throw IllegalStateException If this method is called other than from
  within Persistable::writePersistentObject().

  \throw IllegalArgumentException If a field of the specified name does
  not exist or if its type is not the type defined in method
  \link addArrayField(const char *fldName, T *a) addArrayField()\endlink
  or addObjectArrayField().

  \note That exceptions are internally caught, see errorMsg().

  \attention Array \c arr \b must remain valid until method
  writeMainObject() returns!\n
*/

//---------------------------------------------------------------------------

void PersistentWriter::writeArrayPrivate(Field& fld, const void *arr, int len)
{
  byteWrt.setPos(fld.getOffset());

  if (!arr) byteDataWrt.writeInt(0);
  else {
    if (!fld.type) {
      const char *clsName = curType->getInfo().name();

      char msg[512];
      sprintf(msg,"Type of array \"%s\" is not defined in class \"%s\"",
                                                          fld.name,clsName);
      throw IllegalArgumentException(msg);
    }

    const Array& arrTp = dynamic_cast<const Array&>(*fld.type);

    int id = arrayPool.get(arr,len,arrTp);
    byteDataWrt.writeInt(id);
  }
}

//---------------------------------------------------------------------------

void PersistentWriter::emitBasicArray(const OutArray& arrObj)
{
  const Basic& elTp = dynamic_cast<const Basic&>(arrObj.type.elemType);

  if (elTp.dataType == Type::String) {
    // Make sure the strings are emitted first;

    const wchar_t **strArr = (const wchar_t **)arrObj.arr;

    for (int i=0; i<arrObj.sz; ++i) {
      const wchar_t *s = strArr[i];
      if (!s) continue;

      int sLen = wcslen(s);
      stringPool.get(s,sLen);
    }
  }

  dWrt.writeByte(Record_Array,"PersistentWriter::emitBasicArray");
  Type::writeRecordLen(dWrt,arrObj.sz*elTp.getDataSize());

  switch (elTp.dataType) {
    case Type::Boolean: {
      const bool *bArr = (const bool *)arrObj.arr;
      dWrt.write(bArr,arrObj.sz,"PersistentWriter::emitBasicArray: bool");
    }
    break;

    case Type::WChar: {
      const wchar_t *wcArr = (const wchar_t *)arrObj.arr;
      dWrt.write(wcArr,arrObj.sz,"PersistentWriter::emitBasicArray: wchar_t");
    }
    break;

    case Type::Byte: {
      const char *cArr = (const char *)arrObj.arr;
      dWrt.write(cArr,arrObj.sz,"PersistentWriter::emitBasicArray: char");
    }
    break;

    case Type::Short: {
      const short *sArr = (const short *)arrObj.arr;
      dWrt.write(sArr,arrObj.sz,"PersistentWriter::emitBasicArray: short");
    }
    break;

    case Type::Integer: {
      const long *iArr = (const long *)arrObj.arr;
      dWrt.write(iArr,arrObj.sz,"PersistentWriter::emitBasicArray: integer");
    }
    break;

    case Type::Long: {
      const __int64 *lArr = (const __int64 *)arrObj.arr;
      dWrt.write(lArr,arrObj.sz,"PersistentWriter::emitBasicArray: long");
    }
    break;

    case Type::Float: {
      const float *fArr = (const float *)arrObj.arr;
      dWrt.write(fArr,arrObj.sz,"PersistentWriter::emitBasicArray: float");
    }
    break;

    case Type::Double: {
      const double *dArr = (const double *)arrObj.arr;
      dWrt.write(dArr,arrObj.sz,"PersistentWriter::emitBasicArray: double");
    }
    break;

    case Type::LongDouble: {
      throw OperationNotSupportedException("PersistentWriter::emitBasicArray");
    }
    break;

    case Type::String: {
      const wchar_t **strArr = (const wchar_t **)arrObj.arr;

      for (int i=0; i<arrObj.sz; ++i) {
        const wchar_t *s = strArr[i];

        if (!s) dWrt.writeInt(0,"PersistentWriter::emitBasicArray");
        else {
          int sLen = wcslen(s);
          int id = stringPool.get(s,sLen);
          dWrt.writeInt(id,"PersistentWriter::emitBasicArray");
        }
      }
    }
    break;

    default: throw IllegalStateException("PersistentWriter::emitBasicArray");
  }
}

//---------------------------------------------------------------------------

void PersistentWriter::emitObjectArray(const OutArray& arrObj)
{
  const Struct& elTp = dynamic_cast<const Struct&>(arrObj.type.elemType);

  Persistable **objArr = (Persistable **)arrObj.arr;

  // Make sure types and struct decls are all emitted first

  for (int i=0; i<arrObj.sz; ++i) {
    Persistable *p = objArr[i];

    if (p) {
      Type& tp = typePool.add(typeid(*p));
      structPool.get(p,tp.id);
    }
  }

  dWrt.writeByte(Record_Array,"PersistentWriter::emitObjectArray");
  Type::writeRecordLen(dWrt,arrObj.sz*elTp.getDataSize());

  for (int i=0; i<arrObj.sz; ++i) {
    Persistable *p = objArr[i];

    if (!p) dWrt.writeInt(0,"PersistentWriter::emitObjectArray");
    else {
      Type *tp = typePool.get(typeid(*p));
      if (!tp) throw IllegalStateException("PersistentWriter::emitObjectArray");

      int id = structPool.get(p,tp->id);
      dWrt.writeInt(id,"PersistentWriter::emitObjectArray");
    }
  }

  while (writeNextStruct()) {} // Emit all resulting structs
}

//---------------------------------------------------------------------------

void PersistentWriter::emitArrayArray(const OutArray& /*arrObj*/)
{
  throw OperationNotSupportedException("PersistentWriter::emitArrayArray");
}


} // namespace Ino

//---------------------------------------------------------------------------
