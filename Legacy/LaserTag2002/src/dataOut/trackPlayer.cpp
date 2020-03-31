#include "trackPlayer.h"

// ------------------------		
trackPlayer::trackPlayer(){
	whichTrack = -1;
	numTracks  = 0;
}

//-------------------------
int trackPlayer::loadTracks(string directoryPath){
	DLIST.allowExt("mp3");
	numTracks = DLIST.listDir(directoryPath);
	return numTracks;
}

//-------------------------
bool trackPlayer::playTrack(int _whichTrack){
	if(numTracks == 0 || _whichTrack >= numTracks || _whichTrack < 0) return false;
	
	whichTrack = _whichTrack;
	
	targetPitch  = 1.0;
	currentPitch = 1.0;;
	
	track.loadSound(DLIST.getPath(whichTrack), true);
	track.play();
	
	return true;
}

//-------------------------
string trackPlayer::getCurrentTrackName(){
	if(numTracks == 0 || whichTrack >= numTracks || whichTrack < 0) return "";
	return DLIST.getName(whichTrack);
}

//-------------------------
int trackPlayer::getCurrentTrackNo(){
	return whichTrack;
}

//-------------------------
int trackPlayer::getNumTracks(){
	return numTracks;
}

//-------------------------
void trackPlayer::pause(){
	track.setPaused(true);
}

//-------------------------
void trackPlayer::unPause(){
	track.setPaused(false);
}

//-------------------------
bool trackPlayer::getFinished(){
	if(track.getPosition() >= 0.99){
		return true;
	}else{
		return false;
	}
}

//-------------------------
int trackPlayer::nextTrack(){
	whichTrack+=2;
	if(whichTrack >= numTracks)whichTrack = 0;
	playTrack(whichTrack);
	
	return whichTrack;
}

//-------------------------
int trackPlayer::prevTrack(){
	whichTrack--;
	if(whichTrack < 0)whichTrack = 0;
	playTrack(whichTrack);
	
	return whichTrack;
}

//-------------------------
void trackPlayer::setVolume(int vol){
	float volume = vol;
	if(volume > 100)volume = 100;
	volume *= 0.01;
	track.setVolume(volume);
}

//-------------------------
void trackPlayer::setPitch(float pitch){

	currentPitch = pitch;
	track.setSpeed(currentPitch);
}

//-------------------------
void trackPlayer::updatePitch(float pct){

	currentPitch *= pct;
	currentPitch += (1.0-pct);
	
	track.setSpeed(currentPitch);
}

//-------------------------
void trackPlayer::shiftPos(float posAdj){

	float currentPos = track.getPosition();
	
	currentPos += posAdj;
	
	
	
	if(currentPos <= 0)currentPos = 0.01;
	if(currentPos >= 1)currentPos = 0.9999999;
	
	printf("pos is %f \n", currentPos);

	track.setPosition(currentPos);
}

