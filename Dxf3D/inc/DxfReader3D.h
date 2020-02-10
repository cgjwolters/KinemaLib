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

#ifndef DXFREADER3D_INC
#define DXFREADER3D_INC

#include "DxfRead3D.h"

namespace Ino
{

//---------------------------------------------------------------------------

class ASCIIReader3D;

class DxfReader3D
{
  enum { BufCap = 4096, ValCap = 2048 };

  DxfRead3D& dxfRd;
  ASCIIReader3D& rdr;

  char *buf;
  int readPos, bufSz;

  int lineCount;
  int charCount;
  int progCount;

  DxfRead3D::Status stat;

  DxfReader3D(const DxfReader3D& cp);             // No Copying
  DxfReader3D& operator=(const DxfReader3D& src); // No Assignment

  void nextLine();

public:
  DxfReader3D(DxfRead3D& dxf, ASCIIReader3D& ascRdr);
  ~DxfReader3D();

  bool eof;
  int code;
  
  char *value;
  int valueSz;

  bool next();

  int toInt();
  double toDouble();

  int getCharCount() const { return charCount; }
  int getLineCount() const { return lineCount; }

  DxfRead3D::Status getStatus() const { return stat; }
  void setStatus(DxfRead3D::Status st) { stat = st; }

  void getCounts(int& lineCnt, int& charCnt) const;
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif

