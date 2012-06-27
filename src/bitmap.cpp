#include <iostream>
#include "../include/bitmap.h"

Bitmap::Bitmap(const char *file){
    reset();
	if(!loadBMP(file)) loaded = false;
	else loaded = true;
}

Bitmap::Bitmap(void){
    reset();
}

Bitmap::~Bitmap(void){
    if(colours!=0) {
        delete[] colours;
    }
    if(pixelData!=0) {
        delete[] pixelData;
    }
}

void Bitmap::reset(void){
    loaded = false;
    colours = 0;
    pixelData = 0;
}

bool Bitmap::loadBMP(const char *file){

	loaded = false;

	FILE *in;
	//Open file in binary mode
	in = fopen(file, "rb");
	if(!in){
		std::cout << "Could not open file " << file << std::endl;
		return false;
	}
	
	if(fread(&bmfh, sizeof(BITMAPFILEHEADER), 1, in) == 0){	
		std::cout << "Cannot read file " << file << std::endl;
		return false;
	}
	
	if(bmfh.bfType != BITMAP_MAGIC_NUMBER){
		std::cout << "Not a bitmap file" << std::endl;
		return false;
	}
	
	if(fread(&bmih, sizeof(BITMAPINFOHEADER), 1, in) == 0){
		std::cout << "Error reading Info Header" << std::endl;
		return false;
	}
	//Set the useful data
	width = bmih.biWidth;
	height = bmih.biHeight;
    bpp=bmih.biBitCount;
	
	//calculate the number of available colours
	int numColours = 1 << bmih.biBitCount;
	
	//load the palette for 8 bits per pixel
	if(bmih.biBitCount == 8){
		colours = new RGBQUAD[numColours];
		if(fread(colours, sizeof(RGBQUAD), numColours, in) == 0){
			std::cout << "Error reading Color palette" << std::endl;
			return false;
		}
	}
	
    //calculate the size of the image data
    DWORD dataSize = bmfh.bfSize - bmfh.bfOffBits;
	BYTE *tempPixelData = new BYTE[dataSize];
	//exit if there is not enough memory
    if(tempPixelData == NULL){
        std::cout << "Not enough memory to allocate a temporary buffer" << std::endl;
        fclose(in);
        return false;
    }
	
	if(fread(tempPixelData, sizeof(BYTE), dataSize, in) == 0){
		std::cout << "Error reading Image data" << std::endl;
		return false;
	}
	//close the file now that we have all the info
    fclose(in);
	
	//calculate the witdh of the final image in bytes
	LONG byteWidth, padWidth;
	byteWidth=padWidth=(LONG)((float)width * (float)bpp / 8.0);
	//add any extra space to bring each line to a DWORD boundary
	while(padWidth%4!=0) {
		padWidth++;
	}
	
	
	DWORD diff;
	LONG offset;
	
	diff = width * height * RGB_BYTE_SIZE;
    //allocate the buffer for the final image data
    pixelData = new BYTE[diff];

    //exit if there is not enough memory
    if(pixelData == NULL) {
        std::cout << "Not enough memory to allocate an image buffer" << std::endl;
        delete[] pixelData;
        return false;
    }

    if(height > 0) {
        unsigned int j = dataSize - 3;
        offset = padWidth - byteWidth;
        //count backwards so you start at the front of the image
        for(unsigned int i = 0; i < dataSize; i += 3) {
            //jump over the padding at the start of a new line
            if((i + 1) % padWidth == 0) {
                i += offset;
            }
            //transfer the data
            *(pixelData + j + 2) = *(tempPixelData + i);
            *(pixelData + j + 1) = *(tempPixelData + i + 1);
            *(pixelData + j) = *(tempPixelData + i + 2);
			j-=3;
        }
    }

    //image parser for a forward image
    else {
		height *= -1;
        offset = 0;
		do {
			memcpy((pixelData + (offset * byteWidth)), (tempPixelData + (offset * padWidth)), byteWidth);
			offset++;
        } while(offset < height);
    }
	
	delete[] tempPixelData;
	loaded = true;
	return loaded;
}