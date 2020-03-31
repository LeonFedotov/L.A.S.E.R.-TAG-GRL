#ifndef _OF_THREAD_H_
#define _OF_THREAD_H_

#include "ofConstants.h"

#ifdef TARGET_OSX
    #include <pthread.h>
    #include <semaphore.h>
#else
	#include <process.h>
#endif

class ofThread{

	public:
		ofThread();
		~ofThread();
		bool isThreadRunning();
		void startThread(bool _blocking = true, bool _verbose = true);
		bool lock();
		bool unlock();
		void stopThread();
			
	protected:
	
		//-------------------------------------------------
		//you need to overide this with the function you want to thread
		virtual void threadedFunction(){
			if(verbose)printf("ofThread: overide threadedFunction with your own\n");
		}

		//-------------------------------------------------
		
		#ifdef TARGET_OSX
			static void * thread(void * objPtr){
				ofThread* me	= (ofThread*)objPtr;
				me->threadedFunction();
				return 0;
			}
		#else
			static unsigned int __stdcall thread(void * objPtr){
				ofThread* me	= (ofThread*)objPtr;
				me->threadedFunction();
				return 0;
			}
		#endif	

			
	#ifdef TARGET_OSX
		 pthread_t        myThread;
		 pthread_mutex_t  myMutex;
	#else
		HANDLE            myThread;  
		CRITICAL_SECTION  critSec;  	//same as a mutex 
	#endif
	
	bool threadRunning;
	bool locked;
	bool blocking;
	bool verbose;
};

#endif
