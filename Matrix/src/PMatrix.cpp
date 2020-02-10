// --------------------------------------------------------------------------
// ---------------- Persistent Vector and Matrices --------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// -------------------- Copyright Inofor Hoek Aut BV, Sept 2013 -------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

#include "PMatrix.h"

#include "Exceptions.h"

namespace Ino {

// --------------------------------------------------------------------------

PVector::PVector(int size, bool zeroInit)
: Persistable(), Vector(size,zeroInit)
{
}

// --------------------------------------------------------------------------

PVector::PVector(const PVector& cp)
: Persistable(cp), Vector(cp)
{
}

// --------------------------------------------------------------------------

PVector::PVector(const Vector& cp)
: Persistable(), Vector(cp)
{
}

// --------------------------------------------------------------------------
// Persistence:

static const char fldArray[] = "Array";

// --------------------------------------------------------------------------

void PVector::definePersistentFields(PersistentWriter& po) const
{
  po.addArrayField(fldArray,(double *)0);
}

// --------------------------------------------------------------------------

PVector::PVector(PersistentReader& pi)
: Persistable(pi), 
  Vector((double *)pi.readValArray(fldArray,0),pi.readArraySize(fldArray,0))
{
}

// --------------------------------------------------------------------------

void PVector::writePersistentObject(PersistentWriter& po) const
{
  if (!va || sz < 1)
       po.writeArray(fldArray,(double *)NULL,0);
  else po.writeArray(fldArray,va,sz);
}

// --------------------------------------------------------------------------
// Special purpose: read/write vector as a field

enum { FldBufSz = 512 };

static const char *genFldName(char *buf, const char *masterFldName,
                                                      const char *fldName)
{
#ifdef _WIN32
  strcpy_s(buf,FldBufSz,masterFldName);
  strcat_s(buf,FldBufSz,fldName);
#else
  strcpy(buf,masterFldName);
  strcat(buf,fldName);
#endif

  return buf;
}

//---------------------------------------------------------------------------

void PVector::addPersistentField(PersistentWriter& po,
                                                const char *fldName) const
{
  if (!fldName || !fldName[0])
    throw IllegalArgumentException("PVector::addPersistentField(fldName)");

  char longFldName[FldBufSz] = "";
  genFldName(longFldName,fldName,fldArray);

  po.addArrayField(longFldName,(double *)0);
}

//---------------------------------------------------------------------------

static double *readDblArray(PersistentReader& pi, const char *masterFldName,
                                                  const char *fldName)
{
  char longFldName[FldBufSz] = "";
  genFldName(longFldName,masterFldName,fldName);

  return (double*)pi.readValArray(longFldName,0);
}

//---------------------------------------------------------------------------

static int readDblArraySize(PersistentReader& pi, const char *masterFldName,
                                                  const char *fldName)
{
  char longFldName[FldBufSz] = "";
  genFldName(longFldName,masterFldName, fldName);

  return pi.readArraySize(longFldName,0);
}

//---------------------------------------------------------------------------

PVector::PVector(PersistentReader& pi, const char *fldName)
: Persistable(pi),
  Vector(readDblArray(pi,fldName,fldArray),
  readDblArraySize(pi,fldName,fldArray))
{
}

//---------------------------------------------------------------------------

void PVector::writeArray(PersistentWriter& po, const char *fldName) const
{
  char longFldName[FldBufSz] = "";
  genFldName(longFldName,fldName,fldArray);

  if (!va || sz < 1)
       po.writeArray(longFldName,(double *)NULL,0);
  else po.writeArray(longFldName,va,sz);
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

PMatrix::PMatrix(int rows, int cols, bool zeroInit)
: Persistable(), Matrix(rows,cols,zeroInit)
{
}

// --------------------------------------------------------------------------

PMatrix::PMatrix(const PMatrix& cp)
: Persistable(cp), Matrix(cp)
{
}

// --------------------------------------------------------------------------

PMatrix::PMatrix(const Matrix& cp)
: Persistable(), Matrix(cp)
{
}

// --------------------------------------------------------------------------
// Persistence:

static const char fldMatRows[] = "MatRows";
static const char fldRows[]    = "Rows";
static const char fldColumns[] = "Columns";

// --------------------------------------------------------------------------

void PMatrix::definePersistentFields(PersistentWriter& po) const
{
  po.addArrayField(fldMatRows,(double *)0);
  po.addField(fldRows,typeid(long));
  po.addField(fldColumns,typeid(long));
}

// --------------------------------------------------------------------------

PMatrix::PMatrix(PersistentReader& pi)
: Matrix(pi.readInt(fldRows), pi.readInt(fldColumns),
         (double *)pi.readValArray(fldMatRows))
{
}

// --------------------------------------------------------------------------

void PMatrix::writePersistentObject(PersistentWriter& po) const
{
  po.writeArray(fldMatRows,matRows,rws*cls);
  po.writeInt(fldRows,rws);
  po.writeInt(fldColumns,cls);
}

// --------------------------------------------------------------------------
// Special purpose: read/write matrix as a field

void PMatrix::addPersistentField(PersistentWriter& po,
                                                const char *fldName) const
{
  char longFldName[FldBufSz] = "";

  po.addArrayField(genFldName(longFldName,fldName,fldMatRows),(double *)0);
  po.addField(genFldName(longFldName,fldName,fldRows),typeid(long));
  po.addField(genFldName(longFldName,fldName,fldColumns),typeid(long));

}

//---------------------------------------------------------------------------

static double *readMatArray(PersistentReader& pi, const char *masterFldName,
                                                  const char *fldName)
{
  char longFldName[FldBufSz] = "";
  genFldName(longFldName,masterFldName,fldName);

  return (double*)pi.readValArray(longFldName);
}

//---------------------------------------------------------------------------

static int readMatInt(PersistentReader& pi, const char *masterFldName,
                                                  const char *fldName)
{
  char longFldName[FldBufSz] = "";
  genFldName(longFldName,masterFldName,fldName);

  return pi.readInt(longFldName);
}

// --------------------------------------------------------------------------

PMatrix::PMatrix(PersistentReader& pi, const char *fldName)
: Persistable(pi),
  Matrix(readMatInt(pi,fldName,fldRows),
         readMatInt(pi,fldName,fldColumns),
         readMatArray(pi,fldName,fldMatRows))
{
}

// --------------------------------------------------------------------------

void PMatrix::writeArray(PersistentWriter& po, const char *fldName) const
{
  char longFldName[FldBufSz] = "";
  po.writeArray(genFldName(longFldName,fldName,fldMatRows),matRows,rws*cls);
  po.writeInt(genFldName(longFldName,fldName,fldRows),rws);
  po.writeInt(genFldName(longFldName,fldName,fldColumns),cls);
}

} // namespace