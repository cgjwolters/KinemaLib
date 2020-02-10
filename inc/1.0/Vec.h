// --------------------------------------------------------------------------
// ---------------- Vectors -------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// -------------------- Copyright Inofor Hoek Aut BV, Oct 1996 --------------
// --------------------------------------------------------------------------


#ifndef INOVEC_INC
#define INOVEC_INC

#include "Basics.h"

namespace Ino {

class Trf2;

// --------------------------------------------------------------------------
// ------- 2D Vector --------------------------------------------------------
// --------------------------------------------------------------------------

class Vec2
{
  public:
   static double IdentDist;
   static double IdentDir;

   static const double Pi;  // Pi
   static const double Pi2; // 2.0 * Pi;

   double x, y;
   bool isDerivative;

   Vec2(double cx=0, double cy=0.0) : x(cx), y(cy), isDerivative(false) {};

   double unitLen2();             // Make unitlength, return old length

   double len2() const;
   double lenSq2() const  { return x*x + y*y; }

   double len2(double newLen);    // Set new length return old length;

   double angle() const;

   double distTo2(const Vec2& v) const;
   double sqDistTo2(const Vec2& v) const; // Square of distance
   double angleTo2(const Vec2& v) const;

   double operator * (const Vec2& v) const { return x*v.x + y*v.y; }

   Vec2 operator * (double fact) const;
   Vec2 operator / (double fact) const;

   Vec2 operator + (const Vec2& v) const;
   Vec2 operator - (const Vec2& v) const;

   void operator *= (double fact)   { x *= fact; y *= fact; }
   void operator /= (double fact)   { x /= fact; y /= fact; }

   void operator += (const Vec2& v) { x += v.x; y+= v.y; }
   void operator -= (const Vec2& v) { x -= v.x; y -= v.y; }

   bool operator == (const Vec2& v) const
                          { return distTo2(v) <= IdentDist; }
   bool operator != (const Vec2& v) const
                          { return distTo2(v) > IdentDist; }

   void rot90();                  // Rotate 90 degrees anti clkwise
   void rot180();                 // Reverse vector
   void rot270();                 // Rotate 270 degrees anti clkwise

   Vec2 bisect(const Vec2& v, bool acw) const;

   bool parallelTo2(const Vec2& v, double tol = Vec2::IdentDir) const;
   bool oppositeTo2(const Vec2& v, double tol = Vec2::IdentDir) const;

   void transform2(const Trf2& trf);
};

// --------------------------------------------------------------------------
// ------- 3D Vector --------------------------------------------------------
// --------------------------------------------------------------------------

class Trf3;

class Vec3 : public Vec2
{
  public:
   double z;

   Vec3(double cx=0.0, double cy=0.0, double cz=0.0)
                                    : Vec2(cx,cy), z(cz) {}

   Vec3(const Vec2& v) : Vec2(v), z(0) {} // Up conversion (!!!!!)
   Vec3(const Vec2& v, double cz) : Vec2(v), z(cz) {}
   Vec3(const Vec3& v) : Vec2(v),z(v.z) {}

   double unitLen3();             // Make unitlength, return old length

   double len3() const;           // Return length of vector
   double lenSq3() const  { return x*x + y*y + z*z; }

   double len3(double newLen);    // Set new length return old length;

   double distTo3(const Vec3& v) const;
   double sqDistTo3(const Vec3& v) const; // Square of distance

   double angleTo3(const Vec3& v) const;

   double operator * (const Vec3& v) const;  // Inner product
   Vec3 outer(const Vec3& b) const; // Outer product (*this x b)

   Vec3 operator * (double fact) const;
   Vec3 operator / (double fact) const;

   Vec3 operator + (const Vec3& v) const;
   Vec3 operator - (const Vec3& v) const;

   void operator *= (double fact) { x *= fact; y *= fact; z *= fact;}
   void operator /= (double fact) { x /= fact; y /= fact; z /= fact;}

   void operator += (const Vec3& v) { x += v.x; y += v.y; z += v.z; }
   void operator -= (const Vec3& v) { x -= v.x; y -= v.y; z -= v.z;  }

   bool operator == (const Vec3& v) const
                          { return distTo3(v) <= IdentDist; }
   bool operator != (const Vec3& v) const
                          { return distTo3(v) > IdentDist; }

   void transform3(const Trf3& trf);
};

} // namespace Ino

// --------------------------------------------------------------------------
#endif
