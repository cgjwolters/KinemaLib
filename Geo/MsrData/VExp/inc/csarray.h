#ifndef H_CARRAY
#define H_CARRAY

#include <assert.h>
#include <stddef.h>
#include "trace.h"

template<class TYPE>
class CArray {
	protected:
	long m_size;		//dim size
	long m_capacity;		//capacity
	TYPE* m_p;     		//data pointer of type TYPE
	
	protected:
	void copy(const CArray<TYPE>* s,int deep=1){
		assert(s!=NULL);
		if (this!=s){
			resize(s->size());
			trim(s->size());
			for (long i=0; i<m_size; i++) 
 				operator[](i)= s->operator[](i);
 		}	
	};	
	
	public:
	CArray(void)
	: m_size(0),m_capacity(0),m_p(NULL)
	{};
	CArray(const long size)
	: m_size(0),m_capacity(0),m_p(NULL)
	{
		resize(size);
	};
	CArray(const CArray<TYPE>& a)
	: m_size(0),m_capacity(0),m_p(NULL)
	{
		copy(&a,1);
	};
	virtual ~CArray(void){ 
		if (m_p) delete [] m_p;
	};
	
	CArray<TYPE>& operator=(const CArray<TYPE>& a){
		copy(&a,1);
		return (*this);	
	};
/*
	int operator==(const CArray<TYPE>& a){
		if (this==&a) return 1;
		if (m_size!=a.size()) return 0;	
		for (long i=0; i<m_size; i++) 
 			if (!(operator[](i)==a.operator[](i))) return 0;
 		return 1;
	};
*/
	
    TYPE & operator[](long i){
#ifndef NDEBUG
		if ((i<0)||(i >=size())){
			trace("CArray<TYPE>::operator[%i]: index out of range\n",i);	
			assert(0);
		}
#endif	
		return m_p[i];
	};
    const TYPE & operator[](long i) const{
#ifndef NDEBUG
		if ((i<0)||(i >=size())){
			trace("CArray<TYPE>::operator[%i]: index out of range\n",i);	
			assert(0);
		}
#endif	
		return m_p[i];
	};
    
    int resize(long newsize){
		if (m_size==newsize){
    		if (m_size==m_capacity){
    			return 1;
			}
			//freeze
		} else {    
			if ( (0<=newsize) && (newsize<=m_capacity) ){
	  			m_size = newsize;
    			return 1;
			}
			//expand
		}
    
		TYPE* p=new TYPE [newsize]; 
		if (!p) return 0;
		for (long i=0; i<m_size; i++) p[i]=m_p[i];
		if (m_p) delete [] m_p;
		m_p=p;	
		m_size=m_capacity=newsize;
		return 1; 
	};
    int trim(long newsize){
		if ( (newsize >= 0) && (newsize <= m_capacity) ){
    		m_size = newsize;
    		return 1;
		}
		return 0;
	};  
    int freeze(void){
		return resize(m_size); 
	};
    
    long size(void) const{
		return m_size;
	};
    long capacity(void) const{
		return m_capacity;
	};
    
#ifndef NDEBUG
	virtual void Dump(void) const {
		trace("CArray<TYPE>[\n");
		trace("m_size=%li\n",m_size);
		trace("m_capacity=%li\n",m_capacity);
		trace("m_p=%p\n",m_p);
		trace("]\n");	
	};
#endif   
};

#endif

