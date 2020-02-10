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

#ifndef DXFREAD3D_INC
#define DXFREAD3D_INC

#include "Basics.h"
#include "Trf.h"
#include "Vec.h"

namespace Ino
{

//---------------------------------------------------------------------------
//
// How to use this library:
//
// 1. Derive a class from ASCIIReader3D and implement the virtual method.
// 2. Derive a class from DxfBuilder3D and implement the virtual methods.
// 3. Create an instance of DxfRead3D (pass your ASCIIReader3D and DxfBuilder3D)
// 4. Call method DxfRead3D::readDxf.
//    Your implementation of the methods in DxfBuilder3D will be called as
//    appropiate. Use fields "user1" and "user2" in DxfBlock3D to make a link
//    to your own datastructures.

//---------------------------------------------------------------------------

class ASCIIReader3D
{
public:
  virtual int read(char *buf, int bufSz) = 0;

  virtual bool progress(unsigned int /* bytesRead */,
                        unsigned int /*linesRead*/) { return true; }
};

//---------------------------------------------------------------------------

class DxfBlock3D; // See below
class DxfLayer3D;
class DxfAttr3D;
class DxfRead3D;

class DxfBuilder3D
{
  DxfRead3D *dxfRd;

public:
  DxfBuilder3D() : dxfRd(NULL) {}

  DxfRead3D& getDxfRead() { return *dxfRd; }

  virtual void addLayer(const DxfLayer3D& /*newLayer*/) {}

  virtual void addBlock(DxfBlock3D& newBlk) = 0;

  virtual void addLine(DxfAttr3D& /*attr*/,
                       const Vec3& /*p1*/, const Vec3& /*p2*/) {}

  virtual void addArc(DxfAttr3D& /*attr*/, Trf3& /*trf*/,
                      const Vec2& /*c*/, double /*r*/,
                      double /*startAng*/, double /*endAng*/) {}

  virtual void addCircle(DxfAttr3D& /*attr*/, Trf3& /*trf*/,
                         const Vec2& /*c*/, double /*r*/) {}

  virtual void *startLwPoly(DxfAttr3D& /*attr*/, const Trf3& /*trf*/,
                            int /*elemCount*/, bool /*closed*/) { return NULL; }

  virtual void addLwPolyLine(void * /*usrArg*/, DxfAttr3D& /*attr*/,
                             const Vec2& /*p1*/, const Vec2& /*p2*/) {}

  virtual void addLwPolyArc(void * /*usrArg*/, DxfAttr3D& /*attr*/,
                            const Vec2& /*c*/, double& /*r*/,
                            double /*startAngle*/, double /*sweepAngle*/) {}

  virtual void endLwPoly(void * /*usrArg*/) {}

  virtual void *startPoly(DxfAttr3D& /*attr*/, const Trf3& /*trf*/,
                          int /*elemCount*/, bool /*closed*/) { return NULL; }

  virtual void addPolyLine(void * /*usrArg*/, DxfAttr3D& /*attr*/,
                           const Vec3& /*p1*/, const Vec3& /*p2*/) {}

  virtual void addPolyArc(void * /*usrArg*/, DxfAttr3D& /*attr*/,
                          const Vec2& /*c*/, double /*r*/,
                          double /*startAngle*/, double /*sweepAngle*/) {}

  virtual void endPoly(void * /*usrArg*/) {}

  virtual void insertBlock(DxfAttr3D& /*attr*/, DxfBlock3D& /*insBlk*/,
                           const Trf3& /*trf*/,
                           double /*colSpacing*/, int /*colCount*/,
                           double /*rowSpacing*/, int /*rowCount*/) {}

  friend class DxfRead3D;
};

//---------------------------------------------------------------------------

class DxfReader3D;
class DxfNameTable3D;
class HeaderTable3D;
class LineTypeTable3D;
class TextStyleTable3D;
class LayerTable3D;
class BlockTable3D;
class DxfBlock3D;

class DxfAttr3D;

class DxfRead3D
{
public:
  enum Status { Success, Aborted, NotDxf, FileFormatError, NumberFormatError,
                PrematureEnd, LineTooLong, FileNotFound };

  DxfBuilder3D& builder;

private:
  DxfReader3D   *rdr;

  DxfNameTable3D   *nameTable;
  HeaderTable3D    *hdrTable;
  LineTypeTable3D  *ltTable;
  TextStyleTable3D *txtTable;
  LayerTable3D     *layerTable;
  BlockTable3D     *blockTable;

  void readTables(DxfReader3D& rdr);
  void skipTable(DxfReader3D& rdr);
  void readBlocks(DxfReader3D& rdr);
  DxfBlock3D *readEntities(DxfReader3D& rdr);
  void skipSection(DxfReader3D& rdr);
  void skipGroup(DxfReader3D& rdr);

  DxfRead3D(const DxfRead3D& cp);             // No Copying
  DxfRead3D& operator=(const DxfRead3D& src); // No Assignment

public:
  DxfRead3D(ASCIIReader3D& dxfRdr, DxfBuilder3D& dxfBuilder);
  ~DxfRead3D();

  Status readDxf();

  Status getStatus() const;

  void getCounts(int& lineCount, int& charCount) const;
};

//---------------------------------------------------------------------------

class DxfColor3D
{
  DxfColor3D& operator=(const DxfColor3D& src); // No Assignment

public:
  static DxfColor3D red;
  static DxfColor3D yellow;
  static DxfColor3D green;
  static DxfColor3D cyan;
  static DxfColor3D blue;
  static DxfColor3D magenta;
  static DxfColor3D white;
  static DxfColor3D gray;
  static DxfColor3D lightGray;

  DxfColor3D(unsigned char cr, unsigned char cg, unsigned char cb) : r(cr), g(cg), b(cb) {}

  const unsigned char r;
  const unsigned char g;
  const unsigned char b;
};

//---------------------------------------------------------------------------

class DxfLineType3D
{
  double *dashList;
  unsigned int dashSz;

  char *name;

  DxfLineType3D& operator=(const DxfLineType3D& src); // No Assigment

public:
  DxfLineType3D(const char *ltName, double *dashes, unsigned int dashLen);
  DxfLineType3D(const DxfLineType3D& cp);
  ~DxfLineType3D();

  const char *getName() const { return name; }

  unsigned int dashCount() const { return dashSz; }

  double operator[](unsigned int idx) const;

  static DxfLineType3D solid;
};

//---------------------------------------------------------------------------

class DxfLayer3D
{
  DxfLayer3D& operator=(const DxfLayer3D& src); // No Assignent

public:
  DxfLayer3D(const char *layerName, int colorNr, DxfLineType3D &ltp, short layFlags = 0);
  DxfLayer3D(const DxfLayer3D& cp);
  ~DxfLayer3D();


  const char *const name;
  const int colNr;
  const DxfLineType3D lt;
  const double lw;
  const short flags;

  mutable int userIndex;

  bool isFrozen() const { return (flags & 1) != 0; }
};

//---------------------------------------------------------------------------

class DxfBlock3D
{
public:
    void *user1, *user2; // For use by user.
    int userIndex;

private:
  char *name;
  DxfLayer3D *layer;

  Vec3 base;

  bool root;

  DxfBlock3D(const DxfBlock3D& cp);             // No Copying
  DxfBlock3D& operator=(const DxfBlock3D& src); // No Assignment

public:
  DxfBlock3D();
  DxfBlock3D(const char *blkName, DxfLayer3D *lay, const Vec3& basePt);
  ~DxfBlock3D();

  bool isRoot() const { return root; }

  const char *getName() const { return name; }

  DxfLayer3D *getLayer() const { return layer; }

  const Vec3& getBasePt() const { return base; }
};

//---------------------------------------------------------------------------

class DxfTxtStyle3D
{
  DxfTxtStyle3D& operator=(const DxfTxtStyle3D& src); // No Assignment

public:
  DxfTxtStyle3D(double wid, double sl);

  const double widFac;
  const double slant;
};

//---------------------------------------------------------------------------

class DxfAttr3D
{
  DxfAttr3D& operator=(const DxfAttr3D& src); // No Assignment

public:
  DxfAttr3D(DxfBlock3D& blk, const DxfLayer3D& lay, int colNr,
                  const DxfColor3D& col, const DxfLineType3D& lt, double ltScale);

  DxfBlock3D& block;
  const DxfLayer3D& layer;

  int colorNr;
  const bool colByBlock;
  const DxfColor3D& color;
  const DxfLineType3D& lineType;
  double lineTypeScale;
};

//---------------------------------------------------------------------------

class DxfPattern3D
{
  double angle;
  Vec2 basePt;
  Vec2 offset;

  double *dashes;
  unsigned int dashSz;

  bool read(DxfReader3D& rdr);

  DxfPattern3D(const DxfPattern3D& cp);             // No Copying
  DxfPattern3D& operator=(const DxfPattern3D& src); // No Assignment

public:
  DxfPattern3D();
  ~DxfPattern3D();

  double getAngle() const { return angle; }
  const Vec2& getBasePt() const { return basePt; }
  const Vec2& getOffset() const { return offset; }

  unsigned int getDashCount() const { return dashSz; }
  double getDash(unsigned int idx) const;

  friend class BlockTable3D;
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
