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

#ifndef CCDDB_INC
#define CCDDB_INC

//---------------------------------------------------------------------------

#include "CcdWFile.h"    //class WFile (simple Binary File Output)
#include "CcdLayer.h"    //class Layer
#include "CcdEnt.h"      //class Entity
#include "CsArray.h"  //template array class
                      //use whatever other array template...      
#include <string.h>

/* 
  Vector File DB Structure:
  (A) LayerTable( index, string )   index<0
  (B) EntityArray[index]         index>0                     
  (C) elementindex=SelectionArray[index]: ---> EntityArray[elementindex] (elementindex>0)
  
  Entity{
    int pick: (0,1) : --> is member of Selection?
      int layer; index into Layertable (layer<0) or default (0)
  }
*/    

namespace Ino
{

//---------------------------------------------------------------------------

class CcdDb2
{
  //minimal Vector file dbase structure
  //and some tool functions to put something in the DB 

  void calcModelBox(double& lx, double& ly, double& hx, double& hy);

public:
  CArray<short>     m_selection;  //CSmallArray<int>    
  CArray<CcdEntity> m_ent;        //CSmallArray<Entity> < 64K --> use CSmallArray<Entity*>
  CArray<CcdLayer>  m_layer;      //Table(index,string) implemented as array
  double            m_size;       //dont care

protected:
  
  //STREAMS
  int saveHeader(CAbstractWFile* pstream);
  int saveInfo(CAbstractWFile* pstream);
  int savePreview(CAbstractWFile* pstream);
  int saveLayers(CAbstractWFile* pstream);
  int saveAxismat(CAbstractWFile* pstream);
  int saveSelection(CAbstractWFile* pstream);
  int saveEntities(CAbstractWFile* pstream);
  int saveViewPorts(CAbstractWFile* pstream);
  
  short addselection(const short& index);
  
  public:   
  CcdDb2() : m_size(1)  {  clear(); }
  
  int save(CAbstractWFile& pstream); 

  void clear();
  short addent(const CcdEntity& ent);
  short addlayer(const CcdLayer& layer);

  CcdEntity& getEnt(short index);

  CcdLayer* findLayer(short ilayer);
};

} // namespace Ino

//---------------------------------------------------------------------------
#endif
