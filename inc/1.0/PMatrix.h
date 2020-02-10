// --------------------------------------------------------------------------
// ---------------- Persistent Vector and Matrices --------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// -------------------- Copyright Inofor Hoek Aut BV, Sept 2013 -------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

#ifndef INO_PMATRIX_INC
#define INO_PMATRIX_INC

#include "Matrix.h"
#include "PersistentIO.h"

namespace Ino 
{

// --------------------------------------------------------------------------

class PVector : public Persistable, public Vector
{
public:
  PVector(int size, bool zeroInit=true);
  PVector(const PVector& cp);
  PVector(const Vector& cp);

  // Persistence:

  PVector(PersistentReader& pi);
  virtual void definePersistentFields(PersistentWriter& po) const;
  virtual void writePersistentObject(PersistentWriter& po) const;

  // Special purpose: read/write vector as a field
  // May call, if you know what you are doing.
  void addPersistentField(PersistentWriter& po, const char *fldName) const;
  PVector(PersistentReader& pi, const char *fldName);
  void writeArray(PersistentWriter& po, const char *fldName) const;
};

// --------------------------------------------------------------------------

class PMatrix : public Persistable, public Matrix
{
public:
  PMatrix(int rows, int cols, bool zeroInit=true);
  PMatrix(const PMatrix& cp);
  PMatrix(const Matrix& cp);

  // Persistence:

  PMatrix(PersistentReader& pi);
  virtual void definePersistentFields(PersistentWriter& po) const;
  virtual void writePersistentObject(PersistentWriter& po) const;

  // Special purpose: read/write matrix as a field
  // May call, if you know what you are doing.
  void addPersistentField(PersistentWriter& po, const char *fldName) const;
  PMatrix(PersistentReader& pi, const char *fldName);
  void writeArray(PersistentWriter& po, const char *fldName) const;
};

} // namespace

// --------------------------------------------------------------------------
#endif
