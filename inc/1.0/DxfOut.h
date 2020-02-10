//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Dxf Output Module -------------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV Jan 2004 -----------------------------
//---------------------------------------------------------------------------
//------- C. Wolters Jan 2004 -----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef INODXFOUT_INC
#define INODXFOUT_INC

#include "Contour.h"

//---------------------------------------------------------------------------

namespace Ino
{

class Writer;
class Vec2;
class Vec3;
class Trf3;

class DxfOut
{
  class Layer;
public:
  enum Units { Mtr, Mm, Inch };
  enum Decimals { MinDecimals = 0, MaxDecimals = 16 };
  enum Color { ColByBlock, ColRed, ColYellow, ColGreen, ColCyan,
               ColBlue, ColMagenta, ColBlackWhite, ColDarkGrey, ColGrey,
               ColByLayer = 256 };

private:
  Writer *wrtr;

  bool prologWritten;
  bool epilogWritten;
  bool reverseExtrusionDir;
  bool crlf;

  bool version2007;

  int handleId;
  long seedVal;

  Units userUnit;
  Vec3& extMin;
  Vec3& extMax;

  Layer *layLst;

  int laySz,layCap;

  long currentLayer;
  Color currentColor;
  bool forceElemColor;

  long decimals;
  double zeroTol;

  enum { FmtBufSz = 60 };
  char dblFormat[FmtBufSz];
  char pnt2Format[FmtBufSz];
  char pnt3Format[FmtBufSz];

  Color currentElemColor() const;

  void aaa(const Vec3& zDir, Trf3& trf);

  void writeGroup(int code, const char *val);
  void writeGroup(int code, int val);
  void writeHexGroup(int code, int val);
  void writeGroup(int code, double val);
  void writePoint(double x, double y);
  void writePoint2D(const Vec2& pt);
  void writePoint(double x, double y, double z);
  void writePoint(const Vec3& pt);

  void writeHeader();

  void writeVPortTable();
  void writeViewTable();
  void writeLTypeTable();
  void writeLayerTable();
  void writeStyleTable();
  void writeDimStyleTable();
  void writeUCSTable();
  void writeAppIDTable();
  void writeBlockRecordTable();

  void writeTables();
  void writeBlocks();

  void writeProlog();

  void writeObjects();
  void writeEpilog();

  bool write2DPoly(const Trf3& trf, const Elem_C_Cursor& from,
                   const Elem_C_Cursor& upto, bool closed);

  bool write3DPoly(const Trf3& trf, const Elem_C_Cursor& from,
                   const Elem_C_Cursor& upto, bool closed);

  DxfOut(const DxfOut& cp);             // No copying
  DxfOut& operator=(const DxfOut& src); // No Assignment

public:
  DxfOut(Writer &writer, bool appendCrLf=false);
  ~DxfOut();

  void setVersion2007() { version2007 = true; }

  void finish();

  bool isOpen() const;

  bool setDecimals(int decs);

  bool setReversedExtrusionDir(bool reverse);
  bool getReversedExtrusionDir() const { return reverseExtrusionDir; }

  bool setUnits(Units newUnit);
  bool setExtents(const Vec3& xMin, const Vec3& xMax);

  bool setHandSeed(long newSeed); // See explanation below

  void setForceElemColor(bool forceElColor)
                                    { forceElemColor = forceElColor; }
  bool getForceElemColor() const { return forceElemColor; }

  long getLayer(const char *name) const;
  long addLayer(const char *name, Color col,
                bool bKey=false, long nKey=-1);
  long addLayer(const char *name,
                unsigned char red, unsigned char green, unsigned char blue,
                bool bKey=false, long nKey=0xFFFFFFFF);

  // Call each set method above once BEFORE calling any of the
  // methods below!

  bool setCurrentLayer(long layerIdx);
  bool setCurrentLayer(bool bKey, long nKey);
  bool setCurrentColor(Color color);
  bool setCurrentColor(unsigned char red, unsigned char green, unsigned char blue);
  
  bool addPoint(const Vec3& p1);
  bool addLine(const Vec3& p1, const Vec3& p2);
  bool addArc(const Vec3& c, double radius,
                            double startAng, double endAng, const Trf3& trf);
  bool addCircle(const Vec3& c, double radius, const Trf3& trf);

  bool add2DPoly(const Vec2 *ptLst, int listLen);
  bool add2DPoly(const Trf3& trf, const Contour& cont);

  bool add3DPoly(const Trf3& trf, const Contour& cont);
  bool add3DPoly(const Vec3* ptLst, int listLen);
};

// The hand seed value must be calculated beforehand.
// It is determined als follows:

// Add 1 for each call to addLayer
// Add 1 for each call to addLine
// Add 1 for each call to add2DPoly;
// Add 2 plus the number of points in the poly line
// for each call to add3DPoly.
// Add some slack if desired.

} //namespace Ino

//---------------------------------------------------------------------------
#endif
