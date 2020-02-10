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

#include "CcdEnt.h"

//---------------------------------------------------------------------------

namespace Ino
{

#pragma pack(1) //BYTE PACKING

//---------------------------------------------------------------------------

struct rectv
{
  char  type;     
  short ivers;    
};

//---------------------------------------------------------------------------

struct recglobal1
{
  COLORREF color;
  BYTE pick,vis;
  short layer,free;
  float width;
};

//---------------------------------------------------------------------------

struct recpoint1
{
  double coord[3];
};

//---------------------------------------------------------------------------

struct recline1
{
  char dir;
  BYTE style;
  double coord[3],coord2[3];
};

//---------------------------------------------------------------------------

struct recarc1
{
  char dir;
  BYTE style;
  double coord[3],radius,angs[2],rot[2];
};

//---------------------------------------------------------------------------

struct rectext1
{
  float size,coord[3],angle;
  float rot[2];
  char text[41];
};

#pragma pack() 

//---------------------------------------------------------------------------

void CcdEntity::copy(const CcdEntity* s){
  if (this != s) memcpy(&prec,&(s->prec),sizeof(__recentity));
}         

//---------------------------------------------------------------------------

int CcdEntity::save(CAbstractWFile* ps){      
  switch (prec.type){
  case TPOINT: return savePoint(ps);   
  case TLINE: return saveLine(ps);   
  case TARC: return saveArc(ps);     
  case TTEXT: return saveText(ps);     
  default: return 0;
  }
}

//---------------------------------------------------------------------------

int CcdEntity::saveHeader(CAbstractWFile* ps)
{   
  const int version=1;
  rectv tv;
  tv.type=(char)prec.type;
  tv.ivers=version;
  return ps->put((const char*)&tv,sizeof(tv));  
}

//---------------------------------------------------------------------------

int CcdEntity::saveAttrib(CAbstractWFile* ps)
{
  recglobal1 g;
  g.color = prec.color;
  g.pick = prec.pick;
  g.vis = prec.vis;
  g.layer = (short)prec.layer;
  g.free = (short)prec.free;
  g.width = prec.width;
  return ps->put((const char*)&g,sizeof(g));  
}    

//---------------------------------------------------------------------------

int CcdEntity::zeropad(CAbstractWFile* ps)
{
  //write 20 zero chars   
  char zero=0;
  for (int i=0; i<20; i++)
    if (!ps->put(zero)) return 0;
  return 1;      
}

//---------------------------------------------------------------------------

int CcdEntity::savePoint(CAbstractWFile* ps)
{   
  recpoint1 p;
  
  saveHeader(ps);
  saveAttrib(ps);  
  
  p.coord[0] = prec.p.coord[0];
  p.coord[1] = prec.p.coord[1];
  p.coord[2] = prec.p.coord[2];
  ps->put((const char*)&p,sizeof(p));  
  
  return zeropad(ps);
}

//---------------------------------------------------------------------------

int CcdEntity::saveLine(CAbstractWFile* ps)
{   
  recline1 l;
  
  saveHeader(ps);
  saveAttrib(ps);  
  
  l.dir = prec.l.dir;
  l.style = prec.l.style;
  l.coord[0] = prec.l.coord[0];
  l.coord[1] = prec.l.coord[1];
  l.coord[2] = prec.l.coord[2];
  l.coord2[0] = prec.l.coord2[0];
  l.coord2[1] = prec.l.coord2[1];
  l.coord2[2] = prec.l.coord2[2];
  ps->put((const char*)&l,sizeof(l));  
  
  return zeropad(ps);
}

//---------------------------------------------------------------------------

int CcdEntity::saveArc(CAbstractWFile* ps)
{   
  recarc1 a;
  
  saveHeader(ps);
  saveAttrib(ps);  
  
  a.dir = prec.a.dir;
  a.style = prec.a.style;
  a.coord[0] = prec.a.coord[0];
  a.coord[1] = prec.a.coord[1];
  a.coord[2] = prec.a.coord[2];
  a.radius = prec.a.radius;
  a.angs[0] = prec.a.angs[0];
  a.angs[1] = prec.a.angs[1];
  a.rot[0] = prec.a.rot[0];
  a.rot[1] = prec.a.rot[1];
  ps->put((const char*)&a,sizeof(a));  
  
  return zeropad(ps);
}

//---------------------------------------------------------------------------

int CcdEntity::saveText(CAbstractWFile* ps)
{   
  rectext1 t;
  
  saveHeader(ps);
  saveAttrib(ps);  

  t.size = prec.t.size;
  t.coord[0] = prec.t.coord[0];
  t.coord[1] = prec.t.coord[1];
  t.coord[2] = prec.t.coord[2];
  t.angle = prec.t.angle;
  t.rot[0] = prec.t.rot[0];
  t.rot[1] = prec.t.rot[1];
  strcpy(t.text,prec.t.text);
  ps->put((const char*)&t,sizeof(t));  

  return zeropad(ps);
}

//---------------------------------------------------------------------------

CcdEntity::CcdEntity()
{
  init();
}

//---------------------------------------------------------------------------

CcdEntity::CcdEntity(const CcdEntity& a)
{
  copy(&a);
}

//---------------------------------------------------------------------------

CcdEntity& CcdEntity::operator=(const CcdEntity& a)
{
  copy(&a);
  return (*this);
}

//---------------------------------------------------------------------------

int CcdEntity::operator==(const CcdEntity& a)
{
  if (this != &a) return (memcmp(&prec,&(a.prec),sizeof(__recentity))==0);
  return 1;
}  
  
//---------------------------------------------------------------------------

int CcdEntity::getPick() const
{
  return prec.pick;
}

//---------------------------------------------------------------------------

void CcdEntity::setPick(const int pick)
{
  prec.pick = (unsigned char)pick;
}

//---------------------------------------------------------------------------

int CcdEntity::getVis() const
{
  return !prec.vis;
}

//---------------------------------------------------------------------------

void CcdEntity::setVis(const int vis)
{
  prec.vis=(!vis);
}

//---------------------------------------------------------------------------

int CcdEntity::getLayer() const
{
  return prec.layer;
}

//---------------------------------------------------------------------------

void CcdEntity::setLayer(const int layer)
{
  prec.layer=layer;
}
  
//---------------------------------------------------------------------------

COLORREF CcdEntity::getColor() const
{
  return prec.color;
}

//---------------------------------------------------------------------------

void CcdEntity::setColor(const COLORREF color)
{
  prec.color=color;
}
  
//---------------------------------------------------------------------------

void CcdEntity::init(void)
{
  memset(&prec,0,sizeof(__recentity));
}
  
//---------------------------------------------------------------------------

void CcdEntity::makePoint(const double x, const double y, const double z)
{
  //init();
  prec.type=(char)TPOINT;
  prec.p.coord[0]=x;     
  prec.p.coord[1]=y;     
  prec.p.coord[2]=z;     
}
  
//---------------------------------------------------------------------------

void CcdEntity::makeLine(const double x1, const double y1, const double z1,
                      const double x2, const double y2, const double z2)
{
  //init();
  prec.type=(char)TLINE;
  prec.l.dir=1;    
  prec.l.style=LS_SOLID;
  prec.l.coord[0]=x1;     
  prec.l.coord[1]=y1;     
  prec.l.coord[2]=z1;     
  prec.l.coord2[0]=x2;     
  prec.l.coord2[1]=y2;     
  prec.l.coord2[2]=z2;     
}
  
//---------------------------------------------------------------------------

void CcdEntity::makeArc(const double xc, const double yc, const double zc,
                     const double r, const int ccw,
                     const double fi1, const double fi2,
                     const double rotx, const double roty)
{
  //init();
  prec.type=(char)TARC;
  prec.a.dir=(char)ccw;    
  prec.a.style=LS_SOLID;    
  prec.a.coord[0]=xc;     
  prec.a.coord[1]=yc;     
  prec.a.coord[2]=zc;                         
  prec.a.radius=r;
  prec.a.angs[0]=fi1;
  prec.a.angs[1]=fi2;
  prec.a.rot[0]=rotx;
  prec.a.rot[1]=roty;
}

//---------------------------------------------------------------------------

void CcdEntity::makeText(double size, double x, double y, double z,
                      double angleZ, double angleX, double angleY,
                      char * text)
{
  //init();
  prec.type=(char)TTEXT;
  prec.t.size=(float)size;
  prec.t.coord[0]=(float)x;
  prec.t.coord[1]=(float)y;
  prec.t.coord[2]=(float)z;
  prec.t.angle=(float)angleZ;
  prec.t.rot[0]=(float)angleX;
  prec.t.rot[1]=(float)angleY;
  strncpy(prec.t.text,text,41);    
  prec.t.text[40]=0;
}

} //namespace Ino

//---------------------------------------------------------------------------
