//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Dxf 3D Format Reader --------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Copyright (C) 2008 Inofor Hoek Aut BV ---------------------------
//---------------------------------------------------------------------------
//--------- C.Wolters Sept 2008----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef BLOCKTABLE3D_INC
#define BLOCKTABLE3D_INC

#include "ObjNameTable3D.h"
#include "DxfRead3D.h"
#include "ColorTable3D.h"

//---------------------------------------------------------------------------

namespace Ino
{

class DxfRead3D;
class DxfNameTable3D;
class DxfColor3D;
class DxfColorTable3D;
class LineTypeTable3D;
class TextStyleTable3D;
class LayerTable3D;

class BlockTable3D : ObjNameTable3D<DxfBlock3D>
{
  static char substCodeTab[][6];
  static unsigned char replCodeTab[]; 

  DxfRead3D& dxfRd;

  DxfColorTable3D  colTable;
  DxfNameTable3D&  nameTable;
  LineTypeTable3D& ltTable;
  TextStyleTable3D& txtStyleTable;
  LayerTable3D&    layerTable;

  char *txtBuf;

  double *vertexX;
  double *vertexY;
  double *vertexZ;
  double *vertexB;
  int vertexSz, vertexCap;

  int findCode(int pos, int txtSz);
  int substCode(int pos, int txtSz);
  int substNumCode(int pos, int txtSz);
  int substCodes(int txtSz);
  int substNewLine(int txtSz);

  DxfBlock3D *readBlkHeader(DxfReader3D& rdr);
  void readBlkEnd(DxfReader3D& rdr, DxfBlock3D& blk);
  void skipBlock(DxfReader3D& rdr);
  void readEntities(DxfReader3D& rdr, DxfBlock3D& blk);
  bool isByBlock(DxfReader3D& rdr);
  const DxfColor3D& getColor(int aciNr, DxfLayer3D *lay);
  void readArc(DxfReader3D& rdr, DxfBlock3D& blk);
  void readCircle(DxfReader3D& rdr, DxfBlock3D& blk);
  void readInsert(DxfReader3D& rdr, DxfBlock3D& blk);
  void readLine(DxfReader3D& rdr, DxfBlock3D& blk);
  void lwPolySeg(void *usrArg, DxfAttr3D& attr,
                              const Vec2& p1, const Vec2& p2, double bulge);
  void readLwPoly(DxfReader3D& rdr, DxfBlock3D& blk);
  void readMLine(DxfReader3D& rdr, DxfBlock3D& blk);
  int getTxtRef(int horJust, int verJust);
  void readAttrib(DxfReader3D& rdr, DxfBlock3D& blk);
  void readText(DxfReader3D& rdr, DxfBlock3D& blk);
  static int getMTxtRef(int dxfRef);
  void readMText(DxfReader3D& rdr, DxfBlock3D& blk);
  void readRText(DxfReader3D& rdr, DxfBlock3D& blk);
  void addVertex(double x, double y, double z, double bulge);
  void skipToSeqEnd(DxfReader3D& rdr);
  void readVertex(DxfReader3D& rdr);
  void readVertices(DxfReader3D& rdr);
  void polySeg(void *usrArg, DxfAttr3D& attr,
                              const Vec3& p1, const Vec3& p2, double bulge);
  void readPolyLine(DxfReader3D& rdr, DxfBlock3D& blk);
  void readSpline(DxfReader3D& rdr, DxfBlock3D& blk);
  void readTrace(DxfReader3D& rdr, DxfBlock3D& blk);
  void readDimension(DxfReader3D& rdr, DxfBlock3D& blk);
  void readSolid(DxfReader3D& rdr, DxfBlock3D& blk);
  void readHatch(DxfReader3D& rdr, DxfBlock3D& blk);

  BlockTable3D(const BlockTable3D& cp);             // No Copying
  BlockTable3D& operator=(const BlockTable3D& src); // No Assignment

public:
  BlockTable3D(DxfRead3D& dxf, DxfNameTable3D& nameTbl, LineTypeTable3D& ltTbl,
             TextStyleTable3D& txtStyleTbl, LayerTable3D& layerTbl);

  ~BlockTable3D();

  using ObjNameTable3D<DxfBlock3D>::get;

  void readBlock(DxfReader3D& rdr);

  friend class DxfRead3D;
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
