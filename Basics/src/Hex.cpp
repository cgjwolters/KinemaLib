//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//------- Hex encoding/decoding ---------------------------------------------
//---------------------------------------------------------------------------
//------- Copyright Inofor Hoek Aut BV NOV 2006 -----------------------------
//---------------------------------------------------------------------------
//------- A.N. Kloosterhuis -------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "Hex.h"

#include <cstring>
#include <ctype.h>

namespace Ino
{

//---------------------------------------------------------------------------

static const char b16Chars[] = {
    '0','1','2','3','4','5','6','7',
    '8','9','A','B','C','D','E','F',
  };

static char b16Rev[128];

static void calcRevList()
{
  for (int i=0; i<128; ++i) b16Rev[i] = -1;
  for (int i=0; i<16; ++i) b16Rev[(int)(b16Chars[i])] = (char)i;
  for (int i=0; i<16; ++i) b16Rev[tolower(b16Chars[i])] = (char)i;
}

//---------------------------------------------------------------------------

void Hex::ensureCap(int minCap)
{
  if (minCap < 1) minCap = 1;

  if (!buf) {
    buf = new char[minCap];
    cap = minCap;
  }
  else {
    if (minCap <= cap) return;

    char *newBuf = new char[minCap];
    memmove(newBuf,buf,cap);

    delete[] buf;

    buf = newBuf;
    cap = minCap;
  }
}

//---------------------------------------------------------------------------


Hex::Hex()
: buf(NULL), cap(0)
{
  calcRevList();
}

//---------------------------------------------------------------------------


Hex::~Hex()
{
  delete[] buf;
}

//---------------------------------------------------------------------------

const char *Hex::encode(const char *msg, int len, int& codeLen)
{
  codeLen = 0;

  if (!msg || len < 0) return NULL;

  if (len < 1) {
    ensureCap(1);
    buf[0] = '\0';

    return buf;
  }

  codeLen = len * 2;

  ensureCap(codeLen+1);

  int h1=0, h2=0, pos=0;

  for (int i=0; i<len; ++i) {
    h1 = (msg[i] & 0xF0) >> 4;
    h2 = msg[i] & 0x0F;

    buf[pos++] = b16Chars[h1];
    buf[pos++] = b16Chars[h2];
  }

  buf[codeLen] = '\0';

  return buf;
}

//---------------------------------------------------------------------------

const char *Hex::decode(const char *msg, int len, int& decodeLen)
{
  decodeLen = 0;

  if (!msg || len < 0) return NULL;

  if (len < 1) {
    ensureCap(1);
    buf[0] = '\0';

    return buf;
  }

  decodeLen = (len+1)/2;

  ensureCap(decodeLen+1);

  int pos = 0;

  for (int i=0; i<len; ++i) {

    if (msg[i] < 0) {
      decodeLen = 0;
      return NULL;
    }

    int code = b16Rev[(int)(msg[i])];
    if (code < 0) {
       decodeLen = 0;
       return NULL;
    } else {
      if (i % 2 == 0)
        buf[pos] = (code << 4) & 0xF0; 
      else
        buf[pos++] |= code; 
    }
  }

  return buf;
}

} // namespace Ino

//---------------------------------------------------------------------------
