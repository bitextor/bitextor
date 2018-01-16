#ifndef __SYNC_OBJ_H__
#define __SYNC_OBJ_H__

#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <iostream>

class Mutex{
    private:
       mutable pthread_mutex_t mutex;

    public:
        Mutex(){pthread_mutex_init(&mutex,NULL);};
        ~Mutex(){pthread_mutex_destroy(&mutex);}
    
    public:
        inline void lock() const{pthread_mutex_lock(&mutex);};
        inline void unlock() const{pthread_mutex_unlock(&mutex);};
};

class SyncDouble{
private:
	double i;
	Mutex m;
public:
	SyncDouble(double d) {i=d;};
	SyncDouble() {i=0;};
	//inline operator const double()const{return i;}
	inline bool operator ==(const double& r) const{return i == r;};
	inline void operator +=(const double& r) {m.lock();i += r;m.unlock();};
	inline void operator +=(const SyncDouble& r) {m.lock();i += r.i;m.unlock();};
	inline void operator -=(const double& r) {m.lock();i -= r;m.unlock();};
	inline void operator *=(const double& r) {m.lock();i *= r;m.unlock();};
	inline void operator /=(const double& r) {m.lock();i /= r;m.unlock();};
	inline double operator =(const double& r) {m.lock();i = r;m.unlock();return i;};
	inline double operator =(const int& r) {m.lock();i = r;m.unlock();return i;};
	inline void operator ++() {m.lock();i++;m.unlock();};
	inline double operator +(const SyncDouble& r){return r.i+i;};
	inline double operator /(const SyncDouble& r){return i/r.i;};
	//inline void operator --() {m.lock();i--;m.unlock();};
	//inline const istream& operator<<(const istream& p)const{p<<i;return p;};
	friend  ostream& operator<<( ostream& p,const SyncDouble& d);
};

inline ostream& operator<<( ostream& p, const SyncDouble& d){p<<d.i;return p;};

#endif
