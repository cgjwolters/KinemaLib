// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// ----- Least squares approximations by geometric entities -----------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// ----------------------------------------- Januari 1990,2001 C. Wolters ---
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

#include "LsGeo.h"
#include "Exceptions.h"
#include "MsrCont.h"
#include "Matrix.h"

#include "Geo.h"

#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <algorithm>

namespace Ino
{

using namespace std;

static const double Min_Double       = 1.0E-306;
static const double Max_Double       = 1.0E306;
static const double Double_Precision = 4.440892098500626E-16;

// ATTENTION: All weights must be > 0 !
// Weight[i] is supposed to be the length of the line ending in i

//---------------------------------------------------------------------------
//---- Work matrices and vectors --------------------------------------------
//---------------------------------------------------------------------------

static Matrix mat(20,3); // Initial guess for the rows
static Matrix vt(3,3);
static Matrix b(20,1);
static Matrix solMat(4,1);

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

double LsAprxEl::iSectTol = 1.41;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

LsAprxEl::LsAprxEl(const LsAprxCnt& contour)
: parent(contour), cnt(contour.msrCnt), bIdx(0), eIdx(0),
  p1(), p2(), maxRes(0.0), tangent(false)
{
}

//---------------------------------------------------------------------------
//------- Copy Constructor --------------------------------------------------
//---------------------------------------------------------------------------

LsAprxEl::LsAprxEl(const LsAprxEl& cp)
: parent(cp.parent), cnt(cp.cnt), bIdx(cp.bIdx), eIdx(cp.eIdx),
  p1(cp.p1), p2(cp.p2), maxRes(cp.maxRes), 
  tangent(cp.tangent)
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

LsAprxEl& LsAprxEl::operator=(const LsAprxEl& src)
{
  if (&parent != &src.parent) throw IllegalStateException("LsAprxEl::operator=");
  if (&cnt    != &src.cnt)    throw IllegalStateException("LsAprxEl::operator=");

  bIdx        = src.bIdx;
  eIdx        = src.eIdx;

  p1          = src.p1;
  p2          = src.p2;
  maxRes      = src.maxRes;

  tangent     = src.tangent;

  return *this;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

int LsAprxEl::rangeLen() const
{
  return cnt.rangeLen(bIdx,eIdx);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxEl::checkIp(const LsAprxEl& prvEl, const Vec2& ip, double pr1, double pr2)
{
  const Vec2& jp = cnt[bIdx];

  if (ip.distTo2(jp) < parent.tol * iSectTol) return true;

  double prvLen = prvEl.len2();

  if (pr1 < prvLen/2.0 || pr2 > len2()/2.0) return false;
  if (pr1 > prvLen && pr2 < 0.0) return false;

  Vec2 pp;
  double pr,dist;
  bool ok = true;
  int idx = cnt.prvIdx(bIdx);

  if (pr1 < prvLen) {
    while (idx != prvEl.bIdx) {
      if (!prvEl.project(cnt[idx],pp,pr,dist)) { ok = false; break; }
      if (pr < pr1) break;

      if (!project(cnt[idx],pp,pr,dist) || fabs(dist) > parent.tol) {
        ok = false;
        break;
      }

      idx = cnt.prvIdx(idx);
    }
  }

  idx = cnt.nxtIdx(bIdx);

  if (pr2 > 0.0) {
    while (idx != eIdx) {
      if (!project(cnt[idx],pp,pr,dist)) { ok = false; break; }
      if (pr > pr2) break;

      if (!prvEl.project(cnt[idx],pp,pr,dist) || fabs(dist) > parent.tol) {
        ok = false;
        break;
      }

      idx = cnt.nxtIdx(idx);
    }
  }

  return ok;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxEl::checkJoin(const LsAprxEl& prvEl)
{
  Vec2 ipa, ipb;
  double pr1a, pr1b, pr2a, pr2b;

  int sols = prvEl.intersect(*this,ipa,ipb,pr1a,pr1b,pr2a,pr2b);

  if (sols < 1) {
    if (p2.distTo2(prvEl.p1) < 1e-10) return true;
    return false;
  }

  if (sols > 1) {
    const Vec2& bp = cnt[bIdx];

    if (ipb.distTo2(bp) < ipa.distTo2(bp)) {
      ipa = ipb;
      pr1a = pr1b;
      pr2a = pr2b;
    }
  }

  if (checkIp(prvEl,ipa,pr1a,pr2a)) {
    p1 = ipa;
    return true;
  }

  return false;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

int LsAprxEl::insIdx(double par) const
{
  double eLen = len2();

  if (par < 0.0 || par > eLen) return -1;

  int rLen = rangeLen();

  for (int k=0, i=bIdx; k<rLen; k++) {
    Vec2 pp;
    double pr,dist;

    if (project(cnt[i],pp,pr,dist) &&
                             pr >= 0.0 && pr < eLen && pr > par) return i;

    i = cnt.nxtIdx(i);
  }

  return -1;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//----- Approximated Line Element -------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

LsAprxLine::LsAprxLine(const LsAprxCnt& contour)
: LsAprxEl(contour)
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

LsAprxLine::LsAprxLine(const LsAprxLine& cp)
: LsAprxEl(cp)
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

LsAprxLine& LsAprxLine::operator=(const LsAprxLine& src)
{
  LsAprxEl::operator=(src);
  return *this;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

LsAprxEl *LsAprxLine::clone() const
{
  return new LsAprxLine(*this);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

double LsAprxLine::len2() const
{
  return p1.distTo2(p2);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void LsAprxLine::getIpLow(Vec2& ip, double &par) const
{
  const Vec2& ePt = cnt[eIdx];

  Vec2 dir(p2); dir -= p1; dir.unitLen2();
  Vec2 norm(dir); norm.rot90();

  Vec2 dp(ePt); dp -= p1;

  double len  = dp * dir;
  double dist = fabs(dp * norm);

  if (dist >= iSectTol * parent.tol) dist = 0.0;
  else dist = sqrt(sqr(iSectTol * parent.tol) - sqr(dist));

  par = len - dist;
  ip = p1; ip += dir * par;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void LsAprxLine::getIpHgh(Vec2& ip, double& par) const
{
  const Vec2& ePt = cnt[eIdx];

  Vec2 dir(p2); dir -= p1; dir.unitLen2();
  Vec2 norm(dir); norm.rot90();

  Vec2 dp(ePt); dp -= p1;

  double len  = dp * dir;
  double dist = fabs(dp * norm);

  if (dist >= iSectTol * parent.tol) dist = 0.0;
  else dist = sqrt(sqr(iSectTol * parent.tol) - sqr(dist));

  par = len + dist;
  ip = p1; ip += dir * par;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

Vec2 LsAprxLine::tangentAt(const Vec2& /*ip*/) const
{
  Vec2 dir(p2); dir -= p1; dir.unitLen2();
  return dir;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static void calcAvg(const MsrCont& cnt, int fstIdx, int n, Vec2& avgPt)
{
  int sz = cnt.size();

  avgPt.x = 0.0; avgPt.y = 0.0;

  int i = fstIdx;
  for (int k=0; k<n; k++) {
    avgPt += cnt[i++];
    if (i >= sz) i = 0;
  }

  avgPt /= n;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// length of nrm = must be one
// dir of nrm is normal to line
// so: ip + nrm*dist = point on line

static void configureLine(const MsrCont& cnt, int lwb, int upb,
                          const Vec2& ip, const Vec2& nrm, double dist,
                          double tol,
                          Vec2& p1, Vec2& p2, double& maxRes,
                          int& minIdx, int& maxIdx)
{
  Vec2 dir(nrm); dir.unitLen2(); dir.rot270();

  // Turn in forward direction:
  Vec2 dpr(cnt[upb]); dpr -= cnt[lwb];

  if (dpr*dir < 0.0) {
    dir.rot180();
    dist = -dist;
  }

  Vec2 norm(dir); norm.rot90();

  Vec2 pMin(cnt[lwb]);
  Vec2 pMax(cnt[upb]);

  Vec2 pol(ip); pol += nrm*dist;  // Point on line

  int n = cnt.rangeLen(lwb,upb);

  minIdx = lwb;
  maxIdx = upb;
  maxRes = 0.0;

  int i = lwb;
  for (int k=0; k<n; k++) {
    Vec2 pt = (Vec2 &)cnt[i]; pt -= pMin;

    if (pt*dir < -2.0*tol) {
      minIdx = i;
      pMin = cnt[i];
    }

    pt = cnt[i]; pt -= pMax;

    if (pt*dir > 2.0*tol) {
      maxIdx = i;
      pMax = cnt[i];
    }

    Vec2 dp = (Vec2 &)cnt[i]; dp -= pol;

    double res = fabs(dp*norm);
    if (res > maxRes) maxRes = res;

    i = cnt.nxtIdx(i);
  }

  Vec2 dp(pMin); dp -= pol;
  double d = dp * norm;

  p1 = pMin; p1 -= norm*d;

  dp = pMax; dp -= pol;
  d = dp*norm;

  p2 = pMax; p2 -= norm*d;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxLine::project(const Vec2& p, Vec2& pp, double& pr,
                                                          double& dist) const
{
  return Geo_Project_P_on_Line(p,p1,p2,false,1e-12,pp,pr,dist);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

int LsAprxLine::intersect(const LsAprxEl& el, Vec2& ipa, Vec2& ipb,
                          double& pr1a, double& pr1b,
                          double& pr2a, double& pr2b) const
{
  int sols = 0;

  if (el.getType() == Line) {
    LsAprxLine& pLn = (LsAprxLine&) el;
    sols = Geo_Isct_Lines(p1,p2,pLn.p1,pLn.p2,false,1e-12,ipa,pr1a,pr2a);

    ipb = ipa; pr1b = pr1a; pr2b = pr2a;
  }
  else if (el.getType() == Arc) {
    LsAprxArc& pArc = (LsAprxArc&) el;

    sols = Geo_Isct_Line_Arc(p1,p2,pArc.getP1(),pArc.getP2(),
                             pArc.getCentre(),pArc.getCcw(),
                             false,true,1.0e-12,ipa,ipb,pr1a,pr1b,pr2a,pr2b);
  }
  else throw IllegalStateException("LsAprxLine::intersect");

  return sols;
}

//---------------------------------------------------------------------------
//------ Compute free least squares line from lwb to upb --------------------
//---------------------------------------------------------------------------

bool LsAprxLine::computeLs()
{
  int n = cnt.rangeLen(bIdx,eIdx);
  if (n < 2) throw IllegalArgumentException("LsAprxLine::computeLs");

  if (n == 2) {
    p1 = cnt[bIdx];
    p2 = cnt[eIdx];

    maxRes = 0.0;

    return true;
  }

  if (mat.getRows() < n) mat.setRows(n*2);

  Vec2 avgPt;
  calcAvg(cnt,bIdx,n,avgPt);

  int i = bIdx;

  for (int k=0; k<n; k++) {
    const Vec2& pt = cnt[i];

    mat(k,0) = (pt.x - avgPt.x);
    mat(k,1) = (pt.y - avgPt.y);

    i = cnt.nxtIdx(i);
  }

  int rank = 0;
  int iter = 16;

  if (!mat.solveSvd(n,2,vt,rank,iter)) {
    return false;
  }

  int j;
  if (fabs(mat(0,0)) > fabs(mat(1,1))) j = 0;
  else                                 j = 1;

  Vec2 nrm(-vt(j,1),vt(j,0));
  int minIdx, maxIdx;

  configureLine(cnt,bIdx,eIdx,avgPt,nrm,0.0,parent.tol,p1,p2,maxRes,minIdx,maxIdx);

  tangent = false;

  if (maxRes > parent.tol || minIdx != bIdx || maxIdx != eIdx) return false;
//  if (maxRes > parent.tol) return false;

  return true;
}

//---------------------------------------------------------------------------
//------ Find line through 2 points exact, or more if the points coincide. --
//---------------------------------------------------------------------------

bool LsAprxLine::computeBasic()
{
  int rLen = cnt.size();

  eIdx = cnt.nxtIdx(bIdx);

  for (int i=1; i<rLen; i++) {
    const Vec2& lp1 = cnt[bIdx];
    const Vec2& lp2 = cnt[eIdx];

    if (lp1.distTo2(lp2) > parent.tol) return computeLs();

    eIdx = cnt.nxtIdx(eIdx);
  }

  return false;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxLine::computeLongestLs(int lwb, int uLim)
{
  int upb = cnt.nxtIdx(lwb);

  bIdx = lwb; eIdx = upb;
  p1   = cnt[bIdx]; p2 = cnt[eIdx];
  maxRes = 0;
  tangent = false;

  int maxRange = cnt.rangeLen(lwb,uLim);

  while (rangeLen() < maxRange) {
    LsAprxLine newLine(*this);
    newLine.eIdx = cnt.nxtIdx(newLine.eIdx);

    if (rangeLen() < maxRange-1) {
      int oldIdx = newLine.eIdx;

      newLine.eIdx = cnt.nxtIdx(newLine.eIdx);

      if (!newLine.computeLs()) {
        newLine.eIdx = oldIdx;

        if (newLine.computeLs()) *this = newLine;

        return true;
      }
    }
    else if (!newLine.computeLs()) return true;

    *this = newLine;
  }

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxLine::computePointLs(const Vec2& p)
{
  int n = rangeLen();
  if (n < 2) throw IllegalArgumentException("LsAprxLine::computePointLs");

  if (n == 2) {
    p1 = p;
    p2 = cnt[eIdx];

    maxRes = 0.0;
    tangent = false;

    return true;
  }

  Vec2 avgPt;
  calcAvg(cnt,cnt.nxtIdx(bIdx),n-1,avgPt);

  double dx = avgPt.x - p.x;
  double dy = avgPt.y - p.y;

  double lenSq = sqr(dx) + sqr(dy);

  if (lenSq < sqr(1000*Double_Precision)) return computeLs();

  Vec2 norm = Vec2(dy,-dx); norm.unitLen2();
  int minIdx, maxIdx;
  int fstIdx = cnt.nxtIdx(bIdx);

  configureLine(cnt,fstIdx,eIdx,avgPt,norm,0.0,parent.tol,p1,p2,maxRes,minIdx,maxIdx);

  p1 = p;
  tangent = false;

//  if (maxRes > parent.tol || minIdx != fstIdx || maxIdx != eIdx) return false;
  if (maxRes > parent.tol) return false;

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxLine::computeLongestPointLs(const Vec2& p)
{
  while (rangeLen() >= 2) {
    if (computePointLs(p)) return true;

    eIdx = cnt.prvIdx(eIdx);
  }

  return false;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxLine::computeTangentLs(const LsAprxEl& prvEl)
{
  if (prvEl.getType() == Line) return false;
  int n = rangeLen();
  if (n < 2) return false;

  LsAprxArc& prvArc = (LsAprxArc&) prvEl;

  const Vec2& c = prvArc.getCentre();
  double rad = prvArc.getR();

  Vec2 avgPt;
  calcAvg(cnt,bIdx,n,avgPt);

  Vec2 da(avgPt); da -= c;

  double lAc = da.len2();
  if (lAc < rad+parent.tol) return false;

  da.unitLen2();
  Vec2 da2(da); da2.rot90();

  double lA1 = rad*rad/lAc;
  double lA2 = sqrt(sqr(rad) - sqr(lA1));

  Vec2 norm(da*lA1);
  if (prvArc.getCcw()) norm -= da2*lA2;
  else                 norm += da2*lA2;

  Vec2 pt(norm); pt +=c;

  int minIdx, maxIdx;

  configureLine(cnt,bIdx,eIdx,avgPt,norm,0.0,parent.tol,p1,p2,maxRes,minIdx,maxIdx);

  p1 = pt;
  tangent = true;

//  if (maxRes > parent.tol || minIdx != bIdx || maxIdx != eIdx) return false;
  if (maxRes > parent.tol) return false;

  Vec2 pp;
  double pr1, pr2, dst;

  if (!prvArc.project(pt,pp,pr1,dst)) return false;
  if (!project(pt,pp,pr2,dst)) return false;
  
  return checkIp(prvEl,pt,pr1,pr2);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxLine::computeLongestTangentLs(const LsAprxEl& prvEl)
{
  if (prvEl.getType() == Line) return false;

  int rLen = rangeLen();
  if (rLen < 2) return false;

  int newRLen = (rLen*2)/3;
  if (newRLen < 2) newRLen = 2;

  while (rangeLen() > newRLen) eIdx = cnt.prvIdx(eIdx);

  if (!computeTangentLs(prvEl)) return false;

  while (rangeLen() < rLen) {
    LsAprxLine newLine(*this);
    newLine.eIdx = cnt.nxtIdx(newLine.eIdx);

    if (rangeLen() < rLen-1) {
      int oldIdx = newLine.eIdx;
      newLine.eIdx = cnt.nxtIdx(newLine.eIdx);

      if (!newLine.computeTangentLs(prvEl)) {
        newLine.eIdx = oldIdx;

        if (newLine.computeTangentLs(prvEl)) *this = newLine;

        return true;
      }
    }
    else if (!newLine.computeTangentLs(prvEl)) return true;

    *this = newLine;
  }

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxLine::makeFst()
{
  if (cnt.size() < 2) return false;

  bIdx = 0; eIdx = cnt.nxtIdx(bIdx);

  if (!computeBasic()) return false;

  int rlen = rangeLen();

  if (cnt.closed()) {
    while (rlen < cnt.size()) {
      LsAprxLine l1(*this), l2(*this);

      l1.bIdx = cnt.prvIdx(l1.bIdx);
      l2.eIdx = cnt.nxtIdx(l2.eIdx);

      bool ok1 = l1.computeLs();
      bool ok2 = l2.computeLs();

      if (ok1) {
        if (ok2) {
          if (l2.maxRes < l1.maxRes) *this = l2;
          else                       *this = l1;
        }
        else *this = l1;
      }
      else if (ok2) *this = l2;
      else break;

      rlen = rangeLen();
    }
  }
  else {
    while (rlen < cnt.size()) {
      LsAprxLine l2(*this);

      l2.eIdx = cnt.nxtIdx(l2.eIdx);

      bool ok2 = l2.computeLs();

      if (ok2) *this = l2;
      else break;

      rlen = rangeLen();
    }
  }

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------ Approximated Arc Element -------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

LsAprxArc::LsAprxArc(const LsAprxCnt& contour)
: LsAprxEl(contour), cntr(), ccw(true)
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

LsAprxArc::LsAprxArc(const LsAprxArc& cp)
: LsAprxEl(cp), cntr(cp.cntr), ccw(cp.ccw)
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

LsAprxArc& LsAprxArc::operator=(const LsAprxArc& src)
{
  LsAprxEl::operator=(src);
  cntr = src.cntr;
  ccw  = src.ccw;

  return *this;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

LsAprxEl *LsAprxArc::clone() const
{
  return new LsAprxArc(*this);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

double LsAprxArc::len2() const
{
  return Geo_Arc_Len(p1,p2,cntr,ccw);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void LsAprxArc::getIpLow(Vec2& ip, double &par) const
{
  const Vec2& ePt = cnt[eIdx];

  Vec2 ipb;
  double pr1b, pr2a, pr2b;

  Vec2 s2(ePt); s2.x += iSectTol * parent.tol;

  int sols = Geo_Isct_Circle_Circle(p1, cntr, ccw,
                                    s2, ePt, true, true, 1e-12,
                                    ip, ipb, par, pr1b, pr2a, pr2b);
  if (sols < 1) {
    Geo_Project_P_on_Arc(ePt, p1, p2, cntr, ccw, false, 1e-12,
                                                     ip, par, pr2a);
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void LsAprxArc::getIpHgh(Vec2& ip, double &par) const
{
  const Vec2& ePt = cnt[eIdx];

  Vec2 ipa;
  double pr1a, pr2a, pr2b;

  Vec2 s2(ePt); s2.x += iSectTol * parent.tol;

  int sols = Geo_Isct_Circle_Circle(p1, cntr, ccw,
                                    s2, ePt, true, true, 1e-12,
                                    ipa, ip, pr1a, par, pr2a, pr2b);
  if (sols < 1) {
    Geo_Project_P_on_Arc(ePt, p1, p2, cntr, ccw, false, 1e-12,
                                                     ip, par, pr2a);
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

Vec2 LsAprxArc::tangentAt(const Vec2& ip) const
{
  Vec2 dir(ip); dir -= cntr; dir.unitLen2();

  if (ccw) dir.rot90();
  else     dir.rot270();

  return dir;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

double LsAprxArc::getR() const
{
  return p1.distTo2(cntr);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxArc::estimate(Vec2& c, double& r) const
{
  int n = cnt.rangeLen(bIdx,eIdx);

  Vec2 avgPt;
  calcAvg(cnt,bIdx,n,avgPt);

  int i = bIdx;
  for (int k=0; k<n; k++) {
    const Vec2& pt = cnt[i];

    double x = pt.x-avgPt.x;
    double y = pt.y-avgPt.y;

    b(k,0) = sqr(x) + sqr(y);

    mat(k,0) = 2*x;
    mat(k,1) = 2*y;
    mat(k,2) = -1;

    i = cnt.nxtIdx(i);
  }

  int rank = 0;
  int iter = 24;

  if (!mat.solveLs(n,3,vt,b,solMat,1e-12,rank,iter)) return false;

    // Tijdelijk:
   // printMat(mat,3,iter,rank);

  r = sqrt(fabs(sqr(solMat(0,0)) + sqr(solMat(1,0)) - solMat(2,0)));

  c.x = solMat(0,0) + avgPt.x;
  c.y = solMat(1,0) + avgPt.y;

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static void configureArc(const MsrCont& cnt, int lwb, int upb,
                         const Vec2& c, double rad, double tol,
                         Vec2& p1, Vec2& p2, bool& ccw, double& maxRes,
                         int& minIdx, int& maxIdx)
{
  int n = cnt.rangeLen(lwb,upb);

  Vec2 dr0;
  double rotAng = 0.0;
  maxRes = 0.0;

  int k, i = lwb, lastI = lwb;
  for (k=0; k<n; k++) {
    Vec2 dr(cnt[i]); dr -= c;

    if (k > 0) rotAng += dr0.angleTo2(dr);
    dr0 = dr;

    double res = fabs(dr.len2() - rad);
    if (res > maxRes) maxRes = res;

    if (k > 0) { // Residu between points (allowed to be 2.0 * tol)
      Vec2 drp(cnt[lastI]); drp += cnt[i];
      drp /= 2.0; drp -= c;

      res = fabs(drp.len2() - rad) / 2.0; // Use half the value!
      if (res > maxRes) maxRes = res;
    }

    lastI = i;
    i = cnt.nxtIdx(i);
  }

  ccw = rotAng >= 0.0;

  // Find minIdx and maxIdx, p1 and p2

  minIdx = lwb; maxIdx = upb;

  p1 = cnt[lwb]; p1 -= c; p1.unitLen2(); p1 *= rad; p1 += c;
  p2 = cnt[upb]; p2 -= c; p2.unitLen2(); p2 *= rad; p2 += c;

  for (k=1,i=cnt.nxtIdx(lwb); k<n-1; k++) {
    Vec2 pp;
    double pr,dst;

    if (!Geo_Project_P_on_Arc(cnt[i],p1,p2,c,ccw,true,2.0*tol,pp,pr,dst)) {
      if (pr <= 0.0) {
        p1 = cnt[i]; p1 -= c; p1.unitLen2(); p1 *= rad; p1 += c;
        minIdx = i;
      }
      else {
        p2 = cnt[i]; p2 -= c; p2.unitLen2(); p2 *= rad; p2 += c;
        maxIdx = i;
      }
    }

    i = cnt.nxtIdx(i);
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxArc::project(const Vec2& p, Vec2& pp,
                                            double& pr, double& dist) const
{
  return Geo_Project_P_on_Arc(p,p1,p2,cntr,ccw,false,1e-12,pp,pr,dist);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

int LsAprxArc::intersect(const LsAprxEl& el, Vec2& ipa, Vec2& ipb,
                         double& pr1a, double& pr1b,
                         double& pr2a, double& pr2b) const
{
  int sols = 0;
  
  if (el.getType() == Line) {
    LsAprxLine& pLn = (LsAprxLine&) el;
    sols = Geo_Isct_Line_Arc(pLn.getP1(),pLn.getP2(),p1,p2,cntr,ccw,
                             false,true,1.0e-12,ipa,ipb,pr2a,pr2b,pr1a,pr1b);
    
    if (sols > 1 && pr1a > pr1b) { // Swap
      Vec2 hv(ipa); ipa = ipb; ipb = hv;
      double hd = pr1a; pr1a = pr1b; pr1b = hd;

      hd = pr2a; pr2a = pr2b; pr2b = hd;
    }

  }
  else if (el.getType() == Arc) {
    LsAprxArc& pArc = (LsAprxArc&) el;

    sols = Geo_Isct_Arc_Arc(p1,p2,cntr,ccw,
                            pArc.p1,pArc.p2,pArc.cntr,pArc.ccw,
                            false,true,1e-12, 
                            ipa,ipb,pr1a,pr1b,pr2a,pr2b);
  }
  else throw IllegalStateException("LsAprxArc::intersect");

  return sols;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static void checkArcSizes(int n)
{
  if (mat.getRows() < n) mat.setRows(n*2);
  if (b.getRows()   < n) b.setRows(n*2);
}

//---------------------------------------------------------------------------
// ------- Iterative approximation of a free 2D circle (exact solution) -----
//---------------------------------------------------------------------------

bool LsAprxArc::computeLs()
{
  int n = cnt.rangeLen(bIdx,eIdx);
  if (n < 3) throw IllegalArgumentException("LsAprxArc::computeLs()");

  checkArcSizes(n);

  double r0;
  if (!estimate(cntr,r0)) return false;

  bool done = false;
  int tries = 16;
  int iter = 0;

  double lastUpdNorm = 0.0;

  while (iter <= tries) {
    int i=bIdx;

    for (int k=0; k<n; k++) {
      const Vec2& pt = cnt[i];

      double x = pt.x - cntr.x;
      double y = pt.y - cntr.y;
      double r = sqrt(sqr(x) + sqr(y));
   
      if (r <= max(fabs(x),fabs(y))*1000*Double_Precision) {
	      // st = Ill_Conditioned;
        return false;
      }

      mat(k,0) = -x/r;
      mat(k,1) = -y/r;
      mat(k,2) = -1.0;
      b(k,0)   = r0 - r;

      i = cnt.nxtIdx(i);
    }

    int rank = 0, runs = 0;
    
    if (!mat.solveLs(n,3,vt,b,solMat,0.00001,rank,runs)) {
      // st = Ill_Conditioned;
      return false;
    }

    double upd0 = solMat(0,0), upd1 = solMat(1,0), upd2 = solMat(2,0);

    cntr.x += upd0;
    cntr.y += upd1;
    r0     += upd2;

    if (iter++ < 1) continue;

    double solNorm = sqrt(sqr(cntr.x)+sqr(cntr.y)+sqr(r0));
    double updNorm = sqrt(sqr(upd0)+sqr(upd1)+sqr(upd2));

    if (updNorm < (0.0001*solNorm)) {
      done = true;
      break;
    }

    if (iter > 4 && updNorm > lastUpdNorm) {
      return false; // Diverging
    }

    lastUpdNorm = updNorm;
  }

  if (!done) return false; // No convergence
  else if (r0 < 10.0*parent.tol || r0 > parent.maxRad) return false;
  else {
    int minIdx, maxIdx;

    configureArc(cnt,bIdx,eIdx,cntr,r0,parent.tol,p1,p2,ccw,maxRes,minIdx,maxIdx);

    if (maxRes > parent.tol || minIdx != bIdx || maxIdx != eIdx) return false;
//    if (maxRes > parent.tol) return false;
  }

  tangent = false;

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxArc::computeLongestLs(int lwb, int uLim)
{
  int maxRange = cnt.rangeLen(lwb,uLim);
  if (maxRange < 3) return false;

  bIdx = lwb;
  eIdx = cnt.nxtIdx(lwb); eIdx = cnt.nxtIdx(eIdx);

  if (!computeLs()) return false;

  tangent = false;

  while (rangeLen() < maxRange) {
    LsAprxArc newArc(*this);
    newArc.eIdx = cnt.nxtIdx(newArc.eIdx);

    if (rangeLen() < maxRange-1) {
      int oldIdx = newArc.eIdx;
      newArc.eIdx = cnt.nxtIdx(newArc.eIdx);

      if (!newArc.computeLs()) {
        newArc.eIdx = oldIdx;

        if (newArc.computeLs()) *this = newArc;         

        return true;
      }
    }
    else if (!newArc.computeLs()) return true;

    *this = newArc;
  }

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxArc::computePointLs(const Vec2& p)
{
  int n = rangeLen();
  if (n < 2) throw IllegalArgumentException("LsAprxArc::computePointLs");

  checkArcSizes(n);

  bool done = false;
  int rank = 0;
  int tries = 16;
  int iter = 0;

  double lastUpdNorm = 0.0;

  while (iter <= tries) {
    Vec2 dc(p); dc.x -= cntr.x; dc.y -= cntr.y;

    double r0 = dc.len2();

    if (r0 < 1000.0*Double_Precision) return false;

    int i=bIdx;

    for (int k=0; k<n; k++) {
      const Vec2& pt = cnt[i];

      double x = pt.x - cntr.x;
      double y = pt.y - cntr.y;
      double r = sqrt(sqr(x) + sqr(y));
   
      if (r <= max(fabs(x),fabs(y))*1000*Double_Precision) return false;

      mat(k,0) = -x/r + (p.x-cntr.x)/r0;
      mat(k,1) = -y/r + (p.y-cntr.y)/r0;
      b(k,0) = r0 - r;

      i = cnt.nxtIdx(i);
    }

    int runs = 24;
    
    if (!mat.solveLs(n,2,vt,b,solMat,0.00001,rank,runs)) return false;

    double upd0 = solMat(0,0), upd1 = solMat(1,0);

    cntr.x += upd0;
    cntr.y += upd1;

    if (iter++ < 1) continue;

    double solNorm = sqrt(sqr(cntr.x)+sqr(cntr.y));
    double updNorm = sqrt(sqr(upd0)+sqr(upd1));

    if (updNorm < (0.0001*solNorm)) {
      done = true;
      break;
    }

    if (iter > 4 && updNorm > lastUpdNorm) return false;

    lastUpdNorm = updNorm;
  }

  Vec2 dc(p); dc -= cntr;
  double r0 = dc.len2();

  if (!done) return false; // No convergence
  else if (r0 < 10.0*parent.tol || r0 > parent.maxRad) return false;
  else {
    int minIdx, maxIdx;
    int fstIdx = cnt.nxtIdx(bIdx);

    configureArc(cnt,fstIdx,eIdx,cntr,r0,parent.tol,p1,p2,ccw,maxRes,minIdx,maxIdx);

//    if (maxRes > parent.tol || minIdx != fstIdx || maxIdx != eIdx) return false;
    if (maxRes > parent.tol) return false;
  }

  p1 = p;
  tangent = false;

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxArc::computeLongestPointLs(const Vec2& p)
{
  while (rangeLen() >= 3) {
    if (computePointLs(p)) return true;

    eIdx = cnt.prvIdx(eIdx);
  }

  return false;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxArc::computeTangentToLineLs(const LsAprxEl& prvEl,
                                                       const Vec2& initCntr)
{
  int n = cnt.rangeLen(bIdx,eIdx);
  if (n < 3) throw IllegalArgumentException("LsAprxArc::computeTangentToLineLs");

  checkArcSizes(n);

  cntr = initCntr;

  Vec2 tp;
  double pr,r0;
  if (!Geo_Project_P_on_Line(cntr,prvEl.getP1(),prvEl.getP2(),false,
                                              1e-12,tp,pr,r0)) return false;

  r0 = fabs(r0);

  bool done = false;
  int tries = 16;
  int iter = 0;

  double lastUpdNorm = 0.0;

  while (iter <= tries) {
    int i=bIdx;

    for (int k=0; k<n; k++) {
      const Vec2& pt = cnt[i];

      double x = pt.x - cntr.x;
      double y = pt.y - cntr.y;
      double r = sqrt(sqr(x) + sqr(y));
   
      if (r <= max(fabs(x),fabs(y))*1000*Double_Precision) {
	      // st = Ill_Conditioned;
        return false;
      }

      mat(k,0) = -x/r;
      mat(k,1) = -y/r;
      b(k,0)   = r0 - r;

      i = cnt.nxtIdx(i);
    }

    int runs = 24;
    int rank = 0;
    
    if (!mat.solveLs(n,2,vt,b,solMat,0.00001,rank,runs)) {
      // st = Ill_Conditioned;
      return false;
    }

    double upd0 = solMat(0,0), upd1 = solMat(1,0);

    cntr.x += upd0;
    cntr.y += upd1;

    if (!Geo_Project_P_on_Line(cntr,prvEl.getP1(),prvEl.getP2(),false,
                                              1e-12,tp,pr,r0)) return false;

    r0 = fabs(r0);

    if (iter++ < 1) continue;

    double solNorm = sqrt(sqr(cntr.x)+sqr(cntr.y));
    double updNorm = sqrt(sqr(upd0)+sqr(upd1));

    if (updNorm < (0.0001*solNorm)) {
      done = true;
      break;
    }

    if (iter > 4 && updNorm > lastUpdNorm) {
      return false; // Diverging
    }

    lastUpdNorm = updNorm;
  }

  if (!done) return false; // No convergence
  if (r0 < 10.0*parent.tol || r0 > parent.maxRad) return false;

  int minIdx, maxIdx;
  configureArc(cnt,bIdx,eIdx,cntr,r0,parent.tol,p1,p2,ccw,maxRes,minIdx,maxIdx);

//  if (maxRes > parent.tol || minIdx != bIdx || maxIdx != eIdx) return false;
  if (maxRes > parent.tol) return false;

  p1 = tp;
  tangent = true;

  Vec2 pp;
  double pr1,pr2,dst;
  if (!prvEl.project(tp,pp,pr1,dst)) return false;
  if (!project(tp,pp,pr2,dst)) return false;
  
  return checkIp(prvEl,tp,pr1,pr2);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxArc::computeTangentToArcLs(const LsAprxEl& prvEl,
                                                       const Vec2& initCntr)
{
  int n = cnt.rangeLen(bIdx,eIdx);
  if (n < 3) throw IllegalArgumentException("LsAprxArc::computeTangentToArcLs");

  checkArcSizes(n);

  cntr = initCntr;

  const LsAprxArc& prvArc = (LsAprxArc&)prvEl;

  Vec2 dc(cntr); dc -= prvArc.cntr;
  if (dc.len2() < Vec2::IdentDist) {
    dc = cnt[bIdx]; dc -= prvArc.cntr;
  }

  dc.unitLen2(); dc *= prvArc.getR();

  Vec2 tp(prvArc.cntr); tp += dc;
  Vec2 tp2(prvArc.cntr); tp2 -= dc;

  if (tp2.distTo2(cnt[bIdx]) < tp.distTo2(cnt[bIdx])) tp = tp2;
  double r0 = tp.distTo2(initCntr);

  bool done = false;
  int tries = 16;
  int iter = 0;

  double lastUpdNorm = 0.0;

  while (iter <= tries) {
    int i=bIdx;

    for (int k=0; k<n; k++) {
      const Vec2& pt = cnt[i];

      double x = pt.x - cntr.x;
      double y = pt.y - cntr.y;
      double r = sqrt(sqr(x) + sqr(y));
   
      if (r <= max(fabs(x),fabs(y))*1000*Double_Precision) {
	      // st = Ill_Conditioned;
        return false;
      }

      mat(k,0) = -x/r;
      mat(k,1) = -y/r;
      b(k,0)   = r0 - r;

      i = cnt.nxtIdx(i);
    }

    int runs = 24;
    int rank = 0;
    
    if (!mat.solveLs(n,2,vt,b,solMat,0.00001,rank,runs)) {
      // st = Ill_Conditioned;
      return false;
    }

    double upd0 = solMat(0,0), upd1 = solMat(1,0);

    cntr.x += upd0;
    cntr.y += upd1;

    dc = cntr; dc -= prvArc.cntr;
    if (dc.len2() < Vec2::IdentDist) {
      dc = cnt[bIdx]; dc -= prvArc.cntr;
    }

    dc.unitLen2(); dc *= prvArc.getR();

    tp  = prvArc.cntr; tp  += dc;
    tp2 = prvArc.cntr; tp2 -= dc;

    if (tp2.distTo2(cnt[bIdx]) < tp.distTo2(cnt[bIdx])) tp = tp2;
    r0 = tp.distTo2(cntr);

    if (iter++ < 1) continue;

    double solNorm = sqrt(sqr(cntr.x)+sqr(cntr.y));
    double updNorm = sqrt(sqr(upd0)+sqr(upd1));

    if (updNorm < (0.0001*solNorm)) {
      done = true;
      break;
    }

    if (iter > 4 && updNorm > lastUpdNorm) {
      return false; // Diverging
    }

    lastUpdNorm = updNorm;
  }

  if (!done) return false; // No convergence
  if (r0 < 10.0*parent.tol || r0 > parent.maxRad) return false;

  int minIdx, maxIdx;
  configureArc(cnt,bIdx,eIdx,cntr,r0,parent.tol,p1,p2,ccw,maxRes,minIdx,maxIdx);

//  if (maxRes > parent.tol || minIdx != bIdx || maxIdx != eIdx) return false;
  if (maxRes > parent.tol) return false;

  p1 = tp;
  tangent = true;

  Vec2 pp;
  double pr1,pr2,dst;
  if (!prvEl.project(tp,pp,pr1,dst)) return false;
  if (!project(tp,pp,pr2,dst)) return false;
  
  return checkIp(prvEl,tp,pr1,pr2);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxArc::computeTangentLs(const LsAprxEl& prvEl)
{
  int n = rangeLen();
  if (n < 3) return false;

  Vec2 initCntr = cntr;
  if (prvEl.getType() == Line) return computeTangentToLineLs(prvEl,initCntr);
  else                         return computeTangentToArcLs(prvEl,initCntr);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxArc::computeLongestTangentLs(const LsAprxEl& prvEl)
{
  int rLen = rangeLen();
  if (rLen < 3) return false;

  int newRLen = (rLen*2)/3;
  if (newRLen < 3) newRLen = 3;

  while (rangeLen() > newRLen) eIdx = cnt.prvIdx(eIdx);

  if (!computeTangentLs(prvEl)) return false;

  while (rangeLen() < rLen) {
    LsAprxArc newArc(*this);
    newArc.eIdx = cnt.nxtIdx(newArc.eIdx);

    if (rangeLen() < rLen-1) {
      int oldIdx = newArc.eIdx;
      newArc.eIdx = cnt.nxtIdx(newArc.eIdx);

      if (!newArc.computeTangentLs(prvEl)) {
        newArc.eIdx = oldIdx;

        if (newArc.computeTangentLs(prvEl)) *this = newArc;         

        return true;
      }
    }
    else if (!newArc.computeTangentLs(prvEl)) return true;

    *this = newArc;
  }

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxArc::makeFst()
{
  if (cnt.size() < 3) return false;

  bIdx = 0;
  eIdx = 2;

  if (!computeLs()) return false;

  int rlen = rangeLen();

  if (cnt.closed()) {
    while (rlen < cnt.size()) {
      LsAprxArc a1(*this), a2(*this);

      a1.bIdx = cnt.prvIdx(a1.bIdx);
      a2.eIdx = cnt.nxtIdx(a2.eIdx);

      bool ok1 = a1.computeLs();
      bool ok2 = a2.computeLs();

      if (ok1) {
        if (ok2) {
          if (a2.maxRes < a1.maxRes) *this = a2;
          else                       *this = a1;
        }
        else *this = a1;
      }
      else if (ok2) *this = a2;
      else break;

      rlen= rangeLen();
    }
  }
  else {
    while (rlen < cnt.size()) {
      LsAprxArc a2(*this);

      a2.eIdx = cnt.nxtIdx(a2.eIdx);

      bool ok2 = a2.computeLs();

      if (ok2) *this = a2;
      else break;

      rlen = rangeLen();
    }
  }

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//----- LsAprxCnt Methods ------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void LsAprxCnt::resize(int newSz)
{
  if (newSz < sz) newSz = sz;
  if (newSz < 10) newSz = 10;

  LsAprxEl **newLst = new LsAprxEl*[newSz];

  if (elList) {
    for (int i=0; i<sz; i++) newLst[i] = elList[i];
    delete elList;
  }

  elList = newLst;
  cap = newSz;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void LsAprxCnt::append(LsAprxEl& newElem)
{
  if (sz >= cap) resize(cap*2);

  elList[sz++] = newElem.clone();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void LsAprxCnt::clear()
{
  if (elList) {
    for (int i=0; i<sz; i++) delete elList[i];
    delete elList;
  }

  elList = 0;
  sz = 0;
  cap = 0;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

LsAprxCnt::LsAprxCnt(MsrCont& mCnt, double tolerance, double maxRadius,
                                                               bool noArcs)
: elList(NULL), cap(0), sz(0),
  msrCnt(mCnt), tol(tolerance), maxRad(maxRadius), genNoArcs(noArcs)
{
  if (tolerance <= 0.0 || maxRadius <= 0.0)
                  throw IllegalArgumentException("LsAprxCnt::LsAprxCnt");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

LsAprxCnt::~LsAprxCnt()
{
  clear();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

const LsAprxEl& LsAprxCnt::operator[](int idx) const
{
  if (!elList) throw NullPointerException("LsAprxCnt::operator[]");
  if (idx < 0 || idx >= sz) throw IndexOutOfBoundsException("LsAprxCnt::operator[]");

  return *elList[idx];
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxCnt::approxLine(const LsAprxEl& prvEl, LsAprxLine& line, int uLim)
{
  if (!line.computeLongestLs(prvEl.eIdx,uLim)) return false;

  LsAprxLine line2(line);

  bool ok1 = line.checkJoin(prvEl);
  
  if (!ok1) {
    Vec2 ip, ip2, pp;
    double parLow, parHgh, pr, distLow, distHgh;

    prvEl.getIpLow(ip,parLow);
    prvEl.getIpHgh(ip2,parHgh);

    bool lowOk = line.project(ip,pp,pr,distLow) && fabs(distLow) < 2.0*tol;
    bool hghOk = line.project(ip2,pp,pr,distHgh) && fabs(distHgh) < 2.0*tol;

    if (lowOk) {
      if (hghOk && fabs(distHgh) < fabs(distLow)) ip = ip2;
    }
    else if (hghOk) ip = ip2;
    else {
      // Just connect next two points with a line
      return line.computeLongestLs(prvEl.eIdx,msrCnt.nxtIdx(prvEl.eIdx));
    }

    ok1 = line.computeLongestPointLs(ip);
  }

  bool ok2 = line2.computeLongestTangentLs(prvEl);

  if (ok1) {
    if (ok2) {
      int rLen1 = msrCnt.rangeLen(line.bIdx,line.eIdx);
      int rLen2 = (msrCnt.rangeLen(line2.bIdx,line2.eIdx)*3)/2;

      if (rLen1 < rLen2) line = line2;
    }
  }
  else if (ok2) line = line2;
  else return false;

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool LsAprxCnt::approxArc(const LsAprxEl& prvEl, LsAprxArc& arc, int uLim)
{
  if (!arc.computeLongestLs(prvEl.eIdx,uLim)) return false;

  LsAprxArc arc2(arc);

  bool ok1 = arc.checkJoin(prvEl);

  if (!ok1) {
    Vec2 ip, ip2, pp;
    double parLow, parHgh, pr, distLow, distHgh;

    prvEl.getIpLow(ip,parLow);
    prvEl.getIpHgh(ip2,parHgh);

    bool lowOk = arc.project(ip,pp,pr,distLow) && fabs(distLow) < 2.0*tol;
    bool hghOk = arc.project(ip2,pp,pr,distHgh) && fabs(distHgh) < 2.0*tol;

    if (lowOk) {
      if (hghOk && fabs(distHgh) < fabs(distLow)) ip = ip2;
    }
    else if (hghOk) ip = ip2;
    else return false;

    ok1 = arc.computeLongestPointLs(ip);
  }

  bool ok2 = arc2.computeLongestTangentLs(prvEl);

  if (ok1) {
    if (ok2) {
      int rLen1 = msrCnt.rangeLen(arc.bIdx,arc.eIdx);
      int rLen2 = (msrCnt.rangeLen(arc2.bIdx,arc2.eIdx)*3)/2;

      if (rLen1 < rLen2) arc = arc2;
    }
  }
  else if (ok2) arc = arc2;
  else return false;

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

LsAprxEl *LsAprxCnt::findFstElem()
{
  if (msrCnt.size() < 2) return NULL;

  LsAprxLine fstLine(*this);
  LsAprxArc  fstArc(*this);

  bool okLine = fstLine.makeFst();

  if (genNoArcs) {
    if (okLine) return fstLine.clone();
  }
  else {
    bool okArc  = fstArc.makeFst();

    if (okLine) {
      if (okArc) {
        if      (fstLine.rangeLen() > fstArc.rangeLen()) return fstLine.clone();
        else if (fstLine.rangeLen() < fstArc.rangeLen()) return fstArc.clone();
        else if (fstLine.maxRes     < fstArc.maxRes)     return fstLine.clone();
        else                                             return fstArc.clone();
      }
      else return fstLine.clone();
    }
    else if (okArc) return fstArc.clone();
  }

  return NULL;
}

//---------------------------------------------------------------------------
//------- Check if "alien" points are near an element -----------------------
//------- and reinsert those points at the proper place ---------------------
//---------------------------------------------------------------------------

bool LsAprxCnt::unfoldEl(LsAprxEl& el, int upb)
{
  if (el.eIdx == upb) return false;

  int i = msrCnt.nxtIdx(el.eIdx);
  if (i == upb) return false;

  bool modified = false;

  double eLen = el.len2();

  while (i != upb) {
    const Vec2& p = msrCnt[i];

    Vec2 pp;
    double pr,dist;

    if (el.project(p,pp,pr,dist) &&
           pr >= 0.0 && pr <= eLen && fabs(dist) < tol*sqrt(2.0)) {
      int insIdx = el.insIdx(pr);

      if (msrCnt.moveForward(i,insIdx) && i != insIdx) {
        el.eIdx = msrCnt.nxtIdx(el.eIdx);
        modified = true;
      }
    }
    
    i = msrCnt.nxtIdx(i);
  }

  return modified;
}

//---------------------------------------------------------------------------

bool LsAprxCnt::arcCandidate(const LsAprxLine& line)
{
  double a = abs(tol * 2.0);
  double b = line.len2()/2.0;

  double nom = (sqr(a) + sqr(b))/2.0;

  if (a < nom * 1e-10) return false;

  if (nom/a > maxRad) return false;

  return true;
}

//---------------------------------------------------------------------------
//----------- Generates an interpolated contour over all points -------------
//----------- The contour consists of lines and arcs ------------------------
//---------------------------------------------------------------------------

bool LsAprxCnt::interpolate()
{
  clear();

  if (msrCnt.size() < 2) return false;

  bool msrClosed = msrCnt.closed();
  if (msrClosed) msrCnt.removeLastPt();

  LsAprxEl *prvEl = NULL;
  
  for (;;) {
    prvEl = findFstElem();
    if (!prvEl) {
      if (msrClosed) msrCnt.close();
      return false;
    }

    if (!unfoldEl(*prvEl,prvEl->bIdx)) break;

    if (prvEl) delete prvEl;
  }

  resize(20);
  elList[sz++] = prvEl; // append first element

  int szUpb = msrCnt.size();

  int startIdx = prvEl->bIdx;
  
  if (!msrClosed) {
    startIdx--;
    if (startIdx < 0) startIdx = szUpb-1;
  }

  while (msrCnt.rangeLen(startIdx,prvEl->eIdx) <= szUpb) {
    LsAprxLine line(*this);
    LsAprxArc  arc(*this);

    LsAprxEl *newEl = &line;

    if (genNoArcs) {
      if (!approxLine(*prvEl,line,startIdx)) {
        if (msrClosed) msrCnt.close();
        return false;
      }
    }
    else {
      if (approxLine(*prvEl,line,startIdx)) {
        if (arcCandidate(line) && approxArc(*prvEl,arc,startIdx)) {
          int lineLen = msrCnt.rangeLen(line.bIdx,line.eIdx);
          if (line.tangent) lineLen = (lineLen*3)/2;

          int arcLen  = msrCnt.rangeLen(arc.bIdx,arc.eIdx);
          if (arc.tangent) arcLen = (arcLen*3)/2;

          if (arcLen > lineLen ||
            (arcLen == lineLen && arc.maxRes < line.maxRes)) newEl = &arc;
        }
      }
      else if (approxArc(*prvEl,arc,startIdx)) newEl = &arc;
      else {
        if (msrClosed) msrCnt.close();
        return false;
      }
    }

    if (unfoldEl(*newEl,startIdx)) continue;

    prvEl->eIdx = newEl->bIdx;
    prvEl->p2   = newEl->p1;

    append(*newEl);

    prvEl = elList[sz-1];
  }

  if (!msrClosed) return true;

  msrCnt.close();

  // Now connect the last to the first.

  // For the time being we solve this in a simple manner

  if (sz < 2) return true;

  LsAprxEl& lstEl = *elList[sz-1];
  LsAprxEl& fstEl = *elList[0];

  if (fstEl.checkJoin(lstEl)) lstEl.p2 = fstEl.p1;
  else {
    // For now simply insert a line
    LsAprxLine line(*this);
    line.bIdx = lstEl.eIdx;
    line.eIdx = fstEl.bIdx;
    line.p1 = lstEl.p2;
    line.p2 = fstEl.p1;
    line.tangent = false;

    append(line);
  }

  return true;
}

} // namespace Ino

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
