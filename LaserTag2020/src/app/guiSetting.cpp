#include "guiSetting.h"

//------------------------------------
guiSetting::guiSetting(){
	valueF 	= 0.0;
	incrF  	= 1.0;			
	maxF   	= 999999;
	minF   	= -999999;	
	section	= 0;
	changed = false;
}

//------------------------------------
void guiSetting::setGroupID(int groupID){
	section = groupID;
}

//------------------------------------
void guiSetting::setValue(float val){
	float tmp = valueF;

	valueF = val;
	
	if(tmp != valueF)changed = true;
}

//------------------------------------
void guiSetting::setValue(int val){		
	float tmp = valueF;

	valueF  = (float)val;
	
	if(tmp != valueF)changed = true;
}

//------------------------------------
float guiSetting::getValueF(){
	return valueF;
}

//------------------------------------
int guiSetting::getValueI(){
	return (int)valueF;
}		

//------------------------------------
void guiSetting::setChanged(bool isChanged){
	changed = isChanged;
}

//------------------------------------
bool guiSetting::getChanged(){
	
	bool rtnVal = false;
	
	if(changed){
		rtnVal  = true;
		changed = false;
	}
	
	return rtnVal;
}

//------------------------------------
int guiSetting::getGroupID(){
	return section;
}

//------------------------------------
void guiSetting::setName(string xmlName, string displayName){			
	xml 	= xmlName;
	name 	= displayName;
}

//------------------------------------
void guiSetting::setIncr(float increment)
{
	incrF = increment;
}

//------------------------------------
void guiSetting::setIncr(int increment)
{
	incrF = (float)increment;
}		

//------------------------------------
void guiSetting::setMinMax( float mn, float mx)
{
	maxF = mx;
	minF = mn;
}

//------------------------------------
void guiSetting::increase()
{
	float tmp = valueF;

	valueF += incrF;
	if(valueF >= maxF) valueF = maxF;
	
	if(tmp != valueF)changed = true;
}

//------------------------------------
void guiSetting::decrease()
{
	float tmp = valueF;

	valueF -= incrF;
	if(valueF <= minF) valueF = minF;
	
	if(tmp != valueF)changed = true;
}

//------------------------------------
string guiSetting::getXmlName(){
	return xml;
}

//------------------------------------
string guiSetting::getDisplayName(){
	return name;
}
