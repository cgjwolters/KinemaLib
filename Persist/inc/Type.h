//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library ----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef PERSIST_TYPE_INC
#define PERSIST_TYPE_INC

#include "PersistentIO.h"

#include <typeinfo>

namespace Ino
{
  class DataReader;
  class DataWriter;
  class ByteArrayWriter;
}

namespace InoPersist
{

using namespace Ino;

class InTypePool;

enum RecordType { Record_Eof        = 0,
                  Record_EofReset   = 1,
                  Record_Type       = 2,
                  Record_TypeDef    = 3,
                  Record_StructDecl = 4,
                  Record_ArrayDecl  = 5,
                  Record_String     = 6,
                  Record_Struct     = 7,
                  Record_Array      = 8 };

enum { FormatMajor = 1, FormatMinor = 0 };

//-------------------------------------------------------------------------

class Type
{
  Type *next;

public:
  enum Category { CatBasic, CatStruct, CatArray };

  enum DataType { Boolean = -1,  WChar   = -2,  Byte       = -3,
                  Short   = -4,  Integer = -5,  Long       = -6,
                  Float   = -7,  Double  = -8,  LongDouble = -9,
                  String  = -10, ArrayType = 0xFF80 };

private:
  Type(const Type& cp);             // No Copying
  Type& operator=(const Type& src); // No Assignment

protected:

  Type(short typeId) : id(typeId) {}

public:
  virtual ~Type() {}

  const short id;

  virtual Category getCategory() const = 0;
  virtual const char* getHashName() = 0;

  virtual int getDataSize() const = 0; // Size as field of struct

  virtual bool isRefType() const = 0;
  virtual const type_info& getInfo() const = 0;

  virtual void write(DataWriter& dWrt) const = 0;

  static int readRecordLen(DataReader& dRdr, const char *exceptMsg=NULL);
  static void writeRecordLen(DataWriter& dWrt, int len, const char *exceptMsg=NULL);

  friend class OutTypePool;
};

//-------------------------------------------------------------------------

class Basic: public Type
{
  Basic(const Basic& cp);             // No Copying
  Basic& operator=(const Basic& src); // No Assignment

public:
  Basic(short typeId, DataType dTp) : Type(typeId), dataType(dTp) {}
  virtual ~Basic() {}

  const DataType dataType;
  
  virtual Category getCategory() const { return CatBasic; }
  virtual const char* getHashName();

  virtual int getDataSize() const;

  virtual bool isRefType() const { return false; }
  virtual const type_info& getInfo() const;

  virtual void write(DataWriter& dWrt) const;
};

//-------------------------------------------------------------------------

class Field
{
  int offset;

  Field(const Field& cp);             // No Copying
  Field& operator=(const Field& src); // No Assignment

public:
  Field(const char *fldName, const Type *fieldType);
  ~Field();

  const char *const name;
  const Type* type;

  int getOffset() const { return offset; }

  friend class Struct;
};

//-------------------------------------------------------------------------

class FieldList
{
  enum { CapInc = 16 };

  Field **fldLst;
  short sz, cap;

  void insert(short idx, Field *newFld);

  FieldList(const FieldList& cp);
  FieldList& operator=(const FieldList& src);

public:
  FieldList();
  ~FieldList();

  short size() const { return sz; }
  short find(const char *name) const;

  void add(const char *fldName, const Type *fieldType);

  const Field& operator[](short idx) const;
  Field& operator[](short idx);

  const Field *operator[](const char *name) const;
  Field *operator[](const char *name);
};

//-------------------------------------------------------------------------

class Struct : public Type
{
  FieldList valList;
  FieldList refList;

  bool defined;
  int structSz;

  void calcOffsets();

  Struct(const Struct& cp);             // No Copying
  Struct& operator=(const Struct& src); // No Assignment

public:
  Struct(short typeId, const PersistentBaseType& baseTp);
  virtual ~Struct() {}

  const PersistentBaseType& baseType;

  bool isDefined() { return defined; }

  virtual Category getCategory() const { return CatStruct; }

#ifndef WIN32
  virtual const char* getHashName() { return baseType.info.name(); }
#else
  virtual const char* getHashName() { return baseType.info.raw_name(); }
#endif

  virtual int getDataSize() const { return 4; }

  virtual bool isRefType() const { return true; }
  virtual const type_info& getInfo() const { return baseType.info; }

  int getStructSize() const;

  void addField(const char *fldName, Type& fldType);
  
  int getValFldCnt() const { return valList.size(); }
  int getRefFldCnt() const { return refList.size(); }

  Field& getValField(short idx) { return valList[idx]; }
  Field& getRefField(short idx) { return refList[idx]; }

  Field *getValField(const char *fldName) { return valList[fldName]; }
  Field *getRefField(const char *fldName) { return refList[fldName]; }

  virtual void write(DataWriter& dWrt) const;

  void writeDef(ByteArrayWriter& byteWrt);
  void readDef(DataReader& dRdr, const InTypePool& pool);
};

//-------------------------------------------------------------------------

class Array : public Type
{
  Array(const Array& cp);             // No Copying
  Array& operator=(const Array& src); // No Assignment

public:
  Array(short typeId, const type_info& inf, Type& elType)
      : Type(typeId), typeInf(inf), elemType(elType) {}
  virtual ~Array() {}

  const type_info& typeInf;
  Type& elemType;

  virtual Category getCategory() const { return CatArray; }

#ifndef WIN32
  virtual const char* getHashName() { return typeInf.name(); }
#else
  virtual const char* getHashName() { return typeInf.raw_name(); }
#endif

  virtual int getDataSize() const { return 4; }

  virtual bool isRefType() const { return true; }
  virtual const type_info& getInfo() const { return typeInf; }
  virtual void write(DataWriter& dWrt) const;
};

} // namespace InoPersist

//---------------------------------------------------------------------------
#endif
