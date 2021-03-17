#ifndef __OFXML__
#define __OFXML__

#include "ofConstants.h"
#include <string.h>
#include "tinyxml.h"

using namespace std;


#define MAX_TAG_VALUE_LENGTH_IN_CHARS		1024


class ofXML{

	public:
		ofXML();
		
		void setVerbose(bool _verbose);				
		
		bool loadFile(string xmlFile);
		void saveFile(string  xmlFile);
		
		// 	not done yet
		//	void clearTagContents(string tag);
		//	void removeTag(string  tag);
		
		void	clear();	// removes all tags from the ofXML object

		int 	getValue(string  tag, int  	 	defaultValue);
		float 	getValue(string  tag, float  	defaultValue);
		string 	getValue(string  tag, string 	defaultValue);

		void 	setValue(string  tag, float  	value);
		void 	setValue(string  tag, double  	value);
		void	setValue(string  tag, int    	value);
		void 	setValue(string  tag, string 	value);

		
		TiXmlDocument 	doc;
		bool 			bDocLoaded;
		
	protected:
	
		void 	addTag(string  tag, char * valueString);
		bool 	readTag(string  tag, char * valueString);	// max 1024 chars...
		
		
};

#endif