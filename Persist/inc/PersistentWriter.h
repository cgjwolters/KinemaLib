//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library ----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright AntWorks Engineering Software BV June 2005 --------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef PERSISTENT_OUTPUTSTREAM_INC
#define PERSISTENT_OUTPUTSTREAM_INC

#include "Writer.h"
#include "PersistentTypeDef.h"

//---------------------------------------------------------------------------

namespace AWPersist
{
  class Type;
  class Basic;
  class Field;
  class Struct;
  class Array;

  class OutTypePool;
  class OutStructPool;
  class OutStringPool;
  class OutArrayPool;
  class OutArray;
}

//---------------------------------------------------------------------------

namespace AW
{

using namespace AWPersist;

//---------------------------------------------------------------------------

class PersistentWriter
{
  PersistentTypeDef& tpDef;
  CompressedWriter cWrt;
  DataWriter       dWrt;

  ByteArrayWriter  byteWrt;
  DataWriter       byteDataWrt;

  bool first, compressed;
  bool mustReset;

  void *usrPtr;

  char *errMsg;

  OutTypePool&   typePool;
  OutStructPool& structPool;
  OutStringPool& stringPool;
  OutArrayPool&  arrayPool;

  Struct *curType;

  void writeHeader();

  template <class T> Type& addArrayType(T *a);

  void addArrayField(const char *fldName, const type_info& arrInf,
                                                    const type_info& elInf);
  Field& getValField(const char *fldName, const type_info& inf,
                                                            const char*msg);
  Field& getRefField(const char *fldName);
  Field& getArrayField(const char *fldName, const type_info& inf);

  void writeArrayPrivate(Field& fld, void *arr, int len);

  bool writeNextStruct();
  bool writeNextArray();

  void emitBasicArray(const OutArray& arrObj);
  void emitObjectArray(const OutArray& arrObj);
  void emitArrayArray(const OutArray& arrObj);

  PersistentWriter(const PersistentWriter& cp);
  PersistentWriter& operator=(const PersistentWriter& src);

public:

  PersistentWriter(PersistentTypeDef& typeDef, Writer& wrt,
                                                        bool compress=true);
  ~PersistentWriter();

  bool writeObject(MainPersistable& mps, bool resetWhenDone, void *userPtr=NULL);

  bool isClosed() const { return cWrt.isClosed(); }
  bool isAborted() const { return cWrt.isAborted(); }

  void *getUserPtr() const { return usrPtr; }
  const char *errorMsg() const { return errMsg; }

  void addField(const char *fldName, const type_info& inf);

  template <class T> void addArrayField(const char *fldName, T *a);
  template <class T> void addObjectArrayField(const char *fldName, T **a);
  
  void writeBool(const char *fldName, bool v);
  void writeWChar(const char *fldName, wchar_t wc);
  void writeByte(const char *fldName, char v);
  void writeShort(const char *fldName, short v);
  void writeInt(const char *fldName, long v);
  void writeLong(const char *fldName, _int64 v);
  void writeFloat(const char *fldName, float v);
  void writeDouble(const char *fldName, double v);
  void writeString(const char *fldName, const wchar_t *wc, int wcSz=-1);
  void writeObject(const char *fldName, Persistable *p);
  template <class T> void writeArray(const char *fldName, T *arr, int len);

  friend class OutStructPool;
  friend class OutArrayPool;
};

//---------------------------------------------------------------------------

template <class T>
void PersistentWriter::addArrayField(const char *fldName, T *a)
{
  addArrayField(fldName,typeid(T *),typeid(T));
}

//---------------------------------------------------------------------------

template <class T>
void PersistentWriter::addObjectArrayField(const char *fldName, T **a)
{
  if (!tpDef.get(typeid(T)))
    throw OperationNotSupportedException("Cant write array of Persistable");

  addArrayField(fldName,typeid(T **),typeid(T));
}

//---------------------------------------------------------------------------

template <class T>
  void PersistentWriter::writeArray(const char *fldName, T *arr, int len)
{
  Field& fld = getArrayField(fldName,typeid(T *));
  writeArrayPrivate(fld,arr,len);
}

} // namespace AW

//---------------------------------------------------------------------------
#endif