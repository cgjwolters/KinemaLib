//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library: Object Pools --------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef PERSIST_INPPOOLS_INC
#define PERSIST_INPPOOLS_INC

#include "Exceptions.h"

//---------------------------------------------------------------------------

namespace Ino
{
  class Persistable;
  class PersistentTypeDef;
  class PersistentBaseType;
  class PersistentReader;
  class DataReader;
}

namespace InoPersist
{

using namespace Ino;

class Type;
class Struct;
class Array;

//-------------------------------------------------------------------------

class InTypePool
{
  enum { CapInc = 128 };

  const PersistentTypeDef& typeDef;

  Type **typeArr;
  short sz, cap;

  void incCapacity();

  InTypePool(const InTypePool& cp);
  InTypePool& operator=(const InTypePool& src);

public:
  InTypePool(const PersistentTypeDef& tpDef);
  ~InTypePool();

  void clear();

  void readType(DataReader& dRdr);
  void readTypeDef(DataReader& dRdr);

  Type *get(short idx) const;
};

//---------------------------------------------------------------------------

struct InpStruct;

class InpStructPool
{
  InTypePool& tpPool;

  InpStruct *inpLst[8192];
  int inpSz, inpCap;

  int inpFstInvalid;

  InpStructPool(const InpStructPool& cp);             // No Copying
  InpStructPool& operator=(const InpStructPool& src); // No Assignment

public:
  InpStructPool(InTypePool& typePool);
  ~InpStructPool();

  void setAllValid() { inpFstInvalid = inpSz; }
  void clear();

  void add(short typeId);

  Struct *getType(int idx) const;
  Persistable *get(int idx) const;

  void setPostProcess(int idx);
  void postProcess(PersistentReader& pi);
};

//---------------------------------------------------------------------------

struct InpString;

class InpStringPool
{
  InpString *inpLst[8192];
  int inpSz, inpCap;
  int inpFstInvalid;

  wchar_t *readBuf;
  int readBufCap;

  InpStringPool(const InpStringPool& cp);             // No Copying
  InpStringPool& operator=(const InpStringPool& src); // No Assignment

public:
  InpStringPool();
  ~InpStringPool();

  void setAllValid() { inpFstInvalid = inpSz; }
  void clear();

  void readString(DataReader& dRdr);

  wchar_t *get(int idx);
  int getSz(int idx);
};

//---------------------------------------------------------------------------

struct InpArray;

class InpArrayPool
{
  InTypePool& tpPool;

  InpArray *inpLst[8192];
  int inpSz, inpCap, inpFstInvalid;

  InpArrayPool(const InpArrayPool& cp);             // No Copying
  InpArrayPool& operator=(const InpArrayPool& src); // No Assignment

public:
  InpArrayPool(InTypePool& typePool);
  ~InpArrayPool();

  void setAllValid() { inpFstInvalid = inpSz; }
  void clear();

  void readDecl(DataReader& dRdr);

  Array *getType(int idx);
  int    getSize(int idx);

  void *getPtr(int idx, int& arrSz);
};

} //namespace Ino

//---------------------------------------------------------------------------
#endif
