//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library: Object Pools --------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "OutPools.h"

#include "Type.h"
#include "Basics.h"

#include <cstring>

namespace InoPersist
{

//---------------------------------------------------------------------------

Type *OutTypePool::chainTypes()
{
  Type *fst = NULL;

  for (int i=0; i<cap; ++i) {
    Type *cur = hashArr[i];
    if (!cur) continue;

    Type *lst = cur;
    while (lst->next) lst = lst->next;

    lst->next = fst;
    fst = cur;
  }

  return fst;
}

//---------------------------------------------------------------------------

void OutTypePool::incCapacity()
{
  Type *fst = chainTypes();

  cap += CapInc;

  if (hashArr) delete[] hashArr;
  hashArr = new Type*[cap];
  if (!hashArr) throw OutOfMemoryException("TypePool::incCapacity 2");

  memset(hashArr,0,cap*sizeof(Type *));

  while (fst) {
    Type *tp = fst;
    fst = tp->next;

    int idx = hashCode(tp->getHashName()) % cap;
    if (idx < 0) idx += cap;

    tp->next = hashArr[idx];
    hashArr[idx] = tp;
  }
}

//---------------------------------------------------------------------------

OutTypePool::OutTypePool(const PersistentTypeDef& tpDef, DataWriter& wrt)
: typeDef(tpDef), dWrt(wrt), hashArr(new Type*[CapInc]),
  sz(0), cap(CapInc)
{
  memset(hashArr,0,cap*sizeof(Type*));
}

//---------------------------------------------------------------------------

OutTypePool::~OutTypePool()
{
  Type *fst = chainTypes();

  while (fst) {
    Type *cur = fst;
    fst = fst->next;

    delete cur;
  }

  if (hashArr) delete[] hashArr;
}

//---------------------------------------------------------------------------

void OutTypePool::clear()
{
  Type *fst = chainTypes();

  while (fst) {
    Type *cur = fst;
    fst = fst->next;

    delete cur;
  }

  for (int i=0; i<cap; ++i) hashArr[i] = NULL;

  sz = 0;
}

//---------------------------------------------------------------------------

Type *OutTypePool::get(const type_info& inf)
{
#ifndef WIN32
  int idx = hashCode(inf.name()) % cap;
#else
  int idx = hashCode(inf.raw_name()) % cap;
#endif

  if (idx < 0) idx += cap;

  Type *tp = hashArr[idx];

  while (tp) {
#ifndef WIN32
    if (!strcmp(inf.name(),tp->getHashName())) return tp;
#else
    if (!strcmp(inf.raw_name(),tp->getHashName())) return tp;
#endif

    tp = tp->next;
  }

  return NULL;
}

//---------------------------------------------------------------------------

static const type_info *basicLst[] = 
  {
    &typeid(bool),  &typeid(wchar_t), &typeid(char),
    &typeid(short), &typeid(long),    &typeid(__int64),
    &typeid(float), &typeid(double),  &typeid(long double),
    &typeid(wchar_t *)
  };

//---------------------------------------------------------------------------

static short baseDataType(const type_info& inf)
{
  if (inf == typeid(int)) return -5; // Equivalent to a long

  for (short i=0; i<10; ++i) {
    if (inf == *basicLst[i]) return -i-1;
  }

  return 0;
}

//---------------------------------------------------------------------------

bool OutTypePool::isBasicType(const type_info& inf)
{
  return baseDataType(inf) != 0;
}

//---------------------------------------------------------------------------

Type& OutTypePool::add(const type_info& inf)
{
  Type *tp = get(inf);
  if (tp) {
    switch (tp->getCategory()) {
      case Type::CatBasic:
      case Type::CatStruct: return *tp;

      default: {
        char msg[512];
        sprintf(msg,"OutTypePool::add: wrong type: \"%s\"",inf.name());
        throw IllegalStateException(msg);
      }
    }
  }

  // Not found, must add:

  if (sz >= cap) incCapacity();

  short dataTp = baseDataType(inf);

  if (dataTp) tp = new Basic(sz,(Type::DataType)dataTp);
  else {
    const PersistentBaseType *bTp = typeDef.get(inf);

    if (!bTp) {
      char msg[512];
      sprintf(msg,"OutTypePool::add: illegal/unknown type: \"%s\"",inf.name());
      throw IllegalStateException(msg);
    }

    tp = new Struct(sz,*bTp);
  }

#ifndef WIN32
  int idx = hashCode(inf.name()) % cap;
#else
  int idx = hashCode(inf.raw_name()) % cap;
#endif

  if (idx < 0) idx += cap;

  tp->next = hashArr[idx];
  hashArr[idx] = tp;

  sz++;

  tp->write(dWrt);

  return *tp;
}

//---------------------------------------------------------------------------

Type& OutTypePool::addArray(const type_info& inf, Type& elemType)
{
  Type *tp = get(inf);

  if (tp) {
    if (tp->getCategory() != Type::CatArray) {
      char msg[512] = "";
      sprintf(msg,"OutTypePool::add: type \"%s\" is not an array",inf.name());
    }

    return *tp;
  }

  // Not found, must add:

  if (sz >= cap) incCapacity();

  tp = new Array(sz,inf,elemType);

#ifndef WIN32
  int idx = hashCode(inf.name()) % cap;
#else
  int idx = hashCode(inf.raw_name()) % cap;
#endif

  if (idx < 0) idx += cap;

  tp->next = hashArr[idx];
  hashArr[idx] = tp;

  sz++;

  tp->write(dWrt);

  return *tp;
}

} // namespace Ino

//---------------------------------------------------------------------------
