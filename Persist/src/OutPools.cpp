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
#include "PersistentIO.h"

#include <cstring>

namespace InoPersist
{

//---------------------------------------------------------------------------

struct OutStruct
{
  const int id;
  const short tpId;
  const Persistable *const p;

  OutStruct *nextHash, *nextQ;
  
  OutStruct(const OutStruct& cp);             // No Copying
  OutStruct& operator=(const OutStruct& src); // No Assignment

  OutStruct(int pId, short typeId, const Persistable *po)
   : id(pId), tpId(typeId), p(po),
     nextHash(NULL), nextQ(NULL) {}
};

//---------------------------------------------------------------------------

OutStruct *OutStructPool::chainHash()
{
  OutStruct *list = NULL;

  for (int i=0; i<outCap; ++i) {
    OutStruct* &pRef = outLst[i >> 13][i & 0x1FFF];
    if (!pRef) continue;

    OutStruct *fst = pRef;

    pRef = NULL;

    OutStruct *last = fst;
    while (last->nextHash) last = last->nextHash;

    last->nextHash = list;
    list = fst;
  }

  return list;
}

//---------------------------------------------------------------------------

void OutStructPool::incCapacity()
{
  OutStruct *fst = chainHash();

  OutStruct **newLst = new OutStruct*[8192];
  memset(newLst,0,8192*sizeof(OutStruct*));

  outLst[outCap >> 13] = newLst;
  outCap += 8192;

  while (fst) {
    OutStruct *nxt = fst->nextHash;

    int hashIdx = (int(fst->p) >> 3) % outCap;
    if (hashIdx < 0) hashIdx += outCap;

    OutStruct* &pRef = outLst[hashIdx >> 13][hashIdx & 0x1FFF];

    fst->nextHash = pRef;
    pRef = fst;

    fst = nxt;
  }
}

//---------------------------------------------------------------------------

OutStructPool::OutStructPool(PersistentTypeDef& typeDef, DataWriter& wrt)
: tpDef(typeDef), dWrt(wrt), outSz(0), outCap(0),
  outFirst(NULL), outLast(NULL), ppFirst(NULL), ppLast(NULL)
{
}

//---------------------------------------------------------------------------

OutStructPool::~OutStructPool()
{
  clear();

  int sz = outCap >> 13;

  for (int i=0; i<sz; ++i) {
    if (outLst[i]) delete[] outLst[i];
  }
}

//---------------------------------------------------------------------------

void OutStructPool::clear()
{
  outFirst = outLast = NULL;
  ppFirst  = ppLast  = NULL;

  outSz = 0;

  if (outCap < 1) return;

  for (int i=0; i<outCap; ++i) {
    OutStruct* &pRef = outLst[i >> 13][i & 0x1FFF];

    OutStruct *os = pRef;
    pRef = NULL;

    while (os) {
      OutStruct *nxtOs = os->nextHash;

      delete os;

      os = nxtOs;
    }
  }

  int sz = outCap >> 13;

  for (int i=1; i<sz; ++i) {
    if (outLst[i]) delete[] outLst[i];
    outLst[i] = NULL;
  }

  outCap = 8192;
}

//---------------------------------------------------------------------------

int OutStructPool::get(const Persistable *p, short typeId)
{
  if (!p) return 0;

  if ((outSz >> 1) >= outCap) incCapacity(); // Allow fill factor of two

  int hashIdx = (int(p) >> 3) % outCap;
  if (hashIdx < 0) hashIdx += outCap;

  OutStruct* &pRef = outLst[hashIdx >> 13][hashIdx & 0x1FFF];

  OutStruct *os = pRef;

  while (os) {
    if (os->p == p) {
      if (os->tpId != typeId)
                throw IllegalStateException("OutStructPool::get: typeId");
      return os->id;
    }

    os = os->nextHash;
  }

  // Not found, must add new entry

  OutStruct *newS = new OutStruct(++outSz,typeId,p);

  if (!outFirst) outFirst = outLast = newS;
  else {
    outLast->nextQ = newS;
    outLast = newS;
  }

  newS->nextHash = pRef;
  pRef = newS;

  dWrt.writeByte(Record_StructDecl);
  dWrt.writeShort(typeId);

  return newS->id;
}

//---------------------------------------------------------------------------

const Persistable *OutStructPool::getNext(int& id)
{
  id = 0;

  OutStruct *os = outFirst;
  if (!os) return NULL;

  outFirst = os->nextQ;
  if (!outFirst) outLast = NULL;

  id = os->id;

  return os->p;
}

//---------------------------------------------------------------------------

void OutStructPool::setPostProcess(const Persistable *p)
{
  if (!p) return;

  int hashIdx = (int(p) >> 3) % outCap;
  if (hashIdx < 0) hashIdx += outCap;

  OutStruct* os = outLst[hashIdx >> 13][hashIdx & 0x1FFF];

  while (os) {
    if (os->p == p) {
      if (!ppFirst) {
        ppFirst = ppLast = os;
        os->nextQ = NULL;
      }
      else {
        ppLast->nextQ = os;
        ppLast = os;
      }

      return;
    }

    os = os->nextHash;
  }
}

//---------------------------------------------------------------------------

void OutStructPool::postProcess(PersistentWriter& po)
{
  while (ppFirst) {
    OutStruct *os = ppFirst;
    ppFirst = os->nextQ;

    if (os->p) os->p->postProcess(po);
  }

  ppFirst = ppLast = NULL;
}

//---------------------------------------------------------------------------

struct OutString
{
  const int id;
  const wchar_t *const wc;

  OutString *nextHash;

  OutString(const OutString& cp);            // No Copying
  OutString& operator=(const OutString& src); // No Assignment

  OutString(int sId, const wchar_t *ws) : id(sId),wc(ws), nextHash(NULL) {}
};

//---------------------------------------------------------------------------

OutString *OutStringPool::chainHash()
{
  OutString *list = NULL;

  for (int i=0; i<outCap; ++i) {
    OutString* &pRef = outLst[i >> 13][i & 0x1FFF];
    if (!pRef) continue;

    OutString *fst = pRef;
    pRef = NULL;

    OutString *last = fst;
    while (last->nextHash) last = last->nextHash;

    last->nextHash = list;
    list = fst;
  }

  return list;
}

//---------------------------------------------------------------------------

void OutStringPool::incCapacity()
{
  OutString *fst = chainHash();

  OutString **newLst = new OutString*[8192];
  memset(newLst,0,8192*sizeof(OutString *));

  outLst[outCap >> 13] = newLst;
  outCap += 8192;

  while (fst) {
    OutString *nxt = fst->nextHash;

    int hashIdx = (int(fst->wc) >> 3) % outCap;
    if (hashIdx < 0) hashIdx += outCap;

    OutString* &pRef = outLst[hashIdx >> 13][hashIdx & 0x1FFF];

    fst->nextHash = pRef;
    pRef = fst;

    fst = nxt;
  }
}

//---------------------------------------------------------------------------

OutStringPool::OutStringPool(DataWriter& wrt)
: dWrt(wrt), outSz(0), outCap(0)
{
}

//---------------------------------------------------------------------------

OutStringPool::~OutStringPool()
{
  clear();

  int sz = outCap >> 13;

  for (int i=0; i<sz; ++i) {
    if (outLst[i]) delete[] outLst[i];
  }
}

//---------------------------------------------------------------------------

void OutStringPool::clear()
{
  outSz = 0;

  if (outCap < 1) return;

  for (int i=0; i<outCap; ++i) {
    OutString* &pRef = outLst[i >> 13][i & 0x1FFF];

    OutString *os = pRef;
    pRef = NULL;

    while (os) {
      OutString *nxtOs = os->nextHash;

      delete os;

      os = nxtOs;
    }
  }

  int sz = outCap >> 13;

  for (int i=1; i<sz; ++i) {
    if (outLst[i]) delete[] outLst[i];
    outLst[i] = NULL;
  }

  outCap = 8192;
}

//---------------------------------------------------------------------------

int OutStringPool::get(const wchar_t *wc, int wcSz)
{
  if (!wc) return 0;

  if (wcSz < 0) throw IllegalArgumentException("OutStringPool::get");

  if ((outSz >> 1) >= outCap) incCapacity(); // Allow fill factor of two

  int hashIdx = (int(wc) >> 3) % outCap;
  if (hashIdx < 0) hashIdx += outCap;

  OutString* &pRef = outLst[hashIdx >> 13][hashIdx & 0x1FFF];

  OutString *os = pRef;

  while (os) {
    if (os->wc == wc) return os->id;

    os = os->nextHash;
  }

  // Not found, must add new entry

  OutString *newS = new OutString(++outSz,wc);

  newS->nextHash = pRef;
  pRef = newS;

  // Write the string here

  dWrt.writeByte(Record_String,"OutStringPool::get 1");

  int sLen = dWrt.calcUtf8Len(wc,wcSz);

  Type::writeRecordLen(dWrt,sLen);

  if (!dWrt.writeUtf8(wc,wcSz))
                  throw StreamCorruptedException("OutStringPool::get 2");

  return newS->id;
}

//---------------------------------------------------------------------------

OutArray::OutArray(int pId, const Array& arrType, const void *array, int arrSz)
: nextHash(NULL), nextQ(NULL),
  id(pId), type(arrType), arr(array), sz(arrSz)
{
}

//---------------------------------------------------------------------------

OutArray *OutArrayPool::chainHash()
{
  OutArray *list = NULL;

  for (int i=0; i<outCap; ++i) {
    OutArray* &pRef = outLst[i >> 13][i & 0x1FFF];
    if (!pRef) continue;

    OutArray *fst = pRef;
    pRef = NULL;

    OutArray *last = fst;
    while (last->nextHash) last = last->nextHash;

    last->nextHash = list;
    list = fst;
  }

  return list;
}

//---------------------------------------------------------------------------

void OutArrayPool::incCapacity()
{
  OutArray *fst = chainHash();

  OutArray **newLst = new OutArray*[8192];
  memset(newLst,0,8192*sizeof(OutArray*));

  outLst[outCap >> 13] = newLst;
  outCap += 8192;

  while (fst) {
    OutArray *nxt = fst->nextHash;

    int hashIdx = (int(fst->arr) >> 3) % outCap;
    if (hashIdx < 0) hashIdx += outCap;

    OutArray* &pRef = outLst[hashIdx >> 13][hashIdx & 0x1FFF];

    fst->nextHash = pRef;
    pRef = fst;

    fst = nxt;
  }
}

//---------------------------------------------------------------------------

OutArrayPool::OutArrayPool(DataWriter& wrt)
: dWrt(wrt), outSz(0), outCap(0), outFirst(NULL), outLast(NULL)
{
}

//---------------------------------------------------------------------------

OutArrayPool::~OutArrayPool()
{
  clear();

  int sz = outCap >> 13;

  for (int i=0; i<sz; ++i) {
    if (outLst[i]) delete[] outLst[i];
  }
}

//---------------------------------------------------------------------------

void OutArrayPool::clear()
{
  outSz = 0;

  if (outCap < 1) return;

  for (int i=0; i<outCap; ++i) {
    OutArray* &pRef = outLst[i >> 13][i & 0x1FFF];

    OutArray *oa = pRef;
    pRef = NULL;

    while (oa) {
      OutArray *nxtOa = oa->nextHash;

      delete oa;

      oa = nxtOa;
    }
  }

  int sz = outCap >> 13;

  for (int i=1; i<sz; ++i) {
    if (outLst[i]) delete[] outLst[i];
    outLst[i] = NULL;
  }

  outCap = 8192;
}

//---------------------------------------------------------------------------

int OutArrayPool::get(const void *array, int arrSz, const Array& arrType)
{
  if (!array) return 0;

  if ((outSz >> 1) >= outCap) incCapacity(); // Allow fill factor of two

  int hashIdx = (int(array) >> 3) % outCap;
  if (hashIdx < 0) hashIdx += outCap;

  OutArray* &pRef = outLst[hashIdx >> 13][hashIdx & 0x1FFF];

  OutArray *os = pRef;

  while (os) {
    if (os->arr == array) return os->id;

    os = os->nextHash;
  }

  // Not found, must add new entry
  OutArray *newArr = new OutArray(++outSz,arrType,array,arrSz);

  newArr->nextHash = pRef;
  pRef = newArr;

  if (!outFirst) outFirst = outLast = newArr;
  else {
    outLast->nextQ = newArr;
    outLast = newArr;
  }

  // Write an array size record

  dWrt.writeByte(Record_ArrayDecl);
  dWrt.writeShort(arrType.id);
  dWrt.writeInt(arrSz);

  return newArr->id;
}

//---------------------------------------------------------------------------

const OutArray *OutArrayPool::getNext()
{
  OutArray *oa = outFirst;
  if (!oa) return NULL;

  outFirst = oa->nextQ;
  if (!outFirst) outLast = NULL;

  return oa;
}

} // namespace InoPersist

//---------------------------------------------------------------------------
