//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//-------------- General Matrix and Vector Definitions ----------------------
//---------------------------------------------------------------------------
//------------------------ Copyright Inofor Hoek Aut BV 1991..2004 ----------
//---------------------------------------------------------------------------
//------------------------------------------------------ C.Wolters ----------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Matrix.h"
#include "Exceptions.h"
#include "Svd.h"

#include <stddef.h>
#include <math.h>

#include <string.h>
#include <algorithm>

#ifdef _WIN32
#include <malloc.h>
#else
#include <alloca.h>
#endif

namespace Ino
{

//---------------------------------------------------------------------------

void Vector::setSize(int newSz, bool preserve, bool zeroInit)
{
  if (newSz < 0) throw IllegalArgumentException("Vector::setSize");

  if (newSz >= cap/2 && newSz <= cap) {
    if (zeroInit && newSz > sz) memset(va+sz,0,(newSz-sz)*sizeof(double));
    sz = newSz;
    return;
  }

  if (preserve && va) {
    double* newVa = new double[newSz];

    if (newSz <= sz) memmove(newVa,va,newSz*sizeof(double));
    else {
      memmove(newVa,va,sz*sizeof(double));
      if (zeroInit) memset(newVa+sz,0,(newSz-sz)*sizeof(double));
    }

    delete[] va;

    va = newVa;
  }
  else {
    delete[] va;
    va = new double[newSz];

    if (zeroInit) memset(va,0,newSz * sizeof(double));
  }
  
  cap = newSz;
  sz  = newSz;
}

//---------------------------------------------------------------------------
// For PVector

Vector::Vector(double *arr, int len)
: va(arr), sz(arr ? len : 0), cap(len)
{
}

//---------------------------------------------------------------------------

Vector::Vector(int vSize, bool zeroInit)
: va(NULL), sz(vSize), cap(vSize)
{
  if (vSize < 0) throw IllegalArgumentException("Vector::Vector");

  va = new double[cap];

  if (zeroInit) memset(va,0,sz*sizeof(double));
}

//---------------------------------------------------------------------------

Vector::Vector(const Vector& cp)
: va(NULL), sz(cp.sz), cap(cp.sz)
{
  va = new double[cap];

  memmove(va,cp.va,sz*sizeof(double));
}

//---------------------------------------------------------------------------

Vector::~Vector()
{
  delete[] va;
}

//---------------------------------------------------------------------------

Vector& Vector::operator=(const Vector& src)
{
  setSize(src.sz,false,false);

  memmove(va,src.va,sz*sizeof(double));

  return *this;
}

//---------------------------------------------------------------------------

void Vector::clear()
{
  memset(va,0,sz*sizeof(double));
}

//---------------------------------------------------------------------------

double Vector::operator[](int idx) const
{
  if (idx < 0 || idx >= sz) throw IndexOutOfBoundsException("Vector::operator[]");

  return va[idx];
}

//---------------------------------------------------------------------------

double& Vector::operator[](int idx)
{
  if (idx < 0 || idx >= sz)
                 throw IndexOutOfBoundsException("Vector::operator[]");

  return va[idx];
}

//---------------------------------------------------------------------------

Vector Vector::operator+(const Vector& v) const
{
  if (sz != v.sz) throw IllegalArgumentException("Vector::operator+");

  Vector result(sz,false);

  for (int i=0; i<sz; i++) result.va[i] = va[i] + v.va[i];

  return result;
}

//---------------------------------------------------------------------------

Vector& Vector::operator+=(const Vector& v)
{
  if (sz != v.sz) throw IllegalArgumentException("Vector::operator+=");

  for (int i=0; i<sz; i++) va[i] += v.va[i];

  return *this;
}

//---------------------------------------------------------------------------

Vector Vector::operator-(const Vector& v) const
{
  if (sz != v.sz) throw IllegalArgumentException("Vector::operator-");

  Vector result(sz,false);

  for (int i=0; i<sz; i++) result.va[i] = va[i] - v.va[i];

  return result;
}

//---------------------------------------------------------------------------

Vector& Vector::operator-=(const Vector& v)
{
  if (sz != v.sz) throw IllegalArgumentException("Vector::operator-=");

  for (int i=0; i<sz; i++) va[i] -= v.va[i];

  return *this;
}

//---------------------------------------------------------------------------

double Vector::operator*(const Vector& v) const
{
  if (sz != v.sz) throw IllegalArgumentException("Vector::operator*");

  double prod = 0.0;

  for (int i=0; i<sz; i++) prod += (va[i] * v.va[i]);

  return prod;
}

//---------------------------------------------------------------------------

Vector Vector::operator*(double fact) const
{
  Vector result(sz,false);

  for (int i=0; i<sz; i++) result.va[i] = va[i] * fact;

  return result;
}

//---------------------------------------------------------------------------

Vector& Vector::operator*=(double fact)
{
  for (int i=0; i<sz; i++) va[i] *= fact;

  return *this;
}

//---------------------------------------------------------------------------

double Vector::len() const
{
  double len = 0.0;

  for (int i=0; i<sz; i++) {
    double val = va[i];
    len += val*val;
  }

  return sqrt(len);
}

//---------------------------------------------------------------------------

double Vector::len(int dims) const
{
  if (dims > sz) throw IllegalArgumentException("Vector::length");

  double len = 0.0;

  for (int i=0; i<dims; i++) {
    double val = va[i];
    len += val*val;
  }

  return sqrt(len);
}

//---------------------------------------------------------------------------

void Matrix::alloc(int rows, int cols, bool setZero)
{
  rws = rows;
  cls = cols;

  delete[] mat;
  delete[] matRows;

  mat     = new double*[rws];
  matRows = new double[rws*cls];

  double *p = matRows;

  for (int i=0; i<rws; i++) {
    mat[i] = p;
    p += cls;
  }

  if (setZero) memset(matRows,0,rws*cls*sizeof(double));
}

//---------------------------------------------------------------------------

Matrix::Matrix(int rows, int columns, double *elems) // For PMatrix
: mat(new double*[rows]), matRows(elems),
  rws(rows), cls(columns)
{
  double *p = matRows;

  for (int i=0; i<rws; i++) {
    mat[i] = p;
    p += cls;
  }
}

//---------------------------------------------------------------------------

Matrix::Matrix(int rows, int cols, bool zeroInit)
: mat(NULL), matRows(NULL), rws(rows), cls(cols)
{
  alloc(rows,cols,zeroInit);
}

//---------------------------------------------------------------------------

Matrix::Matrix(const Matrix& cp)
: mat(new double*[cp.rws]), matRows(new double[cp.rws*cp.cls]),
  rws(cp.rws), cls(cp.cls)
{
  double *p = matRows;

  for (int i=0; i<rws; i++) {
    mat[i] = p;
    p += cls;
  }

  memmove(matRows,cp.matRows,rws*cls*sizeof(double));
}

//---------------------------------------------------------------------------

Matrix::~Matrix()
{
  delete[] matRows;
  delete[] mat;
}

//---------------------------------------------------------------------------

void Matrix::setRows(int rows, bool zeroInit)
{
  if (rows < 1) throw IllegalArgumentException("Matrix::setRows");

  alloc(rows,cls,zeroInit);
}

//---------------------------------------------------------------------------

void Matrix::setColumns(int cols, bool zeroInit)
{
  if (cols < 1) throw IllegalArgumentException("Matrix::setColumns");

  alloc(rws,cols,zeroInit);
}

//---------------------------------------------------------------------------

void Matrix::resize(int rows, int cols, bool zeroInit)
{
  if (rows < 1 || cols < 1) throw IllegalArgumentException("Matrix::resize");

  alloc(rows,cols,zeroInit);
}

//---------------------------------------------------------------------------

Matrix& Matrix::operator=(const Matrix& src)
{
  if (rws != src.rws || cls != src.cls) alloc(src.rws,src.cls,false);

  memmove(matRows,src.matRows,rws*cls*sizeof(double));

  return *this;
}

//---------------------------------------------------------------------------

void Matrix::clear()
{
  memset(matRows,0,rws*cls*sizeof(double));
}

//---------------------------------------------------------------------------

void Matrix::upperTriang(int rows, int cols, Matrix& rhs, int rhsCols,
                                                  Ino::ProgressReporter *rep)
{
  if (rws < rows || cls < cols || rhs.rws < rws || cols < 1 || rhsCols < 1) 
                      throw IllegalArgumentException("Matrix2::upperTriang");

  UpperTriang(mat,rows,cols,NULL,false,rhs.mat,rhsCols,rep);
}

//---------------------------------------------------------------------------

bool Matrix::solveLs(int rows, int cols, double **vt,
                             double **rhs, int rhs_cols, double **sol,
                                   double relTol, int& rank, int& svd_iter)
{
  relTol = fabs(relTol);
  if (relTol < Double_Precision) relTol = Double_Precision;

  if (rows < cols) throw IllegalArgumentException("Matrix::solveLs");

  if (!Svd(mat,rows,cols,NULL,false,vt,rhs,rhs_cols,svd_iter)) return false;

  // The square matrix norm of mat is equal to the absolute value 
  // of the largest singular value...

  double norm2 = 0.0;

  for (int i=0; i<cols; i++) {
    double a = fabs(mat[i][i]);

    if (a > norm2) norm2 = a;
  }

  // Determine rank...

  norm2 *= relTol;

  rank = cols;

  for (int i=0; i<cols; i++) {
    if (fabs(mat[i][i]) < norm2) {
      mat[i][i] = 0.0;
      rank--;
    }
  }

  // Find solution...

  if (rhs) {
    for (int i=0; i<rhs_cols; i++) {
      for (int j=0; j<cols; j++) {
        double sum = 0.0;

        for (int k=0; k<cols; k++) {
          double mk = mat[k][k];
          if (mk == 0.0) continue;

          if (fabs(mk) >= norm2) sum += vt[k][j]*rhs[k][i]/mk;
        }
    
        sol[j][i] = sum;
      }
    }
  }

  return true;
}

//---------------------------------------------------------------------------

bool Matrix::solveSvd(int rows, int cols, Matrix& vt, 
                                                 int& rank, int& svd_iter)
{
  if (vt.getRows() < cols) vt.setRows(cols);
  if (vt.getColumns() < cols) vt.setColumns(cols);

  return solveLs(rows,cols,vt.mat,NULL,0,NULL,0.0,rank,svd_iter);
}

//---------------------------------------------------------------------------

bool Matrix::solveSvd(int rows, int cols, Matrix& vt, Matrix& u,
                                   bool fullU, int& rank, int& svd_iter)
{
  if (vt.getRows() < cols) vt.setRows(cols);
  if (vt.getColumns() < cols) vt.setColumns(cols);

  if (u.getRows()    < rows) u.setRows(rows);
  if (u.getColumns() < cols) u.setColumns(cols);

  if (rows < cols) throw IllegalArgumentException("Matrix::solveLs");

  bool ok = Svd(mat,rows,cols,u.mat,fullU,vt.mat,NULL,0,svd_iter);
  if (!ok) return false;

  double norm2 = 0.0;

  for (int i=0; i<cols; i++) {
    double a = fabs(mat[i][i]);

    if (a > norm2) norm2 = a;
  }

  // Determine rank...

  norm2 *= Double_Precision;

  rank = cols;

  for (int i=0; i<cols; i++) {
    if (fabs(mat[i][i]) < norm2) {
      mat[i][i] = 0.0;
      rank--;
    }
  }

  return true;
}

//---------------------------------------------------------------------------

bool Matrix::solveLs(int rows, int cols, Matrix& vt, Vector& rhs,
                     Vector& sol, double relTol, int& rank, int& svd_iter)
{
  if (rhs.size() < rows) throw IllegalArgumentException("Matrix::solveLs");

  if (vt.getRows() < cols || vt.getColumns() < cols) vt.resize(cols,cols);

  if (sol.size() < cols) sol.setSize(cols);

  int rhsSz = rhs.size();

  Matrix matRhs(rhsSz,1);

  for (int i=0; i<rhsSz; i++) matRhs(i,0) = rhs[i];

  int solSz = sol.size();

  Matrix matSol(solSz,1);

  for (int i=0; i<solSz; i++) matSol(i,0) = sol[i];

  bool ok = solveLs(rows,cols,vt.mat,matRhs.mat,1,matSol.mat,relTol,rank,svd_iter);

  for (int i=0; i<rhsSz; i++) rhs[i] = matRhs(i,0);
  for (int i=0; i<solSz; i++) sol[i] = matSol(i,0);

  return ok;
}

//---------------------------------------------------------------------------

bool Matrix::solveLs(int rows, int cols, Matrix& vt, Matrix& rhs,
                      Matrix& sol, double relTol, int& rank, int& svd_iter)
{
  if (rhs.getRows() < rows) throw IllegalArgumentException("Matrix::solveLs");

  if (vt.getRows() < cols || vt.getColumns() < cols) vt.resize(cols,cols);

  if (sol.getRows() < cols ||
       sol.getColumns() < rhs.getColumns()) sol.resize(cols,rhs.getColumns());

  return solveLs(rows,cols,vt.mat,rhs.mat,rhs.cls,sol.mat,relTol,rank,svd_iter);
}

//---------------------------------------------------------------------------

void Matrix::transpose(Matrix& transposedMat) const
{
  if (transposedMat.rws != cls || transposedMat.cls != rws)
    transposedMat.alloc(cls,rws,false);

  for (int i=0; i<rws; ++i) {
    double *row = mat[i];

    for (int j=0; j<cls; ++j) transposedMat.mat[j][i] = row[j];
  }
}

//---------------------------------------------------------------------------

void Matrix::multiply(const Matrix& b, Matrix& result) const
{
  if (b.rws != cls) throw IllegalArgumentException("Matrix::multiply");

  if (result.rws != rws || result.cls != b.cls) result.alloc(rws,b.cls,false);

  for (int i=0; i<rws; ++i) {
    double *row = mat[i];

    for (int j=0; j<b.cls; ++j) {
      double s = 0.0;

      for (int k=0; k<cls; ++k) s += row[k] * b.mat[k][j];

      result.mat[i][j] = s;
    }
  }
}

//---------------------------------------------------------------------------

bool Matrix::invertGauss(Matrix& invMat)
{
  if (rws != cls) throw IllegalFormatException("Matrix::invertGauss");

  int *ipvt = (int *)alloca(rws * sizeof(int));

  Matrix inv(rws,rws);

  for (int i=0; i<rws; ++i) {
    ipvt[i] = i;
    inv.mat[i][i] = 1.0;
  }

  for (int i=0; i<rws; ++i) {
    int idx = -1, lastNonZero = -1;
    double mx = 0.0;

    for (int j=i; j<rws; ++j) {
      double v = abs(mat[ipvt[j]][i]);

      if (v > mx) {
        mx = v;
        idx = j;
      }

      if (v > 0.0) lastNonZero = j;
    }

    if (idx < 0) return false; // Matrix is singular

    if (i != idx) { // Swap rows
      int ir = ipvt[i]; ipvt[i] = ipvt[idx]; ipvt[idx] = ir;
    }

    idx = ipvt[i];
    double sc = 1.0/mat[idx][i];

    double *row = mat[idx];
    double *invRow = inv.mat[idx];

    for (int j=0; j<cls; ++j) {
      row[j] *= sc;
      invRow[j] *= sc;
    }

    for (int j=0; j<=lastNonZero; ++j) {
      if (i == j) continue;

      int dstIdx = ipvt[j];

      double *dstRow    = mat[dstIdx];
      double *dstInvRow = inv.mat[dstIdx];

      sc = dstRow[i];

      if (sc == 0.0) continue;

      for (int k=0; k<cls; ++k) {
        dstRow[k]    -= row[k] * sc;
        dstInvRow[k] -= invRow[k] * sc;
      }

      dstRow[i] = 0.0;
    }
  }

  if (invMat.rws != rws || invMat.cls != rws) invMat.alloc(rws,rws);

  for (int i=0; i<rws; ++i) {
    double *dstRow = invMat.mat[i];
    double *srcRow = inv.mat[ipvt[i]];

    for (int j=0; j<cls; ++j) dstRow[j] = srcRow[j];
  }

  return true;
}

//---------------------------------------------------------------------------

bool Matrix::invertMoorePenrose(Matrix& invMat)
{
  if (invMat.rws != cls || invMat.cls != rws) invMat.alloc(cls,rws);

  if (rws < cls) {
    Matrix tr(cls,rws);
    transpose(tr);

    Matrix t1(rws,rws);
    multiply(tr,t1);

    Matrix t2(rws,rws);
    if (!t1.invertGauss(t2)) return false;

    tr.multiply(t2,invMat);

    return true;
  }
  else if (rws > cls) {
    Matrix tr(cls,rws);
    transpose(tr);

    Matrix t1(cls,cls);
    tr.multiply(*this,t1);

    Matrix t2(cls,cls);
    if (!t1.invertGauss(t2)) return false;

    Matrix t3(rws,cls);
    multiply(t2,t3);

    t3.transpose(invMat);

    return true;
  }
  else return invertGauss(invMat);
}

//---------------------------------------------------------------------------
// Solve the symmetric positve definite banded system mat * x = rhs
// Storage is optimized: only diagonal and upper are stored, set
// the number of colums to the bandwidth.
// Element [i,j] is stored at [i,j-i], where j >= i, i.e. the diagonal is in
// column zero.
//
// Vector rhs contains the solution vector upon return;
//
// Speed is proportional to the number of rows (for small bandwidth)
//
// This method uses LDLT decomposition. On return the diagonal contains
// the elements of D and the uppertriangle contains the off-diagonal elements
// of LT.
//
// As an optimization this routine tries to avoid multiplying zeroes (see lwbIdx)
// This may greatly speed up things if only a few equations actually need the
// matrix bandwidth.

void Matrix::solveLDLT(Vector& rhs)
{
  if (rws < cls)
    throw IllegalArgumentException("Matrix::solveLDLT (rows < columns");

  if (rhs.size() != rws)
    throw IllegalArgumentException("Matrix::solveLDLT (unmatching row counts");

  enum { MaxAllocSz = 4000 };

  int *lwbIdx;
  double *r;

  if (rws > MaxAllocSz) {
    lwbIdx = new int[rws];
    r =      new double[rws];
  }
  else {
    lwbIdx = (int *)alloca(rws * sizeof(int));
    r      = (double *)alloca(rws * sizeof(double));
  }

  for (int i=0; i<cls; ++i) lwbIdx[i] = 0;
  for (int i=cls; i<rws; ++i) lwbIdx[i] = i-cls+1;

//  int ops = 0;

  for (int i=0; i<rws; ++i) {
    double dd = mat[i][0];

    for (int j=lwbIdx[i]; j<i; ++j) {
      double m = mat[j][i-j];

      r[j] = mat[j][0] * m;
      dd -= r[j] * m;
    }

    mat[i][0] = dd;

    int upb = std::min(i + cls,rws);

    for (int j=i+1; j<upb; ++j) {
      double& m = mat[i][j-i];

      int st = std::max(lwbIdx[i],lwbIdx[j]);

      for (int k=st;k<i; ++k) {
        m -= mat[k][j-k] * r[k];
//        ops++;
      }

      m /= dd;

      if (fabs(m) < 1e-12) {
        if (lwbIdx[j] == i) lwbIdx[j]++;
      }
    }
  }

  for (int i=0; i<rws; ++i) {
    for (int j=lwbIdx[i]; j<i; ++j) rhs[i] -= mat[j][i-j] * rhs[j];
  }

  for (int i=0; i<rws; ++i) rhs[i] /= mat[i][0];

  for (int i=rws-1; i>=0; --i) {
    int upb = std::min(i + cls,rws);

    for (int j=i+1; j<upb; ++j) rhs[i] -= mat[i][j-i] * rhs[j];
  }

  if (rws > MaxAllocSz) {
    delete[] lwbIdx;
    delete[] r;
  }

//  ops += 0; // Put breakpoint here
}

//---------------------------------------------------------------------------
// Solve the symmetric positve definite banded system mat * x = rhs
// Storage is optimized: only diagonal and upper are stored, set
// the number of colums to the bandwidth.
// Element [i,j] is stored at [i,j-i], where j >= i, i.e. the diagonal is in
// column zero.
//
// Matrix rhs contains the solution vector(s) upon return;
//
// Speed is proportional to the number of rows (for small bandwidth)
//
// This method uses LDLT decomposition. On return the diagonal contains
// the elements of D and the uppertriangle contains the off-diagonal elements
// of LT.
//
// As an optimization this routine tries to avoid multiplying zeroes (see lwbIdx)
// This may greatly speed up things if only a few equations actually need the
// matrix bandwidth.

void Matrix::solveLDLT(Matrix& rhs)
{
  if (rws < cls)
    throw IllegalArgumentException("Matrix::solveLDLT (less rows than columns");

  if (rhs.rws != rws)
    throw IllegalArgumentException("Matrix::solveLDLT (unmatching row counts");

  enum { MaxAllocSz = 4000 };

  int *lwbIdx;
  double *r;

  if (rws > MaxAllocSz) {
    lwbIdx = new int[rws];
    r =      new double[rws];
  }
  else {
    lwbIdx = (int *)alloca(rws * sizeof(int));
    r      = (double *)alloca(rws * sizeof(double));
  }

  for (int i=0; i<cls; ++i) lwbIdx[i] = 0;
  for (int i=cls; i<rws; ++i) lwbIdx[i] = i-cls+1;

  for (int i=0; i<rws; ++i) {
    double dd = mat[i][0];

    for (int j=lwbIdx[i]; j<i; ++j) {
      double m = mat[j][i-j];

      r[j] = mat[j][0] * m;
      dd -= r[j] * m;
    }

    mat[i][0] = dd;

    int upb = std::min(i + cls,rws);

    for (int j=i+1; j<upb; ++j) {
      double& m = mat[i][j-i];

      int st = std::max(lwbIdx[i],lwbIdx[j]);

      for (int k=st;k<i; ++k) m -= mat[k][j-k] * r[k];

      m /= dd;

      if (fabs(m) < 1e-12) {
        if (lwbIdx[j] == i) lwbIdx[j]++;
      }
    }
  }

  int rhsColSz = rhs.getColumns();

  for (int i=0; i<rws; ++i) {
    for (int j=lwbIdx[i]; j<i; ++j) {
      double m = mat[j][i-j];

      for (int k=0; k<rhsColSz; ++k) rhs(i,k) -= m * rhs(j,k);
    }
  }

  for (int i=0; i<rws; ++i) {
    double d = mat[i][0];
    for (int j=0; j<rhsColSz; ++j) rhs(i,j) /= d;
  }

  for (int i=rws-1; i>=0; --i) {
    int upb = std::min(i + cls,rws);

    for (int j=i+1; j<upb; ++j) {
      double m = mat[i][j-i];

      for (int k=0; k<rhsColSz; ++k) rhs(i,k) -= m * rhs(j,k);
    }
  }

  if (rws > MaxAllocSz) {
    delete[] lwbIdx;
    delete[] r;
  }
}

//---------------------------------------------------------------------------

bool Matrix::gSvd2(Matrix& matA, Matrix& matB,
                   int rowsA, int rowsB, int cols,
                   Matrix& rhsA, Matrix& rhsB, int rhsCols, double tol,
                   Matrix& q, Vector& alpha, Vector& beta,
                   Ino::ProgressReporter *rep)
{
  if (rowsA < cols || rowsB < cols || cols < 2)
                            throw IllegalArgumentException("Matrix2::gSvd2");
  if (matA.getColumns() < cols || matB.getColumns() < cols)
                            throw IllegalArgumentException("Matrix2::gSvd2");
  if (matA.getRows() < rowsA || matB.getRows() < rowsB)
                            throw IllegalArgumentException("Matrix2::gSvd2");

  if (rhsA.getRows() < rowsA || rhsB.getRows() < rowsB)
                          throw IllegalArgumentException("Matrix2::solveLs");

  if (rhsB.getColumns() < rhsCols || rhsB.getColumns() < rhsCols)
                          throw IllegalArgumentException("Matrix2::solveLs");

  if (q.getRows() < cols || q.getColumns() < cols) q.resize(cols,cols);

  if (alpha.size() < cols) alpha.setSize(cols);
  if (beta.size() < cols)  beta.setSize(cols);

  tol = fabs(tol);

  return GSvd2(matA.mat,matB.mat,rowsA,rowsB,cols,
               rhsA.mat,rhsB.mat,rhsCols,tol,q.mat,alpha.va,beta.va,rep);
}

} // namespace Ino

//---------------------------------------------------------------------------
