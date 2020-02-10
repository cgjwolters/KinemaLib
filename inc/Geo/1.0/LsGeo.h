/* --------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------- */
/* ----- Least squares approximations of geometric entities ------------------------------ */
/* --------------------------------------------------------------------------------------- */
/* --------------------------------- Version 1.0 ----------------------------------------- */
/* --------------------------------------------------------------------------------------- */
/* ---------------------------------------------- Januari 1990,2001 C. Wolters ----------- */
/* --------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------- */

#ifndef LSGEO_INC
#define LSGEO_INC

#include "Basics.h"
#include "Vec.h"

namespace Ino
{

class Vector;
class MsrCont;
class LsAprxCnt;

// --------------------------------------------------------------------------
// ---------- Basic Virtual Element -----------------------------------------
// --------------------------------------------------------------------------

class LsAprxEl
{
public:
  enum Type { Line, Arc };

  static double iSectTol;

protected:
  const LsAprxCnt& parent;
  const MsrCont& cnt;

  int bIdx, eIdx;

  Vec2 p1;          // Start Point
  Vec2 p2;          // End Point

  double maxRes;

  bool tangent; // True if tangent (thru point or tangent to previous element)


  LsAprxEl(const LsAprxCnt& contour);
  LsAprxEl(const LsAprxEl& cp);
  virtual ~LsAprxEl() {}

  LsAprxEl& operator=(const LsAprxEl& src);

  virtual LsAprxEl *clone() const = 0;

  virtual double len2() const = 0;
  virtual void getIpLow(Vec2& ip, double &par) const = 0;
  virtual void getIpHgh(Vec2& ip, double &par) const = 0;

  virtual Vec2 tangentAt(const Vec2& ip) const = 0;

  virtual int intersect(const LsAprxEl& el, Vec2& ipa, Vec2& ipb,
                        double& pr1a, double& pr1b,
                        double& pr2a, double& pr2b) const = 0;

  bool checkIp(const LsAprxEl& prvEl, const Vec2& ip, double pr1, double pr2);
  bool checkJoin(const LsAprxEl& prvEl);

  int insIdx(double par) const;
public: 
  virtual Type getType() const = 0;

  int lwbIdx() const { return bIdx; }
  int upbIdx() const { return eIdx; }

  int rangeLen() const;

  const Vec2& getP1()  const { return p1; }
  const Vec2& getP2()  const { return p2; }

  virtual bool project(const Vec2& p, Vec2& pp,
                                 double& pr, double& dist) const = 0;
  bool isTangent() const { return tangent; }

  friend class LsAprxCnt;
};

// --------------------------------------------------------------------------
// ------ Line Element ------------------------------------------------------
// --------------------------------------------------------------------------

class LsAprxLine : public LsAprxEl
{
  LsAprxLine(const LsAprxCnt& contour);
  LsAprxLine(const LsAprxLine& cp);

  LsAprxLine& operator=(const LsAprxLine& src);

  LsAprxEl *clone() const;

  double len2() const;
  void getIpLow(Vec2& ip, double &par) const;
  void getIpHgh(Vec2& ip, double &par) const;

  Vec2 tangentAt(const Vec2& ip) const;

  int intersect(const LsAprxEl& el, Vec2& ipa, Vec2& ipb,
                        double& pr1a, double& pr1b,
                        double& pr2a, double& pr2b) const;
  bool computeLs();
  bool computeBasic();
  bool computeLongestLs(int lwb, int uLim);

  bool computePointLs(const Vec2& p);
  bool computeLongestPointLs(const Vec2& p);

  bool computeTangentLs(const LsAprxEl& prvEl);
  bool computeLongestTangentLs(const LsAprxEl& prvEl);

  bool makeFst();

public:
  Type getType() const { return Line; }

  bool project(const Vec2& p, Vec2& pp, double& pr, double& dist) const;

  friend class LsAprxCnt;
};

// --------------------------------------------------------------------------
// ------ Arc Element -------------------------------------------------------
// --------------------------------------------------------------------------

class LsAprxArc : public LsAprxEl
{
  Vec2 cntr;
  bool ccw;

  LsAprxArc(const LsAprxCnt& contour);
  LsAprxArc(const LsAprxArc& cp);

  LsAprxArc& operator=(const LsAprxArc& src);

  LsAprxEl *clone() const;

  double len2() const;
  void getIpLow(Vec2& ip, double &par) const;
  void getIpHgh(Vec2& ip, double &par) const;

  Vec2 tangentAt(const Vec2& ip) const;
  bool estimate(Vec2& c, double& r) const;

  void configure(const Vec2& c, double r);

  int intersect(const LsAprxEl& el, Vec2& ipa, Vec2& ipb,
                        double& pr1a, double& pr1b,
                        double& pr2a, double& pr2b) const;
  bool computeLs();
  bool computeLongestLs(int lwb, int uLim);

  bool computePointLs(const Vec2& p);
  bool computeLongestPointLs(const Vec2& p);

  bool computeTangentToLineLs(const LsAprxEl& prvEl, const Vec2& initCntr);
  bool computeTangentToArcLs(const LsAprxEl& prvEl, const Vec2& initCntr);
  bool computeTangentLs(const LsAprxEl& prvEl);
  bool computeLongestTangentLs(const LsAprxEl& prvEl);

  bool makeFst();

public:
  Type getType() const { return Arc; }

  const Vec2& getCentre() const { return cntr; }
  double getR() const;
  bool   getCcw() const { return ccw; }

  bool project(const Vec2& p, Vec2& pp, double& pr, double& dist) const;

  friend class LsAprxCnt;
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

class LsAprxCnt
{
  LsAprxEl **elList;
  int cap, sz;

  MsrCont& msrCnt;

  double tol;
  double maxRad;
  bool   genNoArcs;

  void resize(int newSz);
  void append(LsAprxEl& newElem);

  void clear();

  bool unfoldEl(LsAprxEl& el, int uLim);

  LsAprxEl *findFstElem();

  bool arcCandidate(const LsAprxLine& line);
  bool approxLine(const LsAprxEl& prvEl, LsAprxLine& line, int uLim);
  bool approxArc(const LsAprxEl& prvEl, LsAprxArc& arc, int uLim);

  LsAprxCnt(const LsAprxCnt* cp);             // No Copying
  LsAprxCnt& operator=(const LsAprxCnt& src); // No Assignment

public:
  LsAprxCnt(MsrCont& mCmt, double tolerance, double maxRadius, bool noArcs);
  ~LsAprxCnt();

  int size() const { return sz; }
  const LsAprxEl& operator[](int idx) const;

  bool interpolate();

  friend class LsAprxEl;
  friend class LsAprxLine;
  friend class LsAprxArc;
};

} // namespace Ino

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
#endif
