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

#ifndef DXFREADER_INC
#define DXFREADER_INC

#include "DxfRead.h"

namespace Ino
{

//---------------------------------------------------------------------------

class ASCIIReader;

class DxfReader
{
  enum { BufCap = 4096, ValCap = 2048 };

  DxfRead& dxfRd;
  ASCIIReader& rdr;

  char *buf;
  int readPos, bufSz;

  int lineCount;
  int charCount;
  int progCount;

  DxfRead::Status stat;

  DxfReader(const DxfReader& cp);             // No Copying
  DxfReader& operator=(const DxfReader& src); // No Assignment

  void nextLine();

public:
  DxfReader(DxfRead& dxf, ASCIIReader& ascRdr);
  ~DxfReader();

  bool eof;
  int code;
  
  char *value;
  int valueSz;

  bool next();

  int toInt();
  double toDouble();

  int getCharCount() const { return charCount; }
  int getLineCount() const { return lineCount; }

  DxfRead::Status getStatus() const { return stat; }
  void setStatus(DxfRead::Status st) { stat = st; }

  void getCounts(int& lineCnt, int& charCnt) const;
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif

