#include "db.h"
#include "trace.h"


//IMPLEMENTATION
int DB1::saveHeader(CAbstractWFile* ps) {
	char buf[80];
	strcpy(buf,"LSNAH-Magic Ver 1.0 ");	// Special "Hans l" Header
	return ps->put(buf,strlen(buf)+1);	
}       

int DB1::saveInfo(CAbstractWFile* ps) 
{
	short idum = 0;
	double rdum = 0;
	short nument;
	short nsel;
	double size=m_size;

	nument=(short)m_ent.size();
	nsel=(short)m_selection.size();
			
	ps->putInt16(nument);
	ps->putInt16(idum);
	ps->putInt16(nsel);
	ps->putInt16(idum);
		
	ps->putDouble(size);
	ps->putDouble(rdum);
	ps->putDouble(rdum);
	return ps->putDouble(rdum);
}                  

int DB1::savePreview(CAbstractWFile* ps)
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

int DB1::saveLayers(CAbstractWFile* ps)
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

int DB1::saveAxismat(CAbstractWFile* ps)
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

int DB1::saveSelection(CAbstractWFile* ps)
{
	short icode = 3;
	ps->putInt16(icode);
	for (short i=0; i<m_selection.size(); i++)
		if (!ps->putInt16(m_selection[i])) return 0;
	
	return 1;		
}	

int DB1::saveEntities(CAbstractWFile* ps)
{
	short icode = 1;
	
	ps->putInt16(icode);
	for (short i = 0; i < m_ent.size(); i++) 
		if (!m_ent[i].save(ps)) return 0;			
	
	return 1;
}			

int DB1::saveViewPorts(CAbstractWFile* ps)	
{
	short icode = 2;
	double xm=0,ym=0,scale=1; //midpointViewport(xm,ym), scale factor
	double m[4][4]; //modelview matrix

	//make indentity matrix:
	memset(&m,0,sizeof(m));
	m[0][0]=m[1][1]=m[2][2]=m[3][3]=1; 
	
	ps->putInt16(icode);
	for (short i = 0; i < 4; i++) { 
		ps->putDouble(xm);
		ps->putDouble(ym);
		ps->putDouble(scale);
		if (!ps->put((const char*)&m,sizeof(m))) return 0;
	}
	return 1;
}

int DB1::save(CAbstractWFile* ps)
{               
	if (!ps) return 0;
	if (!saveHeader(ps)) return 0;
	if (!saveInfo(ps)) return 0;
	if (!savePreview(ps)) return 0;		//code 4
	if (!saveLayers(ps)) return 0;  	//code 5
	if (!saveAxismat(ps)) return 0;		//code 6
	if (!saveSelection(ps)) return 0;   //code 3
	if (!saveEntities(ps)) return 0;	//code 1
	if (!saveViewPorts(ps)) return 0;	//code 2
	
	short icode = -1;						//code -1: end
	return ps->putInt16(icode);
}

short DB1::addselection(const short& index){
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

short DB1::addent(const Entity& ent){
	//input : ent
	//ouput : index=indexarray+1
		
	short n=(short)m_ent.size();
				
 	if (n>=m_ent.capacity()){	//Expand
 		const short grow=100;
 		m_ent.resize(m_ent.capacity()+grow);
 		m_ent.trim(n);
 	}   
	 	
 	assert(n<=30000);
	 	
 	m_ent.trim(n+1);
 	m_ent[n]=ent;
	 	
 	short index=n+1; 			//(index>0) 
 	if (ent.getPick())
 		addselection(index); //add to selection
	 	
 	return index; //ouput : index=indexarray+1
}

short DB1::addlayer(const Layer& layer){
	//input : layer.str;
	//ouput : negative index
	short n=(short)m_layer.size();
				
 	if (n>=m_layer.capacity()){ //Expand
 		const short grow=100;
 		m_layer.resize(m_ent.capacity()+grow);
 		m_layer.trim(n);
 	}   
	 	
 	m_layer.trim(n+1);
 	m_layer[n]=layer;
	 	
 	short index=(short)(-(n+1)); //negative indices !
 	m_layer[n].prec.ilayer=index;
	 	
 	return index;
}
void DB1::clear(){

	m_selection.resize(100);
	m_selection.trim(0);
	m_ent.resize(100);
	m_ent.trim(0);
	m_layer.resize(10);
	m_layer.trim(0);				

};


#ifndef NDEBUG
void DB1::Dump(void) const
{   
	short i;
	                             
	trace("DB[\n");
	trace("m_size=%g\n",m_size);       
	
	trace("m_selection.size()=%i\n",m_selection.size());       
	for (i=0; i<m_selection.size(); i++)
		trace("selection[%i]=%i\n",i,m_selection[i]);       	
		
	trace("m_ent.size()=%i\n",m_ent.size());       		
	for (i=0; i<m_ent.size(); i++){
		trace("m_ent[%i]=[\n",i);       	
		m_ent[i].Dump();
		trace("]\n",i);       	
	}
	
	trace("m_layer.size()=%i\n",m_layer.size());       			
	for (i=0; i<m_layer.size(); i++)
		trace("m_layer[%i]=(%i,%s)\n",i,m_layer[i].prec.ilayer,m_layer[i].prec.str);       		
	
    trace("]\n");

}
#endif

