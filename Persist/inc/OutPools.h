//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library: Object Pools --------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef PERSIST_OUTPOOLS_INC
#define PERSIST_OUTPOOLS_INC

#include "Exceptions.h"

#include <typeinfo>

//---------------------------------------------------------------------------

namespace Ino
{
  class DataWriter;
  class Persistable;
  class PersistentTypeDef;
  class PersistentWriter;
}

namespace InoPersist
{

using namespace Ino;

class Type;
class Array;

//-------------------------------------------------------------------------

class OutTypePool
{
  enum { CapInc = 128 };

  const PersistentTypeDef& typeDef;
  DataWriter& dWrt;

  Type **hashArr;
  short sz, cap;

  Type *chainTypes();
  void incCapacity();

  OutTypePool(const OutTypePool& cp);
  OutTypePool& operator=(const OutTypePool& src);

public:
  OutTypePool(const PersistentTypeDef& tpDef, DataWriter& wrt);
  ~OutTypePool();

  void clear();

  static bool isBasicType(const type_info& inf);

  Type *get(const type_info& inf);

  Type& add(const type_info& inf);
  Type& addArray(const type_info& inf, Type& elemType);
};

//---------------------------------------------------------------------------

struct OutStruct;

class OutStructPool
{
  PersistentTypeDef& tpDef;
  DataWriter& dWrt;

  OutStruct **outLst[8192];
  int outSz, outCap;

  OutStruct *outFirst, *outLast;
  OutStruct *ppFirst, *ppLast;

  OutStruct *chainHash();
  void incCapacity();

  OutStructPool(const OutStructPool& cp);             // No Copying
  OutStructPool& operator=(const OutStructPool& src); // No Assignment

public:
  OutStructPool(PersistentTypeDef& typeDef, DataWriter& wrt);
  ~OutStructPool();

  void clear();

  int get(const Persistable *p, short typeId);

  const Persistable *getNext(int& id);

  void setPostProcess(const Persistable *p);
  void postProcess(PersistentWriter& po);
};

//---------------------------------------------------------------------------

struct OutString;

class OutStringPool
{
  DataWriter& dWrt;

  OutString **outLst[8192];
  int outSz, outCap;

  OutString *chainHash();
  void incCapacity();

  OutStringPool(const OutStringPool& cp);             // No Copying
  OutStringPool& operator=(const OutStringPool& src); // No Assignment

public:
  OutStringPool(DataWriter& wrt);
  ~OutStringPool();

  void clear();

  int get(const wchar_t *wc, int wcSz);
};

//---------------------------------------------------------------------------

class OutArray
{
  OutArray *nextHash, *nextQ;

  OutArray(const OutArray& cp);             // No Copying
  OutArray& operator=(const OutArray& src); // No Assignment

public:
  OutArray(int pId, const Array& arrType, const void *array, int arrSz);

  const int id;
  const Array &type;
  const void *const arr;
  const int sz;

  friend class OutArrayPool;
};

//---------------------------------------------------------------------------

class OutArrayPool
{
  DataWriter& dWrt;

  OutArray **outLst[8192];
  int outSz, outCap;

  OutArray *outFirst, *outLast;

  OutArray *chainHash();
  void incCapacity();

  OutArrayPool(const OutArrayPool& cp);             // No Copying
  OutArrayPool& operator=(const OutArrayPool& src); // No Assignment

public:
  OutArrayPool(DataWriter& wrt);
  ~OutArrayPool();

  void clear();

  int get(const void *array, int arrSz, const Array& arrType);

  const OutArray *getNext();
};

} // namespace InoPersist

//---------------------------------------------------------------------------
#endif
