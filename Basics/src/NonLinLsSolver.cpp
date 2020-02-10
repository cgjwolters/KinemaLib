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

#include "NonLinLsSolver.h"

#include "Basics.h"
#include "Exceptions.h"

#include <math.h>

namespace Ino
{

//---------------------------------------------------------------------------

NonLinLsSolver::NonLinLsSolver(int maxSolDims, int maxDataPoints)
: maxIterCount(0), relTol(1e-12), absTol(0.0),
  rank(maxSolDims), iterCount(0),
  solDims(maxSolDims), mat(maxDataPoints,maxSolDims),
  secMat(maxDataPoints,maxSolDims),
  vt(maxSolDims,maxSolDims), rhs(maxDataPoints),
  curSol(maxSolDims), deltaSol(maxSolDims)
{
  if (maxDataPoints < maxSolDims)
       throw IllegalArgumentException("NonLinLsSolver::NonLinLsSolver(");
}

//---------------------------------------------------------------------------

NonLinLsSolver::~NonLinLsSolver()
{
}

//---------------------------------------------------------------------------

void NonLinLsSolver::setSolSz(int newSolDims)
{
  if (newSolDims < 0 || newSolDims > mat.getColumns())
               throw IllegalArgumentException("NonLinLsSolver::setSolSz");

  solDims = newSolDims;
}

//---------------------------------------------------------------------------

NonLinLsSolver::Result NonLinLsSolver::solve(double relTolerance,
                                             double absTolerance,
                                             int maxIter, int maxDiverIter)
{
  if (maxIter < 1)
    throw IllegalArgumentException("NonLinLsSolver::solve");

  if (maxDiverIter < 1)
    throw IllegalArgumentException("NonLinLsSolver::solve 2");

  if (relTolerance < NumAccuracy) relTolerance = NumAccuracy;

  relTol = relTolerance;

  if (absTolerance < 0.0) absTolerance = 0.0;
  absTol = absTolerance;

  iterCount = 0;

  double lastSolLen = 0;
  int divCnt = 0;

  for (int iter=0; iter<maxIter; iter++) {
    bool hasSecDer = false;

    if (!buildProblem(mat,rhs,secMat,hasSecDer)) return Aborted;

    int svd_iter = 0;
    rank = solDims;

    if (!mat.solveLs(mat.getRows(),solDims,vt,rhs,deltaSol,
                     0.01*relTol,rank,svd_iter))
                                              return SolverError;

    if (rank < 1) return UnderDetermined;

    adaptDeltaUpdate(deltaSol);

    double solLen = deltaSol.len(solDims);

    if (iter > maxDiverIter) {
      if (solLen > lastSolLen) {
        if (divCnt > maxDiverIter) return Diverging; // Problem diverging
        divCnt++;
      }
      else divCnt = 0;
    }

    lastSolLen = solLen;

    curSol -= deltaSol;

    iterCount++;

    double curLen = curSol.len(solDims);

    if (solLen < absTol || solLen/curLen <= relTol) {
      if (rank < solDims) return UnderDetermined;

      return Ok; // We are done!
    }
  }

  if (rank < solDims) return UnderDetermined;

  return TooManyIterations;
}

//---------------------------------------------------------------------------

bool NonLinLsSolver::getStats(NonLinLsSolverStats& stats) const
{
  if (iterCount < 1) return false;

  stats.maxIterCount = maxIterCount;
  stats.relTol       = relTol;
  stats.absTol       = absTol;
  stats.fullRank     = solDims;

  stats.iterCount    = iterCount;
  stats.rank         = rank;
  stats.absSolAcc    = deltaSol.len(solDims);
  stats.solNorm      = curSol.len(solDims);
  stats.relSolAcc    = stats.absSolAcc/stats.solNorm;

  double minVal = 0.0;
  
  for (int i=0; i<solDims; i++) {
    double val = fabs(mat(i,i));

    if (i < 1 || val > stats.problemNorm) stats.problemNorm = val;

    if (val != 0.0) {
      if (i < 1 || val < minVal) minVal = val;
    }
  }

  if (minVal == 0.0) stats.problemCondition = 0.0;
  else stats.problemCondition = stats.problemNorm/minVal;

  return true;
}

} // namespace Ino

//---------------------------------------------------------------------------
