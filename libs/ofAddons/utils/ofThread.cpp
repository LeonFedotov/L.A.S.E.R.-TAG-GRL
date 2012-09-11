#include "ofThread.h"

//-------------------------------------------------
ofThread::ofThread(){
	threadRunning = false;
}

//-------------------------------------------------
ofThread::~ofThread(){
	stopThread();
}

//-------------------------------------------------	
bool ofThread::isThreadRunning(){	
	//should be thread safe - no writing of vars here	
	if(threadRunning) return true;
	else return false;
}

//-------------------------------------------------	
void ofThread::startThread(bool _blocking, bool _verbose){
	
	//have to put this here because the thread can be running 
	//before the call to create it returns
	threadRunning	= true;		
	
	#ifdef TARGET_OSX
		pthread_mutex_init(&myMutex, NULL);		
		pthread_create(&myThread, NULL, thread, (void *)this);
	#else
		InitializeCriticalSection(&critSec);	
		myThread = (HANDLE)_beginthreadex(NULL, 0, this->thread,  (void *)this, 0, NULL);
	#endif	
	
	locked			= false;
	blocking		=	_blocking;
	verbose			= _verbose;
}

//-------------------------------------------------
//returns false if it can't lock
bool ofThread::lock(){
	if(!threadRunning){
		if(verbose)printf("ofThread: need to call startThread first\n");
		return false;
	}

	if(locked){
		if(verbose)printf("ofThread: already locked! \n");
		return false;
	}

	#ifdef TARGET_OSX
		if(!blocking){
			pthread_mutex_lock(&myMutex);
			if(verbose)printf("ofThread: we are in -- mutext is now locked \n");
		}else{
			int value = pthread_mutex_trylock(&myMutex);
			if(value == 0){
				if(verbose)printf("ofThread: we are in -- mutext is now locked \n");
			}
			else{
				if(verbose)printf("ofThread: mutext is busy locked =  %i \n",locked );
				return false;
			}
		}
	#else
			if(blocking)EnterCriticalSection(&critSec);
			else {
				if(!TryEnterCriticalSection(&critSec)){
					if(verbose)printf("ofThread: mutext is busy \n");
					return false;
				}
			}
			if(verbose)printf("ofThread: we are in -- mutext is now locked \n");
	#endif
	
	locked = true;
	return true;
}

//-------------------------------------------------	
bool ofThread::unlock(){

	if(!threadRunning){
		if(verbose)printf("ofThread: need to call startThread first\n");
		return false;
	}

	#ifdef TARGET_OSX
		pthread_mutex_unlock(&myMutex);
	#else
		LeaveCriticalSection(&critSec);
	#endif 
	
	locked = false;
	if(verbose)printf("ofThread: we are out -- mutext is now unlocked \n");
	
	return true;
}

//-------------------------------------------------	
void ofThread::stopThread(){
	if(threadRunning){
		#ifdef TARGET_OSX
			pthread_mutex_destroy(&myMutex);
			pthread_detach(myThread);
		#else
			CloseHandle(myThread);
		#endif
		if(verbose)printf("ofThread: thread stopped\n");
		threadRunning = false;
	}else{
		if(verbose)printf("ofThread: thread already stopped\n");
	}
}	

