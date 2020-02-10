//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library ----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef PERSISTENT_IO_INC
#define PERSISTENT_IO_INC

#include "Reader.h"
#include "Writer.h"

#include <typeinfo>

//---------------------------------------------------------------------------

// These are hidden implementation classes:

namespace InoPersist
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

  class OutTypePool;
  class OutStructPool;
  class OutStringPool;
  class OutArrayPool;
  class OutArray;
}

//---------------------------------------------------------------------------

namespace Ino
{

//---------------------------------------------------------------------------

class PersistentReader;
class PersistentWriter;

class Persistable
{
public:
  Persistable() {}
  Persistable(PersistentReader&) {}
  virtual ~Persistable() {}

  virtual void definePersistentFields(PersistentWriter& po) const = 0;
  virtual void writePersistentObject(PersistentWriter& po) const = 0;

  virtual void postProcess(PersistentReader& /*pi*/) {}
  virtual void postProcess(PersistentWriter& /*po*/) const {}
};

//---------------------------------------------------------------------------

class MainPersistable : public Persistable
{
public:
  MainPersistable() : Persistable() {}
  MainPersistable(PersistentReader& pi) : Persistable(pi) {}
  virtual ~MainPersistable() {}

  virtual void readPersistentComplete(PersistentReader& pi) = 0;
};

//---------------------------------------------------------------------------

class PersistentBaseType
{
  PersistentBaseType *nameNext;
  PersistentBaseType *infoNext;

  PersistentBaseType(const PersistentBaseType& cp);             // No Copying
  PersistentBaseType& operator=(const PersistentBaseType& src); // No Assignment

  static bool legalName(const char *name);

protected:
  virtual void construct(Persistable* p, PersistentReader& is) const = 0;

public:
  PersistentBaseType(const char *tpName, const type_info& inf, int tpSz);
  virtual ~PersistentBaseType();

  const type_info&  info;
  const int         sz;
  const char* const name;

  virtual bool canCast(const Persistable *) const { return false; }

  friend class PersistentReader;
  friend class PersistentTypeDef;
};

//---------------------------------------------------------------------------

template <class T> class PersistentType : public PersistentBaseType
{
  PersistentType(const PersistentType& cp);             // No Copying
  PersistentType& operator=(const PersistentType& src); // No Assignment

protected:

  virtual void construct(Persistable* p, PersistentReader& is) const
  { if (!p) throw NullPointerException("PersistentType::construct");
    new (p) T(is);
  }

public:
  PersistentType(const char *tpName)
    : PersistentBaseType(tpName,typeid(T),sizeof(T)) {}

  virtual bool canCast(const Persistable *p) const
                      { return dynamic_cast<const T*>(p) != NULL; }

  friend class PersistentReader;
  friend class PersistentTypeDef;
};

//---------------------------------------------------------------------------

template <class T> class PersistentAbstractType : public PersistentBaseType
{
  PersistentAbstractType(const PersistentAbstractType& cp);             // No Copying
  PersistentAbstractType& operator=(const PersistentAbstractType& src); // No Assignment

protected:

  virtual void construct(Persistable* /*p*/, PersistentReader& /*is*/) const
  {
    throw IllegalStateException("PersistentAbstractType::construct");
  }

public:
  PersistentAbstractType(const char *tpName)
    : PersistentBaseType(tpName,typeid(T),sizeof(T)) {}

  virtual bool canCast(const Persistable *p) const
                      { return dynamic_cast<const T*>(p) != NULL; }

  friend class PersistentReader;
  friend class PersistentTypeDef;
};

//---------------------------------------------------------------------------

class PersistentTypeDef
{
public:
  const long famMagic;
  const long magic;
  const char mmajor;
  const char mminor;

private:
  enum { IncCap = 128 };

  PersistentBaseType **nameLst;
  PersistentBaseType **infoLst;

  int sz,cap;

  mutable bool completed;

  PersistentBaseType *makeChain();
  void incCapacity();

  const PersistentBaseType *find(const type_info& inf) const;

  PersistentTypeDef(const PersistentTypeDef& cp);
  PersistentTypeDef& operator=(const PersistentTypeDef& src);

protected:
  PersistentTypeDef(long famMagicNr, long magicNr, char majorNr, char minorNr);
  virtual ~PersistentTypeDef();

  void add(PersistentBaseType *tp);

public:

  const PersistentBaseType *get(const char *name) const;
  const PersistentBaseType *get(const type_info& inf) const;
};

//---------------------------------------------------------------------------

using namespace InoPersist;

class PersistentReader
{
  PersistentTypeDef& tpDef;
  CompressedReader cRdr;
  DataReader dRdr;

  ByteArrayReader byteRdr;
  DataReader byteDataRdr;

  char mmajor;
  char mminor;

  bool first;
  void *usrPtr;

  char *errMsg;

  InTypePool&    typePool;
  InpStructPool& structPool;
  InpStringPool& stringPool;
  InpArrayPool&  arrayPool;

  Struct *curType;
  int curStructId;
  int curArrayId;

  void readHeader();

  void processEof(MainPersistable *mps, bool reset);

  Field *getValField(const char *fldName, const type_info& inf,
                               const char *msg, bool nullOk);
  Field *getRefField(const char *fldName, bool nullOk);
  Field *getArrayField(const char *fldName, bool nullOk);

  const wchar_t *getString(int strId);

  void readStruct();
  void readArray();

  void readBasicArray(void *arr, int items, Basic &elType);
  void readStructArray(Persistable **arr, int items);
  void readArrayArray(void *arr, int items, InoPersist::Array &elType);

  // No copying or assignment:
  PersistentReader(const PersistentReader& cp);
  PersistentReader& operator=(const PersistentReader& src);

public:
  PersistentReader(PersistentTypeDef& typeDef, Reader& reader);
  ~PersistentReader();

  short getMajor();
  short getMinor();

  bool isEof() const;
  bool isAborted() const;

  MainPersistable *readMainObject(void *userPtr = NULL);

  void *getUserPtr() { return usrPtr; }
  const char *errorMsg() const { return errMsg; }

  bool fieldExists(const char *fldName);

  void callPostProcess();

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

  __int64 readLong(const char *fldName);
  __int64 readLong(const char *fldName, __int64 defVal);
  
  float readFloat(const char *fldName);
  float readFloat(const char *fldName, float defVal);

  double readDouble(const char *fldName);
  double readDouble(const char *fldName, double defVal);

  wchar_t *readString(const char *fldName);
  wchar_t *readString(const char *fldName, wchar_t *defVal);

  Persistable *readObject(const char *fldName);
  Persistable *readObject(const char *fldName, Persistable *defVal);

  int readArraySize(const char *fldName);
  int readArraySize(const char *fldName, int defVal);

  void *readValArray(const char *fldName);
  void *readValArray(const char *fldName, void *defVal);

  Persistable **readObjArray(const char *fldName);
  Persistable **readObjArray(const char *fldName, Persistable **defVal);
};

//---------------------------------------------------------------------------

class PersistentWriter
{
  PersistentTypeDef& tpDef;
  CompressedWriter cWrt;
  DataWriter       dWrt;

  ByteArrayWriter  byteWrt;
  DataWriter       byteDataWrt;

  bool first, compressed;
  void *usrPtr;

  char *errMsg;

  OutTypePool&   typePool;
  OutStructPool& structPool;
  OutStringPool& stringPool;
  OutArrayPool&  arrayPool;

  Struct *curType;
  const Persistable *curObject;

  void writeHeader();

  template <class T> InoPersist::Type& addArrayType(T *a);

  void addArrayField(const char *fldName, const type_info& arrInf,
                                                    const type_info& elInf);
  Field& getValField(const char *fldName, const type_info& inf,
                                                            const char*msg);
  Field& getRefField(const char *fldName);
  Field& getArrayField(const char *fldName, const type_info& inf);

  void writeArrayPrivate(Field& fld, const void *arr, int len);

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

  bool writeMainObject(const MainPersistable& mps, bool resetWhenDone,
                                                          void *userPtr=NULL);

  bool isClosed() const;
  bool isAborted() const;

  void *getUserPtr() const { return usrPtr; }
  const char *errorMsg() const { return errMsg; }

  void addField(const char *fldName, const type_info& inf);
  template <typename T> void addArrayField(const char *fldName, T *a);
  template <typename T> void addObjectArrayField(const char *fldName, T **a);
  
  void callPostProcess();

  void writeBool(const char *fldName, bool v);
  void writeWChar(const char *fldName, wchar_t wc);
  void writeByte(const char *fldName, char v);
  void writeShort(const char *fldName, short v);
  void writeInt(const char *fldName, long v);
  void writeLong(const char *fldName, __int64 v);
  void writeFloat(const char *fldName, float v);
  void writeDouble(const char *fldName, double v);
  void writeString(const char *fldName, const wchar_t *wc, int wcSz=-1);
  void writeObject(const char *fldName, const Persistable *p);
  template <typename T> void writeArray(const char *fldName, const T *arr, int len);

  friend class OutStructPool;
  friend class OutArrayPool;
};

//---------------------------------------------------------------------------

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4100)
#endif

template <typename T>
void PersistentWriter::addArrayField(const char *fldName, T * /*a*/)
{
  addArrayField(fldName,typeid(T *),typeid(T));
}

//---------------------------------------------------------------------------

template <typename T>
void PersistentWriter::addObjectArrayField(const char *fldName, T ** /*a*/)
{
  if (!tpDef.get(typeid(T)))
    throw OperationNotSupportedException("Cant write array of Persistable");

  addArrayField(fldName,typeid(T **),typeid(T));
}

#ifdef WIN32
#pragma warning( pop )
#endif

//---------------------------------------------------------------------------

template <typename T>
  void PersistentWriter::writeArray(const char *fldName, const T *arr, int len)
{
  Field& fld = getArrayField(fldName,typeid(T *));
  writeArrayPrivate(fld,arr,len);
}

} // namespace Ino

//---------------------------------------------------------------------------
#endif
