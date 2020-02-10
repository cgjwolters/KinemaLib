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

#include "CcdOut.h"

#include "Vec.h"
#include "Trf.h"
#include "Writer.h"
#include "Exceptions.h"

#include "CcdDb.h"

#include <math.h>

namespace Ino
{

//---------------------------------------------------------------------------
/** \file CcdOut.h
    \brief Defines class CcdOut, which writes geometry in Ccd (Vector) format.

    Class CcdOut writes a stream in CCD (Vector) format.

    \author C. Wolters
    \date Feb 2008
*/

//---------------------------------------------------------------------------
/** \class CcdOut
  Writes geometry in CCD (Vector) format.

  Vector is a CAD/CAM program by Centriforce BV, The Netherlands.\n
  \n
  The data for an entire drawing is first stored in a memory data structure.\n
  The data is then written to the output stream when method flush() is called.

  \attention When flush() is called the memory data structure is cleared
  immediately after the data has been written.\n
  Subsequent calls that write geometry will start a new data structure!

  \author C. Wolters
  \date Feb 2008
*/

//---------------------------------------------------------------------------
/** \enum CcdOut::Units
   Defines the possible CCD output units.
*/

//---------------------------------------------------------------------------
/** \var CcdOut::Units CcdOut::Inch
    Denotes outout in inches.
*/

//---------------------------------------------------------------------------
/** \var CcdOut::Units CcdOut::Mm
    Denotes outout in millimeters.
*/

//---------------------------------------------------------------------------
/** Constructor.
  \param writer The Writer that is used to write the formatted output.
  \param units The Units to use.
*/

CcdOut::CcdOut(Writer& writer, Units units)
: wrt(writer), db(NULL), layer(*new CcdLayer("0")), selElems(false),
  unit(units), col(0)
{
}

//---------------------------------------------------------------------------
/** Destructor.

  Calls method flush() if that has not already been done.\n
  The Writer stream is <em>NOT</em> closed!
*/

CcdOut::~CcdOut()
{
  if (db) flush();

  delete &layer;
}

//---------------------------------------------------------------------------
/** \fn CcdOut::Units CcdOut::getUnits() const
    Returns the current Units.
*/

//---------------------------------------------------------------------------
/** Sets the output units.

  \param newUnit The units to use.

  \note This method must be called before the first call
  that generates output.

  \throw IllegalStateException If this method is called after some output
  has already been generated.
*/

void CcdOut::setUnits(Units newUnit)
{
  if (db) throw IllegalStateException("CcdOut::setUnits");

  unit = newUnit;
}

//---------------------------------------------------------------------------
/** Creates a new named layer.

  \param name The name of the layer.
  \param selected If \c true, all elements written to this new layer will\n
  automatically be selected when the drawing is opened in Vector.

  \throw NullPointerException if <tt>name == NULL</tt>.
*/

void CcdOut::newLayer(const char *name, bool selected)
{
  if (!name) throw NullPointerException("CcdOut::newLayer");

  selElems = selected;

  if (!db) db = new CcdDb2;

  layer.setString(name);
  layer.prec.ilayer = db->addlayer(layer);
}

//---------------------------------------------------------------------------
/** Returns the current color.

   \param red The red part of the current color.
   \param green The green part of the current color.
   \param blue The blue part of the current color.
*/

void CcdOut::getCurrentColor(unsigned char& red, unsigned char& green,
                                                unsigned char& blue) const
{
  red   = (unsigned char)(col & 0xff);
  green = (unsigned char)((col >> 8) & 0xff);
  blue  = (unsigned char)((col >> 16) & 0xff);
}

//---------------------------------------------------------------------------
/** Sets the current color.

  Elements that are subsequently added will have the new current color.

  \param red The red part of the new color.
  \param green The green part of the new color.
  \param blue The blue part of the new color.
*/

void CcdOut::setCurrentColor(unsigned char red, unsigned char green,
                                                       unsigned char blue)
{
  col = (blue << 16) | (green << 8) | red;
}

//---------------------------------------------------------------------------
/** Adds a point.

   The point is added to the current layer with the current color.

   \param p The coordinates of the point.
*/

void CcdOut::addPoint(const Vec3& p)
{
  if (!db) {
    db = new CcdDb2;
    newLayer("0");
  }

  Vec3 lp(p);
  if (unit == Inch) lp /= InchInMm;

  CcdEntity point;

  point.makePoint(lp.x,lp.y,lp.z);
  point.setLayer(layer.prec.ilayer);
  point.setPick(selElems ? 1 : 0);
  point.setColor(col);

  db->addent(point);
}

//---------------------------------------------------------------------------
/** Adds a line.

    A solid line is added to the current layer with the current color.

    \param p1 The start point of the line.
    \param p2 The end point of the line.
*/

void CcdOut::addLine(const Vec3& p1, const Vec3& p2)
{
  if (!db) {
    db = new CcdDb2;
    newLayer("0");
  }

  Vec3 lp1(p1);
  Vec3 lp2(p2);

  if (unit == Inch) {
    lp1 /= InchInMm;
    lp2 /= InchInMm;
  }

  CcdEntity line;

  line.makeLine(lp1.x,lp1.y,lp1.z,lp2.x,lp2.y,lp2.z);
  line.setLayer(layer.prec.ilayer);
  line.setPick(selElems ? 1 : 0);
  line.setColor(col);
  line.prec.l.style = LS_SOLID;

  db->addent(line);
}

//---------------------------------------------------------------------------
/** Adds an arc.
    A solid arc is added to the current layer with the current color.

    \param c The centre of the arc.
    \param radius The radius of the arc.
    \param startAng The start angle of the arc in radians.
    \param endAng The end angle of the arc in radians.
    \param trf The transform to apply to the arc before it is output.

    If <tt>endAng > startAng</tt> a counterclockwise arc is output,
    else a clockwise arc is output.\n
    If \c trf is a mirroring transform the arc will be the reverse of that.

    \note
    Transform \c trf defines the transform to apply to the arc before
    it is being output.
*/

void CcdOut::addArc(const Vec3& c, double radius,
                          double startAng, double endAng, const Trf3& trf)
{
  if (!db) {
    db = new CcdDb2;
    newLayer("0");
  }

  Vec3 centre(c); centre.transform3(trf);
  radius *= trf.scaleX();

  if (unit == Inch) {
    centre /= InchInMm;
    radius /= InchInMm;
  }

  int ccw = endAng > startAng  ? 1 : -1;

  if (ccw < 0) {
    double h = startAng; startAng = endAng; endAng = h;
  }

  Trf3 cTrf; cTrf(0,3) = c.x; cTrf(1,3) = c.y; cTrf(2,3) = c.z;
  Trf3 lTrf(trf); lTrf *= cTrf;

  // Rotation sequence: Z-X-Y

  double cx = sqrt(sqr(lTrf(1,0)) + sqr(lTrf(1,1)));
  double rotX = atan2(-lTrf(1,2),cx);
  double rotY = atan2(lTrf(0,2),lTrf(2,2));
  double rotZ = atan2(lTrf(1,0),lTrf(1,1));

  startAng += rotZ;
  endAng   += rotZ;

  CcdEntity arc;

  arc.setLayer(layer.prec.ilayer);
  arc.setPick(selElems ? 1 : 0);
  arc.prec.a.style=LS_SOLID;
  arc.setColor(col);
  arc.makeArc(centre.x,centre.y,centre.z,radius,ccw,
                                               startAng,endAng,rotX,rotY);

  db->addent(arc);
}

//---------------------------------------------------------------------------
/** Adds a circle.
    A solid circle is added to the current layer with the current color.

    \param c The centre of the circle.
    \param radius The radius of the circle.
    \param trf The transform to apply to the circle before it is output.

    \note Transform \c trf defines the transform to apply to the arc before it
    is being output.
*/

void CcdOut::addCircle(const Vec3& c, double radius, const Trf3& trf)
{
  if (!db) {
    db = new CcdDb2;
    newLayer("0");
  }

  Vec3 centre(c); centre.transform3(trf);
  radius *= trf.scaleX();

  if (unit == Inch) {
    centre /= InchInMm;
    radius /= InchInMm;
  }

  Trf3 cTrf; cTrf(0,3) = c.x; cTrf(1,3) = c.y; cTrf(2,3) = c.z;
  Trf3 lTrf(trf); lTrf *= cTrf;

  // Rotation sequence: Z-X-Y

  double cx = sqrt(sqr(lTrf(1,0)) + sqr(lTrf(1,1)));
  double rotX = atan2(-lTrf(1,2),cx);
  double rotY = atan2(lTrf(0,2),lTrf(2,2));

  CcdEntity arc;

  arc.setLayer(layer.prec.ilayer);
  arc.setPick(selElems ? 1 : 0);
  arc.prec.a.style=LS_SOLID;
  arc.setColor(col);
  arc.makeArc(centre.x,centre.y,centre.z,radius,true,0.0,
                                                    Vec2::Pi2,rotX,rotY);

  db->addent(arc);
}

//---------------------------------------------------------------------------
/** Write the CCD data.

    This method writes the accumulated data to the output stream Writer and
    then clears the data.

    \return \c true If the data was successfully written,\n
    if \c false is returned, inspect the Writer to find out what was wrong.

    \note The method may be called explicitly, but, if not, it will be called
    by the destructor instead.
*/

bool CcdOut::flush()
{
  CWFile cwFile(wrt);

  db->save(cwFile);

  bool ok = wrt.flush();
  delete db;
  db = NULL;

  return ok;
}

} // namespace Ino
