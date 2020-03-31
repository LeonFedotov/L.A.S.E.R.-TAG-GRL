#ifndef _GUI_SETTING_
#define _GUI_SETTING_

#include "ofMain.h"
#include "ofAddons.h"

//float/int/bool settings control


class guiSetting
{
	public:
		
		guiSetting();
		
		void setGroupID(int groupID);
		void setValue(float val);
		void setValue(int val);
		
		float getValueF();
		int   getValueI();
		
		void setChanged(bool isChanged);
		bool getChanged();
		int  getGroupID();
		
		void setName(string xmlName, string displayName);
		
		void setIncr(float increment);
		void setIncr(int increment);
		void setMinMax( float mn, float mx);
		
		void increase();
		void decrease();
		
		string getXmlName();
		string getDisplayName();

	protected:
	
		float maxF;
		float minF;		
		float incrF;		
		float valueF;
				
		string name;
		string xml;
		int section;
		bool changed;
};

#endif