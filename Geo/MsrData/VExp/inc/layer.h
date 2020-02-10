#ifndef H_LAYER
#define H_LAYER

typedef struct {
	char str[32];
	short ilayer;
} __reclayer;

class Layer {
	public: //!
	__reclayer prec;    

	protected:	
	void copy(const Layer* s){
		if (this!=s)
			memcpy(&prec,&(s->prec),sizeof(__reclayer));
	}         
	
	public:
	Layer(void){
		memset(&prec,0,sizeof(__reclayer));
	}	
	Layer(const Layer& a){
		copy(&a);
	}	
	Layer(const char * name){
		memset(&prec,0,sizeof(__reclayer));
		setString(name);		
	}	
	Layer& operator=(const Layer& a){
		copy(&a);
		return (*this);
	}	
	int operator==(const Layer& a){
		if (this!=&a)
			return (strncmp((const char*)prec.str,(const char*)&(a.prec.str),32)==0);
		return 1;
	}	
	
	void setString(const char * name){
		assert(name);
		strncpy((char*)prec.str,name,32);
		prec.str[31]=0;
	}	
};



#endif
#ifdef WIN32
#pragma comment(compiler)
#endif

