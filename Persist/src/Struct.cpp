//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Persistent Objects Library ----------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV June 2005 ----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters --------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Type.h"
#include "PersistentIO.h"
#include "InpPools.h"

#include <cstring>

namespace InoPersist
{

//---------------------------------------------------------------------------

void Struct::calcOffsets()
{
  if (defined) return;

  structSz = 0;

  for (short i=0; i<valList.size(); i++) {
    Field& fld = valList[i];

    fld.offset = structSz;
    if (fld.type) structSz += fld.type->getDataSize();
    else structSz += 4; // Can only be an unknown struct or array
  }

  for (short i=0; i<refList.size(); i++) {
    Field& fld = refList[i];

    fld.offset = structSz;
    if (fld.type) structSz += fld.type->getDataSize();
    else structSz += 4; // Can only be an unknown struct or array
  }

  defined = true;
}

//---------------------------------------------------------------------------

Struct::Struct(short typeId, const PersistentBaseType& baseTp)
: Type(typeId), valList(), refList(), defined(false), structSz(0),
  baseType(baseTp)
{
}

//---------------------------------------------------------------------------

int Struct::getStructSize() const
{
  if (!defined) throw IllegalStateException("Struct::getStructSize");

  return structSz;
}

//---------------------------------------------------------------------------

void Struct::addField(const char *fldName, Type& fldType)
{
  if (defined) throw IllegalStateException("Struct::addField");

  if (!fldName) throw NullPointerException("Struct::addField");
  if (strlen(fldName) < 1) throw IllegalArgumentException("Struct::addField");

  if (fldType.isRefType()) {
    if (valList[fldName])
      throw DuplicateNameException("Struct::addField");
    refList.add(fldName,&fldType);
  }
  else {
    if (refList[fldName]) 
      throw DuplicateNameException("Struct::addField");
    valList.add(fldName,&fldType);
  }
}

//---------------------------------------------------------------------------

void Struct::write(DataWriter& dWrt) const
{
  dWrt.writeByte(Record_Type,"Struct::write");

  int len = strlen(baseType.name);
  dWrt.writeShort((short)len,"Struct::write");
  dWrt.write(baseType.name,len,"Struct::write");
}

//---------------------------------------------------------------------------

void Struct::writeDef(ByteArrayWriter& byteWrt)
{
  calcOffsets();

  byteWrt.setSize(0);

  DataWriter dWrt(byteWrt);

  dWrt.writeShort(id,"Struct::writeDef");

  dWrt.writeByte(1); // Class major version (future)
  dWrt.writeByte(0); // Class major version (future)

  dWrt.writeShort(valList.size());

  for (short i=0; i<valList.size(); ++i) {
    const Field& fld = valList[i];

    if (!fld.type) throw IllegalStateException("Struct::writeDef: NULL type 1");
    dWrt.writeShort(fld.type->id);

    int len = strlen(fld.name);
    if (len > 127) throw IllegalArgumentException("Struct::writeDef: name too long");

    dWrt.writeByte((char)len);
    dWrt.write(fld.name,len);
  }

  dWrt.writeShort(refList.size());

  for (short i=0; i<refList.size(); ++i) {
    const Field& fld = refList[i];

    if (!fld.type) throw IllegalStateException("Struct::writeDef: NULL type 2");
    dWrt.writeShort(fld.type->id);

    int len = strlen(fld.name);
    if (len > 127) throw IllegalArgumentException("Struct::writeDef: name too long");

    dWrt.writeByte((char)len);
    dWrt.write(fld.name,len);
  }
}

//---------------------------------------------------------------------------

void Struct::readDef(DataReader& dRdr, const InTypePool& pool)
{
  if (defined) throw IllegalStateException("Struct::readDef");

  dRdr.readByte("Struct::readDef"); // major version, not used yet
  dRdr.readByte("Struct::readDef"); // minor version, not used yet

  short valSz = dRdr.readShort("Struct::readDef 2");

  if (valSz < 0) throw StreamCorruptedException("Struct::readDef 3");

  for (int i=0; i<valSz; ++i) {
    short tpId = dRdr.readShort("Struct::readDef 4");

    const Type *fldTp = pool.get(tpId);

    short namLen = dRdr.readByte("Struct::readDef 5");
    if (namLen < 1 || namLen > 127) throw StreamCorruptedException("Struct::readDef 6");

    char name[128];
    dRdr.read(name,namLen,"Struct::readDef 7");
    name[namLen] = '\0';

    valList.add(name,fldTp);
  }

  short refSz = dRdr.readShort("Struct::readDef (Ref) 2");

  if (refSz < 0) throw StreamCorruptedException("Struct::readDef (Ref) 3");

  for (int i=0; i<refSz; ++i) {
    short tpId = dRdr.readShort("Struct::readDef (Ref) 4");

    const Type *fldTp = pool.get(tpId);

    short namLen = dRdr.readByte("Struct::readDef (Ref) 5");
    if (namLen < 1 || namLen > 127) throw StreamCorruptedException("Struct::readDef (Ref) 6");

    char name[128];
    dRdr.read(name,namLen,"Struct::readDef 7");
    name[namLen] = '\0';

    refList.add(name,fldTp);
  }

  calcOffsets();
}

} // namespace InoPersist

//---------------------------------------------------------------------------
