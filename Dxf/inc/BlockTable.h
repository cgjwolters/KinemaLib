//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Dxf Format Reader -----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Copyright (C) 2005 Inofor Hoek Aut BV ---------------------------
//---------------------------------------------------------------------------
//--------- C.Wolters June 2005----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef BLOCKTABLE_INC
#define BLOCKTABLE_INC

#include "ObjNameTable.h"
#include "DxfRead.h"
#include "ColorTable.h"

//---------------------------------------------------------------------------

namespace Ino
{

class DxfRead;
class DxfNameTable;
class DxfColor;
class DxfColorTable;
class LineTypeTable;
class TextStyleTable;
class LayerTable;

class BlockTable : ObjNameTable<DxfBlock>
{
  static char substCodeTab[][6];
  static unsigned char replCodeTab[]; 

  DxfRead& dxfRd;

  DxfColorTable  colTable;
  DxfNameTable&  nameTable;
  LineTypeTable& ltTable;
  TextStyleTable& txtStyleTable;
  LayerTable&    layerTable;

  char *txtBuf;

  double *vertexX;
  double *vertexY;
  double *vertexB;
  int vertexSz, vertexCap;

  int findCode(int pos, int txtSz);
  int substCode(int pos, int txtSz);
  int substNumCode(int pos, int txtSz);
  int substCodes(int txtSz);
  int substNewLine(int txtSz);

  DxfBlock *readBlkHeader(DxfReader& rdr);
  void readBlkEnd(DxfReader& rdr, DxfBlock& blk);
  void skipBlock(DxfReader& rdr);
  void readEntities(DxfReader& rdr, DxfBlock& blk);
  bool isByBlock(DxfReader& rdr);
  const DxfColor& getColor(int aciNr, DxfLayer *lay);
  void readArc(DxfReader& rdr, DxfBlock& blk);
  void readCircle(DxfReader& rdr, DxfBlock& blk);
  void readInsert(DxfReader& rdr, DxfBlock& blk);
  void readLine(DxfReader& rdr, DxfBlock& blk);
  void lwPolySeg(void *usrArg, DxfAttr& attr,
                              const Vec2& p1, const Vec2& p2, double bulge);
  void readLwPoly(DxfReader& rdr, DxfBlock& blk);
  void readMLine(DxfReader& rdr, DxfBlock& blk);
  int getTxtRef(int horJust, int verJust);
  void readAttrib(DxfReader& rdr, DxfBlock& blk);
  void readText(DxfReader& rdr, DxfBlock& blk);
  static int getMTxtRef(int dxfRef);
  void readMText(DxfReader& rdr, DxfBlock& blk);
  void readRText(DxfReader& rdr, DxfBlock& blk);
  void addVertex(double x, double y, double bulge);
  void skipToSeqEnd(DxfReader& rdr);
  void readVertex(DxfReader& rdr);
  void readVertices(DxfReader& rdr);
  void polySeg(void *usrArg, DxfAttr& attr,
                              const Vec2& p1, const Vec2& p2, double bulge);
  void readPolyLine(DxfReader& rdr, DxfBlock& blk);
  void readSpline(DxfReader& rdr, DxfBlock& blk);
  void readTrace(DxfReader& rdr, DxfBlock& blk);
  void readDimension(DxfReader& rdr, DxfBlock& blk);
  void readSolid(DxfReader& rdr, DxfBlock& blk);
  void readHatch(DxfReader& rdr, DxfBlock& blk);

  BlockTable(const BlockTable& cp);             // No Copying
  BlockTable& operator=(const BlockTable& src); // No Assignment

public:
  BlockTable(DxfRead& dxf, DxfNameTable& nameTbl, LineTypeTable& ltTbl,
             TextStyleTable& txtStyleTbl, LayerTable& layerTbl);

  ~BlockTable();

  using ObjNameTable<DxfBlock>::get;

  void readBlock(DxfReader& rdr);

  friend class DxfRead;
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
