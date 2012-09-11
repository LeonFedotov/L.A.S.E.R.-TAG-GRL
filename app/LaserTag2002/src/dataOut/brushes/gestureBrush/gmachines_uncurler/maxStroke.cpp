#include "maxStroke.h"



//--------------------------------------------------------------
maxStroke::maxStroke(){
	nMaximumPts 	= 250;
	nPts 			= 0;
	input 			= new ofVec3f[nMaximumPts + 1];
	resampled		= new ofVec3f[nMaximumPts];
}


//--------------------------------------------------------------
void maxStroke::addPoint(ofVec3f pt){
	
	input[nPts] = pt;
	nPts++;;
	
	//if (nPts > 1) {
		resamplePts(input, nPts, resampled, nMaximumPts);	
	//}
	
	if (nPts == nMaximumPts+1){
		memcpy(input, resampled, nMaximumPts*sizeof(ofVec3f));
		nPts--;
	} 

	
}

//--------------------------------------------------------------
void maxStroke::draw(){
	ofSetColor(0xff0000);
	glBegin(GL_LINE_STRIP);
	
	
	if (nPts > 1){
	for (int i = 0; i < nMaximumPts; i++){
		glVertex2f(resampled[i].x, resampled[i].y);
	}
	}
	glEnd();
}


//---------------------------------------------------------------------
void maxStroke::resamplePts(	ofVec3f *input,  		int inNum,
						    	ofVec3f *output, 		int outNum){
	ofVec3f	  lower;
	ofVec3f	  upper;

	if(inNum == 0 || outNum == 0 ) return;

	// calculate length of input
	float inlen = 0;
	for (int i = 0; i < inNum-1; i++){
		inlen += (input[i] - input[i + 1]).length();
	}

	
	float	     RSL = inlen / (float)(outNum-1); // resampledSegmentLength
	float	     Dx, Dy, Dp = 0;
	float	     dx, dy, dp = 0;
	float	     px, py = 0;
	float 		 pp;
	float	     RSLdx, RSLdy;
	float 		 RSLdp;

	float	     segLength, ASL; // available segment length
	float	     remainder, neededSpace;
	float	     prevRemainder = RSL;
	int 	     nsegs;
	int 	     p=0;
	int	     	 nPtsToDo;
	int	     	 nResamples = 0;

    for (int i=0; i< inNum-1; i++){
      lower = input[i];
      upper = input[i+1];
      Dx = upper.x - lower.x;
      Dy = upper.y - lower.y;
      Dp = upper.z - lower.z; // using z for pressure
      if (Dx == 0 && Dy == 0){ Dx = 0.0001f; Dy = 0.0001f; }
      segLength = (float) sqrt (Dx*Dx + Dy*Dy);
      ASL = segLength;	// available segment length
      dx = (Dx/segLength);	// unit vector components
      dy = (Dy/segLength);
      dp = (Dp/segLength);
      RSLdx = dx * RSL;	// resampled segment vector components
      RSLdy = dy * RSL;
      RSLdp = dp * RSL;

      neededSpace = RSL-prevRemainder;
      if (ASL >= neededSpace){
		// if there is enough room to place the first point
		// then place the first resample point in the latest segment
		remainder = ASL;
		px = lower.x + (neededSpace*dx);
		py = lower.y + (neededSpace*dy);
		pp = lower.z + (neededSpace*dp);

	if (p < (outNum-1)){
	  output[nResamples++].set(px, py, pp);
	  remainder -= neededSpace;
	  p++;
	}

	nPtsToDo = (int)(remainder/RSL);
	for (int d=0; d<nPtsToDo; d++){
	  px += RSLdx;
	  py += RSLdy;
	  pp += RSLdp;
	  if (p < (outNum-1)){
	    output[nResamples++].set(px, py, pp);
	    remainder -= RSL;
	    p++;
	  }
	}
	prevRemainder = remainder;


      } else {
	// if there is not enough room to place the first point
	prevRemainder += ASL;
      }
    }
    upper = input[inNum-1];
    output[nResamples++].set(upper.x, upper.y, upper.z);
  


	while(nResamples < outNum){
		output[nResamples].x = upper.x;
		output[nResamples].y = upper.y;
		output[nResamples].z = upper.z;
		nResamples++;
	}

}
