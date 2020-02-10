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

#include "DxfReader3D.h"

#include "Exceptions.h"

#include <cstring>
#include <cstdlib>

using namespace std;

namespace Ino
{

//---------------------------------------------------------------------------

DxfReader3D::DxfReader3D(DxfRead3D& dxf, ASCIIReader3D& ascRdr)
: dxfRd(dxf), rdr(ascRdr), buf(new char[BufCap]),
  readPos(0), bufSz(0),
  lineCount(0), charCount(0), progCount(0), stat(DxfRead3D::Success),
  eof(false), code(-1), value(new char[ValCap+1]), valueSz(0)
{
}

//---------------------------------------------------------------------------

DxfReader3D::~DxfReader3D()
{
  if (buf) delete[] buf;
  if (value) delete[] value;
}

//---------------------------------------------------------------------------

void DxfReader3D::nextLine()
{
  valueSz = 0;

  while (!eof) {
    if (readPos >= bufSz) {
      readPos = 0;

      bufSz = rdr.read(buf,BufCap);

      if (bufSz < 0) {
        eof = valueSz < 1;
        bufSz = 0;
        stat = DxfRead3D::PrematureEnd;
        return;
      }
      else if (bufSz == 0) {
        eof = valueSz < 1;
        bufSz = 0;
        return;
      }
    }

    char c = buf[readPos++];

    if (c == '\n') {
      lineCount++;
      charCount += valueSz; charCount++;
      progCount += valueSz; charCount++;

      while (valueSz > 0 && value[valueSz-1] == '\r') valueSz--;

      value[valueSz] = '\0';

      if (progCount > 1024) {
        progCount = 0;
        if (!rdr.progress(charCount,lineCount)) {
          eof = true;
          bufSz = 0;
          stat = DxfRead3D::Aborted;
          throw InterruptedException("Interrupted on Request");
        }
      }

      return;
    }
    else if (valueSz >= ValCap) {
      eof = true;
      bufSz = 0;
      stat = DxfRead3D::LineTooLong;
      return;
    }

    value[valueSz++] = c;
  }
}

//---------------------------------------------------------------------------

bool DxfReader3D::next()
{
  if (eof) return false;

  try {
    do {
      nextLine();
      if (eof) return false;

      code = toInt();
      nextLine();
    }
    while (code == 999); // Continue if comment
  }
  catch (exception) {
    eof = true;
    return false;
  }

  return true;
}

//---------------------------------------------------------------------------

int DxfReader3D::toInt()
{
  trim(value);
  valueSz = strlen(value);

  char *endPt;

  int val = strtol(value,&endPt,10);
  if (endPt - value < valueSz) throw NumberFormatException("Not an integer");

  return val;
}

//---------------------------------------------------------------------------

double DxfReader3D::toDouble()
{
  trim(value);
  valueSz = strlen(value);

  char *endPt;

  double val = strtod(value,&endPt);
  if (endPt - value < valueSz) throw NumberFormatException("Not a double");

  return val;
}

//---------------------------------------------------------------------------

void DxfReader3D::getCounts(int& lineCnt, int& charCnt) const
{
  lineCnt = lineCount;
  charCnt = charCount;
}

} // namespace Ino

//---------------------------------------------------------------------------
