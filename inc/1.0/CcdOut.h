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

#ifndef CCDOUT_INC
#define CCDOUT_INC

//---------------------------------------------------------------------------

namespace Ino
{
class Vec3;
class Trf3;
class Writer;
class CcdDb2;
class CcdLayer;

class CcdOut
{
public:
  enum Units { Mm, Inch };

private:
  Writer& wrt;
  CcdDb2 *db;
  CcdLayer& layer;
  bool selElems;
  Units unit;
  long col;

  CcdOut(const CcdOut& cp);             // No Copying
  CcdOut& operator=(const CcdOut& src); // No Assignment

public:
  CcdOut(Writer& writer, Units units);
  ~CcdOut();

  Units getUnits() const { return unit; }
  void setUnits(Units newUnit);

  void newLayer(const char *name, bool selected=false);

  void getCurrentColor(unsigned char& red, unsigned char& green,
                                               unsigned char& blue) const;
  void setCurrentColor(unsigned char red, unsigned char green,
                                                unsigned char blue);
  void addPoint(const Vec3& p);
  void addLine(const Vec3& p1, const Vec3& p2);
  void addArc(const Vec3& c, double radius,
                            double startAng, double endAng, const Trf3& trf);
  void addCircle(const Vec3& c, double radius, const Trf3& trf);

  bool flush();
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif

