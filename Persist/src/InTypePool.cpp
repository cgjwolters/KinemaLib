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

#include "Type.h"
#include "PersistentIO.h"

#include <cstdlib>

namespace InoPersist
{

//---------------------------------------------------------------------------

void InTypePool::incCapacity()
{
  cap += CapInc;

  typeArr = (Type **)realloc(typeArr,cap*sizeof(Type *));
}

//---------------------------------------------------------------------------

InTypePool::InTypePool(const PersistentTypeDef& tpDef)
: typeDef(tpDef), typeArr(new Type*[CapInc]),
  sz(0),cap(CapInc)
{
}

//---------------------------------------------------------------------------

InTypePool::~InTypePool()
{
  for (int i=0; i<sz; ++i) delete typeArr[i];

  delete[] typeArr;
}

//---------------------------------------------------------------------------

void InTypePool::clear()
{
  for (int i=0; i<sz; ++i) delete typeArr[i];

  sz = 0;
}

//---------------------------------------------------------------------------

void InTypePool::readType(DataReader& dRdr)
{
  if (sz >= cap) incCapacity();

  short s = dRdr.readShort("InTypePool::readType");

  int tp = Type::ArrayType;
  short tps = (short)(tp & 0xFFFF);

  Type *newTp = NULL;

  if (s == tps) {
    short elTpId = dRdr.readShort("InTypePool::readType 2");

    if (elTpId < 0 || elTpId >= sz)
               throw StreamCorruptedException("InTypePool::readType 3");

    if (get(elTpId)) newTp = new Array(sz,typeid(void *),*typeArr[elTpId]);
  }
  else if (s < 0) {
    if (s < Type::String)
               throw StreamCorruptedException("InTypePool::readType 4");

    newTp = new Basic(sz,(Type::DataType)s);
  }
  else {
    if (s >= 512) throw StreamCorruptedException("InTypePool::readType 5");

    char buf[512];
    dRdr.read(buf,s,"InTypePool::readType 6");

    buf[s] = '\0';

    const PersistentBaseType *bTp = typeDef.get(buf);

    if (bTp) newTp = new Struct(sz,*bTp);
  }

  typeArr[sz++] = newTp;
}

//--------------------------------------------------------------------------

static void skipRecord(DataReader& rdr, int bytes)
{
  char buf[1024];

  while (bytes > 0) {
    int bSz = bytes;
    if (bSz > 1024) bSz = 1024;

    int bRd = rdr.read(buf,bSz);
    if (bRd <= 0) return;

    bytes -= bRd;
  }
}

//---------------------------------------------------------------------------

void InTypePool::readTypeDef(DataReader& dRdr)
{
  int recLen = Type::readRecordLen(dRdr,"InTypePool::readTypeDef");

  if (recLen < 6) throw StreamCorruptedException("InTypePool::readTypeDef 2");

  short typeId = dRdr.readShort("InTypePool::readTypeDef 3");
  
  if (typeId < 0 || typeId >= sz)
                throw StreamCorruptedException("InTypePool::readTypeDef 2");

  Type *tp = typeArr[typeId];

  if (!tp) {
    skipRecord(dRdr,recLen-2);
    return;
  }

  if (!tp->getCategory() == Type::CatStruct)
                throw StreamCorruptedException("InTypePool::readTypeDef 3");

  Struct *structTp = dynamic_cast<Struct *>(tp);
  if (!structTp) throw StreamCorruptedException("InTypePool::readTypeDef 4");

  structTp->readDef(dRdr,*this);
}

//---------------------------------------------------------------------------

Type *InTypePool::get(short idx) const
{
  if (idx < 0 || idx >= sz)
                     throw IndexOutOfBoundsException("InTypePool::get");

  return typeArr[idx];
}

} // namespace InoPersist

//---------------------------------------------------------------------------
