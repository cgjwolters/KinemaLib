#ifndef H_WFILE
#define H_WFILE

#include <assert.h>

class CAbstractWFile {
	public:                            
	virtual int put(const char& c) =0;
 	virtual int put(const char * s,int n) =0;
 	
 	virtual int putInt16(const short& i){         
		assert(sizeof(short)==2); 	
 		return put((const char *)&i,sizeof(short));
 	}
 	virtual int putInt32(const long& l){
 		assert(sizeof(long)==4); 	
 		return put((const char *)&l,sizeof(long));
 	}
 	virtual int putUInt32(const unsigned long& l){
 		assert(sizeof(unsigned long)==4); 	
 		return put((const char *)&l,sizeof(unsigned long));
 	}
 	virtual int putDouble(const double& d){
 		return put((const char *)&d,sizeof(double));
 	}
};

#include <fstream>

using namespace std;

class CWFile: public CAbstractWFile  {
  CWFile(const CWFile& cp);
  CWFile& operator=(const CWFile& src);

	public:
	ofstream m_f;
	public:
	CWFile(const char *	filename)
	:m_f(filename,ios::out|ios::binary)
	{
	}
	~CWFile(void){
	 	if (m_f.is_open())
	 		m_f.close();
	}
	
	virtual int put(const char& c){
		m_f.put(c);
		return m_f.good();		
	}
 	virtual int put(const char * s,int n){
		m_f.write(s,n);		
		return m_f.good();		
 	}

};

#endif
#ifdef WIN32
#pragma comment(compiler)
#endif

