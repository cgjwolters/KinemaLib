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

#ifndef CCDENT_INC
#define CCDENT_INC

#include "CcdWFile.h"

#include <string.h>

namespace Ino
{

//---------------------------------------------------------------------------

#define COLORREF unsigned long
#define BYTE unsigned char

typedef enum {
   TNONE = 0,TPOINT=1,TLINE=2,TARC=3,TTEXT=7,
} Type;

typedef enum {
  LS_SOLID      = 1,
  LS_DASH       = 2,
  LS_DOT        = 3,
  LS_DASHDOT    = 4,
  LS_DASHDOTDOT = 5,
} LineStyle; //Windows linestyles

//---------------------------------------------------------------------------

typedef struct
{
  char type;
  COLORREF color; //unsigned long
  BYTE pick,vis;
  int layer,free;
  float width;
  long id;

  union {
    struct {
      double coord[3];
    } p; 
    struct {
      char dir;
      BYTE style;
      double coord[3],coord2[3];
    } l;
    struct {
      char dir;
      BYTE style;
      double coord[3],radius,angs[2],rot[2];
    } a;
    struct  {
      float size,coord[3],angle;
      float rot[2];
      char text[41];    
    } t;
  };
} __recentity;        

//---------------------------------------------------------------------------

class CcdEntity {
  //simple wrapper
  
public: //
  __recentity prec;    

private:  
  void copy(const CcdEntity* s);
  
  //STREAMS
  int saveHeader(CAbstractWFile* pstream);
  int saveAttrib(CAbstractWFile* pstream);
  int savePoint(CAbstractWFile* pstream);
  int saveLine(CAbstractWFile* pstream);
  int saveArc(CAbstractWFile* pstream);
  int saveText(CAbstractWFile* ps);
  int zeropad(CAbstractWFile* ps);
    
public:
  CcdEntity(); 
  CcdEntity(const CcdEntity& a);

  CcdEntity& operator=(const CcdEntity& a);
  int operator==(const CcdEntity& a);
  
  int save(CAbstractWFile* pstream);
  
  //Attributes
  int getPick() const;
  void setPick(const int pick);

  int getVis() const;
  void setVis(const int vis);

  int getLayer() const;
  void setLayer(const int layer);
  
  COLORREF getColor() const;
  void setColor(const COLORREF color);
  
  void init();
  
  void makePoint(const double x,const double y,const double z);
  
  void makeLine(const double x1,const double y1,const double z1,
                const double x2,const double y2,const double z2);
  
  void makeArc(const double xc, const double yc, const double zc,
               const double r, const int ccw,
               const double fi1, const double fi2,
               const double rotx, const double roty);

  void makeText(double size, double x, double y, double z,
                double angleZ, double angleX, double angleY,
                char * text);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
