#ifndef H_DB
#define H_DB

#include <string.h>

#include "wfile.h"		//class WFile (simple Binary File Output)
#include "layer.h"      //class Layer
#include "ent.h"		//class Entity
#include "csarray.h"	//template array class
                        //use whatever other array template...      
                        
                        
/* 
	Vector File DB Structure:
	(A) LayerTable( index, string )   index<0
	(B) EntityArray[index] 			  index>0                     
	(C) elementindex=SelectionArray[index]: ---> EntityArray[elementindex] (elementindex>0)
	
	Entity{
		int pick: (0,1) : --> is member of Selection?
	    int layer; index into Layertable (layer<0) or default (0)
	}
*/		

class DB1 {
	//minimal Vector file dbase structure
	//and some tool functions to put something in the DB 
	public:
	CArray<short> 		m_selection;  //CSmallArray<int>    
	CArray<Entity>  	m_ent;		 //CSmallArray<Entity> < 64K --> use CSmallArray<Entity*>
	CArray<Layer>   	m_layer;      //Table(index,string) implemented as array
	double 		   		m_size;       //dont care
	
	
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
	DB1(void)
	: m_size(1)
	{
		clear();	
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	int save(CAbstractWFile* pstream); 
	void Dump(void) const;
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void clear();
	short addent(const Entity& ent);
	short addlayer(const Layer& layer);
	////////////////////////////////////////////////////////////////////////////////////////////////////
	Entity& getEnt(short index){
		//to show shift in index:
		assert(index>=1);
		assert(index<=m_ent.size());
	 	return m_ent[index-1]; //index in [1,...,m_ent.size()]
	}
	Layer* findLayer(short ilayer){
		//naive look up in layer table
		//to show how layertable is defined
	 	for (short i=0; i<m_layer.size(); i++){
	 	 	if (ilayer==m_layer[i].prec.ilayer) return &m_layer[i];
	 	}   
	 	return NULL;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
		
};

class DB2 {
	//minimal Vector file dbase structure
	//and some tool functions to put something in the DB 

  //---------------------------------------------------------------------------
  // Addition by C. Wolters 03-10-2000
  //---------------------------------------------------------------------------

  void calcModelBox(double& lx, double& ly, double& hx, double& hy);

  //---------------------------------------------------------------------------
  // End addition by C. Wolters
  //---------------------------------------------------------------------------


	public:
	CArray<short> 		m_selection;  //CSmallArray<int>    
	CArray<Entity>  	m_ent;		 //CSmallArray<Entity> < 64K --> use CSmallArray<Entity*>
	CArray<Layer>   	m_layer;      //Table(index,string) implemented as array
	double 		   		m_size;       //dont care
	
	
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
	DB2(void)
	: m_size(1)
	{
		clear();	
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	int save(CAbstractWFile* pstream); 
	void Dump(void) const;
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void clear();
	short addent(const Entity& ent);
	short addlayer(const Layer& layer);
	////////////////////////////////////////////////////////////////////////////////////////////////////
	Entity& getEnt(short index){
		//to show shift in index:
		assert(index>=1);
		assert(index<=m_ent.size());
	 	return m_ent[index-1]; //index in [1,...,m_ent.size()]
	}
	Layer* findLayer(short ilayer){
		//naive look up in layer table
		//to show how layertable is defined
	 	for (short i=0; i<m_layer.size(); i++){
	 	 	if (ilayer==m_layer[i].prec.ilayer) return &m_layer[i];
	 	}   
	 	return NULL;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
		
};


#endif


