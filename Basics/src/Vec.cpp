// --------------------------------------------------------------------------
// ---------------- Vectors 2D and 3D ---------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// -------------------- Copyright Inofor Hoek Aut BV, Oct 1996 --------------
// --------------------------------------------------------------------------

#include "Vec.h"
#include "Trf.h"

#include <cmath>
#include <cfloat>

namespace Ino
{

// -------------------------------------------------------------------------
/** \addtogroup geometry 2D/3D Geometry
  @{
*/

// -------------------------------------------------------------------------
/** \file Vec.h
  Defines basic 2D (\ref Ino::Vec2 "\c Vec2") and 3D vectors
  (\ref Ino::Vec3 "\c Vec3").

  \see Trf.h

  \author C. Wolters
  \date Oct 1996
*/

// -------------------------------------------------------------------------
/** \class Vec2
  Defines a basic 2D vector with fields \c x and \c y.

  All fields and methods of this class are public.

  \see Trf.h

  \author C. Wolters
  \date Oct 1996
*/

/**
@}
*/

// -------------------------------------------------------------------------
/** A small distance value that is used to compare \ref Ino::Vec2 "vectors"
    for equality.

    Since this static field is public its value may be changed.\n
    It should however be changed only at the beginning of a program, since
    various methods depend on its value.
*/
double Vec2::IdentDist = 1.0E-04;

/** A small angle value (in radians) that is used to compare
    \ref Ino::Vec2 "vector" angles for equality.

    Since this static field is public its value may be changed.\n
    It should however be changed only at the beginning of a program, since
    various methods depend on its value.
*/
double Vec2::IdentDir  = 1.0E-03;

/// The value of \c Pi, i.e. \c 3.1415926535...
const double Vec2::Pi  = 3.141592653589793238462643383279502884197169;

/// The value of \c Pi*2.
const double Vec2::Pi2 = 2.0 * 3.141592653589793238462643383279502884197169;

// --------------------------------------------------------------------------
/** \var double Vec2::x
   The x-coordinate of this vector.
*/

// --------------------------------------------------------------------------
/** \var double Vec2::y
   The y-coordinate of this vector.
*/

// --------------------------------------------------------------------------
/** \var bool Vec2::isDerivative
  The mode of this vector.

  If \c false the implicit third homogenous coordinate has a value of one (1),
  otherwise is has a value of zero.

  \par
  Some will say that if this field is \c true then this is a \b point,
  whereas if this field is \c false then this is a \b vector.
  \par
  The difference is only apparent when method \ref transform2(const Trf2&)
  "transform2" is called:\n
  If this field is \c true then the transform will only scale and rotate,
  wheras if this field is \c false then the transform will also translate.
*/

// --------------------------------------------------------------------------
/** \fn Vec2::Vec2(double cx=0, double cy=0.0)
   Constructor.

   Field \ref isDerivative is initialized to \c false.
*/

// --------------------------------------------------------------------------
// ------- Return length of vector ------------------------------------------
// --------------------------------------------------------------------------
/** Returns the length of this vector.
  \return This vectors length.
*/
double Vec2::len2() const
{
  return sqrt(x*x + y*y);
}

// --------------------------------------------------------------------------
/** \fn double Vec2::lenSq2() const
  Returns the <b> square of</b> the length of this vector.
  This method avoids the calculation of a square root.
  (May be advantageous for performance reasons).
  \return The square of the length of this vector
*/

// --------------------------------------------------------------------------
// ---------- Make vectors length equal to 1.0 and return old length --------
// --------------------------------------------------------------------------
/** Scales this vector to unit length.
    This is a no-op if <tt>len2() == 0.0</tt>.

    \return The previous length.
*/
double Vec2::unitLen2()
{
  double ln = len2();

  if (ln > 0.0) {
   x /= ln; y /=ln;
  }

  return ln;
}

// --------------------------------------------------------------------------
// -------- Set length of vector, return old length -------------------------
// --------------------------------------------------------------------------
/** Scales this vector to a new length.
   This is a no-op if <tt>len2() == 0.0</tt> was \c true on entry to this
   method.
   \param newLen The new length of this vector.
   \return The previous length of this vector.
*/
double Vec2::len2(double newLen)
{
  double oldLen = len2();

  if (oldLen > 0.0) {
    double fact = newLen/oldLen;
    x *= fact; y*= fact;
  }

  return oldLen;
}

// --------------------------------------------------------------------------
// ---------- Return direction of vector in radians (-pi .. pi) -------------
// --------------------------------------------------------------------------
/** Returns the direction of this vector in radians.
   \return The vector direction: <tt>-Pi < direction <= Pi</tt>.
*/
double Vec2::angle() const
{
  if (fabs(x) < DBL_MIN && fabs(y) < DBL_MIN) {
    return 0.0;
  }

  return atan2(y,x);
}

// --------------------------------------------------------------------------
// ----------- Return distance of two vectors -------------------------------
// --------------------------------------------------------------------------
/** Returns the distance of this vector to another vector.
  \param v The vector to calculate the distance to.
  \return The distance between this vector and vector \c v.
*/
double Vec2::distTo2(const Vec2& v) const
{
  Vec2 lv(v.x-x,v.y-y);

  return lv.len2();
}

// --------------------------------------------------------------------------
// ----------- Return square of distance of two vectors ---------------------
// --------------------------------------------------------------------------
/** Returns the <b> square of</b> the distance of this vector to another vector.
  This method avoids the calculation of a square root.
  (May be advantageous for performance reasons).
  \param v The vector to calculate the squared distance to.
  \return The square of the distance between this vector and vector \c v.
*/
double Vec2::sqDistTo2(const Vec2& v) const
{
  Vec2 lv(v.x-x,v.y-y);

  return lv.x*lv.x + lv.y*lv.y;
}

// -------------------------------------------------------------------------
// ----------- Return angle between vectors (anticlkwise to v) -------------
// -------------------------------------------------------------------------
/** Calculates the angle between this and another vector.
    The angle (in radians) is calculated in counterclockwise direction from
    this vector to the supplied vector.\n
    If either vector has a zero length, zero will be returned.
    \param v The vector to calculate the angle to.
    \return The angle between \c this and vector \c v
    (<tt>-Pi < angle <= Pi</tt>).
*/
double Vec2::angleTo2(const Vec2& v) const
{
  double yy = x*v.y - y*v.x;
  double xx = x*v.x + y*v.y;

  if (fabs(xx) <= DBL_MIN && fabs(yy) <= DBL_MIN) return 0.0;

  return atan2(yy,xx);
}

// --------------------------------------------------------------------------
/** \fn double Vec2::operator *(const Vec2& v) const
  Calculates the inner product of this vector and another vector.

  \param v The vector to calculate the inner product with.
  \return The inner product of this vector and vector \c v.
*/

// --------------------------------------------------------------------------
// ------------- Multiply by a factor ---------------------------------------
// --------------------------------------------------------------------------
/** Scalar multiply operator, scales this vector by a factor.
   \param fact The factor by which to multiply this vector.
   \return The scaled vector.

   \note Use operator*=(double fact) instead of this operator
    whenever possible.\n
    Returning a vector object is an expensive operation.
*/
Vec2 Vec2::operator * (double fact) const
{
  Vec2 v(x*fact, y*fact);

  return v;
}

// --------------------------------------------------------------------------
// ------------- Divide by a factor -----------------------------------------
// --------------------------------------------------------------------------
/** Scalar divide operator, scales this vector by division.
   \param fact The factor by which to divide this vector.
   \return The scaled vector.

   \note Use operator/=(double fact) instead of this operator
    whenever possible.\n
    Returning a vector object is an expensive operation.
*/
Vec2 Vec2::operator / (double fact) const
{
  Vec2 v(x/fact, y/fact);

  return v;
}

// --------------------------------------------------------------------------
// ------------- Sum of two vectors -----------------------------------------
// --------------------------------------------------------------------------
/** Vector addition, adds this vector to another vector.
    \param v The vector to add to this vector.
    \return The sum of this vector and vector \c v.

    \note Use operator+=(const Vec2& v) instead of this operator
    whenever possible.\n
    Returning a vector object is an expensive operation.
*/
Vec2 Vec2::operator + (const Vec2& v) const
{
  return Vec2(x+v.x,y+v.y);
}

// --------------------------------------------------------------------------
// ------------- Difference of two vectors ----------------------------------
// --------------------------------------------------------------------------
/** Vector difference, subtracts another vector from this vector.
    \param v The vector to subtract from this vector.
    \return The difference between this vector and vector \c v.

    \note Use operator-=(const Vec2& v) instead of this operator
    whenever possible.\n
    Returning a vector object is an expensive operation.
*/
Vec2 Vec2::operator - (const Vec2& v) const
{
  return Vec2(x-v.x,y-v.y);
}

// --------------------------------------------------------------------------
/** \fn void Vec2::operator *= (double fact)
   Scalar multiply/assign operator, scales this vector by a factor.
   \param fact The factor by which to multiply this vector.
*/

// --------------------------------------------------------------------------
/** \fn void Vec2::operator /= (double fact)
   Scalar divide/assign operator, scales this vector by division.
   \param fact The factor by which to divide this vector.
*/

// --------------------------------------------------------------------------
/** \fn void Vec2::operator += (const Vec2& v)
   Vector addition/assign operator, adds another vector to this vector.
   \param v The vector to add to this vector.
*/

// --------------------------------------------------------------------------
/** \fn void Vec2::operator -= (const Vec2& v)
   Vector difference/assign operator, subtracts another vector
   from this vector.
   \param v The vector to subtract from this vector.
*/

// --------------------------------------------------------------------------
/** \fn bool Vec2::operator == (const Vec2& v) const
   Equality operator.

   Uses static field \ref IdentDist to determine the result.
   \param v The vector to compare this vector with.
   \return \c true if <tt>distTo2(v) <= IdentDist</tt>.
*/

// --------------------------------------------------------------------------
/** \fn bool Vec2::operator != (const Vec2& v) const
   Inequality operator.

   Uses static field \ref IdentDist to determine the result.
   \param v The vector to compare this vector with.
   \return \c true if <tt>distTo2(v) > IdentDist</tt>.
*/

// --------------------------------------------------------------------------
// ------------ Rotate vector 90 degrees anticlockwise ----------------------
// --------------------------------------------------------------------------
/** Rotates this vector 90 degrees counterclockwise.
*/
void Vec2::rot90()
{
  double h = x; x = -y; y = h;
}

// --------------------------------------------------------------------------
// ------------ Rotate vector 180 degrees -----------------------------------
// --------------------------------------------------------------------------
/** Rotates this vector 180 degrees.
*/
void Vec2::rot180()
{
  x = -x; y = -y;
}

// --------------------------------------------------------------------------
// ------------ Rotate vector 270 degrees anticlockwise ---------------------
// --------------------------------------------------------------------------
/** Rotates this vector 270 degrees counterclockwise.
    The rotation is equivalent to 90 degrees clockwise (off course).
*/
void Vec2::rot270()
{
  double h = x; x = y; y = -h;
}

// --------------------------------------------------------------------------
// ------------ Find bisectrice from this to v according to acw -------------
// --------------------------------------------------------------------------
/** Finds the bisectrice between this vector and another vector.
    \param v The vector to use.
    \param acw (Anticlockwise) If \c true the bisectrice will lie between
    this vector and vector \c v, going in counterclockwise direction.
    If \c false the bisectrice is found in clockwise direction.
    \return The calculated bisectrice, it has unity length.

    If either this vector or the supplied vector have zero length, the
    result is undefined.
*/
Vec2 Vec2::bisect(const Vec2& v, bool acw) const
{
  Vec2 bisec;
  Vec2 or1 = *this; or1.rot90();

  if ((*this * v) > 0.0) {
    bisec = *this + v;

    if (or1 * v < 0.0) bisec.rot180();
  }
  else {
    Vec2 or2 = v; or2.rot270();

    bisec = or1 + or2;
  }

  if (!acw) bisec.rot180();

  bisec.unitLen2();

  return bisec;
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
/** Determines if this vector has the same directon as another vector.
   \param v The vector to compare with.
   \param tol The comparison tolerance, must be <tt> > 0</tt>.
   \return \c true If this vector has the same direction as vector \c v,
   \c false otherwise.

   \par Method
   A copy of this vector is rotated 90 degrees. The inner product of
   that copy with vector \c v is divided by the length of both vectors.\n
   \c true will be returned if the result is less than \c tol.
*/
bool Vec2::parallelTo2(const Vec2& v, double tol) const
{
  if (operator *(v) <= 0.0) return false;

  Vec2 nrm(*this); nrm.rot90();

  if (fabs(nrm * v) < tol * len2() * v.len2()) return true;

  return false;
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
/** Determines if this vector has opposite directon compared to another vector.
   \param v The vector to compare with.
   \param tol The comparison tolerance, must be <tt> > 0</tt>.
   \return \c true If this vector has the opposite direction with respect
   to vector \c v, \c false otherwise.

   \par Method
   A copy of this vector is rotated 90 degrees. The inner product of
   that copy with vector \c v is divided by the length of both vectors.\n
   \c true will be returned if the (absolute value of) the result is less
   than \c tol.
*/
bool Vec2::oppositeTo2(const Vec2& v, double tol) const
{
  if (operator *(v) >= 0.0) return false;

  Vec2 nrm(*this); nrm.rot90();

  if (fabs(nrm * v) < tol * len2() * v.len2()) return true;

  return false;
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
/** Transforms this vector.
   \param trf The transform to use.

   \note If \ref isDerivative is \c true, this vector is only scaled
   and rotated but \b not translated.\n
   Additionally if <tt>trf.isDerivative == true</tt> then \ref isDerivative
   of this vector will be \c true on return.
*/
void Vec2::transform2(const Trf2& trf)
{
  double nx = trf.m[0][0] * x + trf.m[0][1] * y;
          y = trf.m[1][0] * x + trf.m[1][1] * y;

  x = nx;

  if (!isDerivative) {
    x += trf.m[0][2];
    y += trf.m[1][2];

    isDerivative = trf.isDerivative;
  }
}

// --------------------------------------------------------------------------
// ------- 3D Vectors -------------------------------------------------------
// --------------------------------------------------------------------------

// -------------------------------------------------------------------------
/** \addtogroup geometry 2D/3D Geometry
  @{
*/

// -------------------------------------------------------------------------
/** \class Vec3
  Defines a basic 3D vector with fields \c x, \c y and \c z.

  This class derives from Vec2.\n
  All fields and methods of this class are public.

  \see Trf.h

  \author C. Wolters
  \date Oct 1996
*/

// --------------------------------------------------------------------------
/**
  @}
  */

// --------------------------------------------------------------------------
/** \var double Vec3::z
   The z-coordinate of this 3D vector.
*/

// --------------------------------------------------------------------------
/** \fn Vec3::Vec3(double cx, double cy, double cz)
   Constructor.
   \param cx The initial value of \c x.
   \param cy The initial value of \c y.
   \param cz The initial value of \c z.

   Field \ref isDerivative is initialized to \c false.
*/

// --------------------------------------------------------------------------
/** \fn Vec3::Vec3(const Vec2& v)
   Up conversion constructor, converts a \ref Vec2 to a 3D vector.
   \param v Supplies the \c x and \c y coordinates.

   Coordinate \c z is set to zero.\n
   Field \ref isDerivative is initialized to \c false.
*/

// --------------------------------------------------------------------------
/** \fn Vec3::Vec3(const Vec2& v, double cz)
   Constructor.
   \param v Supplies the \c x and \c y coordinates.
   \param cz The initial value of \c z.

   Field \ref isDerivative is initialized to \c false.
*/

// --------------------------------------------------------------------------
/** \fn Vec3::Vec3(const Vec3& v)
   Copy constructor.
   \param v The vector to copy from.
*/

// --------------------------------------------------------------------------
// ------- Return length of vector ------------------------------------------
// --------------------------------------------------------------------------
/** Returns the length of this vector.
  \return This vectors length.
*/
double Vec3::len3() const
{
  return sqrt(x*x + y*y + z*z);
}

// --------------------------------------------------------------------------
/** \fn double Vec3::lenSq3() const
  Returns the <b> square of</b> the length of this vector.
  This method avoids the calculation of a square root.
  (May be advantageous for performance reasons).
  \return The square of the length of this vector
*/

// --------------------------------------------------------------------------
// ---------- Make vectors length equal to 1.0 and return old length --------
// --------------------------------------------------------------------------
/** Scales this vector to unit length.
    This is a no-op if <tt>len3() == 0.0</tt>.

    \return The previous length.
*/

double Vec3::unitLen3()
{
  double ln = len3();

  if (ln > 0.0) {
    x /= ln; y /=ln; z /= ln;
  }

  return ln;
}

// --------------------------------------------------------------------------
// -------- Set length of vector, return old length -------------------------
// --------------------------------------------------------------------------
/** Scales this vector to a new length.
   This is a no-op if <tt>len3() == 0.0</tt> was \c true on entry to this
   method.
   \param newLen The new length of this vector.
   \return The previous length of this vector.
*/
double Vec3::len3(double newLen)
{
  double oldLen = len3();

  if (oldLen > 0.0) {
    double fact = newLen/oldLen;
    x *= fact; y*= fact; z *= fact;
  }

  return oldLen;
}

// --------------------------------------------------------------------------
// ----------- Return distance of two vectors -------------------------------
// --------------------------------------------------------------------------
/** Returns the distance of this vector to another vector.
  \param v The vector to calculate the distance to.
  \return The distance between this vector and vector \c v.
*/
double Vec3::distTo3(const Vec3& v) const
{
  Vec3 lv(v.x-x,v.y-y,v.z-z);

  return lv.len3();
}

// --------------------------------------------------------------------------
// ----------- Return square of distance of two vectors ---------------------
// --------------------------------------------------------------------------
/** Returns the <b> square of</b> the distance of this vector to another vector.
  This method avoids the calculation of a square root.
  (May be advantageous for performance reasons).
  \param v The vector to calculate the squared distance to.
  \return The square of the distance between this vector and vector \c v.
*/
double Vec3::sqDistTo3(const Vec3& v) const
{
  Vec3 lv(v.x-x,v.y-y,v.z-z);

  return lv.x*lv.x + lv.y*lv.y + lv.z*lv.z;
}

// --------------------------------------------------------------------------
/** Calculates the angle between this and another 3D vector.
    The calculated angle (in radians) is between 0 and <tt>pi</tt>.\n
    If either vector has a zero length, zero will be returned.
    \param v The vector to calculate the angle to.
    \return The angle between \c this and vector \c v
    (<tt>-0 <= angle <= Pi</tt>).
*/
double Vec3::angleTo3(const Vec3& v) const
{
  Vec3 out = outer(v);

  double lsq1 = sqr(x) + sqr(y) + sqr(z);
  double lsq2 = sqr(v.x) + sqr(v.y) + sqr(v.z);

  double lp = lsq1 * lsq2;
  double lo = sqr(out.x) + sqr(out.y) + sqr(out.z);

  if (lp < 1e-30 || lo < 1e-30 * lp) {
    if (operator*(v) >= 0.0) return 0.0;
    else return Vec2::Pi;
  }
  else if (lo >= lp) return Vec2::Pi/2.0;
  else {
    double a = asin(sqrt(lo/lp));
    if (operator*(v) < 0.0) a = Vec2::Pi - a;
    if (a > Vec2::Pi) a -= Vec2::Pi;
    if (a < 0.0) a += Vec2::Pi;
    
    return a;
  }
}

// --------------------------------------------------------------------------
// ------- Inner product ----------------------------------------------------
// --------------------------------------------------------------------------
/** Calculates the inner product of this vector and another vector.
  \param v The vector to calculate the inner product with.
  \return The inner product of this vector and vector \c v.
*/
double Vec3::operator * (const Vec3& v) const
{
  return x*v.x + y*v.y + z*v.z;
}

// --------------------------------------------------------------------------
// ------- Outer product ----------------------------------------------------
// --------------------------------------------------------------------------
/** Calculates the outer product of this vector and another vector.
  \param b The vector to calculate the outer product with.
  \return The outer product of this vector and vector \c v.
*/
Vec3 Vec3::outer(const Vec3& b) const
{
  Vec3 r(y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x);

  return r;
}

// --------------------------------------------------------------------------
/** \fn void Vec3::operator *= (double fact)
   Scalar multiply/assign operator, scales this vector by a factor.
   \param fact The factor by which to multiply this vector.
*/

// --------------------------------------------------------------------------
/** \fn void Vec3::operator /= (double fact)
   Scalar divide/assign operator, scales this vector by division.
   \param fact The factor by which to divide this vector.
*/

// --------------------------------------------------------------------------
/** \fn void Vec3::operator += (const Vec3& v)
   Vector addition/assign operator, adds another vector to this vector.
   \param v The vector to add to this vector.
*/

// --------------------------------------------------------------------------
/** \fn void Vec3::operator -= (const Vec3& v)
   Vector difference/assign operator, subtracts another vector
   from this vector.
   \param v The vector to subtract from this vector.
*/

// --------------------------------------------------------------------------
/** \fn bool Vec3::operator == (const Vec3& v) const
   Equality operator.

   Uses static field \ref IdentDist to determine the result.
   \param v The vector to compare this vector with.
   \return \c true if <tt>distTo3(v) <= IdentDist</tt>.
*/

// --------------------------------------------------------------------------
/** \fn bool Vec3::operator != (const Vec3& v) const
   Inequality operator.

   Uses static field \ref IdentDist to determine the result.
   \param v The vector to compare this vector with.
   \return \c true if <tt>distTo3(v) > IdentDist</tt>.
*/

// --------------------------------------------------------------------------
// ------------- Multiply by a factor ---------------------------------------
// --------------------------------------------------------------------------
/** Scalar multiply operator, scales this vector by a factor.
   \param fact The factor by which to multiply this vector.
   \return The scaled vector.

   \note Use operator*=(double fact) instead of this operator
    whenever possible.\n
    Returning a vector object is an expensive operation.
*/
Vec3 Vec3::operator * (double fact) const
{
  Vec3 v(x*fact, y*fact, z*fact);

  return v;
}

// --------------------------------------------------------------------------
// ------------- Divide by a factor -----------------------------------------
// --------------------------------------------------------------------------
/** Scalar divide operator, scales this vector by division.
   \param fact The factor by which to divide this vector.
   \return The scaled vector.

   \note Use operator/=(double fact) instead of this operator
    whenever possible.\n
    Returning a vector object is an expensive operation.
*/
Vec3 Vec3::operator / (double fact) const
{
  Vec3 v(x/fact, y/fact, z/fact);

  return v;
}

// --------------------------------------------------------------------------
// ------------- Sum of two vectors -----------------------------------------
// --------------------------------------------------------------------------
/** Vector addition, adds this vector to another vector.
    \param v The vector to add to this vector.
    \return The sum of this vector and vector \c v.

    \note Use operator+=(const Vec3& v) instead of this operator
    whenever possible.\n
    Returning a vector object is an expensive operation.
*/
Vec3 Vec3::operator + (const Vec3& v) const
{
  return Vec3(x+v.x,y+v.y,z+v.z);
}

// --------------------------------------------------------------------------
// ------------- Difference of two vectors ----------------------------------
// --------------------------------------------------------------------------
/** Vector difference, subtracts another vector from this vector.
    \param v The vector to subtract from this vector.
    \return The difference between this vector and vector \c v.

    \note Use operator-=(const Vec3& v) instead of this operator
    whenever possible.\n
    Returning a vector object is an expensive operation.
*/
Vec3 Vec3::operator - (const Vec3& v) const
{
  return Vec3(x-v.x,y-v.y,z-v.z);
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
/** Transforms this vector.
   \param trf The transform to use.

   \note If \ref isDerivative is \c true, this vector is only scaled
   and rotated but \b not translated.\n
   Additionally if <tt>trf.isDerivative == true</tt> then \ref isDerivative
   of this vector will be \c true on return.
*/
void Vec3::transform3(const Trf3& trf)
{
  double nx = trf.m[0][0]*x + trf.m[0][1]*y + trf.m[0][2]*z;
  double ny = trf.m[1][0]*x + trf.m[1][1]*y + trf.m[1][2]*z;
          z = trf.m[2][0]*x + trf.m[2][1]*y + trf.m[2][2]*z;

  x = nx; y = ny;

  if (!isDerivative) {
    x += trf.m[0][3];
    y += trf.m[1][3];
    z += trf.m[2][3];

    isDerivative = trf.isDerivative;
  }
}

} // namespace Ino

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
