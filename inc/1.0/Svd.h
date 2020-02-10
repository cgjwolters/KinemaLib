//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------- Singular Value Decomposition -----------------------------------
//-------------- (Matrix Equation Solver) ---------------------------------------
//-------------------------------------------------------------------------------
//------------------------ Copyright Inofor Hoek Aut BV 1991..1999 --------------
//------------------------------------------------------ C.Wolters --------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

#ifndef SVD_INC
#define SVD_INC

#include "Basics.h"

typedef double *DVec ;
typedef  DVec *DMat ;

namespace Ino
{

//-------------------------------------------------------------------------------

extern const double Double_Precision;

//-------------------------------------------------------------------------------

extern bool UpperTriang(DMat mat, int m, int n, DMat u, bool fullU,
                        DMat rhs, int rhsCols, Ino::ProgressReporter *rep=NULL);

//-------------------------------------------------------------------------------

// If iter > 0 on entry, is signifies the max number of iterations allowed.
// If iter == 0 there is no limit on the number of iteration steps.
// false is returned if convergence is not reached within the specified
// number of iterations.
// true is returned upon success.

extern bool Svd(DMat Mat, int m, int n, DMat U, bool Full_U, DMat VT,
                               DMat Rhs, int Rhscols, int& iter);

//-------------------------------------------------------------------------------

extern bool GSvd2(DMat matA, DMat matB, int rowsA, int rowsB, int cols,
                  DMat rhsA, DMat rhsB, int rhsCols, double tol,
                  DMat q, DVec alpha, DVec beta, Ino::ProgressReporter *rep=NULL);

} // namespace Ino

//-------------------------------------------------------------------------------
#endif

