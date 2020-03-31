#include "ofXML.h"

#include <vector>
#include <string>
#include <iostream>


//----------------------------------------
// a pretty useful tokenization system:
std::vector<std::string> tokenize(const std::string & str, const std::string & delim);
std::vector<std::string> tokenize(const std::string & str, const std::string & delim)
{
  using namespace std;
  vector<string> tokens;

  size_t p0 = 0, p1 = string::npos;
  while(p0 != string::npos)
  {
    p1 = str.find_first_of(delim, p0);
    if(p1 != p0)
    {
      string token = str.substr(p0, p1 - p0);
      tokens.push_back(token);
    }
    p0 = str.find_first_not_of(delim, p1);
  }
  return tokens;
}
//----------------------------------------




ofXML::ofXML(){
}

//---------------------------------------------------------
void ofXML::setVerbose(bool _verbose){
}

//---------------------------------------------------------
void ofXML::clear(){
	doc.Clear();
}

//---------------------------------------------------------
bool ofXML::loadFile(string xmlFile){
	bool loadOkay = doc.LoadFile(xmlFile.c_str());
	return loadOkay;
}

//---------------------------------------------------------
void ofXML::saveFile(string xmlFile){
	doc.SaveFile(xmlFile.c_str());

}

/*//---------------------------------------------------------
void ofXML::clearTagContents(string tag){
}

//---------------------------------------------------------
void ofXML::removeTag(string tag){
	
}*/

//---------------------------------------------------------
int ofXML::getValue(string tag, int defaultValue){
	char * tempStr = new char[MAX_TAG_VALUE_LENGTH_IN_CHARS];
	memset(tempStr, 0, MAX_TAG_VALUE_LENGTH_IN_CHARS);
	int returnValue = defaultValue;

	if (readTag(tag, tempStr)){
		returnValue = atoi(tempStr);
	} 
	delete tempStr;
	return returnValue;
}

//---------------------------------------------------------
float ofXML::getValue(string tag, float defaultValue){
	char * tempStr = new char[MAX_TAG_VALUE_LENGTH_IN_CHARS];
	memset(tempStr, 0, MAX_TAG_VALUE_LENGTH_IN_CHARS);
	float returnValue = defaultValue;

	if (readTag(tag, tempStr)){
		returnValue = atof(tempStr);
	} 
	delete tempStr;
	return returnValue;
}

//---------------------------------------------------------
string ofXML::getValue(string tag, string defaultValue){
	
	// lots of char *, string kung-fu here...
	
	char * tempStr = new char[MAX_TAG_VALUE_LENGTH_IN_CHARS];
	memset(tempStr, 0, MAX_TAG_VALUE_LENGTH_IN_CHARS);
	char * returnPtr = (char *) defaultValue.c_str();
	if (readTag(tag, tempStr)){
		returnPtr = tempStr;
	} 
	string returnString(returnPtr);
	delete tempStr;
	return returnString;
}

//---------------------------------------------------------
bool ofXML::readTag(string  tag, char * valueString){

	std::vector<std::string> tokens = tokenize(tag,":");
		
	
	TiXmlHandle docHandle(&doc);
	TiXmlHandle tagHandle = docHandle;
	for(int x=0;x<tokens.size();x++){
		tagHandle = tagHandle.FirstChildElement( tokens.at(x) );
	}

	// once we've walked, let's get that value...
	TiXmlHandle valHandle = tagHandle.Child( 0 );
    
    //now, clear that vector! 
	tokens.clear();		
	
    // if that value is really text, let's get the value out of it ! 
    if (valHandle.Text()){
    	int maxLen = MIN(MAX_TAG_VALUE_LENGTH_IN_CHARS, strlen(valHandle.Text()->Value()));
    	memcpy(valueString, valHandle.Text()->Value(), maxLen);
    	return true;	
    }  else {
		return false;
	}
}




//---------------------------------------------------------
void ofXML::addTag(string  tag, char * valueStr){
	
	std::vector<std::string> tokens = tokenize(tag,":");
	
	// allocate then clean up :
	
	TiXmlElement ** elements = new TiXmlElement*[tokens.size()];
	for(int x=0;x<tokens.size();x++){
		elements[x] = new TiXmlElement(tokens.at(x));
	}
	
	TiXmlText Value(valueStr);			
	
	// search our way up - do these tags exist?
	// find the first that DOESNT exist, then move backwards...
	TiXmlHandle docHandle(&doc);
	TiXmlHandle tagHandle = docHandle;
	
	for(int x=0;x<tokens.size();x++){
		
		TiXmlHandle isRealHandle  = tagHandle.FirstChildElement( tokens.at(x) );
		
		if (!isRealHandle.Node()){
			
			for(int i=tokens.size()-1;i>=x;i--){
				if (i == tokens.size()-1){
					elements[i]->InsertEndChild(Value);
				} else {
					elements[i]->InsertEndChild(*(elements[i+1]));
				}
			}
			
			
			
			tagHandle.ToNode()->InsertEndChild(*(elements[x]));
			
			break;
			
		} else {
			 tagHandle = isRealHandle;
			 if (x == tokens.size()-1){
				// what we want to change : TiXmlHandle valHandle = tagHandle.Child( 0 );
				tagHandle.ToNode()->Clear();
				tagHandle.ToNode()->InsertEndChild(Value);
			}
		}
	}
	
	//now, clear that vector! 
	tokens.clear();		
}
		

//---------------------------------------------------------
void ofXML::setValue(string tag, float 	value){
	char valueStr[255];
	sprintf(valueStr, "%f", value);
	addTag(tag, valueStr);
}

//---------------------------------------------------------
void ofXML::setValue(string tag, int	value){
	char valueStr[255];
	sprintf(valueStr, "%i", value);
	addTag(tag, valueStr);
}

//---------------------------------------------------------
void ofXML::setValue(string  tag, double 	value){
	char valueStr[255];
	sprintf(valueStr, "%d", value);
	addTag(tag, valueStr);
}

//---------------------------------------------------------
void ofXML::setValue(string tag, string value){
	addTag(tag, (char *)value.c_str());
	
}
