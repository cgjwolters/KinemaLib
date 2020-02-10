#ifndef H_ENT
#define H_ENT

#include <windows.h>  
#include <string.h>
#include "wfile.h"

typedef enum {
	 TNONE = 0,TPOINT=1,TLINE=2,TARC=3,TTEXT=7,
} Type;

typedef enum {
	LS_SOLID		=1,
	LS_DASH			=2,
	LS_DOT          =3,
	LS_DASHDOT      =4,
	LS_DASHDOTDOT   =5,
} LineStyle; //Windows linestyles

typedef struct {
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

/*          
	//attributes:
	char type 		(in enum Type=(1,2,3))
	COLORREF color  (=RGB(r,g,b))
	BYTE pick;		(IsInSelection?)
	BYTE vis;		(IsVisible?:=(!vis))
	int layer;		(index of db::layertable[].ilayer)
	int free;       (not used)
	float width;    (line width, default 0.0)
	long id;		(not used)
	
	//Curves:
    BYTE style;     (in enum LineStyle (1,...,5) )
    
	//path definition: 
	Line: 
		if (dir==1)
			x(t) = x1 + t* (x2-x1); (0<=t<=1)
		else (dir==-1)
			x(t) = x1 + (1-t)* (x2-x1); (0<=t<=1)
			--> x(t) = x2 + (t)* (x1-x2); 	(0<=t<=1)
					
	Arc:        
		matrix=RotationY(rot[1])*RotationX(rot[0]); //pure rotation
		
		0 <= angs[0] <= 2 * Pi();
		0 <= angs[1]-angs[0] <= 2 * Pi();		
		
		if (dir==1)
			fi=angs[0] + t* (angs[1]-angs[0]);	(0<=t<=1)
			x(fi)=matrix * (cos(fi),sin(fi),0)	+ (xc,yc,zc)
		else (dir==-1)		         
			fi=angs[0] + (1-t)* (angs[1]-angs[0]);	(0<=t<=1)
            x(fi)=matrix * (cos(fi),sin(fi),0)	+ (xc,yc,zc)

*/



class Entity {
	//simple wrapper
	
	public: //
	__recentity prec;    

	protected:	
	void copy(const Entity* s){
		if (this!=s)
			memcpy(&prec,&(s->prec),sizeof(__recentity));
	}         
	
	
	
	//STREAMS
	int saveHeader(CAbstractWFile* pstream);
	int saveAttrib(CAbstractWFile* pstream);
	int savePoint(CAbstractWFile* pstream);
	int saveLine(CAbstractWFile* pstream);
	int saveArc(CAbstractWFile* pstream);
	int saveText(CAbstractWFile* ps);
	int zeropad(CAbstractWFile* ps);
		
	public:
	Entity(void){
		init();
	}	
	Entity(const Entity& a){
		copy(&a);
	}	
	Entity& operator=(const Entity& a){
		copy(&a);
		return (*this);
	}	
	int operator==(const Entity& a){
		if (this!=&a)
			return (memcmp(&prec,&(a.prec),sizeof(__recentity))==0);
		return 1;
	}	
	
	//////////////////////////////////////////////////////////////////////////////////////////
	
	void Dump(void) const;
	int save(CAbstractWFile* pstream);
	
	//////////////////////////////////////////////////////////////////////////////////////////
	
	//Attributes
	int getPick(void) const { 
		return prec.pick;
	}
	void setPick(const int pick) { 
		prec.pick=(BYTE)pick;
	};
	int getVis(void) const { 
		return !prec.vis;
	}
	void setVis(const int vis) { 
		prec.vis=(!vis);
	};  
	int getLayer(void) const { 
		return prec.layer;
	}
	void setLayer(const int layer) { 
		prec.layer=layer;
	};
	
	COLORREF getColor(void) const { 
		return prec.color;
	}
	void setColor(const COLORREF color) { 
		prec.color=color;
	};
	
	//etc
	
	//////////////////////////////////////////////////////////////////////////////////////////

	void init(void){
		memset(&prec,0,sizeof(__recentity));
	}
	
	void makePoint(const double x,const double y,const double z){
	    //init();
	    prec.type=(char)TPOINT;
		prec.p.coord[0]=x; 		
		prec.p.coord[1]=y; 		
		prec.p.coord[2]=z; 		
	}
	
	void makeLine(const double x1,const double y1,const double z1,
				  const double x2,const double y2,const double z2){
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
	
	void makeArc(const double xc,
				 const double yc,
				 const double zc,
				 const double r,
				 const int ccw,
				 const double fi1,
				 const double fi2,
				 const double rotx,
				 const double roty)
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

	void makeText(double size,
				  double x,double y,double z,
				  double angleZ,
				  double angleX,
				  double angleY,
				  char * text){
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
	
};

#endif

