//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//-------------- General Abstract Non-linear Least Squares Solver------------
//-------------- (Newton-Raphson Method) ------------------------------------
//---------------------------------------------------------------------------
//------------------------ Copyright Inofor Hoek Aut BV 2004 ----------------
//---------------------------------------------------------------------------
//------------------------------------------------------ C.Wolters ----------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INO_NONLINLSSOLVER_INC
#define INO_NONLINLSSOLVER_INC

#include "Matrix.h"

namespace Ino
{

//---------------------------------------------------------------------------

struct NonLinLsSolverStats
{
  int maxIterCount;
  double relTol;
  double absTol;
  int fullRank;

  int iterCount;
  int rank;

  double relSolAcc;
  double absSolAcc;
  double solNorm;
  double problemNorm;
  double problemCondition;
};

//---------------------------------------------------------------------------

class NonLinLsSolver
{
public:
  enum Result { Ok, Aborted, UnderDetermined, Diverging,
                TooManyIterations, SolverError };
private:
  int maxIterCount;
  double relTol;
  double absTol;
  int rank;
  int iterCount;

  int solDims;

  Matrix mat;
  Matrix secMat;
  Matrix vt;
  Vector rhs;
  Vector curSol;
  Vector deltaSol;

protected:
  virtual bool buildProblem(Matrix& fstDerMat, Vector& rhs,
                            Matrix& secDerMat, bool& hasSecDer) = 0;

  virtual bool adaptDeltaUpdate(Vector& /* dSol */) { return false; }

public:
  NonLinLsSolver(int maxSolDims, int maxDataPoints);
  virtual ~NonLinLsSolver();

  void setSolSz(int newSolDims);
  int getSolSz() const { return solDims; }
  int getDataSz() const { return mat.getRows(); }

  virtual Result solve(double relTolerance, double absTolerance,
                                     int maxIter, int maxDiverIter=5);

  Vector& getSol() { return curSol; }
  const Vector& getSol() const { return curSol; }

  bool getStats(NonLinLsSolverStats& stats) const;
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
