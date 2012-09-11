#include "ofDirList.h"

//----------------------------------------------------------
ofDirList::ofDirList()
{
	
	allowedFileExt = new char*[OF_DL_MAX_EXTS];
	nameArray = new char*[OF_DL_MAX_RESULTS];
	pathArray = new char*[OF_DL_MAX_RESULTS];
	
	for(int k = 0; k < OF_DL_MAX_EXTS; k++)
		allowedFileExt[k] = new char[OF_DL_EXT_SIZE];
	
	for(int k = 0; k < OF_DL_MAX_RESULTS; k++){
		nameArray[k] = new char[OF_DL_FILELEN];
		pathArray[k] = new char[OF_DL_FILELEN];
	}
	//an allowCount of 0 means no extensions are to be checked, 
	allowCount  = 0;
	verbose		= true;
	
}

//----------------------------------------------------------
void ofDirList::setVerbose(bool _verbose)
{
	verbose = _verbose;
}

//----------------------------------------------------------
bool ofDirList::allowExt(string ext)
{
	
	if(allowCount >= OF_DL_MAX_EXTS -1){if(verbose)printf("ofDirList - no of extensions > OF_DL_MAX_EXTS\n");   return false; }
	if(ext.length() >= OF_DL_EXT_SIZE){if(verbose)printf("ofDirList - extension is > OF_DL_EXT_SIZE\n");   return false; }

	sprintf(allowedFileExt[allowCount],"%s", ext.c_str());
	allowCount++;
		
	return true;
}

//----------------------------------------------------------
string ofDirList::getName(int pos)
{
	string str = "";

	if(pos >= count) return str;
	return str = nameArray[pos];
}

//----------------------------------------------------------
string ofDirList::getPath(int pos)
{
	string str = "";

	if(pos >= count) return str;
	return  str = pathArray[pos];
}

//----------------------------------------------------------
int ofDirList::listDir(string directory)
{
	printf("listing %s \n", directory.c_str());
	DIR *dir;
	char *extension;
	struct dirent *entry;
	bool skip = false;
	
	//if no dir is specified look in same folder as executable
	if(directory.length() == 0) dir = opendir(".");
	else dir = opendir(directory.c_str());

	if (dir == NULL){
		if(verbose)printf("ofDirList - error opening directory\n");
		return 0;
	}
	
	count = 0;
	while ((entry = readdir(dir)) != NULL && count < OF_DL_MAX_RESULTS) 
	{
		
		//make sure we are not looking at ./  or ../ on unix based systems
		char * pch;
		pch = (char*) memchr ((char *)entry->d_name, '.', 1);
		if(pch != NULL) continue;	
		
		//by default we don't skip files unless we are checking extensions
		skip = false;

		if(allowCount > 0)
		{
			//we will skip this files unless it has an allowed extension
			skip = true;
			for(int i = 0; i < allowCount; i++)
			{
				
				//if the wildecard * has been entered for an ext type then don't check any extensions
				if(allowedFileExt[i] == "*"){ skip = false; break; }
				
				//get the lengths of both the filename and the extention
				int extLen  = strlen(allowedFileExt[i]);
				int fileLen = strlen(entry->d_name);
				
				//the extension has to be shorter than the filename
				if(extLen >= fileLen) continue; 
				
				//lets check if there is a fullstop/peroid where we expect it to be
				if(entry->d_name[fileLen-extLen-1] != '.') continue; 
				
				//so now lets see if the extensions match
				bool checkExt = true;
				for(int j = 0; j < extLen; j++)
				{
					if(entry->d_name[fileLen-j] != allowedFileExt[i][extLen-j] ){checkExt = false; break; }
				}
				
				//they do match so we don't skip this entry
				if(checkExt == true){ skip = false; break;}
			}
		}

		if(skip) continue;		
		
		//if the file is too long for our string
		if(strlen(entry->d_name) >= OF_DL_FILELEN)
		{ 
			if(verbose)printf("ofDirList - file name > OF_DL_FILELEN\n"); 
			return 0; 
		}
		
		//Otherwise lets copy it to our results list
		
		
		//MAKE OUR PATH ARRAY
		//lets check that the full path can fit in the char * we have for it
		if(OF_DL_FILELEN > strlen(entry->d_name) + directory.length() )
		{
			//if some muppet forgot the trailing slash lets add it for them
			if(directory[ directory.length()-1] == '/') sprintf(pathArray[count],"%s%s",directory.c_str(),entry->d_name);
			else  sprintf(pathArray[count],"%s/%s",directory.c_str(),entry->d_name);
		}
		else
		{ 
			if(verbose)printf("ofDirList - path %s too long for string",entry->d_name);
			continue;
		}
		
		//MAKE OUR NAME ARRAY
		//lets check that the filename can fit in the char * we have for it
		if(OF_DL_FILELEN > strlen(entry->d_name) )sprintf(nameArray[count],entry->d_name);
		else 
		{
			if(verbose)printf("ofDirList - path %s too long for string",entry->d_name);
			continue;
		}
		
		
		if(verbose)printf("ofDirList - listing %s \n", nameArray[count]);
		count++;
	}
	
	if(verbose)printf("ofDirList - listed %i files in %s\n", count, directory.c_str());	
	return count;
}  

//Clean up
//----------------------------------------------------------
ofDirList::~ofDirList()
{
	delete pathArray;
	delete nameArray;
	delete allowedFileExt;
}
