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

#ifndef DXFREAD_INC
#define DXFREAD_INC

#include "Basics.h"
#include "Trf.h"
#include "Vec.h"

namespace Ino
{

//---------------------------------------------------------------------------
//
// How to use this library:
//
// 1. Derive a class from ASCIIReader and implement the virtual method.
// 2. Derive a class from DxfBuilder and implement the virtual methods.
// 3. Create an instance of DxfRead (pass your ASCIIReader and DxfBuilder)
// 4. Call method DxfRead::readDxf.
//    Your implementation of the methods in DxfBuilder will be called as
//    appropiate. Use fields "user1" and "user2" in DxfBlock to make a link
//    to your own datastructures.

//---------------------------------------------------------------------------

class ASCIIReader
{
public:
  virtual int read(char *buf, int bufSz) = 0;

  virtual bool progress(unsigned int /* bytesRead */,
                        unsigned int /*linesRead*/) { return true; }
};

//---------------------------------------------------------------------------

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4100 ) // parameter not referenced warning
#endif

class DxfBlock; // See below
class DxfAttr;
class DxfRead;

class DxfBuilder
{
  DxfRead *dxfRd;

public:
  DxfBuilder() : dxfRd(NULL) {}

  DxfRead& getDxfRead() { return *dxfRd; }

  virtual void addBlock(DxfBlock& newBlk) = 0;

  virtual void addLine(DxfAttr& /*attr*/,
                       const Vec2& /*p1*/, const Vec2& /*p2*/) {}

  virtual void addArc(DxfAttr& /*attr*/, const Vec2& /*c*/, double /*r*/,
                      double /*startAng*/, double /*endAng*/) {}

  virtual void addCircle(DxfAttr& /*attr*/, const Vec2& /*c*/, double /*r*/) {}

  virtual void *startLwPoly(DxfAttr& /*attr*/,
                            int /*elemCount*/, bool /*closed*/) { return NULL; }

  virtual void addLwPolyLine(void * /*usrArg*/, DxfAttr& /*attr*/,
                             const Vec2& /*p1*/, const Vec2& /*p2*/) {}

  virtual void addLwPolyArc(void * /*usrArg*/, DxfAttr& /*attr*/,
                            const Vec2& /*c*/, double& /*r*/,
                            double& /*startAngle*/, double& /*sweepAngle*/) {}

  virtual void endLwPoly(void * /*usrArg*/) {}

  virtual void *startPoly(DxfAttr& /*attr*/, int /*elemCount*/,
                          bool /*closed*/) { return NULL; }

  virtual void addPolyLine(void * /*usrArg*/, DxfAttr& /*attr*/,
                             const Vec2& /*p1*/, const Vec2& /*p2*/) {}

  virtual void addPolyArc(void * /*usrArg*/, DxfAttr& /*attr*/,
                            const Vec2& /*c*/, double /*r*/,
                            double& /*startAngle*/, double& /*sweepAngle*/) {}

  virtual void endPoly(void * /*usrArg*/) {}

  virtual void insertBlock(DxfAttr& /*attr*/, const Trf2& /*trf*/,
                           double /*colSpacing*/, int /*colCount*/,
                           double /*rowSpacing*/, int /*rowCount*/) {}
  friend class DxfRead;
};

#ifdef WIN32
#pragma warning( pop ) // parameter not referenced warning
#endif

//---------------------------------------------------------------------------

class DxfReader;
class DxfNameTable;
class HeaderTable;
class LineTypeTable;
class TextStyleTable;
class LayerTable;
class BlockTable;
class DxfBlock;

class DxfAttr;

class DxfRead
{
public:
  enum Status { Success, Aborted, NotDxf, FileFormatError, NumberFormatError,
                PrematureEnd, LineTooLong, FileNotFound };

  DxfBuilder& builder;

private:
  DxfReader   *rdr;

  DxfNameTable   *nameTable;
  HeaderTable    *hdrTable;
  LineTypeTable  *ltTable;
  TextStyleTable *txtTable;
  LayerTable     *layerTable;
  BlockTable     *blockTable;

  void readTables(DxfReader& rdr);
  void skipTable(DxfReader& rdr);
  void readBlocks(DxfReader& rdr);
  DxfBlock *readEntities(DxfReader& rdr);
  void skipSection(DxfReader& rdr);
  void skipGroup(DxfReader& rdr);

  DxfRead(const DxfRead& cp);             // No Copying
  DxfRead& operator=(const DxfRead& src); // No Assignment

public:
  DxfRead(ASCIIReader& dxfRdr, DxfBuilder& dxfBuilder);
  ~DxfRead();

  Status readDxf();

  Status getStatus() const;

  void getCounts(int& lineCount, int& charCount) const;
};

//---------------------------------------------------------------------------

class DxfColor
{
  DxfColor& operator=(const DxfColor& src); // No Assignment

public:
  static DxfColor red;
  static DxfColor yellow;
  static DxfColor green;
  static DxfColor cyan;
  static DxfColor blue;
  static DxfColor magenta;
  static DxfColor white;
  static DxfColor gray;
  static DxfColor lightGray;

  DxfColor(unsigned char cr, unsigned char cg, unsigned char cb) : r(cr), g(cg), b(cb) {}

  const unsigned char r;
  const unsigned char g;
  const unsigned char b;
};

//---------------------------------------------------------------------------

class DxfLineType
{
  double *dashList;
  unsigned int dashSz;

  char *name;

  DxfLineType& operator=(const DxfLineType& src); // No Assigment

public:
  DxfLineType(const char *ltName, double *dashes, unsigned int dashLen);
  DxfLineType(const DxfLineType& cp);
  ~DxfLineType();

  const char *getName() const { return name; }

  unsigned int dashCount() const { return dashSz; }

  double operator[](unsigned int idx) const;

  static DxfLineType solid;
};

//---------------------------------------------------------------------------

class DxfLayer
{
  DxfLayer& operator=(const DxfLayer& src); // No Assignent

public:
  DxfLayer(const char *layerName, int colorNr, DxfLineType &ltp);
  DxfLayer(const DxfLayer& cp);
  ~DxfLayer();


  const char *const name;
  const int colNr;
  const DxfLineType lt;
  const double lw;
};

//---------------------------------------------------------------------------

class DxfBlock
{
public:
    void *user1, *user2; // For use by user.

private:
  char *name;
  DxfLayer *layer;

  double baseX, baseY;

  bool root;

  DxfBlock(const DxfBlock& cp);             // No Copying
  DxfBlock& operator=(const DxfBlock& src); // No Assignment

public:
  DxfBlock();
  DxfBlock(const char *blkName, DxfLayer *lay, double bx, double by);
  ~DxfBlock();

  bool isRoot() const { return root; }

  const char *getName() const { return name; }

  DxfLayer *getLayer() const { return layer; }

  double getBaseX() const { return baseX; }
  double getBaseY() const { return baseY; }
};

//---------------------------------------------------------------------------

class DxfTxtStyle
{
  DxfTxtStyle& operator=(const DxfTxtStyle& src); // No Assignment

public:
  DxfTxtStyle(double wid, double sl);

  const double widFac;
  const double slant;
};

//---------------------------------------------------------------------------

class DxfAttr
{
  DxfAttr& operator=(const DxfAttr& src); // No Assignment

public:
  DxfAttr(DxfBlock& blk, const DxfLayer& lay, int colNr,
                  const DxfColor& col, const DxfLineType& lt, double ltScale);

  DxfBlock& block;
  const DxfLayer& layer;

  int colorNr;
  const DxfColor& color;
  const DxfLineType& lineType;
  double lineTypeScale;
};

//---------------------------------------------------------------------------

class DxfPattern
{
  double angle;
  Vec2 basePt;
  Vec2 offset;

  double *dashes;
  unsigned int dashSz;

  bool read(DxfReader& rdr);

  DxfPattern(const DxfPattern& cp);             // No Copying
  DxfPattern& operator=(const DxfPattern& src); // No Assignment

public:
  DxfPattern();
  ~DxfPattern();

  double getAngle() const { return angle; }
  const Vec2& getBasePt() const { return basePt; }
  const Vec2& getOffset() const { return offset; }

  unsigned int getDashCount() const { return dashSz; }
  double getDash(unsigned int idx) const;

  friend class BlockTable;
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
