#ifndef OF_DIRLIST
#define OF_DIRLIST

/************************************************************
																				
OpenFrameworks Library												
																			
File: 			ofDirList.h															
Description: 	List the contents of a directory
Notes:			Mac users need "../../../" to get to the folder where the app lies
																				
Last Modified: 2007.01.01											
Creator: Theodore Watson									

************************************************************/

#include "ofMain.h"
#include <dirent.h>

#ifdef TARGET_WIN32
	#include <stdio.h>
	#include <iostream>
	#include <string.h>
#endif

#define OF_DL_MAX_RESULTS 200
#define OF_DL_FILELEN 200
#define OF_DL_MAX_EXTS 100
#define OF_DL_EXT_SIZE 100

class ofDirList{
	
	public:
		ofDirList();
		~ofDirList();
		void setVerbose(bool _verbose);		
		string getName(int pos);									// returns requested filename
		string getPath(int pos);								// returns full path to file
		bool allowExt(string  ext);									// returns true if ext is accepted
		int listDir(string directory);								// returns number of files found

	private:
		char ** allowedFileExt;
		char ** nameArray;
		char ** pathArray;		
		int 	allowCount;
		int		count;
		bool	verbose;
};

#endif