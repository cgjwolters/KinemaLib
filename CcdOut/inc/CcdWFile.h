//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Ccd (Vector) Format Writer --------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------- Copyright (C) 2005 Prodim International BV ----------------------
//---------------------------------------------------------------------------
//--------- C.Wolters Feb 2008 ----------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef CCDWFILE_INC
#define CCDWFILE_INC

namespace Ino
{

//---------------------------------------------------------------------------

class CAbstractWFile
{
public:
  virtual int put(const char& c) = 0;
  virtual int put(const char * s,int n) = 0;

  virtual int putInt16(const short& i) {
    return put((const char *)&i,sizeof(short));
  }

  virtual int putInt32(const long& l) {
    return put((const char *)&l,sizeof(long));
   }

  virtual int putUInt32(const unsigned long& l) {
    return put((const char *)&l,sizeof(unsigned long));
  }

  virtual int putDouble(const double& d) {
    return put((const char *)&d,sizeof(double));
  }
};

//---------------------------------------------------------------------------

class Writer;

class CWFile: public CAbstractWFile
{
  Writer& wrt;

  CWFile& operator=(const CWFile& src); // No Assignment
public:
  CWFile(Writer& writer) : wrt(writer) {}
  
  virtual int put(const char& c);
  virtual int put(const char *s,int n);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
