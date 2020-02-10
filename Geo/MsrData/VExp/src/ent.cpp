#include "ent.h"
#include "trace.h"


//DEFINES
#pragma pack(1) //BYTE PACKING
//
// Version 1
// 

// datatypes 	sizeof      
// 	char             1 
//  short			 2 
//  long int		 4 
//  float            4 
//  double           8 
//  BYTE			 1 
//  COLORREF		 4 

//
// Version 1
// 
struct rectv {
	char  type;     
	short ivers;    
};
struct recglobal1 {
	COLORREF color;
	BYTE pick,vis;
	short layer,free;
	float width;
};
struct recpoint1 {
	double coord[3];
};
struct recline1 {
	char dir;
	BYTE style;
	double coord[3],coord2[3];
};
struct recarc1 {
	char dir;
	BYTE style;
	double coord[3],radius,angs[2],rot[2];
};
struct rectext1 {
	float size,coord[3],angle;
	float rot[2];
	char text[41];
};

#pragma pack() 

//IMPLEMENTATION class Entity
int Entity::save(CAbstractWFile* ps){      
	switch (prec.type){
	case TPOINT: return savePoint(ps); 	
	case TLINE: return saveLine(ps); 	
	case TARC: return saveArc(ps); 		
	case TTEXT: return saveText(ps); 		
	default: return 0;
	}
}

int Entity::saveHeader(CAbstractWFile* ps)
{   
	const int version=1;
	rectv tv;
	tv.type=(char)prec.type;
	tv.ivers=version;
	return ps->put((const char*)&tv,sizeof(tv));	
}

int Entity::saveAttrib(CAbstractWFile* ps)
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

int Entity::zeropad(CAbstractWFile* ps)
{
	//write 20 zero chars   
	char zero=0;
	for (int i=0; i<20; i++)
		if (!ps->put(zero)) return 0;
	return 1;			
}
int Entity::savePoint(CAbstractWFile* ps)
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

int Entity::saveLine(CAbstractWFile* ps)
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

int Entity::saveArc(CAbstractWFile* ps)
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

int Entity::saveText(CAbstractWFile* ps)
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

void Entity::Dump(void) const {
return;

#ifdef NEVER
	trace("Entity[\n");
	trace("type=%i\n",(int)prec.type);
	trace("pick=%i\n",(int)prec.pick);
	trace("layer=%i\n",(int)prec.layer);
	trace("color=%li\n",(long)prec.color);

	switch (prec.type){
	case TPOINT: 
		trace("point(%g,%g,%g)\n",
		prec.p.coord[0],prec.p.coord[1],prec.p.coord[2]);
	break;
	case TLINE: 
		trace("line(dir=%i,style=%i)\n",prec.l.dir,prec.l.style);
		trace("line(%g,%g,%g,%g,%g,%g)\n",
		prec.l.coord[0],prec.l.coord[1],prec.l.coord[2],
		prec.l.coord2[0],prec.l.coord2[1],prec.l.coord2[2]);
	break;
	case TARC:                             
		trace("arc(dir=%i,style=%i)\n",prec.a.dir,prec.a.style);
		trace("arc(%g,%g,%g,%g,%g,%g,%g,%g)\n",
		prec.a.coord[0],prec.a.coord[1],prec.a.coord[2],
		prec.a.radius,
		prec.a.angs[0],prec.a.angs[1],
		prec.a.rot[0],prec.a.rot[1]);
	break;
	case TTEXT: 
		trace("text(size=%g,(x,y,z)=(%g,%g,%g),rotzxy=(%g,%g,%g),s=%s)\n",
		prec.t.size,prec.t.coord[0],prec.t.coord[1],prec.t.coord[2],
		prec.t.angle,prec.t.rot[0],prec.t.rot[1],prec.t.text);
	break;
	default:  
	break;
	}
    trace("]\n");
#endif
}