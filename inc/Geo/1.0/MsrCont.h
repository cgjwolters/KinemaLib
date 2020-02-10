//---------------------------------------------------------------------------
//------- Defines a measurement point, a measurement contour ----------------
//------- and a list of measurement contours --------------------------------
//---------------------------------------------------------------------------
//------- Copyright Prodim BV Dec 2000 --------------------------------------
//---------------------------------------------------------------------------
//------- Inofor Hoek Aut BV C. Wolters Dec 2000 ----------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef MSRCONT_INC
#define MSRCONT_INC

#include "Basics.h"
#include "Vec.h"

class DB2;
class Layer;

namespace Ino
{
  class Trf3;
}

namespace Ino
{
  class DxfOut;
  class Contour;

//---------------------------------------------------------------------------
//------- A single measurement point ----------------------------------------
//---------------------------------------------------------------------------

class MsrPoint
{
  Vec3 pt;
  bool point; // true is point mode, false is stream mode point

  MsrPoint() : pt(), point(false) {}

  MsrPoint(const MsrPoint& cp);

  MsrPoint& operator=(const MsrPoint& src);

  void set(const Vec3& p, bool pointMode);

public:
  bool isPoint() const { return point; }

  operator const Vec3&() const { return pt; }

  void transform(const Trf3& trf);

  friend class MsrCont;
};

//---------------------------------------------------------------------------
//------ A contour (list of (immutable) MsrPoints) --------------------------
//---------------------------------------------------------------------------

class MsrCont
{
private:
  double radCorr;
  unsigned int layerNr;
  MsrPoint *itList;
  int cap, sz;

  void resize(int newCap);

  MsrCont(double radiusCorr, int layer);

  void addPt(bool pointMode, const Vec3& p);
  void checkEnd(double tolSq);

  void applyOffset(double axDist, double rollRad,
                                  double horOffset, const Vec3& zDir);
public:
  MsrCont(const MsrCont& cp);
  ~MsrCont();

  MsrCont& operator=(const MsrCont& src);

  int size() const { return sz; }
  bool isEmpty() const { return sz < 1; }

  unsigned int getLayer() const { return layerNr; }

  void removeLastPt();
  void close();

  const MsrPoint& operator[](int idx) const;

  bool calcHullRect(Vec3& minPt, Vec3& maxPt) const;

  double getRadCorr() const { return radCorr; }
  void setRadCorr(double newRadCorr) { radCorr = newRadCorr; }

  bool closed() const;

  bool interpolateInto(double tolerance, double maxRad, bool noArcs,
                                                        Contour& newCont);

  void transform(const Trf3& trf);

  void appendToCcd(DB2* ccdDb, bool threeD, bool unitInch,
                   const Layer& layer) const;

  void appendToCcd(DB2* ccdDb, bool threeD, bool unitInch,
                   const Layer& layer, int r, int g, int b) const;

  void appendCcdZLines(DB2* ccdDb, bool unitInch, const char *contNam,
                                        int r, int g , int b) const;

  void appendToDxf(DxfOut& dxf,bool threeD, bool unitInch) const;
  void appendDxfZLines(DxfOut& dxf, bool unitInch) const;

  int prvIdx(int idx) const;
  int nxtIdx(int idx) const;
  int rangeLen(int lwb, int upb) const;
  void limitRange(int maxLen, int lwb, int& upb) const;
  int midOf(int lwb, int upb) const;

  bool swap(int idx1, int idx2);
  bool moveForward(int srcIdx, int dstIdx);

  friend class MsrContLst;
};

//---------------------------------------------------------------------------
//-------- A list of contours (MsrConts) ------------------------------------
//---------------------------------------------------------------------------

class MsrContLst
{
public:
  enum Orient {
    XFirst = 0, YFirst = 1,
    XLast  = 2, YLast  = 3
  };

private:
  MsrCont **contList;
  int sz, cap;

  Orient orient;    // Requested orientation of the data
  double horOffset; // Horizontal pen offset
  double verOffset; // Vertical   pen offset

  bool pointMode;   // Current mode (point or stream)
  double radius;    // Current radius correction

  double pointTolSq;

  bool idPtValid;
  Vec3 idPt;

  void resize(int newCap);

  bool getFirstDir(Vec3& dir) const;
  bool getLastDir(Vec3& dir) const;

  bool getDir(Vec3& p2) const;
  void findOrgZXDir(bool projectFst,
                    Vec3& org, Vec3& zDir, Vec3& xDir) const;

public:
  MsrContLst(double pointTolerance);
  MsrContLst(const MsrContLst& cp);
  ~MsrContLst();

  MsrContLst& operator=(const MsrContLst& src);

  int size() const { return sz; }
  bool isEmpty() const { return sz < 1; }

  void clear();

  void setOrientation(int orientation);
  void setHorOffset(double hor);
  void setVerOffset(double ver);

  void newContour(int layer);

  void setPointMode(bool on);
  void setRadiusCorr(double rad);
  void addPt(const Vec3& pt);

  void complete();
  void projectToPlane(double axDist, double rollRad,
                      bool projectFst, Trf3& planeTrf);

  const MsrCont& operator[](int idx) const;
        MsrCont& operator[](int idx);

  void transform(const Trf3& trf);

  bool calcHullRect(Vec3& minPt, Vec3& maxPt) const;

  bool appendToCcd(DB2* ccdDb, bool threeD, bool unitInch,
                   const char *contourTag) const;

  bool appendCcdZLines(DB2* ccdDb, bool unitInch,
                       const char *contourTag) const;

  bool appendToDxf(DxfOut& dxf,bool threeD, bool unitInch) const;
  bool appendDxfZLines(DxfOut& dxf, bool unitInch) const;
};

} // namespace Ino


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#endif

