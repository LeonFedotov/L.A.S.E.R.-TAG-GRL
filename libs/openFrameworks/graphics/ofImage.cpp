
#include "ofImage.h"
#include "FreeImage.h"


static bool bFreeImageInited = false;

//-----------------------------------------
void ofCloseFreeImage(){
	if (bFreeImageInited){
		FreeImage_DeInitialise();
		bFreeImageInited = false;
	}
}

//-----------------------------------------
class freeImageBitmap{

	public:

		FIBITMAP * bmp;

		// when OF users ask for pixels
		// we will get them contiguous memory
		// ie, non-aligned
		// see the pixel access chapter of the freeImage pdf
		// these pixel will also be RGB
		// on both platforms
		// bmp will be RGB / BGR depending
		// on the endianess of the platform.

		unsigned char * pixels;


		//-----------------------
		freeImageBitmap(){
			bmp = NULL;
		}

		//-----------------------
		~freeImageBitmap(){
			// currently we do all the memory deallocation 
			// in the main class ofImage
		}

		//-----------------------
		//swap my image out in a (hopefully) non-memory leakish way
		void swap(FIBITMAP *dst) {
			if(dst == NULL) 	return;
			if(bmp != NULL)		FreeImage_Unload(bmp);
			bmp = dst;
		}

		//-----------------------
		void setSize(FREE_IMAGE_TYPE image_type, WORD width, WORD height, WORD bpp) {
			if(bmp == NULL) {
				FreeImage_Unload(bmp);
			}
			bmp = FreeImage_AllocateT(image_type, width, height, bpp);
		}

		//-----------------------
		bool isValid(){
			return (bmp != NULL) ? true : false;
		}

};



//----------------------------------------------------------
ofImage::ofImage(){

	//----------------------- init free image if necessary
	if (!bFreeImageInited){
		FreeImage_Initialise();
		bFreeImageInited = true;
	}
	//-----------------------
	IMG 					= new freeImageBitmap();
	width = height = bpp = 0;
	type 					= OF_IMAGE_UNDEFINED;
	bUseTexture 			= true;		// the default is, yes, use a texture
	bAllocatedPixels		= false;
}

//----------------------------------------------------------
ofImage::~ofImage(){
	// memory cleanup
	//-------------------------------
	if(IMG->isValid()) {
		FreeImage_Unload(IMG->bmp);
	}
	//-------------------------------
	if (IMG->pixels != NULL && bAllocatedPixels == true){
		delete IMG->pixels;				// delete anything that might have existed
	}
	//-------------------------------
	if (IMG != NULL) delete IMG;
	width = height = bpp = 0;
	type 					= OF_IMAGE_UNDEFINED;
	bUseTexture 			= true;		// the default is, yes, use a texture
	bAllocatedPixels		= false;
	
	tex.clear();
	
}


//----------------------------------------------------------
void ofImage::grabScreen(int _x, int _y, int _w, int _h){
	if (IMG->isValid()){

		// flip y if origin is top:
		int height = ofGetHeight();
		_y = height - _y;
		_y -= _h; // top, bottom issues


		if (!(width == _w && height == _h)){
			resize(_w, _h);
		}
		unsigned char * pixels = (unsigned char *)FreeImage_GetBits(IMG->bmp);
		switch (bpp){
			case 8:
				glReadPixels(_x, _y, _w, _h, GL_LUMINANCE,GL_UNSIGNED_BYTE, pixels);
				break;
			case 24:

				#ifdef TARGET_LITTLE_ENDIAN
					glReadPixels(_x, _y, _w, _h, GL_BGR_EXT,GL_UNSIGNED_BYTE, pixels);
				#else
					glReadPixels(_x, _y, _w, _h, GL_RGB,GL_UNSIGNED_BYTE, pixels);
				#endif

				break;
			case 32:
				glReadPixels(_x, _y, _w, _h, GL_RGBA,GL_UNSIGNED_BYTE, pixels);
				break;
		}
		// we will need to flip images
		// if the 0,0 position is in the top left:
		FreeImage_FlipVertical(IMG->bmp);


		update();
	} else {
		// assume that this is color....
		IMG->setSize(FIT_BITMAP, _w, _h, 24);
		unsigned char * pixels = (unsigned char *)FreeImage_GetBits(IMG->bmp);

		// flip y if origin is top:
		int height = ofGetHeight();
		_y = height - _y;
		_y -= _h; // top, bottom issues


		#ifdef TARGET_LITTLE_ENDIAN
			glReadPixels(_x, _y, _w, _h, GL_BGR_EXT,GL_UNSIGNED_BYTE, pixels);
		#else
			glReadPixels(_x, _y, _w, _h, GL_RGB,GL_UNSIGNED_BYTE, pixels);
		#endif

		// we will need to flip images
		// if the 0,0 position is in the top left:
		FreeImage_FlipVertical(IMG->bmp);


		update();
	}
}

//----------------------------------------------------------
void ofImage::allocate(int _w, int _h, int _type){
	int newbpp;
	switch (_type){
		case OF_IMAGE_GRAYSCALE:
			newbpp = 8;
			break;
		case OF_IMAGE_COLOR:
			newbpp = 24;
			break;
		case OF_IMAGE_COLOR_ALPHA:
			newbpp = 32;
			break;
		default:
			newbpp = 24;
			break;
	}
	IMG->setSize(FIT_BITMAP, _w, _h, newbpp);
}

//----------------------------------------------------------
void ofImage::update(){

	width 		= FreeImage_GetWidth(IMG->bmp);
	height 		= FreeImage_GetHeight(IMG->bmp);
	bpp 		= FreeImage_GetBPP(IMG->bmp);


	// this is check to see if it's a gif
	bool bNoPallette 		= (FreeImage_GetColorType(IMG->bmp) != FIC_PALETTE);

	switch (bpp){
		case 8:
			if (bNoPallette) { //!bUsesPallette){
				type = OF_IMAGE_GRAYSCALE;
			} else {
				// 	convert to RGB -
				// 	this is a gif (or other) with 256 colors, not b&w.
				setImageType(OF_IMAGE_COLOR);
			}
			break;
		case 24:
			type = OF_IMAGE_COLOR;
			break;
		case 32:
			type = OF_IMAGE_COLOR_ALPHA;
			break;
		default:
			// if we've got here, something is wrong, the image
			// is not 8bit, 24bit or 32bit, so let's at least
			// make it color:
			setImageType(OF_IMAGE_COLOR);
			break;
	}

	//----------------------------------
	// now, since the "freeImage" memory is padded
	// (see freeImage392.pdf - "Pixel access functions")
	// lets get the data into contiguous memory - to make it easier
	// for folks to work with...

	if (IMG->pixels != NULL && bAllocatedPixels == true)
		delete IMG->pixels;				// delete anything that might have existed
	int byteCount = bpp / 8;		// bytes ber pixel (not bits per pixel)
	IMG->pixels = new unsigned char[width*height*byteCount];	//allocate that memory for the image (contiguously)
	bAllocatedPixels = true;
	FreeImage_ConvertToRawBits(IMG->pixels, IMG->bmp, width*byteCount, bpp, FI_RGBA_RED_MASK,FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);  // get bits

	#ifdef TARGET_LITTLE_ENDIAN
		if (type == OF_IMAGE_COLOR) 		swapChannels(false);
		if (type == OF_IMAGE_COLOR_ALPHA)   swapChannels(true);
	#endif
	//----------------------------------

	// if we are using a texture and the ofImage is upadate,
	// allocate (or reallocate) the texture and upload the data
	// I will upload from the contguous memory, and use pixelStorei to
	// make sure opengl knows I am contiguous.

	if (bUseTexture){
		switch (type){
			case OF_IMAGE_GRAYSCALE:
				tex.allocate(width,height,GL_LUMINANCE);
				tex.loadData(IMG->pixels, width,height, GL_LUMINANCE);
				break;
			case OF_IMAGE_COLOR:
				tex.allocate(width,height,GL_RGB);
				tex.loadData(IMG->pixels, width,height, GL_RGB);
				break;
			case OF_IMAGE_COLOR_ALPHA:
				tex.allocate(width,height,GL_RGBA);
				tex.loadData(IMG->pixels, width,height, GL_RGBA);
				break;
		}
	}




}

//----------------------------------------------------------
void ofImage::loadImage(string fileName){

	fileName = ofToDataPath(fileName);

	bool bLoaded = false;

	//----------------------------- find the format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	fif = FreeImage_GetFileType(fileName.c_str(), 0);
	if(fif == FIF_UNKNOWN) {
		// or guess via filename
		fif = FreeImage_GetFIFFromFilename(fileName.c_str());
	}
	if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
		if(IMG->isValid()) {
			FreeImage_Unload(IMG->bmp);
		}
		IMG->bmp = FreeImage_Load(fif, fileName.c_str(), 0);
		bLoaded = true;
		if(!IMG->isValid()){
			bLoaded = false;
		}
	}
	//-----------------------------

	if (bLoaded){

		// we will need to flip images
		// if the 0,0 position is in the top left:
		FreeImage_FlipVertical(IMG->bmp);


		update();
	} else {
		width = height = bpp = 0;
	}
}

//----------------------------------------------------------
void ofImage::saveImage(string fileName){

	fileName = ofToDataPath(fileName);

	if (IMG->isValid()){
		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType(fileName.c_str(), 0);
		if(fif == FIF_UNKNOWN) {
			// or guess via filename
			fif = FreeImage_GetFIFFromFilename(fileName.c_str());
		}
		if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
			// ok, if we are 0,0 top left, flip once, save and flip back
			// this is because the native format for freeImage is 0,0 lower left
			// there  be a smarter way to do this (ie, just draw upside down)
			// as opposed to continually
			FreeImage_FlipVertical(IMG->bmp);
			FreeImage_Save(fif, IMG->bmp, fileName.c_str(), 0);
			FreeImage_FlipVertical(IMG->bmp);

		}
	}
}



//------------------------------------
void ofImage::clone(ofImage &mom){
	
	// delete everything if we exist:
	if(IMG->isValid()) {
		FreeImage_Unload(IMG->bmp);
	}
	
	// clone:
	IMG->bmp = FreeImage_Clone(mom.IMG->bmp);
	
	// update:
	update();
	
	// (note: update call should delete pixels 
	// if pixels exist)
		
}


//----------------------------------------------------------
void ofImage::setFromPixels(unsigned char * pixels, int w, int h, int newType){

	// ok this RGB BGR stuff is biting back
	// now on a pc, we need to convert RGB to BGR for FreeImage_ConvertFromRawBits
	// otherwise in this system we will always be wrong.
	// in grabFrame notice the call to BGR_EXT that's doing the same thing as in here....

	// someone smarter then me is invited to take a look at solving all the various
	// BGR RGB issues once and for all...

	type 	= newType;
	bpp 	= 8;
	switch (newType){
		case OF_IMAGE_GRAYSCALE: 		bpp = 8; break;
		case OF_IMAGE_COLOR: 			bpp = 24; break;
		case OF_IMAGE_COLOR_ALPHA: 		bpp = 32; break;
	}


	if (type == OF_IMAGE_GRAYSCALE){
		int bytesPerPixel = bpp / 8;
		IMG->bmp = FreeImage_ConvertFromRawBits(pixels, w,h, w*bytesPerPixel, bpp, 0,0,0, true);
		update();
	} else {
		#ifdef TARGET_LITTLE_ENDIAN

			int bytesPerPixel = bpp / 8;
			unsigned char temp;
			int pos;
			//---------------------------- swap RGB to BGR
			for (int i = 0; i < w*h; i++){
				(type == OF_IMAGE_COLOR) ? pos = i * 3 : pos = i * 4;
				temp = pixels[pos  ];
				pixels[pos  ] = pixels[pos+2];
				pixels[pos+2] = temp;
			}

			IMG->bmp = FreeImage_ConvertFromRawBits(pixels, w,h, w*bytesPerPixel, bpp, 0,0,0, true);

			update();

			//---------------------------- back to RGB
			for (int i = 0; i < w*h; i++){
				(type == OF_IMAGE_COLOR) ? pos = i * 3 : pos = i * 4;
				temp = pixels[pos  ];
				pixels[pos  ] = pixels[pos+2];
				pixels[pos+2] = temp;
			}


		#else
			int bytesPerPixel = bpp / 8;
			IMG->bmp = FreeImage_ConvertFromRawBits(pixels, w,h, w*bytesPerPixel, bpp, 0,0,0, true);
			update();
		#endif
	}
}


//----------------------------------------------------------
unsigned char * ofImage::getPixels(){
	// be careful!	might not be allocated.  you need to check for null...
	if (IMG->isValid()){
		return (unsigned char *)IMG->pixels;
	} else {
		return NULL;
	}
}


//----------------------------------------------------------
void ofImage::resize(int newWidth, int newHeight){
	if (IMG->isValid()){
		FIBITMAP * scaledBmp = FreeImage_Rescale(IMG->bmp, newWidth, newHeight, FILTER_BICUBIC);
		IMG->swap(scaledBmp);
		update();
	}
}

//----------------------------------------------------------
void ofImage::setImageType(int newType){

	if (IMG->isValid()){
		switch (newType){
			//------------------------------------
			case OF_IMAGE_GRAYSCALE:
				if (type != OF_IMAGE_GRAYSCALE){
					FIBITMAP * convertedBmp = FreeImage_ConvertToGreyscale(IMG->bmp);
					IMG->swap(convertedBmp);
					type = OF_IMAGE_GRAYSCALE;
				}
				break;
			//------------------------------------
			case OF_IMAGE_COLOR:
				if (type != OF_IMAGE_COLOR){
					FIBITMAP * convertedBmp = FreeImage_ConvertTo24Bits(IMG->bmp);
					IMG->swap(convertedBmp);
					type = OF_IMAGE_COLOR;
				}
				break;
			//------------------------------------
			case OF_IMAGE_COLOR_ALPHA:
				if (type != OF_IMAGE_COLOR_ALPHA){
					FIBITMAP * convertedBmp = FreeImage_ConvertTo32Bits(IMG->bmp);
					IMG->swap(convertedBmp);
					type = OF_IMAGE_COLOR_ALPHA;
				}
				break;
		}
		update();
	}

}

//------------------------------------
void ofImage::swapChannels(bool bAlpha){


	// this swaps the red and blue channel of an image
	// turning an bgr image to rgb and vice versa...
	// needed because of windows byte order...


	if (IMG->isValid()){

		unsigned char * pixels = IMG->pixels;
		unsigned char temp;
		int totalPixels = width*height;
		// unoptimized for now...

		if (bAlpha == false){
			for (int i = 0; i < totalPixels; i++){
				temp = pixels[i*3 + 2];
				pixels[i*3 + 2] = pixels[i*3];
				pixels[i*3] = temp;

			}

		} else {
			for (int i = 0; i < totalPixels; i++){
				temp = pixels[i*4];
				pixels[i*4] = pixels[i*4 + 2];
				pixels[i*4 + 2] = temp;

			}
		}
	}
}

//------------------------------------
void ofImage::setUseTexture(bool bUse){
	bUseTexture = bUse;
}


//------------------------------------
void ofImage::draw(float _x, float _y, float _w, float _h){
	if (bUseTexture){
		tex.draw(_x, _y, _w, _h);
	}
}

//------------------------------------
void ofImage::draw(float x, float y){
	draw(x,y,width,height);
}
