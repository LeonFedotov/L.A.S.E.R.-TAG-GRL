#ifndef _GUI_SETTING_MANAGER_
#define _GUI_SETTING_MANAGER_

#include "ofMain.h"
#include "ofAddons.h"
#include "guiSetting.h"
#include "baseGui.h"

#define MAX_SETTINGS 100

class guiSettingsManager : public baseGui
{
	public:
		
		guiSettingsManager();
		
		void readFromFile(string filePath);
		void addTitle(string title);
		
		void addSetting(string displayName, string xmlName, int value, int min, int max, int incr);
		void addSetting(string displayName, string xmlName, float value, float min, float max, float incr);
		
		void updateValueFromXml(int which);
		void reloadSettings();
		void saveToFile(string filePath);
		
		int  getI(string varName);
		float getF(string varName);
		bool hasChanged(string varName);
		int set(string varName, float value);
		
		void selectNext();
		void selectPrev();
		void increase();
		void decrease();
		
		void drawSelected(float x, float y, float spacing);
		void drawCollapsedStyle(float x, float y, float spacing, float lineHeight);
		void drawAll(float x, float y, float spacing, float lineHeight);
		
		
	protected:
		
		ofXML 			xml;
		guiSetting 		gs[MAX_SETTINGS];
		bool			bI[MAX_SETTINGS];
		string 			handyString;
		string			titles[MAX_SETTINGS];
		int 			selected, index, section, whichSection;
	
};

#endif