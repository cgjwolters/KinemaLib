//---------------------------------------------------------------------------
//------- Defines a measurement point, a measurement contour ----------------
//------- and a list of measurement contours --------------------------------
//---------------------------------------------------------------------------
//------- Copyright Prodim BV Dec 2000 --------------------------------------
//---------------------------------------------------------------------------
//------- Inofor Hoek Aut BV C. Wolters Dec 2000 ----------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "MsrCont.h"

#ifndef __GNUC__
#include "DB.H"
#endif

#include "Basics.h"
#include "Exceptions.h"

#include "Vec.h"
#include "Trf.h"

#include "Contour.h"
#include "El_Line.h"
#include "El_Arc.h"

#include "LsGeo.h"
#include "Matrix.h"

#include "DxfOut.h"

#include <cstdio>
#include <cmath>
#include <cstring>

namespace Ino
{
 using namespace std;

//---------------------------------------------------------------------------
//------MsrPoint Methods ----------------------------------------------------
//---------------------------------------------------------------------------

void MsrPoint::set(const Vec3& p, bool pointMode)
{
  pt = p;
  point = pointMode;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MsrPoint::MsrPoint(const MsrPoint& cp)
: pt(cp.pt), point(cp.point)
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MsrPoint& MsrPoint::operator=(const MsrPoint& src)
{
  pt = src.pt;
  point = src.point;

  return *this;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrPoint::transform(const Trf3& trf)
{
  pt.transform3(trf);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static void calcWireDirTo(double axDist, double rollRad, 
                                                   const Vec3& p, Vec3& dir)
{
  dir = p;

  Vec3 dst(dir); dst.z = 0.0; dst.unitLen3(); dst *= axDist;  dir -= dst;

  Vec3 nrm(dir.outer(Vec3(0,0,1))); nrm.unitLen3();
  Vec3 dir1(dir); dir1.unitLen3();

  Trf3 trf(Vec3(0,0,0),nrm,dir1);

  dir.transform3(trf); // y and z of dir should be 0.0 here!

  double l = dir.x;
  double m = sqrt(sqr(l) - sqr(rollRad));

  dir.x = sqr(m)/l;
  dir.y = -rollRad*m/l;

  trf.invert();
  dir.transform3(trf);
}

//---------------------------------------------------------------------------
//----- MsrCont Methods -----------------------------------------------------
//---------------------------------------------------------------------------

void MsrCont::resize(int newCap)
{
  if (newCap < sz) newCap = sz;
  if (newCap < 10) newCap = 10;

  MsrPoint* newList = new MsrPoint[newCap];

  for (int i=0; i<sz; i++) newList[i] = itList[i];

  if (itList) delete[] itList;

  itList = newList;
  cap = newCap;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MsrCont::MsrCont(double radCorrection, int layer)
: radCorr(radCorrection), layerNr(layer),
  itList(new MsrPoint[100]), cap(100), sz(0)
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MsrCont::MsrCont(const MsrCont& cp)
: radCorr(cp.radCorr), layerNr(cp.layerNr),
  itList(new MsrPoint[cp.sz]),cap(cp.sz), sz(cp.sz)
{
  memmove(itList,cp.itList,sz*sizeof(MsrPoint));
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MsrCont::~MsrCont()
{
  if (itList) delete[] itList;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MsrCont& MsrCont::operator=(const MsrCont& src)
{
  radCorr = src.radCorr;
  layerNr = src.layerNr;
  sz      = src.sz;
  cap     = src.sz;

  if (itList) delete[] itList;

  itList = new MsrPoint[sz];

  memmove(itList,src.itList,sz*sizeof(MsrPoint));

  return *this;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

const MsrPoint& MsrCont::operator[](int idx) const
{
  if (!itList || idx < 0 || idx >= sz)
       throw IndexOutOfBoundsException("MsrCont::operator[]");

  return itList[idx];
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrCont::addPt(bool pointMode, const Vec3& p)
{
  if (sz >= cap) resize(cap*2);

  MsrPoint& it = itList[sz++];

  it.set(p,pointMode);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrCont::checkEnd(double /*tolSq*/)
{
// Must not open up contour, now there is a difference between
// open and closed!

#ifdef NEVER
  while (sz > 0) {
    Vec2 dp(itList[sz-1].pt);
    dp -= itList[0].pt;

    if (dp.Lensq_2() < tolSq) sz--;
    else break;
  }

  if (sz < 2) sz = 0;
#endif
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrCont::applyOffset(double axDist, double rollRad,
                                         double horOffset, const Vec3& zDir)
{
  for (int i=0; i<sz; i++) {
    Vec3& pt = itList[i].pt;

    Vec3 dir;
    calcWireDirTo(axDist,rollRad,pt,dir); dir.unitLen3();

    Vec3 nrm = dir.outer(zDir);
    if (nrm.len3() < 1e-7) return; // Sorry, cant oblige

    nrm.unitLen3();
    nrm = zDir.outer(nrm);

    nrm.unitLen3();
    nrm *= horOffset;

    pt += nrm;
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrCont::removeLastPt()
{
  if (sz > 0) sz--;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static void minOfPt(Vec3& pt, const Vec3& newPt)
{
  if (newPt.x < pt.x) pt.x = newPt.x;
  if (newPt.y < pt.y) pt.y = newPt.y;
  if (newPt.z < pt.z) pt.z = newPt.z;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static void maxOfPt(Vec3& pt, const Vec3& newPt)
{
  if (newPt.x > pt.x) pt.x = newPt.x;
  if (newPt.y > pt.y) pt.y = newPt.y;
  if (newPt.z > pt.z) pt.z = newPt.z;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool MsrCont::calcHullRect(Vec3& minPt, Vec3& maxPt) const
{
  if (sz < 1) return false;

  minPt = maxPt = itList[0].pt;

  for (int i=1; i<sz; ++i) {
    const Vec3& pt = itList[i].pt;

     minOfPt(minPt,pt);
     maxOfPt(maxPt,pt);
  }

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrCont::close()
{
  if (!closed()) addPt(itList[0].point,itList[0].pt);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool MsrCont::closed() const
{
  if (sz < 2) return false;

  const Vec3& p1 = itList[sz-1];
  const Vec3& p2 = itList[0];

  return p2.distTo3(p1) < 1e-12;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool MsrCont::interpolateInto(double tolerance, double maxRad,
                              bool noArcs, Contour& newCont)
{
  Vec3 startPt;
  if (itList != NULL && sz > 0) startPt = itList[0];

  newCont.Delete();

  MsrCont cpCont(*this); // Take a copy, because the contour may be reordered.

  LsAprxCnt apCont(cpCont,tolerance,maxRad,noArcs);

  if (!apCont.interpolate()) return false;

  Elem_List elLst;

  int aSz = apCont.size();
  for (int i=0; i<aSz; i++) {
    const LsAprxEl& apEl = apCont[i];

    if (apEl.getType() == LsAprxEl::Line) {
      LsAprxLine& apLine = (LsAprxLine &)apEl;
      Elem_Line newLine(apLine.getP1(),apLine.getP2());
      if (apLine.isTangent()) newLine.Id(1);
      elLst.Push_Back(newLine);
    }
    else if (apEl.getType() == LsAprxEl::Arc) {
      LsAprxArc& apArc = (LsAprxArc &)apEl;
      Elem_Arc arc(apArc.getP1(),apArc.getP2(),apArc.getCentre(),apArc.getCcw());
      if (apArc.isTangent()) arc.Id(1);
      elLst.Push_Back(arc);
    }
  }

  Contour cnt(elLst);

  if (closed()) {
    Cont_Pnt cPnt;
    double distXy = 0;

    if (cnt.Project_Pnt_XY(startPt,cPnt,distXy)) cnt.Start_At(cPnt);
  }
  
  cnt.Move_To(newCont);

#ifdef NEVER
  if (fabs(radCorr) < 0.01) {
    Contour cnt(elLst);
    cnt.Move_To(newCont);
  }
  else {
    Contour lCont1(elLst);
    Contour lCont2(lCont1);
    lCont1.MakeNonIntersecting(true);
    lCont2.MakeNonIntersecting(false);

    if (lCont1.Len_XY() > lCont2.Len_XY()) lCont1.Move_To(newCont);
    else                                   lCont2.Move_To(newCont);

    // This is a desperate measure, lets do it again:
    try {
      lCont1 = newCont;
      lCont2 = lCont1;
      lCont1.MakeNonIntersecting(true);
      lCont2.MakeNonIntersecting(false);

      if (lCont1.Len_XY() > lCont2.Len_XY()) lCont1.Move_To(newCont);
      else                                   lCont2.Move_To(newCont);
    }
    catch (IllegalStateException) {
      return true;
    }
  }
#endif

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrCont::transform(const Trf3& trf)
{
  if (!itList) return;

  for (int i=0; i<sz; ++i) itList[i].transform(trf);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef __GNUC__
void MsrCont::appendToCcd(DB2* ccdDb, bool threeD,bool unitInch,
                          const Layer& layer,
                          int r, int g , int b) const
{
  if (!ccdDb) throw NullPointerException("MsrCont::appendToCcd");

  if (!itList || sz < 1) return;

  if (sz == 1) { // Write only a point
    Vec3 p(itList[0]);

    if (!threeD) p.z = 0.0;

    if (unitInch) p /= InchInMm;

    Entity point;

    point.makePoint(p.x,p.y,p.z);
    point.setLayer(layer.prec.ilayer);
    point.setPick(0);
    point.setColor(RGB(0,0,0));

    ccdDb->addent(point);

    return;
  }

  int i = 0;

  Vec3 lastPt(itList[i]);
  if (!threeD) lastPt.z = 0.0;

  if (unitInch) lastPt /= InchInMm;

  i++;
  if (i >= sz) i = 0;

  for (; i<sz; i++) {
    Vec3 p(itList[i]);
    if (!threeD) p.z = 0.0;

    if (unitInch) p /= InchInMm;

    Entity line;

    line.makeLine(lastPt.x,lastPt.y,lastPt.z,p.x,p.y,p.z);
    line.setLayer(layer.prec.ilayer);
    line.setPick(0);
    line.setColor(RGB(r,g,b));
    line.prec.l.style=LS_SOLID;

    ccdDb->addent(line);

    lastPt = p;
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrCont::appendToCcd(DB2* ccdDb, bool threeD, bool unitInch,
                          const Layer& layer) const
{
  if (threeD) appendToCcd(ccdDb,threeD,unitInch,layer,0,0,255);
  else        appendToCcd(ccdDb,threeD,unitInch,layer,0,0,0);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrCont::appendCcdZLines(DB2* ccdDb, bool unitInch,
                              const char *contNam,
                              int r, int g , int b) const
{
  if (!ccdDb || !contNam)
       throw NullPointerException("MsrCont::appendCcdZLines");

  if (!itList || sz < 1) return;

  Layer curLayer(contNam);
  curLayer.prec.ilayer = ccdDb->addlayer(curLayer);

  for (int i=0; i<sz; i++) {
    Vec3 p(itList[i]);

    if (fabs(p.z) < 0.01) continue;

    if (unitInch) p /= InchInMm;

    Entity line;

    line.makeLine(p.x,p.y,0.0,p.x,p.y,p.z);
    line.setLayer(curLayer.prec.ilayer);
    line.setPick(0);
    line.setColor(RGB(r,g,b));
    line.prec.l.style=LS_SOLID;

    ccdDb->addent(line);
  }
}
#endif

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrCont::appendToDxf(DxfOut& dxf, bool threeD, bool unitInch) const
{
  Vec3 *ptLst = new Vec3[sz];
  
  for (int i=0; i<sz; i++) {
    ptLst[i] = itList[i];

    if (!threeD) ptLst[i].z = 0.0;

    if (unitInch) ptLst[i] /= InchInMm;
  }

  dxf.add3DPoly(ptLst,sz);

  delete[] ptLst;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrCont::appendDxfZLines(DxfOut& dxf, bool unitInch) const
{
  for (int i=0; i<sz; i++) {
    Vec3 pt3D((Vec3&)itList[i]);

    if (unitInch) pt3D /= InchInMm;

    Vec3 pt2D(pt3D); pt2D.z = 0.0;

    dxf.addLine(pt2D,pt3D);
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

int MsrCont::prvIdx(int idx) const
{
  if (idx < 0 || idx >= sz)
        throw IndexOutOfBoundsException("MsrCont::prvIdx");

  if (idx < 1) return sz-1;
  return idx-1;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

int MsrCont::nxtIdx(int idx) const
{
  if (idx < 0 || idx >= sz)
        throw IndexOutOfBoundsException("MsrCont::nxtIdx");

  if (idx >= sz-1) return 0;
  return idx+1;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

int MsrCont::rangeLen(int lwb, int upb) const
{
  if (lwb >= sz || upb >= sz)
       throw IndexOutOfBoundsException("MsrCont::rangeLen");

  if (upb == lwb) return sz+1;

  if (upb < lwb) upb += sz;
  
  return upb-lwb+1;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrCont::limitRange(int maxLen, int lwb, int& upb) const
{
  if (maxLen < 2) throw IllegalArgumentException("MsrCont::limitRange");

  int rLen = rangeLen(lwb,upb);

  if (rLen <= maxLen) return;

  upb = lwb + maxLen-1;

  if (upb >= sz) upb -= sz;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

int MsrCont::midOf(int lwb, int upb) const
{
  int rLen = rangeLen(lwb,upb);
  if (rLen <= 2) return lwb;

  lwb += (rLen/2);

  if (lwb >= sz) lwb -= sz;

  return lwb;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool MsrCont::swap(int idx1, int idx2)
{
  if (idx1 < 0 || idx1 >= sz || idx2 < 0 || idx2 >= sz) return false;

  MsrPoint hp(itList[idx1]);
  itList[idx1] = itList[idx2];
  itList[idx2] = hp;

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool MsrCont::moveForward(int srcIdx, int dstIdx)
{
  if (srcIdx < 0 || srcIdx >= sz || dstIdx < 0 || dstIdx >= sz) return false;

  if (srcIdx == dstIdx) return true;

  MsrPoint hp(itList[srcIdx]);

  if (srcIdx > dstIdx) {
    memmove(itList+dstIdx+1,itList+dstIdx,(srcIdx-dstIdx)*sizeof(MsrPoint));
  }
  else {
    memmove(itList+1,itList,srcIdx*sizeof(MsrPoint));
    memmove(itList,itList+sz-1,sizeof(MsrPoint));
    memmove(itList+dstIdx+1,itList+dstIdx,(sz-1-dstIdx)*sizeof(MsrPoint));
  }

  itList[dstIdx] = hp;

  return true;
}

//---------------------------------------------------------------------------
//------- MsrContLst methods ------------------------------------------------
//---------------------------------------------------------------------------

void MsrContLst::resize(int newCap)
{
  if (newCap < sz) newCap = sz;
  if (newCap < 10) newCap = 10;

  if (!contList) sz = 0;

  MsrCont** newList = new MsrCont*[newCap];

  for (int i=0; i<sz; i++) newList[i] = contList[i];

  if (contList) delete[] contList;

  contList = newList;
  cap = newCap;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MsrContLst::MsrContLst(double pointTolerance)
: contList(new MsrCont*[32]), sz(0), cap(32),
  orient(XFirst), horOffset(0.0), verOffset(0.0),
  pointMode(false), radius(0.0),
  pointTolSq(pointTolerance*pointTolerance),idPtValid(false), idPt()
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MsrContLst::MsrContLst(const MsrContLst& cp)
: contList(new MsrCont*[cp.cap]), sz(cp.sz), cap(cp.cap),
  orient(cp.orient), horOffset(cp.horOffset), verOffset(cp.verOffset),
  pointMode(cp.pointMode), radius(cp.radius),
  pointTolSq(cp.pointTolSq),idPtValid(cp.idPtValid), idPt(cp.idPt)
{
  for (int i=0; i<sz; ++i) {
    contList[i] = new MsrCont(*cp.contList[i]);
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MsrContLst::~MsrContLst()
{
  clear();

  if (contList) delete[] contList;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MsrContLst& MsrContLst::operator=(const MsrContLst& src)
{
  clear();

  sz         = src.sz;
  cap        = src.cap;
  orient     = src.orient;
  horOffset  = src.horOffset;
  verOffset  = src.verOffset;
  pointMode  = src.pointMode;
  radius     = src.radius;
  pointTolSq = src.pointTolSq;
  idPtValid  = src.idPtValid;
  idPt       = src.idPt;

  if (contList) delete[] contList;

  contList = new MsrCont*[src.cap];

  for (int i=0; i<sz; ++i) contList[i] = new MsrCont(*src.contList[i]);

  return *this;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrContLst::clear()
{
  if (!contList) {
    sz = 0;
    return;
  }

  for (int i=0; i<sz; i++) {
    delete contList[i];
    contList[i] = NULL;
  }

  sz = 0;

  idPtValid = false;

  orient = XFirst;
  horOffset = 0.0;
  verOffset = 0.0;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrContLst::setOrientation(int orientation)
{
  switch (orientation) {
    case 1: orient = YFirst;
      break;

    case 2: orient = XLast;
      break;

    case 3: orient = YLast;
      break;

    default: orient = XFirst;
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrContLst::setHorOffset(double hor)
{
  horOffset = hor;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrContLst::setVerOffset(double ver)
{
  verOffset = ver;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrContLst::newContour(int layer)
{
  if (sz > 0) contList[sz-1]->checkEnd(pointTolSq);

  idPtValid = false;

  if (sz > 0 && contList[sz-1]->isEmpty()) return;

  if (sz >= cap) resize(cap*2);

  contList[sz++] = new MsrCont(radius,layer);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrContLst::setPointMode(bool on)
{
  pointMode = on;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrContLst::setRadiusCorr(double rad)
{
  radius = rad;
  if (sz > 0) contList[sz-1]->setRadCorr(rad);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrContLst::addPt(const Vec3& pt)
{
  if (sz < 1) newContour(0);

  Vec3 p(pt);

  // Simple point filter
  if (!idPtValid) idPtValid = true;
  else {
    Vec3 dp(p); dp -= idPt;

    if (dp.x*dp.x + dp.y*dp.y < pointTolSq) return; // 2D comparison!
  }

  idPt = p;

  contList[sz-1]->addPt(pointMode,p);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool MsrContLst::getFirstDir(Vec3& dir) const
{
  dir.x = 1.0; dir.y = 0.0; dir.z = 0.0;

  if (sz < 1) return false;

  const MsrCont& cnt = *contList[0];

  if (cnt.sz < 1) return false;

  Vec3 p1 = cnt.itList[0];

  for (int j=1; j<cnt.sz; j++) {
    const MsrPoint& pnt = cnt.itList[j];
    const Vec3& pt = pnt;

    if (pt.distTo3(p1) > 0.1) {
      dir = pt; dir -= p1;
      dir.unitLen3();
      return true;
    }
  }

  return false;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool MsrContLst::getLastDir(Vec3& dir) const
{
  dir.x = 1.0; dir.y = 0.0; dir.z = 0.0;

  if (sz < 1) return false;

  const MsrCont& cnt = *contList[sz-1];

  if (cnt.sz < 1) return false;

  Vec3 p1 = cnt.itList[cnt.sz-1];

  for (int j=cnt.sz-2; j>=0; j++) {
    const MsrPoint& pnt = cnt.itList[j];
    const Vec3& pt = pnt;

    if (pt.distTo3(p1) > 0.01) {
      dir = pt; dir -= p1;
      dir.unitLen3();
      return true;
    }
  }

  return false;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool MsrContLst::getDir(Vec3& p2) const
{
  if (sz < 1) return false;

  const MsrCont& cnt = *contList[0];
  if (cnt.sz < 1) return false;

  Vec3 p1 = cnt[0];

  for (int i=1; i<cnt.sz; i++) {
    const MsrPoint& pnt = cnt.itList[i];

    const Vec3& pt = pnt;

    if (pt.distTo3(p1) > 0.1) {
      p2 = pt;
      return true;
    }
  }

  return false;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrContLst::findOrgZXDir(bool projectFst,
                              Vec3& org, Vec3& zDir, Vec3& xDir) const
{
  // Find origin (avg of all points)

  int pntCnt = 0;

  org.x  = 0.0; org.y  = 0.0; org.z  = 0.0;
  xDir.x = 1.0; xDir.y = 0.0; xDir.z = 0.0;
  zDir.x = 0.0; zDir.y = 0.0; zDir.z = 1.0;

  org.x = 0.0; org.y = 0.0; org.z= 0.0;

  Vec3 p1;
  int i;

  int upb = sz;
  if (projectFst) upb = 1;

  for (i=0; i<upb; i++) {
    const MsrCont& cnt = *contList[i];

    for (int j=0; j<cnt.sz; j++) {
      const MsrPoint& pnt = cnt.itList[j];
      const Vec3& pt = pnt;

      if (i==0 && j==0) p1 = pt;

      org += pt;

      pntCnt++;
    }
  }

  if (pntCnt < 1) return;

  org.x /= pntCnt;
  org.y /= pntCnt;
  org.z /= pntCnt;

  if (pntCnt < 2) return;

  bool xDirSet = true;
  Vec3 p21;

  if (!getDir(p21)) {
    if (pntCnt < 3) return;

    xDirSet = false;
  }
  else {
    p21 -= p1;
    p21.unitLen3();
  }
 

  if (pntCnt < 3) {
    if (xDirSet) {
      if (p21*zDir > 3.0) {
        Vec3 yDir = xDir.outer(p21);
        xDir = p21;
        zDir = xDir.outer(yDir);
      }
      else {
        xDir = p21;
        Vec3 yDir = zDir.outer(xDir);
        zDir = xDir.outer(yDir);
      }

      if (orient == YFirst || orient == YLast) xDir = xDir.outer(zDir);
    }

    return;
  }

  Matrix mat(pntCnt,3);
  Matrix vt(3,3);

  pntCnt = 0;

  for (i=0; i<upb; i++) {
    const MsrCont& cnt = *contList[i];

    for (int j=0; j<cnt.sz; j++) {
      const MsrPoint& pnt = cnt.itList[j];
      const Vec3& pt = pnt;

      mat(pntCnt,0) = pt.x - org.x;
      mat(pntCnt,1) = pt.y - org.y;
      mat(pntCnt,2) = pt.z - org.z;

      pntCnt++;
    }
  }

  int rank = 0;
  int iter = 16+pntCnt*2;

  if (!mat.solveSvd(pntCnt,3,vt,rank,iter)) return;

  double minLabda = fabs(mat(0,0));
  zDir.x = vt(0,0);
  zDir.y = vt(0,1);
  zDir.z = vt(0,2);

  for (i=1; i<3; ++i) {
    if (fabs(mat(i,i)) < minLabda) {
      minLabda = fabs(mat(i,i));

      zDir.x = vt(i,0);
      zDir.y = vt(i,1);
      zDir.z = vt(i,2);
    }
  }

  xDirSet = false;

  switch (orient) {
    case XFirst:
    case YFirst:
    default: xDirSet = getFirstDir(xDir);
      break;

    case XLast:
    case YLast: xDirSet = getLastDir(xDir);
      break;
  }

  if (xDirSet) {
    Vec3 yDir = zDir.outer(xDir);
    if (yDir.len3() < 1e-6) xDirSet = false; // xDir not usable
  }

  if (xDirSet) {
    Vec3 yDir = zDir.outer(xDir);
    yDir.unitLen3();

    xDir = yDir.outer(zDir);
  }
  else {
    Vec3 yDir = zDir.outer(Vec3(1,0,0));

    if (yDir.len3() > 1e-6) {
      yDir.unitLen3();
      xDir = yDir.outer(zDir);
    }
    else {
      yDir = Vec3(0,1,0);

      xDir = yDir.outer(zDir);
      xDir.unitLen3();
    }
  }

  if (orient == YFirst || orient == YLast) xDir = xDir.outer(zDir);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrContLst::projectToPlane(double axDist, double rollRad,
                                bool projectFst, Trf3& planeTrf)
{
  planeTrf = Trf3();

  if (sz < 1) return;

  Vec3 org,zDir,xDir;

  findOrgZXDir(projectFst,org,zDir,xDir);

  Vec3 dir;
  calcWireDirTo(axDist,rollRad,org,dir);
  
  // Wire must come from positive z, so:
  if (dir*zDir > 0.0) zDir *= -1.0;

  // Apply horizontal pen offsets:

  for (int i=0; i<sz; i++) contList[i]->applyOffset(axDist,rollRad,horOffset,zDir);

  Trf3 trf(org,zDir,xDir);
  for (int i=0; i<sz; i++) contList[i]->transform(trf);

  // Now make first point the origin (at same z_level)
  MsrCont *cnt = contList[0];
  if (cnt == NULL || cnt->sz < 1) return;

  const Vec3& pt0 = cnt->itList[0];
  Vec3 p0(pt0); p0.z = 0;

  Trf3 trf2(p0,Vec3(0,0,1),Vec3(1,0,0));

  for (int i=0; i<sz; i++) contList[i]->transform(trf2);

  planeTrf = trf2; planeTrf *= trf;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrContLst::complete()
{
  if (sz < 1) return;

  contList[sz-1]->checkEnd(pointTolSq);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

const MsrCont& MsrContLst::operator[](int idx) const
{
  if (!contList || idx < 0 || idx >= sz)
              throw IndexOutOfBoundsException("MsrContLst::operator[]");

  return *contList[idx];
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MsrCont& MsrContLst::operator[](int idx)
{
  if (!contList || idx < 0 || idx >= sz)
               throw IndexOutOfBoundsException("MsrContLst::operator[]");

  return *contList[idx];
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void MsrContLst::transform(const Trf3& trf)
{
  if (!contList) return;

  for (int i=0; i<sz; ++i) {
    if (contList[i]) contList[i]->transform(trf);
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool MsrContLst::calcHullRect(Vec3& minPt, Vec3& maxPt) const
{
  if (sz < 1 || !contList[0]->calcHullRect(minPt,maxPt)) return false;

  for (int i=1; i<sz; ++i) {
    Vec3 lMinPt,lMaxPt;

    if (contList[i]->calcHullRect(lMinPt,lMaxPt)) {
      minOfPt(minPt,lMinPt);
      maxOfPt(maxPt,lMaxPt);
    }
  }

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef __GNUC__
bool MsrContLst::appendToCcd(DB2* ccdDb, bool threeD,bool unitInch,
                                                const char *contourTag) const
{
  if (!ccdDb) throw NullPointerException("MsrContLst::appendToCcd");
 
  if (isEmpty()) return false;

  char contTag[256] = "Contour";
  if (contourTag) strcpy(contTag,contourTag);
  strcat(contTag," %ld");

  unsigned int lastLayNr = 0xFFFFFFFF;
  Layer curLayer;

  for (int i=0; i<sz; i++) {
    const MsrCont *cnt = contList[i];

    unsigned int layNr = cnt->getLayer();
    char contNam[256] = "";

    if (i == 0 || layNr != lastLayNr) {
      sprintf(contNam,contTag,layNr);

      curLayer.setString(contNam);
      curLayer.prec.ilayer = ccdDb->addlayer(curLayer);
    }

    lastLayNr = layNr;

    cnt->appendToCcd(ccdDb,threeD,unitInch,curLayer);
  }

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool MsrContLst::appendCcdZLines(DB2* ccdDb, bool unitInch,
                                 const char *contourTag) const
{
  if (!ccdDb) throw NullPointerException("MsrContLst::appendCcdZLines");
 
  if (isEmpty()) return false;

  char contTag[256] = "Contour";
  if (contourTag) strcpy(contTag,contourTag);
  strcat(contTag," %ld");

  for (int i=0; i<sz; i++) {
    char contNam[256] = "";
    sprintf(contNam,contTag,i);

    contList[i]->appendCcdZLines(ccdDb,unitInch, contNam,0,0,255);
  }

  return false;
}
#endif

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool MsrContLst::appendToDxf(DxfOut& dxf,bool threeD, bool unitInch) const
{
  if (isEmpty()) return false;

  for (int i=0; i<sz; i++) contList[i]->appendToDxf(dxf,threeD,unitInch);

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool MsrContLst::appendDxfZLines(DxfOut& dxf, bool unitInch) const
{
  if (isEmpty()) return false;

  for (int i=0; i<sz; i++) contList[i]->appendDxfZLines(dxf,unitInch);

  return true;
}

} // namespace Ino

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
