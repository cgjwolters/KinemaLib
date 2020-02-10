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

#include "CcdDb.h"

#include <math.h>

namespace Ino
{

//---------------------------------------------------------------------------
//Incremental differences in ccd file format 2.0 :
//(1) CcdDb2::saveHeader(CAbstractWFile* ps);
//(2) CcdDb2::saveInfo(CAbstractWFile* ps);      
//(3) CcdDb2::saveViewPorts(CAbstractWFile* ps)

int CcdDb2::saveHeader(CAbstractWFile* ps) {
  char buf[80];
  strcpy(buf,"LSNAH-Magic Ver 2.0 ");  // Special "Hans l" Header
  return ps->put(buf,(int)(strlen(buf)+1));
}       

//---------------------------------------------------------------------------

int CcdDb2::saveInfo(CAbstractWFile* ps) 
{
  short idum = 0;
  double rdum = 0;
  short nument;
  short npntrs=0;  //must be zero!
  short nsel;
  double size=m_size;
  long id=0; 

  nument=(short)m_ent.size();
  nsel=(short)m_selection.size();

  ps->putInt16(nument);
  ps->putInt16(npntrs);
  ps->putInt16(nsel);
  ps->putInt16(idum);
    
  ps->putDouble(size);
  ps->putDouble(rdum);
  ps->putDouble(rdum);   
  ps->putInt32(id);   
  ps->putDouble(idum);
  return ps->putDouble(idum);
}                  

//---------------------------------------------------------------------------

int CcdDb2::savePreview(CAbstractWFile* ps)
{
   short icode = 4;  
   short bmWidthBytes=64;
   short bmWidth=8*bmWidthBytes;
   short bmHeight=64;
   short n = bmWidthBytes*bmHeight;
   
  ps->putInt16(icode);
  ps->putInt16(bmWidth);
  ps->putInt16(bmHeight);
    
  
  char* pbmp = new char[n];
  if (!pbmp) return 0;
  {          
      //empty bitmap:
    char* p=pbmp;
    for (short i=0; i<n; i++) *p++=0;
  }    
    int rtn=ps->put(pbmp,n);
    if (pbmp) delete [] pbmp;
    return rtn;
}   

//---------------------------------------------------------------------------

int CcdDb2::saveLayers(CAbstractWFile* ps)
{
  short icode = 5;  
  short nlayers;

  nlayers=(short)m_layer.size();
  
  ps->putInt16(icode);
  ps->putInt16(nlayers);  
  for (short i=0; i<nlayers; i++){       
    ps->put((const char*)m_layer[i].prec.str,32);
    if (!ps->putInt16(m_layer[i].prec.ilayer)) return 0;
  }
  return 1;  
}

//---------------------------------------------------------------------------

int CcdDb2::saveAxismat(CAbstractWFile* ps)
{
  short icode = 6;
  double m[4][4];
  double mrot[3];    //rotation angles
  double mscale=1;   
  double mdet=1;     
  char   mvalid=0;

  //make indentity matrix:
  memset(&m,0,sizeof(m));
  m[0][0]=m[1][1]=m[2][2]=m[3][3]=1; 
  mrot[0]=mrot[1]=mrot[2]=0;
  
  ps->putInt16(icode);
  assert(sizeof(m)==16*sizeof(double));
  ps->put((const char*)&m,sizeof(m));
  assert(sizeof(mrot)==3*sizeof(double));
  ps->put((const char*)&mrot,sizeof(mrot));
  ps->putDouble(mscale);
  ps->putDouble(mdet);
  return ps->put(mvalid);
}                    

//---------------------------------------------------------------------------

int CcdDb2::saveSelection(CAbstractWFile* ps)
{
  short icode = 3;
  ps->putInt16(icode);
  for (short i=0; i<m_selection.size(); i++)
    if (!ps->putInt16(m_selection[i])) return 0;
  
  return 1;    
}  

//---------------------------------------------------------------------------

int CcdDb2::saveEntities(CAbstractWFile* ps)
{
  short icode = 1;
  
  ps->putInt16(icode);
  for (short i = 0; i < m_ent.size(); i++) 
    if (!m_ent[i].save(ps)) return 0;      
  
  return 1;
}      

//---------------------------------------------------------------------------

typedef struct tagVRECT
{
   short left;
   short top;
   short right;
   short bottom;
} VRECT;

//---------------------------------------------------------------------------

int CcdDb2::saveViewPorts(CAbstractWFile* ps)  
{
  short icode = 2;
  double xm=0,ym=0,scale=1;   //midpointViewport(xm,ym), scale factor
  double m[4][4];       //modelview matrix  
  double mrot[3];          //modelview matrix attrib : rotation angles
  double mscale=1;         //modelview matrix attrib : scale
  double mdet=1;           //modelview matrix attrib : det
  char   mvalid=0;      //modelview matrix attrib : attrib's valid? 
  short  nwind=1;          //one window
  VRECT  rect;          //window rectangle
  unsigned long style=0;    //window style //DWORD
  
  //Set Window Style: (w.showCmd in WINDOWPLACEMENT)
  // style=0;   //SW_NORMAL
  style=1;   //SW_SHOWMAXIMIZED
  //style=2;   //SW_SHOWMINIMIZED    
  
  //window rectangle: inverted y-axis: (rect.top,rect.bottom) !
  memset(&rect,0,sizeof(rect));
  rect.left=0;  rect.right=800;
  rect.top=0;    rect.bottom=600;
  
  {
    const double epsilon=1e-6;
    double xmin[3],xmax[3];

    xmin[0]=-10.0; xmin[1]= 0.0; xmin[2]= 0.0;
    xmax[0]=180.0; xmax[1]=40.0; xmax[2]= 0.0;

    xm=(xmin[0]+xmax[0])*0.5;
    ym=(xmin[1]+xmax[1])*0.5;
    
    //-------------------------------------------------------------------------
    // Addition by C. Wolters 03-10-2000
  
    calcModelBox(xmin[0],xmin[1],xmax[0],xmax[1]); // Minimum box around model

/*
    if (fabs(xmax[0]-xmin[0]) > 1e-8 && fabs(xmax[1]-xmin[1]) > 1e-8) {
      mscale = (rect.right-rect.left)/((xmax[0]-xmin[0])*1.2);
      double hscale = (rect.bottom-rect.top)/((xmax[1]-xmin[1])*1.2);

      if (hscale < mscale) mscale = hscale;
    }
*/
    xm = (xmin[0] + xmax[0])/2.0;
    ym = (xmin[1] + xmax[1])/2.0;

    // End addition by C. Wolters
   
    const double cfactor=1.2;    
    double dx=cfactor*(xmax[0]-xmin[0]);
    double dy=cfactor*(xmax[1]-xmin[1]);
    double sx=1.0,sy=1.0,ss=1.0;

    //-----------------------------------------------------------------------
    // Addition by C. Wolters 02-05-2001

    double pixpmmx=5.0;
    double pixpmmy=7.0;

    // End addition by C. Wolters
    //-----------------------------------------------------------------------

    ss=1.0; //default
    
    if (fabs(dx)>epsilon)
      sx = fabs( (rect.right-rect.left)/(dx*pixpmmx));
    if (fabs(dy)>epsilon)
      sy = fabs( (rect.bottom-rect.top)/(dy*pixpmmy));

    if (fabs(dx)<epsilon){
      // sx invalid 
      if (fabs(dy)>epsilon){
        //sy valid
        ss=sy;
      } 
      // else both scales invalid: use default
    } else if (fabs(dy)<epsilon){
      // sy invalid
      if (fabs(dx)>epsilon){
        //sx valid
        ss=sx;
      }
      // else both scales invalid: use default
    } else {
      // sx,sy both valid :
      
      if (sx<sy){
        ss=sx;
      } else {
        ss=sy;
      }
    }

    scale = fabs(1.0/ss);
    if (epsilon > scale) scale = epsilon;
  }

  //make identity matrix:
  memset(&m,0,sizeof(m));
  m[0][0]=m[1][1]=m[2][2]=m[3][3]=1.0; 
  mrot[0]=mrot[1]=mrot[2]=0;
  
  ps->putInt16(icode);
  ps->putInt16(nwind);  
  for (int i = 0; i < nwind; i++) {  
      
      //Window information:
      ps->put((const char*)&rect,sizeof(rect));
    assert(sizeof(rect)==4*sizeof(short));    
      ps->putUInt32(style);
      
      //2D mapping:
    ps->putDouble(xm);
    ps->putDouble(ym);
    ps->putDouble(scale);
    
    //model view matrix:
    ps->put((const char*)&m,sizeof(m));
    assert(sizeof(m)==16*sizeof(double));
    ps->put((const char*)&mrot,sizeof(mrot));
    assert(sizeof(mrot)==3*sizeof(double));    
    ps->putDouble(mscale);
    ps->putDouble(mdet);
    if (!ps->put(mvalid)) return 0;
    
  }
  return 1;
}

//---------------------------------------------------------------------------

int CcdDb2::save(CAbstractWFile& ps)
{               
  if (!saveHeader(&ps)) return 0;
  if (!saveInfo(&ps)) return 0;
  if (!savePreview(&ps)) return 0;    //code 4
  if (!saveLayers(&ps)) return 0;    //code 5
  if (!saveAxismat(&ps)) return 0;    //code 6
  if (!saveSelection(&ps)) return 0;   //code 3
  if (!saveEntities(&ps)) return 0;  //code 1
  if (!saveViewPorts(&ps)) return 0;  //code 2
  
  short icode = -1;            //code -1: end

  return ps.putInt16(icode);
}

//---------------------------------------------------------------------------

short CcdDb2::addselection(const short& index){
  short n=(short)m_selection.size();
    
  assert(index>0);
    
   if (n>=m_selection.capacity()){ //Expand
     const short grow=100;
     m_selection.resize(m_selection.capacity()+grow);
     m_selection.trim(n);
   }   
   m_selection.trim(n+1);
   m_selection[n]=index;  
     
   return index;
}

//---------------------------------------------------------------------------

short CcdDb2::addent(const CcdEntity& ent){
  //input : ent
  //ouput : index=indexarray+1
    
  const short maxent=30000;
  short n=(short)m_ent.size();
        
   if (n>=m_ent.capacity()){  //Expand
     const short grow=100;
     m_ent.resize(m_ent.capacity()+grow);
     m_ent.trim(n);
   }   
     
   assert(n<=maxent);
     
   m_ent.trim(n+1);
   m_ent[n]=ent;
     
   short index=n+1;       //(index>0) 
   if (ent.getPick())
     addselection(index); //add to selection
     
   return index; //ouput : index=indexarray+1
}

//---------------------------------------------------------------------------

short CcdDb2::addlayer(const CcdLayer& CcdLayer){
  //input : CcdLayer.str;
  //ouput : negative index
  short n=(short)m_layer.size();
        
   if (n>=m_layer.capacity()){ //Expand
     const short grow=100;
     m_layer.resize(m_ent.capacity()+grow);
     m_layer.trim(n);
   }   
     
   m_layer.trim(n+1);
   m_layer[n]=CcdLayer;
     
   short index=-(n+1); //negative indices !
   m_layer[n].prec.ilayer=index;
     
   return index;
}

//---------------------------------------------------------------------------

void CcdDb2::clear()
{
  m_selection.resize(100);
  m_selection.trim(0);
  m_ent.resize(100);
  m_ent.trim(0);
  m_layer.resize(10);
  m_layer.trim(0);        
};

//---------------------------------------------------------------------------
// Addition by C. Wolters 03-10-2000
//---------------------------------------------------------------------------

// Calculate bounding box of arc

static void calcArcBox(const __recentity& ent,
                       double hlx, double hly, double hhx, double hhy)
{
  double phi1 = ent.a.angs[0];
  double phi2 = ent.a.angs[1];

  if (!ent.a.dir) {  // Is not counter clockwise
    double h = phi1; phi1 = phi2; phi2 = h; // So swap
  }

  double c1 = cos(phi1), s1 = sin(phi1);
  double c2 = cos(phi2), s2 = sin(phi2);

  if (c1 < c2) {
    hlx = c1; hhx = c2;

    if (s1 >= 0.0) hlx = -1.0;
    if (s2 >= 0.0) hhx =  1.0;
  }
  else {
    hlx = c2; hhx = c1;

    if (s1 <= 0.0) hhx =  1.0;
    if (s2 <= 0.0) hlx = -1.0;
  }

  if (s1 < s2) {
    hly = s1; hhy = s2;

    if (c1 <= 0.0) hly = -1.0;
    if (c2 <= 0.0) hhy =  1.0;
  }
  else {
    hly = s2; hhy = s1;

    if (c1 >= 0.0) hhy =  1.0;
    if (c2 >= 0.0) hly = -1.0;
  }

  hlx *= ent.a.radius; hly *= ent.a.radius;
  hhx *= ent.a.radius; hhy *= ent.a.radius;
}

//---------------------------------------------------------------------------
// Calculate bounding around all entities

void CcdDb2::calcModelBox(double& lx, double& ly, double& hx, double& hy)
{
  for (int i=0; i < m_ent.size(); i++) {
    const __recentity& ent = m_ent[i].prec;

    double hlx=0.0, hly=0.0, hhx=0.0, hhy=0.0;

    switch (ent.type) {
    case TPOINT:
      hlx = hhx = ent.p.coord[0];
      hly = hhy = ent.p.coord[1];
      break;

    case TLINE:
      hlx = ent.l.coord[0]; 
      hly = ent.l.coord[1];

      hhx = ent.l.coord2[0];
      hhy = ent.l.coord2[1];
      break;

    case TARC:
      calcArcBox(ent,hlx,hly,hhx,hhy);
      break;
    }

    if (hlx > hhx) { double h = hlx; hlx = hhx; hhx = h; } // Swap
    if (hly > hhy) { double h = hly; hly = hhy; hhy = h; } // Swap

    if (i < 1 || hlx < lx) lx = hlx;
    if (i < 1 || hly < ly) ly = hly;

    if (i < 1 || hhx > hx) hx = hhx;
    if (i < 1 || hhy > hy) hy = hhy;
  }
}

//---------------------------------------------------------------------------

CcdEntity& CcdDb2::getEnt(short index)
{
  //to show shift in index:
  assert(index>=1);
  assert(index<=m_ent.size());

  return m_ent[index-1]; //index in [1,...,m_ent.size()]
}

//---------------------------------------------------------------------------

CcdLayer* CcdDb2::findLayer(short ilayer)
{
  //naive look up in CcdLayer table
  //to show how layertable is defined

  for (short i=0; i<m_layer.size(); i++){
    if (ilayer==m_layer[i].prec.ilayer) return &m_layer[i];
  }   

  return NULL;
}

} // namespace Ino
