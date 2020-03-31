#ifndef _TRACK_PLAYER_
#define _TRACK_PLAYER_

#include "ofMain.h"
#include "ofAddons.h"

class trackPlayer{
	public:
		
		trackPlayer();
		
		int 	loadTracks(string directoryPath);
		bool 	playTrack(int _whichTrack);
		string 	getCurrentTrackName();
		int 	getCurrentTrackNo();
		int 	getNumTracks();
		void 	pause();
		void 	unPause();
		bool 	getFinished();
		int 	nextTrack();
		int 	prevTrack();
		void 	setVolume(int vol);
		void 	setPitch(float pitch);
		void 	updatePitch(float pct);
		void 	shiftPos(float posAdj);
			
	protected:
		int  numTracks, whichTrack;
		float targetPitch, updatePct, currentPitch;
	
		ofDirList DLIST;
		ofSoundPlayer track;
};

#endif