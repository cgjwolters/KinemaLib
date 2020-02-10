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

#include "BlockTable3D.h"

#include "Exceptions.h"
#include "DxfRead3D.h"
#include "DxfReader3D.h"
#include "DxfNameTable3D.h"
#include "LayerTable3D.h"
#include "TextStyleTable3D.h"
#include "Vec.h"
#include "Trf.h"

#include <cmath>
#include <cstring>

using namespace std;

namespace Ino
{

//---------------------------------------------------------------------------

char BlockTable3D::substCodeTab[][6] = 
  {  "%%%",   // Percent
     "%%c",   // Diameter
     "%%C",   // Diameter
     "%%d",   // Degree
     "%%D",   // Degree
     "%%P",   // PlusMinus
     "%%p",   // PlusMinus
     "%%u",   // Underscore
     "%%U",   // Underscore
     "%%o",   // Overscore
     "%%O",   // Overscore
     "%%237", // Degree
     "%%248", // Diameter
     "%%132", // German ae
     "%%148", // German oe
     "%%129", // German ue
     "%%142", // German AE
     "%%153", // German OE
     "%%154", // German UE
     "%%225", // German sz
     "%%224", // Alpha
     "%%230", // Mu
     "%%241", // Plusminus
  };

// ------------------------------------------------------------------------

unsigned char BlockTable3D::replCodeTab[] = 
  {
    '%',        // Percent
    (unsigned char)248,  // Diameter
    (unsigned char)248,  // Diameter
    (unsigned char)176,  // Degree
    (unsigned char)176,  // Degree
    (unsigned char)177,  // PlusMinus
    (unsigned char)177,  // PlusMinus
    (unsigned char)  0,  // Underscore (not supported)
    (unsigned char)  0,  // Underscore (not supported)
    (unsigned char)  0,  // Overscore (not supported)
    (unsigned char)  0,  // Overscore (not supported)
    (unsigned char)176,  // Degree
    (unsigned char)248,  // Diameter
    (unsigned char)204,  // German ae
    (unsigned char)206,  // German oe
    (unsigned char)207,  // German ue
    (unsigned char)216,  // German AE
    (unsigned char)218,  // German OE
    (unsigned char)219,  // German UE
    (unsigned char)222,  // German sz
    (unsigned char)242,  // Alpha
    (unsigned char)181,  // Mu
    (unsigned char)177   // Plusminus
  };

//---------------------------------------------------------------------------

DxfBlock3D *BlockTable3D::readBlkHeader(DxfReader3D& rdr)
{
  if (rdr.code != 0) return NULL;

  int id = nameTable.getNameId(rdr.value);
  if (id == DxfNameTable3D::ID_ENDSEC) return NULL;

  if (id != DxfNameTable3D::ID_BLOCK) throw FileFormatException("Expecting BLOCK Section");

  char blkName[256] = "";
  DxfLayer3D *layer = NULL;

  double baseX=0.0, baseY=0.0, baseZ=0.0;

  while (rdr.next()) {
    if (rdr.code == 0) break;

    switch (rdr.code) {
      case 2:
        strcpy(blkName,rdr.value);
      break;

      case 8:
        layer = layerTable.get(rdr.value);
      break;

      case 10: baseX = rdr.toDouble();
      break;

      case 20: baseY = rdr.toDouble();
      break;

      case 30: baseZ = rdr.toDouble();
      break;

      case 102: skipGroup(rdr);
      break;
    }
  }

  // if (!blkName) return NULL;

  Vec3 basePt(baseX,baseY,baseZ);
  DxfBlock3D *blk = new DxfBlock3D(blkName,layer,basePt);

  add(blkName,blk);

  dxfRd.builder.addBlock(*blk);

  return blk;
}

//---------------------------------------------------------------------------

void BlockTable3D::readBlkEnd(DxfReader3D& rdr, DxfBlock3D& /*blk*/)
{
  while (rdr.next()) {
    if (rdr.code == 0) return;
  }
}

//---------------------------------------------------------------------------

void BlockTable3D::skipBlock(DxfReader3D& rdr)
{
  for (;;) {
    if (rdr.code == 0) {
      int id = nameTable.getNameId(rdr.value);

      if (id == DxfNameTable3D::ID_ENDBLK) {
        rdr.next();
        return;
      }

      if (id == DxfNameTable3D::ID_ENDSEC) return;
    }

    if (!rdr.next()) return;
  }
}

//---------------------------------------------------------------------------

void BlockTable3D::readEntities(DxfReader3D& rdr, DxfBlock3D& blk)
{
  while (!rdr.eof) {
    if (rdr.code != 0) {
      rdr.next();
      continue;
    }

    int id = nameTable.getNameId(rdr.value);

    switch (id) {
      case DxfNameTable3D::ID_ENDSEC:
      case DxfNameTable3D::ID_EOF:     // Shouldn't happen
      case DxfNameTable3D::ID_ENDBLK:
      return;

      case DxfNameTable3D::ID_ARC: readArc(rdr,blk);
      break;

      case DxfNameTable3D::ID_ATTRIB: readAttrib(rdr,blk);
      break;

      case DxfNameTable3D::ID_CIRCLE: readCircle(rdr,blk);
        break;

      case DxfNameTable3D::ID_INSERT: readInsert(rdr,blk);
        break;

      case DxfNameTable3D::ID_LINE: readLine(rdr,blk);
        break;

      case DxfNameTable3D::ID_LWPOLYLINE: readLwPoly(rdr,blk);
        break;

      case DxfNameTable3D::ID_MLINE: readMLine(rdr,blk);
        break;

      case DxfNameTable3D::ID_TEXT: readText(rdr,blk);
        break;

      case DxfNameTable3D::ID_MTEXT: readMText(rdr,blk);
        break;

      case DxfNameTable3D::ID_RTEXT: readRText(rdr,blk);
        break;

      case DxfNameTable3D::ID_POLYLINE: readPolyLine(rdr,blk);
        break;

      case DxfNameTable3D::ID_SPLINE: readSpline(rdr,blk);
        break;

      case DxfNameTable3D::ID_TRACE: readTrace(rdr,blk);
        break;

      case DxfNameTable3D::ID_DIMENSION: readDimension(rdr,blk);
      break;

      case DxfNameTable3D::ID_HATCH: readHatch(rdr,blk);
      break;

      case DxfNameTable3D::ID_SOLID: readSolid(rdr,blk);
      break;

      default: // Skip entity
        rdr.next();
      break;
    }
  }
}

//---------------------------------------------------------------------------

bool BlockTable3D::isByBlock(DxfReader3D& rdr)
{
  int id = nameTable.getNameId(rdr.value);

  return id == DxfNameTable3D::ID_BYBLOCK;
}

//---------------------------------------------------------------------------


const DxfColor3D& BlockTable3D::getColor(int aciNr, DxfLayer3D *lay)
{
  if (aciNr == 256) {
    if (lay != NULL) return colTable.getColor(lay->colNr);
    else return DxfColor3D::white;
  }

  return colTable.getColor(aciNr);
}

//---------------------------------------------------------------------------
// AutoCad Arbitrary Axis Algorithm:

static void aaa(const Vec3& zDir, Trf3& trf)
{
  Vec3 lZDir(zDir); lZDir.unitLen3();

  Vec3 xDir;

  if (abs(lZDir.x) < 1.0/64 && abs(lZDir.y) < 1.0/64)
       xDir = Vec3(0,1,0).outer(zDir);
  else xDir = Vec3(0,0,1).outer(zDir);

  xDir.unitLen3();

  trf = Trf3(Vec3(),lZDir,xDir);
}

//---------------------------------------------------------------------------

void BlockTable3D::readArc(DxfReader3D& rdr, DxfBlock3D& blk)
{
  Vec2 c;
  Vec3 zDir(0,0,1);
  double r = 0,startAng = 0,endAng = 0, elevation = 0.0;
  int   inVisible=0, colNr = 256;
  const DxfLineType3D *lt = NULL;
  DxfLayer3D *layer = NULL;
  double ltScale = 1.0;
  bool ltByBlock = false;

  while (rdr.next()) {
    if (rdr.code == 0) break;

    switch (rdr.code) {
      case 10: c.x = rdr.toDouble();
      break;

      case 20: c.y = rdr.toDouble();
      break;

      case 30: elevation = rdr.toDouble();
      break;

      case 40: r  = rdr.toDouble();
      break;

      case 50: startAng = rdr.toDouble()*Vec2::Pi/180.0;
      break;

      case 51: endAng = rdr.toDouble()*Vec2::Pi/180.0;
      break;

      case 6: if (isByBlock(rdr)) ltByBlock = true;
              else lt = ltTable.get(rdr.value);
      break;

      case 8: layer = layerTable.get(rdr.value);
      break;

      case 48: ltScale = fabs(rdr.toDouble());
      break;

      case 60: inVisible = rdr.toInt();
      break;

      case 62: colNr = rdr.toInt();
      break;

      case 210: zDir.x = rdr.toDouble();
      break;

      case 220: zDir.y = rdr.toDouble();
      break;

      case 230: zDir.z = rdr.toDouble();
      break;

      case 102: skipGroup(rdr);
      break;
    }
  }

  if (inVisible) return; // Invisible

  if (layer == NULL) layer = blk.getLayer();

  if (lt == NULL && !ltByBlock) {
    if (layer != NULL) lt = &layer->lt; //  By layer
    else lt = &DxfLineType3D::solid;
  }

  const DxfColor3D& col = getColor(colNr,layer);

  DxfAttr3D attr(blk,*layer,colNr,col,*lt,ltScale);

  Trf3 trf; aaa(zDir,trf);
  trf(2,3) -= elevation; trf.invert();

  dxfRd.builder.addArc(attr,trf,c,r,startAng,endAng);
}

//---------------------------------------------------------------------------

void BlockTable3D::readCircle(DxfReader3D& rdr, DxfBlock3D& blk)
{
  Vec2 c;
  Vec3 zDir(0,0,1);
  double r = 0, elevation = 0.0;
  int   inVisible=0, colNr = 256;
  const DxfLineType3D *lt = NULL;
  DxfLayer3D *layer = NULL;
  double ltScale = 1.0;
  bool ltByBlock = false;

  while (rdr.next()) {
    if (rdr.code == 0) break;

    switch (rdr.code) {
      case 10: c.x = rdr.toDouble();
      break;

      case 20: c.y = rdr.toDouble();
      break;

      case 30: elevation = rdr.toDouble();
      break;

      case 40: r  = rdr.toDouble();
      break;

      case 6: if (isByBlock(rdr)) ltByBlock = true;
              else lt = ltTable.get(rdr.value);
      break;

      case 8: layer = layerTable.get(rdr.value);
      break;

      case 48: ltScale = fabs(rdr.toDouble());
      break;

      case 60: inVisible = rdr.toInt();
      break;

      case 62: colNr = rdr.toInt();
      break;

      case 210: zDir.x = rdr.toDouble();
      break;

      case 220: zDir.y = rdr.toDouble();
      break;

      case 230: zDir.z = rdr.toDouble();
      break;

      case 102: skipGroup(rdr);
      break;
    }
  }

  if (inVisible) return; // Invisible

  if (r == 0) return; // Invalid circle!

  if (layer == NULL) layer = blk.getLayer();

  const DxfColor3D& col = getColor(colNr,layer);

  if (lt == NULL && !ltByBlock) {
    if (layer != NULL) lt = &layer->lt; //  By layer
    else lt = &DxfLineType3D::solid;
  }

  DxfAttr3D attr(blk,*layer,colNr,col,*lt,ltScale);

  Trf3 trf; aaa(zDir,trf);
  trf(2,3) -= elevation; trf.invert();

  dxfRd.builder.addCircle(attr,trf,c,r);
}

//---------------------------------------------------------------------------

void BlockTable3D::readInsert(DxfReader3D& rdr, DxfBlock3D& blk)
{
  Vec3 insPt,zDir(0,0,1);
  double xScale = 1, yScale = 1,zScale = 1;
  double angle = 0;

  int rowCount = 1, colCount = 1;
  double rowSpacing = 0, colSpacing = 0;

  DxfBlock3D *insBlk = NULL;

  int   inVisible = 0, colNr = 256;
  const DxfLineType3D *lt = NULL;
  DxfLayer3D *layer = NULL;
  double ltScale = 1.0;
  bool ltByBlock = false;

  while (rdr.next()) {
    if (rdr.code == 0) break;

    switch (rdr.code) {
      case  2: insBlk = get(rdr.value);
      break;

      case 10: insPt.x = rdr.toDouble();
      break;

      case 20: insPt.y = rdr.toDouble();
      break;

      case 30: insPt.z = rdr.toDouble();
      break;

      case 41: xScale = rdr.toDouble();
               if (fabs(xScale) < 1e-10) xScale = 1.0;
      break;

      case 42: yScale = rdr.toDouble();
               if (fabs(yScale) < 1e-10) yScale = 1.0;
      break;

      case 43: zScale = rdr.toDouble();
               if (fabs(zScale) < 1e-10) zScale = 1.0;
      break;

      case 50: angle = rdr.toDouble()*Vec2::Pi/180.0;
      break;

      case 44: colSpacing = rdr.toDouble();
      break;

      case 45: rowSpacing = rdr.toDouble();
      break;

      case 70: colCount = rdr.toInt();
      break;

      case 71: rowCount = rdr.toInt();
      break;

      case 6: if (isByBlock(rdr)) ltByBlock = true;
              else lt = ltTable.get(rdr.value);
      break;

      case 8: layer = layerTable.get(rdr.value);
      break;

      case 48: ltScale = fabs(rdr.toDouble());
      break;

      case 60: inVisible = rdr.toInt();
      break;

      case 62: colNr = rdr.toInt();
      break;

      case 210: zDir.x = rdr.toDouble();
      break;

      case 220: zDir.y = rdr.toDouble();
      break;

      case 230: zDir.z = rdr.toDouble();
      break;

      case 102: skipGroup(rdr);
      break;
    }
  }

  if (inVisible) return; // Invisible

  if (insBlk == NULL) return; // Not a valid insert

  if (layer == NULL) layer = blk.getLayer();

  const DxfColor3D& col = getColor(colNr,layer);

  if (lt == NULL && !ltByBlock) {
    if (layer != NULL) lt = &layer->lt; //  By layer
    else lt = &DxfLineType3D::solid;
  }

  DxfAttr3D attr(blk,*layer,colNr,col,*lt,ltScale);

  // Base point shift
  const Vec3& basePt  = insBlk->getBasePt();

  Trf3 trf1(1.0,0.0,0.0,-basePt.x,
            0.0,1.0,0.0,-basePt.y,
            0.0,0.0,1.0,-basePt.z);

  double cs = cos(angle);
  double sn = sin(angle);

  Trf3 trf2(cs*xScale, -sn*yScale,    0.0, insPt.x,
            sn*xScale,  cs*yScale,    0.0, insPt.y,
                  0.0,        0.0, zScale, insPt.z);

  Trf3 trf;
  aaa(zDir,trf);
  
  trf.invert(); trf *= trf2; trf *= trf1;

  dxfRd.builder.insertBlock(attr,*insBlk,trf,
                            colSpacing,colCount,rowSpacing,rowCount);
}

//---------------------------------------------------------------------------

void BlockTable3D::readLine(DxfReader3D& rdr, DxfBlock3D& blk)
{
  Vec3 p1,p2;
  int   inVisible=0, colNr = 256;
  const DxfLineType3D *lt = NULL;
  DxfLayer3D *layer = NULL;
  double ltScale = 1.0;
  bool ltByBlock = false;

  while (rdr.next()) {
    if (rdr.code == 0) break;

    switch (rdr.code) {
      case 10: p1.x = rdr.toDouble();
      break;

      case 20: p1.y = rdr.toDouble();
      break;

      case 30: p1.z = rdr.toDouble();
      break;

      case 11: p2.x = rdr.toDouble();
      break;

      case 21: p2.y = rdr.toDouble();
      break;

      case 31: p2.z = rdr.toDouble();
      break;

      case 6: if (isByBlock(rdr)) ltByBlock = true;
              else lt = ltTable.get(rdr.value);
      break;

      case 8: layer = layerTable.get(rdr.value);
      break;

      case 48: ltScale = fabs(rdr.toDouble());
      break;

      case 60: inVisible = rdr.toInt();
      break;

      case 62: colNr = rdr.toInt();
      break;

      case 102: skipGroup(rdr);
      break;
    }
  }

  if (inVisible) return; // Invisible

  if (p1.sqDistTo3(p2) < 1e-14) return; // Invalid line

  if (layer == NULL) layer = blk.getLayer();

  const DxfColor3D& col = getColor(colNr,layer);

  if (lt == NULL && !ltByBlock) {
    if (layer != NULL) lt = &layer->lt; //  By layer
    else lt = &DxfLineType3D::solid;
  }

  DxfAttr3D attr(blk,*layer,colNr,col,*lt,ltScale);

  dxfRd.builder.addLine(attr,p1,p2);
}

//---------------------------------------------------------------------------

void BlockTable3D::lwPolySeg(void *usrArg, DxfAttr3D& attr,
                              const Vec2& p1, const Vec2& p2, double bulge)
{
  if (fabs(bulge) < 0.001) dxfRd.builder.addLwPolyLine(usrArg,attr,p1,p2);
  else {
    Vec2 dir(p2); dir -= p1;

    dir.rot270(); dir *= bulge/2.0;

    Vec2 c(p1); c += p2; c /= 2.0; c += dir;

    dir = c; dir -= p1;
    
    dir.rot90(); dir /= bulge*2.0;

    c += p1; c /= 2.0; c += dir;

    double r = c.distTo2(p1);

    Vec2 dp1(p1); dp1 -= c;
    double startAngle = dp1.angle();
    if (startAngle < 0.0) startAngle += Vec2::Pi2;

    double sweepAngle = 4.0 * atan(bulge);

    dxfRd.builder.addLwPolyArc(usrArg,attr,c,r,startAngle,sweepAngle);
  }
}

//---------------------------------------------------------------------------

void BlockTable3D::readLwPoly(DxfReader3D& rdr, DxfBlock3D& blk)
{
  double x = 0.0, y = 0.0, bulge = 0.0, elevation = 0.0;
  Vec3 zDir(0,0,1);

  vertexSz = 0;

  bool closed = false, first = true;

  int   inVisible=0, colNr = 256;
  const DxfLineType3D *lt = NULL;
  DxfLayer3D *layer = NULL;
  double ltScale = 1.0;
  bool ltByBlock = false;

  while (rdr.next()) {
    if (rdr.code == 0) {
      if (!first) addVertex(x,y,0.0,bulge);
      break;
    }

    switch (rdr.code) {
      case 10:
        if (!first) addVertex(x,y,0.0,bulge);
        first = false;
        bulge = 0.0;

        x = rdr.toDouble();
      break;

      case 20:
        y = rdr.toDouble();
      break;

      case 42:
        bulge = rdr.toDouble();
      break;

      case 70: closed = (rdr.toInt() & 1) != 0;
      break;

      case 6: if (isByBlock(rdr)) ltByBlock = true;
              else lt = ltTable.get(rdr.value);
      break;

      case 8: layer = layerTable.get(rdr.value);
      break;

      case 38: elevation = rdr.toDouble();
      break;

      case 48: ltScale = fabs(rdr.toDouble());
      break;

      case 60: inVisible = rdr.toInt();
      break;

      case 62: colNr = rdr.toInt();
      break;

      case 210: zDir.x = rdr.toDouble();
      break;

      case 220: zDir.y = rdr.toDouble();
      break;

      case 230: zDir.z = rdr.toDouble();
      break;

      case 102: skipGroup(rdr);
      break;
    }
  }

  if (inVisible) return; // Invisible

  if (vertexSz < 2) return;

  if (layer == NULL) layer = blk.getLayer();

  const DxfColor3D& col = getColor(colNr,layer);

  if (lt == NULL && !ltByBlock) {
    if (layer != NULL) lt = &layer->lt; //  By layer
    else lt = &DxfLineType3D::solid;
  }

  Trf3 trf; aaa(zDir,trf);
  trf(2,3) -= elevation; trf.invert();

  DxfAttr3D attr(blk,*layer,colNr,col,*lt,ltScale);

  int elCount = vertexSz;
  if (!closed) elCount--;

  void *usrArg = dxfRd.builder.startLwPoly(attr,trf,elCount,closed);

  Vec2 p1(vertexX[0],vertexY[0]);

  for (int i=1; i<vertexSz; ++i) {
    Vec2 p2(vertexX[i],vertexY[i]);

    lwPolySeg(usrArg,attr,p1,p2,vertexB[i-1]);

    p1 = p2;
  }

  if (closed) {
    Vec2 p2(vertexX[0],vertexY[0]);

    lwPolySeg(usrArg,attr,p1,p2,vertexB[vertexSz-1]);
  }

  dxfRd.builder.endLwPoly(usrArg);
}

//---------------------------------------------------------------------------

void BlockTable3D::readMLine(DxfReader3D& rdr, DxfBlock3D& /*blk*/)
{
  while (rdr.next()) {
    if (rdr.code == 0) break;
  }
}

//---------------------------------------------------------------------------


int BlockTable3D::getTxtRef(int horJust, int verJust)
{
  switch (horJust) {
    case 0:
    case 3:
    default:
      switch (verJust) {
        default: return 1;
        case 2:  return 4;
        case 3:  return 7;
      }

    case 1:
    case 4:
      switch (verJust) {
        default: return 2;
        case 2:  return 5;
        case 3:  return 8;
      }

    case 2:
    case 5:
      switch (verJust) {
        default: return 3;
        case 2:  return 6;
        case 3:  return 9;
      }
    }
}

// ------------------------------------------------------------------------

int BlockTable3D::findCode(int pos, int txtSz)
{
  int sz = sizeof(substCodeTab)/6;

  for (int i=0; i<sz; i++) {
    int codeLen = strlen(substCodeTab[i]);
    if (txtSz - pos < codeLen) continue;

    int j=0, idx = pos;

    for (j=0; j<codeLen; j++) {
      if (txtBuf[idx] != substCodeTab[i][j]) break;
      idx++;
    }

    if (j >= codeLen) return i;
  }

  return -1;
}

// ------------------------------------------------------------------------

int BlockTable3D::substCode(int pos, int txtSz)
{
  int idx = findCode(pos,txtSz);
  if (idx < 0) return substNumCode(pos,txtSz);

  int dst = pos;
  if (replCodeTab[idx] != 0) txtBuf[dst++] = replCodeTab[idx];

  pos += strlen(substCodeTab[idx]);

  while (pos < txtSz) {
    txtBuf[dst++] = txtBuf[pos++];
  }

  txtBuf[dst] = '\0';

  return dst;
}

// ------------------------------------------------------------------------

int BlockTable3D::substNumCode(int pos, int txtSz)
{
  if (pos >= txtSz-2) return txtSz;
  if (txtBuf[pos] != '%' || txtBuf[pos+1] != '%') return txtSz;

  int chVal = 0, src = pos;
  bool found = false;

  src += 2;

  while (src < txtSz && chVal < 256) {
    char c = txtBuf[src];
    if (c < '0' || c > '9') break;

    src++;
    found = true;
    chVal = chVal * 10 + c - '0';
  }

  if (!found) return txtSz;

  txtBuf[pos++] = (char)chVal;

  while (src < txtSz) txtBuf[pos++] = txtBuf[src++];

  txtBuf[pos] = '\0';

  return pos;
}

//---------------------------------------------------------------------------

int BlockTable3D::substCodes(int txtSz)
{
  for (int i=0; i<txtSz; i++) {
    if (txtBuf[i] != '%') continue; // Speed things up

    int newSz = substCode(i,txtSz);
    if (newSz != txtSz) i--;

    txtSz = newSz;
  }

  return txtSz;
}

//---------------------------------------------------------------------------

void BlockTable3D::readAttrib(DxfReader3D& rdr, DxfBlock3D& blk)
{
  Vec3 txtPt, txtPt2, zDir(0,0,1);
  double slant = 0.0, xScale = 1.0;
  bool slantSet = false, justSet = false, txtPt2Set = false;

  int inVisible=0, colNr = 256;
  DxfTxtStyle3D *st = NULL;
  DxfLayer3D *layer = NULL;
  char stName[256] = "";

  double txtHgt = 1.0;
  int horJust = 0, verJust = 0;
  int txtSz = 0; // Nr of chars
  int flags = 0;

  double txtDirAng = 0.0;
  bool dirSet = false;

  while (rdr.next()) {
    if (rdr.code == 0) break;

    switch (rdr.code) {
      case 10: txtPt.x = rdr.toDouble();
      break;

      case 20: txtPt.y = rdr.toDouble();
      break;

      case 30: txtPt.z = rdr.toDouble();
      break;

      case 11: txtPt2.x = rdr.toDouble();
               txtPt2Set = true;
      break;

      case 21: txtPt2.y = rdr.toDouble();
               txtPt2Set = true;
      break;

      case 31: txtPt2.z = rdr.toDouble();
               txtPt2Set = true;
      break;

      case 40: txtHgt = rdr.toDouble();
      break;

      case 41: xScale = fabs(rdr.toDouble());
               if (xScale < 0.001) xScale = 0.001;
               else if (xScale > 1000.0) xScale = 1000.0;
      break;

      case 50: txtDirAng = rdr.toDouble()*Vec2::Pi/180.0;
               dirSet = true;
      break;

      case 51: slant = rdr.toDouble();
               if (slant < -45.0) slant = 45.0;
               else if (slant > 45.0) slant = 45.0;
               slant /= 45.0;
               slantSet = true;
      break;

      case 70: flags = rdr.toInt();
               if ((flags & 1) != 0) { // Attrib is invisible
                 return;
               }
      break;

      case 72: horJust = rdr.toInt();
               justSet = true;
      break;

      case 74: verJust = rdr.toInt();
               justSet = true;
      break;

      case 1: strcpy(txtBuf,rdr.value);
              txtSz = rdr.valueSz;
      break;

      case 7: st = txtStyleTable.get(rdr.value);
              strcpy(stName,rdr.value);
      break;

      case  8: layer = layerTable.get(rdr.value);
      break;

      case 60: inVisible = rdr.toInt();
      break;

      case 62: colNr = rdr.toInt();
      break;

      case 210: zDir.x = rdr.toDouble();
      break;

      case 220: zDir.y = rdr.toDouble();
      break;

      case 230: zDir.z = rdr.toDouble();
      break;

      case 102: skipGroup(rdr);
      break;
    }
  }

  if (inVisible) return; // Invisible

  if (txtSz < 1) return;

  int txtRef = 1;
  if (justSet) {
    txtRef = getTxtRef(horJust,verJust);

    if (txtPt2Set) txtPt = txtPt2;
  }

  txtSz = substNewLine(txtSz);
  txtSz = substCodes(txtSz);

  if (st == NULL) st = txtStyleTable.getStdStyle();

  if (!slantSet) slant = st->slant;
  xScale *= st->widFac;

  if (layer == NULL) layer = blk.getLayer();

  // const DxfColor3D& col = getColor(colNr,layer);

  float txtLw = 0.25f;
  if (txtLw > 0.2 * txtHgt)
            txtLw = (float)(0.2 * txtHgt);

  // DxfAttr3D attr(blk,*layer,colNr,col,DxfLineType3D::solid,1.0);
}

//---------------------------------------------------------------------------

void BlockTable3D::readText(DxfReader3D& rdr, DxfBlock3D& blk)
{
  Vec3 txtPt,txtPt2,zDir(0,0,1);
  double slant = 0.0, xScale = 1.0;
  bool slantSet = false, justSet = false, txtPt2Set = false;

  int inVisible=0, colNr = 256;
  DxfTxtStyle3D *st = NULL;
  DxfLayer3D *layer = NULL;
  char stName[256] = "";

  double txtHgt = 1.0;
  int horJust = 0, verJust = 0;
  int txtSz = 0; // Nr of chars

  double txtDirAng = 0.0;
  bool dirSet = false;

  while (rdr.next()) {
    if (rdr.code == 0) break;

    switch (rdr.code) {
      case 10: txtPt.x = rdr.toDouble();
      break;

      case 20: txtPt.y = rdr.toDouble();
      break;

      case 30: txtPt.z = rdr.toDouble();
      break;

      case 11: txtPt2.x = rdr.toDouble();
               txtPt2Set = true;
      break;

      case 21: txtPt2.y = rdr.toDouble();
               txtPt2Set = true;
      break;

      case 31: txtPt2.z = rdr.toDouble();
               txtPt2Set = true;
      break;

      case 40: txtHgt = rdr.toDouble();
      break;

      case 41: xScale = fabs(rdr.toDouble());
               if (xScale < 0.001) xScale = 0.001;
               else if (xScale > 1000.0) xScale = 1000.0;
      break;

      case 50: txtDirAng = rdr.toDouble()*Vec2::Pi/180.0;
               dirSet = true;
      break;

      case 51: slant = rdr.toDouble();
               if (slant < -45.0) slant = 45.0;
               else if (slant > 45.0) slant = 45.0;
               slant /= 45.0;
               slantSet = true;
      break;

      case 72: horJust = rdr.toInt();
               justSet = true;
      break;

      case 73: verJust = rdr.toInt();
               justSet = true;
      break;

      case 1:  strcpy(txtBuf,rdr.value);
               txtSz = rdr.valueSz;
      break;

      case 7:  st = txtStyleTable.get(rdr.value);
               strcpy(stName,rdr.value);
      break;

      case  8: layer = layerTable.get(rdr.value);
      break;

      case 60: inVisible = rdr.toInt();
      break;

      case 62: colNr = rdr.toInt();
      break;

      case 210: zDir.x = rdr.toDouble();
      break;

      case 220: zDir.y = rdr.toDouble();
      break;

      case 230: zDir.z = rdr.toDouble();
      break;

      case 102: skipGroup(rdr);
      break;
    }
  }

  if (inVisible) return; // Invisible

  if (txtSz < 1) return;

  int txtRef = 1;
  if (justSet) {
    txtRef = getTxtRef(horJust,verJust);

    if (txtPt2Set) txtPt = txtPt2;
  }

  txtSz = substNewLine(txtSz);
  txtSz = substCodes(txtSz);

  if (st == NULL) st = txtStyleTable.getStdStyle();

  if (!slantSet) slant = st->slant;
  xScale *= st->widFac;

  if (layer == NULL) layer = blk.getLayer();

  // const DxfColor3D& col = getColor(colNr,layer);

  float txtLw = 0.25f;
  if (txtLw > 0.2 * txtHgt) txtLw = (float)(0.2 * txtHgt);

  // DxfAttr3D attr(blk,*layer,colNr,col,DxfLineType3D::solid,1.0);
}

//---------------------------------------------------------------------------

int BlockTable3D::getMTxtRef(int dxfRef)
{
  switch (dxfRef) {
    case 1: return 7;
    case 2: return 8;
    case 3: return 8;
    case 4: return 4;
    case 5: return 5;
    case 6: return 6;
    case 7: return 1;
    case 8: return 2;
    case 9: return 3;
    default: return 1;
  }
}

//---------------------------------------------------------------------------

int BlockTable3D::substNewLine(int txtSz)
{
  int src = 0, dst = 0;

  while (src < txtSz) {
    if (txtBuf[src] == '\\') {
      if (src+1 < txtSz && (txtBuf[src+1] == 'P' || txtBuf[src+1] == 'p')) {
        txtBuf[dst++] = '\n';
        src += 2;
        continue;
      }
    }

    txtBuf[dst++] = txtBuf[src++];
  }

  txtBuf[dst] = '\0';

  return dst;
}

//---------------------------------------------------------------------------

void BlockTable3D::readMText(DxfReader3D& rdr, DxfBlock3D& blk)
{
  Vec3 txtPt, txtDir(1,0,0), zDir(0,0,1);
  double txtDirAng = 0.0;
  int inVisible=0, colNr = 256;
  DxfTxtStyle3D *st = NULL;
  DxfLayer3D *layer = NULL;
  char stName[256] = "";

  double txtHgt = 1.0, txtBlkWid = 0.0, txtBlkHgt = 0.0;
  bool blkWidSet = false, blkHgtSet = false;

  double lineSpacing = 1.0;
  int txtRef = 1;
  int txtSz = 0; // Nr of chars

  bool dirSet = false;

  while (rdr.next()) {
    if (rdr.code == 0) break;

    switch (rdr.code) {
      case 10: txtPt.x = rdr.toDouble();
      break;

      case 20: txtPt.y = rdr.toDouble();
      break;

      case 30: txtPt.z = rdr.toDouble();
      break;

      case 40: txtHgt = rdr.toDouble();
      break;

      case 42: txtBlkWid = rdr.toDouble();
               blkWidSet = true;
      break;

      case 43: txtBlkHgt = rdr.toDouble();
               blkHgtSet = true;
      break;

      case 44: lineSpacing = rdr.toDouble();
               if (lineSpacing < 0.25) lineSpacing = 0.25;
               else if (lineSpacing > 4.0) lineSpacing = 4.0;
      break;

      case 71: txtRef = getMTxtRef(rdr.toInt());
      break;

      case 1:
      case 3: strcpy(txtBuf,rdr.value);
              txtSz = rdr.valueSz;
      break;

      case 7: st = txtStyleTable.get(rdr.value);
              strcpy(stName,rdr.value);
      break;

      case 11: txtDir.x = rdr.toDouble();
               dirSet = true;
      break;

      case 21: txtDir.y = rdr.toDouble();
               dirSet = true;
      break;

      case 31: txtDir.z = rdr.toDouble();
               dirSet = true;
      break;

      case 50: txtDirAng = rdr.toDouble()*Vec2::Pi/180.0;
      break;

      case  8: layer = layerTable.get(rdr.value);
      break;

      case 60: inVisible = rdr.toInt();
      break;

      case 62: colNr = rdr.toInt();
      break;

      case 210: zDir.x = rdr.toDouble();
      break;

      case 220: zDir.y = rdr.toDouble();
      break;

      case 230: zDir.z = rdr.toDouble();
      break;

      case 102: skipGroup(rdr);
      break;
    }
  }

  if (inVisible) return; // Invisible

  if (txtSz < 1) return;

  txtSz = substNewLine(txtSz);
  txtSz = substCodes(txtSz);

  if (st == NULL) st = txtStyleTable.getStdStyle();

  if (!dirSet) txtDir = Vec3(cos(txtDirAng),sin(txtDirAng),0.0);

  if (layer == NULL) layer = blk.getLayer();

  // const DxfColor3D& col = getColor(colNr,layer);

  float txtLw = 0.25f;
  if (txtLw > 0.2 * txtHgt)
                      txtLw = (float)(0.2 * txtHgt);

  // double widRel = 0.6  * st->widFac;
  // double spRel  = 0.71 * st->widFac;
  // double hgtRel = 1.25*lineSpacing;

  // DxfAttr3D attr(blk,*layer,colNr,col,DxfLineType3D::solid,1.0);
}

//---------------------------------------------------------------------------

void BlockTable3D::readRText(DxfReader3D& rdr, DxfBlock3D& /*blk*/)
{
  while (rdr.next()) {
    if (rdr.code == 0) break;
  }
}

//---------------------------------------------------------------------------

void BlockTable3D::addVertex(double x, double y, double z, double bulge)
{
  if (vertexSz >= vertexCap) {
    vertexCap += 1024;

    double *newV = new double[vertexCap];
    memmove(newV,vertexX,vertexSz*sizeof(double));
    delete[] vertexX;
    vertexX = newV;

    newV = new double[vertexCap];
    memmove(newV,vertexY,vertexSz*sizeof(double));
    delete[] vertexY;
    vertexY = newV;

    newV = new double[vertexCap];
    memmove(newV,vertexZ,vertexSz*sizeof(double));
    delete[] vertexZ;
    vertexZ = newV;

    newV = new double[vertexCap];
    memmove(newV,vertexB,vertexSz*sizeof(double));
    delete[] vertexB;
    vertexB = newV;
  }

  vertexX[vertexSz] = x;
  vertexY[vertexSz] = y;
  vertexZ[vertexSz] = z;
  vertexB[vertexSz] = bulge;

  vertexSz++;
}

//---------------------------------------------------------------------------

void BlockTable3D::skipToSeqEnd(DxfReader3D& rdr)
{
  while (rdr.next()) {
    if (rdr.code != 0) continue;

    int id = nameTable.getNameId(rdr.value);
    if (id == DxfNameTable3D::ID_SEQEND) {
      while (rdr.next()) {
        if (rdr.code == 0) return;
      }
    }
  }
}

//---------------------------------------------------------------------------

void BlockTable3D::readVertex(DxfReader3D& rdr)
{
  double x = 0.0, y = 0.0, z = 0.0, bulge = 0.0;

  while (rdr.next()) {
    if (rdr.code == 0) {
      addVertex(x,y,z,bulge);
      return;
    }

    switch (rdr.code) {
      case 10: x = rdr.toDouble();
      break;

      case 20: y = rdr.toDouble();
      break;

      case 30: z = rdr.toDouble();
      break;

      case 42: bulge = rdr.toDouble();
      break;
    }
  }
}

//---------------------------------------------------------------------------

void BlockTable3D::readVertices(DxfReader3D& rdr)
{
  while (!rdr.eof) {
    if (rdr.code != 0) continue;

    int id = nameTable.getNameId(rdr.value);

    switch (id) {
    case DxfNameTable3D::ID_VERTEX: readVertex(rdr);
      break;

    case DxfNameTable3D::ID_SEQEND:
        while (rdr.next()) {
          if (rdr.code == 0) return;
        }
      break;

      default: return;
    }
  }
}

//---------------------------------------------------------------------------

void BlockTable3D::polySeg(void *usrArg, DxfAttr3D& attr,
                              const Vec3& p1, const Vec3& p2, double bulge)
{
  if (fabs(bulge) < 0.001) dxfRd.builder.addPolyLine(usrArg,attr,p1,p2);
  else {
    Vec2 dir(p2); dir -= p1;

    dir.rot270(); dir *= bulge/2.0;

    Vec2 c(p1); c += p2; c /= 2.0; c += dir;

    dir = c; dir -= p1;
    
    dir.rot90(); dir /= bulge*2.0;

    c += p1; c /= 2.0; c += dir;

    double r = c.distTo2(p1);

    Vec2 dp1(p1); dp1 -= c;
    double startAngle = dp1.angle();
    if (startAngle < 0.0) startAngle += Vec2::Pi2;

    double sweepAngle = 4.0 * atan(bulge);

    dxfRd.builder.addPolyArc(usrArg,attr,c,r,startAngle,sweepAngle);
  }
}

//---------------------------------------------------------------------------

void BlockTable3D::readPolyLine(DxfReader3D& rdr, DxfBlock3D& blk)
{
  Vec3 zDir(0,0,1);

  bool closed = false, is3D = false;
  int inVisible=0, colNr = 256;
  const DxfLineType3D *lt = NULL;
  DxfLayer3D *layer = NULL;
  double ltScale = 1.0, elevation = 0.0;
  bool ltByBlock = false;

  while (rdr.next()) {
    if (rdr.code == 0) break;

    switch (rdr.code) {
      case 70: {
        int flags = rdr.toInt();
        closed = (flags & 1) != 0;
        is3D   = (flags & 8) != 0;
        if ((flags & (16 | 64)) != 0) { // Polygon mesh (mess), so skip
          skipToSeqEnd(rdr);
          return;
        }
      }
      break;

      case 6: if (isByBlock(rdr)) ltByBlock = true;
              else lt = ltTable.get(rdr.value);
      break;

      case 8: layer = layerTable.get(rdr.value);
      break;

      case 30: elevation = rdr.toDouble();
      break;

      case 48: ltScale = fabs(rdr.toDouble());
      break;

      case 60: inVisible = rdr.toInt();
      break;

      case 62: colNr = rdr.toInt();
      break;

      case 210: zDir.x = rdr.toDouble();
      break;

      case 220: zDir.y = rdr.toDouble();
      break;

      case 230: zDir.z = rdr.toDouble();
      break;

      case 102: skipGroup(rdr);
      break;
    }
  }

  vertexSz = 0;
  readVertices(rdr);

  if (!is3D) {
    for (int i=0; i<vertexSz; ++i) vertexZ[i] = 0.0; // To make sure
  }

  if (rdr.eof || vertexSz < 2) return;

  if (inVisible) return; // Invisible

  if (layer == NULL) layer = blk.getLayer();

  const DxfColor3D& col = getColor(colNr,layer);

  if (lt == NULL && !ltByBlock) {
    if (layer != NULL) lt = &layer->lt; //  By layer
    else lt = &DxfLineType3D::solid;
  }

  Trf3 trf;

  if (is3D) trf(2,3) = elevation;
  else {
    aaa(zDir,trf);
    trf(2,3) -= elevation;
    trf.invert();
  }

  DxfAttr3D attr(blk,*layer,colNr,col,*lt,ltScale);

  int elCount = vertexSz;
  if (!closed) elCount--;

  void *usrArg = dxfRd.builder.startPoly(attr,trf,elCount,closed);

  Vec3 p1(vertexX[0],vertexY[0],vertexZ[0]);

  for (int i=1; i<vertexSz; ++i) {
    Vec3 p2(vertexX[i],vertexY[i],vertexZ[i]);

    polySeg(usrArg,attr,p1,p2,vertexB[i-1]);

    p1 = p2;
  }

  if (closed) {
    Vec3 p2(vertexX[0],vertexY[0],vertexZ[0]);

    polySeg(usrArg,attr,p1,p2,vertexB[vertexSz-1]);
  }

  dxfRd.builder.endPoly(usrArg);
}

//---------------------------------------------------------------------------


void BlockTable3D::readSpline(DxfReader3D& rdr, DxfBlock3D& /*blk*/)
{
  while (rdr.next()) {
    if (rdr.code == 0) break;
  }
}

//---------------------------------------------------------------------------

void BlockTable3D::readTrace(DxfReader3D& rdr, DxfBlock3D& /*blk*/)
{
  while (rdr.next()) {
    if (rdr.code == 0) break;
  }
}

//---------------------------------------------------------------------------

void BlockTable3D::readDimension(DxfReader3D& rdr, DxfBlock3D& blk)
{
  int inVisible=0, colNr = 256;
  const DxfLineType3D *lt = NULL;
  DxfLayer3D *layer = NULL;
  double ltScale = 1.0;
  bool ltByBlock = false;

  double insX = 0.0, insY = 0.0;

  DxfBlock3D *dimBlk = NULL;

  while (rdr.next()) {
    if (rdr.code == 0) break;

    switch (rdr.code) {
     // Common codes
      case 6: if (isByBlock(rdr)) ltByBlock = true;
              else lt = ltTable.get(rdr.value);
      break;

      case 8: layer = layerTable.get(rdr.value);
      break;

      case 48: ltScale = fabs(rdr.toDouble());
      break;

      case 60: inVisible = rdr.toInt();
      break;

      case 62: colNr = rdr.toInt();
      break;

      case 102: skipGroup(rdr);
      break;

      // Dimension Specific Codes:

      case 2: dimBlk = get(rdr.value);
      break;

      case 12: insX = rdr.toDouble();
      break;

      case 22: insY = rdr.toDouble();
      break;
    }
  }

  if (inVisible) return; // Invisible

  if (dimBlk == NULL) return;

  if (layer == NULL) layer = blk.getLayer();

  // const DxfColor3D& col = getColor(colNr,layer);

  if (lt == NULL && !ltByBlock) {
    if (layer != NULL) lt = &layer->lt; //  By layer
    else lt = &DxfLineType3D::solid;
  }

  // DxfAttr3D attr(blk,*layer,colNr,col,*lt,ltScale);
}

//---------------------------------------------------------------------------

void BlockTable3D::readSolid(DxfReader3D& rdr, DxfBlock3D& blk)
{
  double x1 = 0, y1 = 0, x2 = 0, y2 = 0, x3 = 0, y3 = 0, x4 = 0, y4 = 0;

  int inVisible=0, colNr = 256;
  const DxfLineType3D *lt = NULL;
  DxfLayer3D *layer = NULL;
  double ltScale = 1.0;
  bool ltByBlock = false;

  int pntCntX = 0, pntCntY = 0;

  while (rdr.next()) {
    if (rdr.code == 0) break;

    switch (rdr.code) {
      case 10: x1 = rdr.toDouble();
               pntCntX++;
      break;

      case 11: x2 = rdr.toDouble();
               pntCntX++;
      break;

      case 12: x3 = rdr.toDouble();
               pntCntX++;
      break;

      case 13: x4 = rdr.toDouble();
               pntCntX++;
      break;

      case 20: y1 = rdr.toDouble();
               pntCntY++;
      break;

      case 21: y2 = rdr.toDouble();
               pntCntY++;
      break;

      case 22: y3 = rdr.toDouble();
               pntCntY++;
      break;

      case 23: y4 = rdr.toDouble();
               pntCntY++;
      break;

      case 6: if (isByBlock(rdr)) ltByBlock = true;
              else lt = ltTable.get(rdr.value);
      break;

      case 8: layer = layerTable.get(rdr.value);
      break;

      case 48: ltScale = fabs(rdr.toDouble());
      break;

      case 60: inVisible = rdr.toInt();
      break;

      case 62: colNr = rdr.toInt();
      break;

      case 102: skipGroup(rdr);
      break;
    }
  }

  if (inVisible) return; // Invisible

  if (pntCntY < pntCntX) pntCntX = pntCntY;
  if (pntCntX < 3) return;

  if (pntCntX > 3) {
    // Must sort the points (often cris-cross sorted)

    double dx2 = x2 - x1, dy2 = y2 - y1;
    double dx3 = x3 - x1, dy3 = y3 - y1;
    double dx4 = x4 - x1, dy4 = y4 - y1;

    double inpr3 =  dy3 * dx2 - dx3 * dy2;
    double inpr4 =  dy4 * dx2 - dx4 * dy2;

    if (inpr3 * inpr4 < 0.0) { // Lines are crossing: swap 2 and 3
      double h = x2; x2 = x3; x3 = h;
             h = y2; y2 = y3; y3 = h;

             h = dx2; dx2 = dx3; dx3 = h;
             h = dy2; dy2 = dy3; dy3 = h;
    }

    inpr3 = dx3 * dx2 + dy3 * dy2;
    inpr4 = dx4 * dx2 + dy4 * dy2;

    if (inpr3 < inpr4) { // Swap 3 and 4
      double h = x3; x3 = x4; x4 = h;
             h = y3; y3 = y4; y4 = h;
    }
  }

  if (layer == NULL) layer = blk.getLayer();

  // const DxfColor3D& col = getColor(colNr,layer);

  if (lt == NULL && !ltByBlock) {
    if (layer != NULL) lt = &layer->lt; //  By layer
    else lt = &DxfLineType3D::solid;
  }

  // DxfAttr3D attr(blk,*layer,colNr,col,*lt,ltScale);
}

//---------------------------------------------------------------------------

enum { HATCH_STYLE_NORMAL = 0, HATCH_STYLE_OUTER  = 1, HATCH_STYLE_IGNORE = 2 };

void BlockTable3D::readHatch(DxfReader3D& rdr, DxfBlock3D& blk)
{
  int inVisible=0, colNr = 256;
  DxfLayer3D *layer = NULL;

  char patternName[256] = "";
  bool solidFill = false;

  int hatchStyle = HATCH_STYLE_NORMAL;

  double hatchPatAngle = 0.0;
  double hatchPatScale = 1.0;
  bool hatchPatDouble = false;

  DxfPattern3D *patLst = NULL;
  int patCap = 0;

  if (!rdr.next()) return;

  for (;;) {
    if (rdr.code == 0) break;

    switch (rdr.code) {
      // Common codes
      case 8: layer = layerTable.get(rdr.value);
      break;

      case 60: inVisible = rdr.toInt();
      break;

      case 62: colNr = rdr.toInt();
      break;

      case 102: skipGroup(rdr);
      break;

      // Hatch Specific Codes:

      case 2: strcpy(patternName,rdr.value);
      break;

      case 41: hatchPatScale = rdr.toDouble();
      break;

      case 52: hatchPatAngle = rdr.toDouble()*Vec2::Pi/180.0;
      break;

      case 70: solidFill = rdr.toInt() != 0;
      break;

#ifdef NEVER
      case 91: boundary = readBoundary(rdr.toInt(),rdr);
               if (boundary == NULL) { // Too bad
                 while (rdr.code != 0 && rdr.next());
                 if (patLst) delete[] patLst;
                 return;
               }

      continue;
#else
      case 91: while (rdr.code != 0 && rdr.next()) {}
               if (patLst) delete[] patLst;
               return;
#endif

      case 75 : hatchStyle = rdr.toInt();
                if (hatchStyle < HATCH_STYLE_NORMAL)
                                            hatchStyle = HATCH_STYLE_NORMAL;
                if (hatchStyle > HATCH_STYLE_IGNORE)
                                            hatchStyle = HATCH_STYLE_IGNORE;
      break;

      case 77: hatchPatDouble = rdr.toInt() != 0;
      break;

      case 78: {
        int patCnt = rdr.toInt();
        if (patCnt > 0) {
          if (!rdr.next()) return;

          patCap = patCnt;
          patLst = new DxfPattern3D[patCnt];

          for (int i=0; i<patCnt; i++) {
            if (!patLst[i].read(rdr)) {
              delete[] patLst;
              patLst = NULL;
              patCap = 0;
              break;
            }
          }

          if (patLst == NULL) { // Too bad
            while (rdr.code != 0 && rdr.next()) {}
            return;
          }

          continue;
        }
      }
      break;
    }

    if (!rdr.next()) return;
  }

  if (inVisible) return; // Invisible

  if (layer == NULL) layer = blk.getLayer();

  // const DxfColor3D& col = getColor(colNr,layer);

  // DxfAttr3D attr(blk,*layer,colNr,col,DxfLineType3D::solid,1.0);

  // Dont forget to deallocate the Pattern list.
}

//---------------------------------------------------------------------------

BlockTable3D::BlockTable3D(DxfRead3D& dxf, DxfNameTable3D& nameTbl, LineTypeTable3D& ltTbl,
                       TextStyleTable3D& txtStyleTbl, LayerTable3D& layerTbl)
: ObjNameTable3D<DxfBlock3D>(100,100), dxfRd(dxf),
  colTable(), nameTable(nameTbl), ltTable(ltTbl),
  txtStyleTable(txtStyleTbl), layerTable(layerTbl),
  txtBuf(new char[4096]),
  vertexX(new double[1024]),
  vertexY(new double[1024]),
  vertexZ(new double[1024]),
  vertexB(new double[1024]),
  vertexSz(0), vertexCap(1024)
{
  txtBuf[0] = '\0';
}

//---------------------------------------------------------------------------

BlockTable3D::~BlockTable3D()
{
  if (txtBuf) delete[] txtBuf;
  if (vertexX) delete[] vertexX;
  if (vertexY) delete[] vertexY;
  if (vertexB) delete[] vertexB;
}

//---------------------------------------------------------------------------

void BlockTable3D::readBlock(DxfReader3D& rdr)
{
  DxfBlock3D *blk = readBlkHeader(rdr);
  if (!blk) return;

  readEntities(rdr,*blk);

  while (!rdr.eof && rdr.code != 0) rdr.next();

  if (!rdr.eof) {
    int id = nameTable.getNameId(rdr.value);
    if (id == DxfNameTable3D::ID_ENDBLK) readBlkEnd(rdr,*blk);
  }
}

} // namespace Ino

//---------------------------------------------------------------------------
