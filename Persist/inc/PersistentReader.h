//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library ----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright AntWorks Engineering Software BV June 2005 --------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef PERSIST_INPUTSTREAM_INC
#define PERSIST_INPUTSTREAM_INC

#include "Reader.h"
#include "PersistentTypeDef.h"

//---------------------------------------------------------------------------

namespace AWPersist
{
  class Type;
  class Basic;
  class Field;
  class Struct;
  class Array;

  class InTypePool;
  class InpStructPool;
  class InpStringPool;
  class InpArrayPool;
}

//---------------------------------------------------------------------------

namespace AW
{

class Persistable;
class MainPersistable;

using namespace AWPersist;

//---------------------------------------------------------------------------

class PersistentReader
{
  PersistentTypeDef& tpDef;
  CompressedReader cRdr;
  DataReader dRdr;

  ByteArrayReader byteRdr;
  DataReader byteDataRdr;

  char major;
  char minor;

  bool first;
  void *usrPtr;

  char *errMsg;

  InTypePool&    typePool;
  InpStructPool& structPool;
  InpStringPool& stringPool;
  InpArrayPool&  arrayPool;

  Struct *curType;

  void readHeader();

  void processEof(bool reset);

  Field *getValField(const char *fldName, const type_info& inf,
                               const char *msg, bool nullOk);
  Field *getRefField(const char *fldName, bool nullOk);
  Field *getArrayField(const char *fldName, bool nullOk);

  const wchar_t *getString(int strId);

  void readStruct(int recLen, Type& tp);
  void readArray(int recLen, Type& tp);

  void readBasicArray(void *arr, int items, Basic &elType);
  void readStructArray(Persistable **arr, int items, Struct &elType);
  void readArrayArray(void *arr, int items, Array &elType);

  // No copying or assignment:
  PersistentReader(const PersistentReader& cp);
  PersistentReader& operator=(const PersistentReader& src);

public:
  PersistentReader(PersistentTypeDef& typeDef, Reader& reader);
  ~PersistentReader();

  short getMajor();
  short getMinor();

  MainPersistable *readMainObject(void *userPtr = NULL);

  void *getUserPtr() { return usrPtr; }

  bool fieldExists(const char *fldName);

  bool readBool(const char *fldName);
  bool readBool(const char *fldName, bool defVal);

  wchar_t readWChar(const char *fldName);
  wchar_t readWChar(const char *fldName, wchar_t defVal);

  char readByte(const char *fldName);
  char readByte(const char *fldName, char defVal);

  short readShort(const char *fldName);
  short readShort(const char *fldName, short defVal);

  long readInt(const char *fldName);
  long readInt(const char *fldName, long defVal);

  _int64 readLong(const char *fldName);
  _int64 readLong(const char *fldName, _int64 defVal);
  
  float readFloat(const char *fldName);
  float readFloat(const char *fldName, float defVal);

  double readDouble(const char *fldName);
  double readDouble(const char *fldName, double defVal);

  wchar_t *readString(const char *fldName);
  wchar_t *readString(const char *fldName, wchar_t *defVal);

  Persistable *readObject(const char *fldName);
  Persistable *readObject(const char *fldName, Persistable *defVal);

  void *readValArray(const char *fldName, int& arrSz);
  void *readValArray(const char *fldName, int& arrSz, void *defVal);

  Persistable **readObjArray(const char *fldName, int& arrSz);
  Persistable **readObjArray(const char *fldName, int& arrSz,
                                                    Persistable **defVal);
};

} // namespace AW

//---------------------------------------------------------------------------
#endif

