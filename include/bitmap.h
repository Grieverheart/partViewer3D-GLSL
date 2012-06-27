#ifndef __BITMAP_H
#define __BITMAP_H

#include <cstdio>
#include <windows.h>

const short BITMAP_MAGIC_NUMBER=19778;
const int RGB_BYTE_SIZE=3;

class Bitmap {
public:
	//variables
	RGBQUAD *colours;
	BYTE *pixelData;
	bool loaded;
	LONG width,height;
	WORD bpp;
	//methods
	Bitmap(void);
	Bitmap(const char *);
	~Bitmap();
	bool loadBMP(const char *);
private:
	//variables
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	//methods
	void reset(void);
};

#endif