//useful code for figuring out the best blend function to use

static int myArray[9] = {GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, 
GL_DST_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA };

static whichMode = 0;

static void useBlendMode(){
	
	int src = whichMode%9;
	int dst = whichMode/9;
		
	glBlendFunc(myArray[src], myArray[dst]);
	
} 

static void nextBlendMode(){
	
	int src = whichMode%9;
	int dst = whichMode/9;
		
	printf("no: %i src is %i dst is %i \n", whichMode, src, dst);
	
	if(whichMode >= 80){
		printf("Done \N\N");
		whichMode = 0;
	}else{
		whichMode++;
	}
	
} 