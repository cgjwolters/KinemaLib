//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Dxf 3D Format Reader --------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Copyright (C) 2008 Inofor Hoek Aut BV ---------------------------
//---------------------------------------------------------------------------
//--------- C.Wolters Sept 2008----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef OBJNAMETABLE3D_INC
#error Include ObjNameTable3D.h instead of this file
#endif

namespace Ino
{

//---------------------------------------------------------------------------

template <class T> class ObjEntry3D : public ObjEntryBase3D
{
  ObjEntry3D(const ObjEntry3D& cp);             // No copying
  ObjEntry3D& operator=(const ObjEntry3D& src); // No Assignment

public:

  ObjEntry3D(const char *val, int hashCode)
     : ObjEntryBase3D(val,hashCode), object(NULL) {}

  virtual ~ObjEntry3D() { if (object) delete object; }

  T *object;
};

//---------------------------------------------------------------------------

template <class T>
ObjEntryBase3D *ObjNameTable3D<T>::newObjEntry(const char *val, int hashCode)
{
  return new ObjEntry3D<T>(val,hashCode);
}

//---------------------------------------------------------------------------

template <class T> T *ObjNameTable3D<T>::get(const char *val) const
{
  ObjEntry3D<T> *ent = (ObjEntry3D<T> *)getEnt(val);
  if (!ent) return NULL;

  return ent->object;
}

//---------------------------------------------------------------------------

template <class T> T *ObjNameTable3D<T>::add(const char *val, T *v)
{
  ObjEntry3D<T> *ent = (ObjEntry3D<T> *)addEnt(val);
  if (!ent) return NULL; // Must not occur!!

  if (!ent->object) ent->object = v;

  return ent->object;
}

} // namespace Ino

//---------------------------------------------------------------------------

