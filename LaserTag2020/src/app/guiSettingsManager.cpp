#include "guiSettingsManager.h"

// ------------------------
guiSettingsManager::guiSettingsManager(){
	
	index 			= 0;	//current setting
	section			= -1;	//current section
	selected 		= 0;    //which setting is selected?
	whichSection	= 0;	//which section are we in?
	
	for(int i = 0; i < MAX_SETTINGS; i++) bI[i] = false; 
}

// ------------------------
void guiSettingsManager::readFromFile(string filePath){
	//xml.setVerbose(false);
	xml.loadFile(filePath);
}

// ------------------------
void guiSettingsManager::addTitle(string title){
	section++;
	titles[section] = title;
}


// ------------------------
void guiSettingsManager::addSetting(string displayName, string xmlName, int value, int min, int max, int incr){
	bI[index] = true; //we know to display it as an int
	addSetting(displayName, xmlName, (float)value, (float)min, (float)max, (float)incr);
}

// ------------------------
void guiSettingsManager::addSetting(string displayName, string xmlName, float value, float min, float max, float incr){
	if(index >= MAX_SETTINGS)return;
	
	gs[index].setName(xmlName, displayName);
	gs[index].setValue(value);
	gs[index].setMinMax(min,max);
	gs[index].setIncr(incr);
	gs[index].setGroupID(section);
	index++;	
	
	//now we actually read the value from
	//the xml file into our setting
	updateValueFromXml(index-1);
}

// ------------------------
void guiSettingsManager::updateValueFromXml(int which){
	//if(which >= index)return;

	if(bI[which]){
		int tmp = gs[which].getValueI();
		tmp = xml.getValue(gs[which].getXmlName(), tmp);
		gs[which].setValue(tmp);				
	}else{
		float tmp = gs[which].getValueF();
		tmp = xml.getValue(gs[which].getXmlName(), tmp);
		gs[which].setValue(tmp);
	}			

	//don't want to trigger changed events from xml
	//gs[which].setChanged(false);

}

// ------------------------
void guiSettingsManager::reloadSettings(){
	for(int i = 0; i < index; i++){
		updateValueFromXml(i);
	}
}

// ------------------------
void guiSettingsManager::saveToFile(string filePath){
	
	for(int i = 0; i < index; i++){
	
		if(bI[i])xml.setValue(gs[i].getXmlName(), gs[i].getValueI());
		else xml.setValue(gs[i].getXmlName(), gs[i].getValueF());
	}
	
	xml.saveFile(filePath);
}

// ------------------------
int guiSettingsManager::getI(string varName){
	
	for(int i = 0; i < index; i++){
		if( varName == gs[i].getXmlName() ){
			return gs[i].getValueI();
		}
	}
	return 0;
}	

// ------------------------
float guiSettingsManager::getF(string varName){
	
	for(int i = 0; i < index; i++){
		if( varName == gs[i].getXmlName() ){
			return gs[i].getValueF();
		}
	}
	return 0;
}

// ------------------------
bool guiSettingsManager::hasChanged(string varName){
	for(int i = 0; i < index; i++){
		if( varName == gs[i].getXmlName() ){
			return gs[i].getChanged();
		}
	}
	return false;
}

// ------------------------
int guiSettingsManager::set(string varName, float value){
	
	for(int i = 0; i < index; i++){
		if( varName == gs[i].getXmlName() ){
			gs[i].setValue(value);
		}
	}
	return 0;
}	

// ------------------------
void guiSettingsManager::selectNext(){
	selected++;
	if(selected >= index)selected = selected-1;
	
	whichSection = gs[selected].getGroupID();
}

// ------------------------
void guiSettingsManager::selectPrev(){
	selected--;
	if(selected < 0)selected = 0;
	
	whichSection = gs[selected].getGroupID();
}

// ------------------------
void guiSettingsManager::increase(){
	gs[selected].increase();
}

// ------------------------
void guiSettingsManager::decrease(){
	gs[selected].decrease();
}

// ------------------------
void guiSettingsManager::drawSelected(float x, float y, float spacing){
	drawText( gs[selected].getDisplayName(), x, y);
	
	if(bI[selected]) handyString = ofToString(gs[selected].getValueI());
	else  handyString = ofToString(gs[selected].getValueF());
	
	handyString = "["+handyString+"]";
	
	drawText( handyString, x + spacing, y);
}

// ------------------------
void guiSettingsManager::drawCollapsedStyle(float x, float y, float spacing, float lineHeight){

	int pos = 0;	
	int k = 0;		
				
	for(int k = 0; k <= section; k++){
					
		pos++;
		float drawPosY = y + ( (float)pos*lineHeight);
		
		if(whichSection != k) glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
		else glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		drawText( titles[k], x, drawPosY);
		
		if(k == whichSection ){
			pos++;						
			for(int i = 0; i < index; i++){
				if(	whichSection == gs[i].getGroupID() ){
				
					drawPosY = y + ( (float)pos*lineHeight);	
										
					i != selected ? glColor4f(0.6f, 0.6f, 0.6f, 1.0f) : glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
					drawText( gs[i].getDisplayName(), x + 6, drawPosY);
				
					if(bI[i]) handyString = ofToString(gs[i].getValueI());
					else  handyString = ofToString(gs[i].getValueF());
					
					if(i == selected) {
						handyString = "["+handyString+"]";
						drawText( handyString, x + spacing, drawPosY);
					}else{
						drawText( handyString, x + spacing, drawPosY);
					}
					
					pos++;						
				}
			}
		}
	}
}
		
// ------------------------
void guiSettingsManager::drawAll(float x, float y, float spacing, float lineHeight){
	
	int pos = 0;	
	int k = -1;		
				
	for(int i = 0; i < index; i++){
		
		float drawPosY = y + ( (float)pos*lineHeight);
		
		//this is where we draw the titles
		if(k != gs[i].getGroupID() ){
			
			k = gs[i].getGroupID();
		
			pos++;
			drawPosY = y + ( (float)pos*lineHeight);
			
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			drawText( titles[k], x, drawPosY);

			pos++;
			drawPosY = y + ( (float)pos*lineHeight);
		}
		
		//this is where we draw the setting names
		//and their values
		i != selected ? glColor4f(0.6f, 0.6f, 0.6f, 1.0f) : glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		drawText( gs[i].getDisplayName(), x + 6, drawPosY);
	
		if(bI[i]) handyString = ofToString(gs[i].getValueI());
		else  handyString = ofToString(gs[i].getValueF());
		
		if(i == selected) {
			handyString = "["+handyString+"]";
			drawText( handyString, x + spacing, drawPosY);
		}else{
			drawText( handyString, x + spacing-1, drawPosY);
		}
		
		pos++;
	}
}
