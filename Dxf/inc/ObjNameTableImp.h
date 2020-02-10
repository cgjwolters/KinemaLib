//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Dxf Format Reader -----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Copyright (C) 2005 Inofor Hoek Aut BV ---------------------------
//---------------------------------------------------------------------------
//--------- C.Wolters June 2005----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef OBJNAMETABLE_INC
#error Include ObjNameTable.h instead of this file
#endif

namespace Ino
{

//---------------------------------------------------------------------------

template <class T> class ObjEntry : public ObjEntryBase
{
  ObjEntry(const ObjEntry& cp);             // No copying
  ObjEntry& operator=(const ObjEntry& src); // No Assignment

public:

  ObjEntry(const char *val, int hashCode)
     : ObjEntryBase(val,hashCode), object(NULL) {}

  virtual ~ObjEntry() { if (object) delete object; }

  T *object;
};

//---------------------------------------------------------------------------

template <class T>
ObjEntryBase *ObjNameTable<T>::newObjEntry(const char *val, int hashCode)
{
  return new ObjEntry<T>(val,hashCode);
}

//---------------------------------------------------------------------------

template <class T> T *ObjNameTable<T>::get(const char *val) const
{
  ObjEntry<T> *ent = (ObjEntry<T> *)getEnt(val);
  if (!ent) return NULL;

  return ent->object;
}

//---------------------------------------------------------------------------

template <class T> T *ObjNameTable<T>::add(const char *val, T *v)
{
  ObjEntry<T> *ent = (ObjEntry<T> *)addEnt(val);
  if (!ent) return NULL; // Must not occur!!

  if (!ent->object) ent->object = v;

  return ent->object;
}

} // namespace Ino

//---------------------------------------------------------------------------

