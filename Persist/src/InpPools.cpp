//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library: Object Pools --------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "InpPools.h"

#include "PersistentIO.h"
#include "Type.h"

#include <cstdlib>
#include <cstring>

namespace InoPersist
{

//---------------------------------------------------------------------------

struct InpStruct
{
  Struct *type;
  Persistable *p;
  bool postProcess;

  InpStruct() : type(NULL), p(NULL), postProcess(false) {}
};

//---------------------------------------------------------------------------

InpStructPool::InpStructPool(InTypePool& typePool)
: tpPool(typePool), inpSz(1), inpCap(0),
  inpFstInvalid(1) // First entry is not used
{
}

//---------------------------------------------------------------------------

InpStructPool::~InpStructPool()
{
  for (int i=inpFstInvalid; i<inpSz; ++i) {
    InpStruct& is = inpLst[i >> 13][i & 0x1FFF];

    if (is.p) delete[] (char *)(is.p); // Must not call destructor!!!
  }

  int sz = inpCap >> 13;

  for (int i=0; i<sz; ++i) {
    if (inpLst[i]) delete[] inpLst[i];
  }
}

//---------------------------------------------------------------------------

void InpStructPool::clear()
{
  for (int i=inpFstInvalid; i<inpSz; ++i) {
    InpStruct& is = inpLst[i >> 13][i & 0x1FFF];

    if (is.p) delete[] (char *)(is.p); // Must not call destructor!!!
    is.p = NULL;
    is.type = NULL;
  }

  inpSz = 1;
  inpFstInvalid = 1;

  if (inpCap < 1) return;

  int sz = inpCap >> 13;

  for (int i=1; i<sz; ++i) { // All but first block of 8192
    if (inpLst[i]) delete[] inpLst[i];
    inpLst[i] = NULL;
  }

  inpCap = 8192;

  for (int i=0; i<inpCap; ++i) {
    InpStruct& is = inpLst[i >> 13][i & 0x1FFF];

    is.type = NULL;
    is.p = NULL;
    is.postProcess = false;
  }
}

//---------------------------------------------------------------------------

void InpStructPool::add(short typeId)
{
  if (typeId < 0) throw StreamCorruptedException("InpStructPool::add");

  Type *tp = tpPool.get(typeId);

  Struct *sTp = dynamic_cast<Struct *>(tp);
  // if (!sTp) throw IllegalStateException("InpStructPool::add");

  if (inpSz >= inpCap) {
    inpLst[inpCap >> 13] = new InpStruct[8192];
    inpCap += 8192;
  }

  InpStruct &is = inpLst[inpSz >> 13][inpSz & 0x1FFF];

  is.type = sTp;
  is.p    = NULL;
  is.postProcess = false;

  inpSz++;
}

//---------------------------------------------------------------------------

Struct *InpStructPool::getType(int idx) const
{
  if (idx <= 0 || idx >= inpSz)
               throw IndexOutOfBoundsException("InpStructPool::get");

  InpStruct& is = inpLst[idx >> 13][idx & 0x1FFF];
  // if (!is.type) throw IllegalStateException("InpStructPool::getType");

  return is.type;
}

//---------------------------------------------------------------------------

Persistable *InpStructPool::get(int idx) const
{
  if (idx <= 0 || idx >= inpSz)
               throw IndexOutOfBoundsException("InpStructPool::get");

  InpStruct& is = inpLst[idx >> 13][idx & 0x1FFF];

  if (!is.p) {
    // if (!is.type) throw IllegalStateException("InpStructPool::get");
    if (!is.type) return NULL;

    is.p = (Persistable *)new char[is.type->baseType.sz];
  }

  return is.p;
}

//---------------------------------------------------------------------------

void InpStructPool::setPostProcess(int idx)
{
  if (idx <= 0 || idx >= inpSz)
               throw IndexOutOfBoundsException("InpStructPool::setPostProcess");

  InpStruct& is = inpLst[idx >> 13][idx & 0x1FFF];

  is.postProcess = true;
}

//---------------------------------------------------------------------------

void InpStructPool::postProcess(PersistentReader& pi)
{
  for (int i=inpFstInvalid; i<inpSz; ++i) {
    InpStruct& is = inpLst[i >> 13][i & 0x1FFF];

    if (is.postProcess && is.p) is.p->postProcess(pi);
  }
}

//---------------------------------------------------------------------------

struct InpString
{
  wchar_t *wc;
  int wcSz;

  InpString() : wc(NULL), wcSz(0) {}
};

//---------------------------------------------------------------------------

InpStringPool::InpStringPool()
: inpSz(1), inpCap(0), inpFstInvalid(1), // First entry is not used
  readBuf(new wchar_t[1024]), readBufCap(1024)
{
}

//---------------------------------------------------------------------------

InpStringPool::~InpStringPool()
{
  for (int i=inpFstInvalid; i<inpSz; ++i) {
    InpString& is = inpLst[i >> 13][i & 0x1FFF];

    if (is.wc) delete[] is.wc;
  }

  int sz = inpCap >> 13;

  for (int i=0; i<sz; ++i) {
    if (inpLst[i]) delete[] inpLst[i];
  }

  if (readBuf) delete[] readBuf;
}

//---------------------------------------------------------------------------

void InpStringPool::clear()
{
  if (inpCap < 1) return;

  for (int i=inpFstInvalid; i<inpSz; ++i) {
    InpString& is = inpLst[i >> 13][i & 0x1FFF];

    if (is.wc) delete[] is.wc;
    
    is.wc = NULL;
    is.wcSz = 0;
  }

  inpSz = 1;
  inpFstInvalid = 1;

  int sz = inpCap >> 13;

  for (int i=1; i<sz; ++i) { // All but first block of 8192
    if (inpLst[i]) delete[] inpLst[i];
    inpLst[i] = NULL;
  }

  inpCap = 8192;

  for (int i=0; i<inpCap; ++i) {
    InpString& is = inpLst[i >> 13][i & 0x1FFF];

    is.wc = NULL;
    is.wcSz = 0;
  }
}

//---------------------------------------------------------------------------

void InpStringPool::readString(DataReader& dRdr)
{
  int recLen = Type::readRecordLen(dRdr,"InpStringPool::readString");

  if (inpSz >= inpCap) {
    inpLst[inpCap >> 13] = new InpString[8192];
    inpCap += 8192;
  }

  InpString& is = inpLst[inpSz >> 13][inpSz & 0x1FFF];

  if (recLen >= readBufCap) {
    readBufCap = recLen + 1024;
    readBuf = (wchar_t *)realloc(readBuf,readBufCap*sizeof(wchar_t));
  }

  is.wcSz = dRdr.readUtf8(readBuf,readBufCap,recLen);
  if (is.wcSz < 0)
             throw StreamCorruptedException("InpStringPool::readString 3");

  is.wc = new wchar_t[is.wcSz + 1];

  memcpy(is.wc,readBuf,is.wcSz*sizeof(wchar_t));
  is.wc[is.wcSz] = L'\0';

  inpSz++;
}

//---------------------------------------------------------------------------

wchar_t *InpStringPool::get(int idx)
{
  if (idx <= 0 || idx >= inpSz)
               throw IndexOutOfBoundsException("InpStringPool::get");

  InpString& is = inpLst[idx >> 13][idx & 0x1FFF];

  return is.wc;
}

//---------------------------------------------------------------------------

int InpStringPool::getSz(int idx)
{
  if (idx <= 0 || idx >= inpSz)
               throw IndexOutOfBoundsException("InpStringPool::get");

  InpString& is = inpLst[idx >> 13][idx & 0x1FFF];

  return is.wcSz;
}

//---------------------------------------------------------------------------

struct InpArray
{
  Array *type;
  char  *arr;
  int  arrSz;

  InpArray() : type(NULL), arr(NULL), arrSz(0) {}
};

//---------------------------------------------------------------------------

InpArrayPool::InpArrayPool(InTypePool& typePool)
: tpPool(typePool), inpSz(1), inpCap(0), inpFstInvalid(1)
{
}

//---------------------------------------------------------------------------

InpArrayPool::~InpArrayPool()
{
  if (inpCap < 1) return;

  for (int i=inpFstInvalid; i<inpSz; ++i) {
    InpArray& ia = inpLst[i >> 13][i & 0x1FFF];

    if (ia.arr) delete[] ia.arr;
  }

  int sz = inpCap >> 13;

  for (int i=0; i<sz; ++i) {
    if (inpLst[i]) delete[] inpLst[i];
  }
}

//---------------------------------------------------------------------------

void InpArrayPool::clear()
{
  if (inpCap < 1) return;

  for (int i=inpFstInvalid; i<inpSz; ++i) {
    InpArray& ia = inpLst[i >> 13][i & 0x1FFF];

    if (ia.arr) delete[] ia.arr;
  }

  int sz = inpCap >> 13;

  for (int i=1; i<sz; ++i) { // All but first block of 8192
    if (inpLst[i]) delete[] inpLst[i];
    inpLst[i] = NULL;
  }

  inpSz  = 1;
  inpFstInvalid = 1;
  inpCap = 8192;

  for (int i=0; i<inpCap; ++i) {
    InpArray& ia = inpLst[i>>13][i & 0x1FFF];

    ia.type  = NULL;
    ia.arr   = NULL;
    ia.arrSz = 0;
  }
}

//---------------------------------------------------------------------------

void InpArrayPool::readDecl(DataReader& dRdr)
{
  short tpId = dRdr.readShort("InpArrayPool::readDecl 1");
  int arrSz  = dRdr.readInt("InpArrayPool::readDecl 2");

  if (inpSz >= inpCap) {
    inpLst[inpCap >> 13] = new InpArray[8192];
    inpCap += 8192;
  }

  InpArray& ia = inpLst[inpSz >> 13][inpSz & 0x1FFF];
  
  Type *tp = tpPool.get(tpId);

  if (tp) {
    if (tp->getCategory() != Type::CatArray)
                  throw StreamCorruptedException("InpArrayPool::readDecl 3");

    ia.type  = dynamic_cast<Array *>(tp);
    ia.arrSz = arrSz;
  }
  else {
    ia.type  = NULL;
    ia.arrSz = 0;
  }

  ia.arr = NULL;

  inpSz++;
}

//---------------------------------------------------------------------------

Array *InpArrayPool::getType(int idx)
{
  if (idx <= 0 || idx >= inpSz)
               throw IndexOutOfBoundsException("InpArrayPool::get");

  InpArray& ia = inpLst[idx >> 13][idx & 0x1FFF];
  // if (!ia.type) throw IllegalStateException("InpArrayPool::getType");

  return ia.type;
}

//---------------------------------------------------------------------------

int InpArrayPool::getSize(int idx)
{
  if (idx <= 0 || idx >= inpSz)
               throw IndexOutOfBoundsException("InpArrayPool::get");

  InpArray& ia = inpLst[idx >> 13][idx & 0x1FFF];

  // if (!ia.type) throw IllegalStateException("InpArrayPool::getSize");
  if (!ia.type) return 0;

  return ia.arrSz;
}

//---------------------------------------------------------------------------

void *InpArrayPool::getPtr(int idx, int& arrSz)
{
  arrSz = 0;

  if (idx <= 0 || idx >= inpSz)
               throw IndexOutOfBoundsException("InpArrayPool::get");

  InpArray& ia = inpLst[idx >> 13][idx & 0x1FFF];
  // if (!ia.type) throw IllegalStateException("InpArrayPool::getPtr");
  
  if (!ia.type) return NULL;

  if (!ia.arr) ia.arr = new char[ia.arrSz*ia.type->elemType.getDataSize()];

  arrSz = ia.arrSz;

  return ia.arr;
}

} // namespace Ino

//---------------------------------------------------------------------------
