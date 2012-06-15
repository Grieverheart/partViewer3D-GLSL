#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "main.h"
#include "opengl_3.h"

class CKeyboard{
public:
	CKeyboard(OpenGLContext *context);
	~CKeyboard(void);
	void keyOps(void);
	void keySpOps(void);
	void keyIsDown(int key);
	void keyIsUp(int key);
	void keySpIsDown(int key);
	void keySpIsUp(int key);
private:
	bool keyMap[256];
	bool keySpMap[256];
	bool keyRepeat;
	bool fullscreen;
	OpenGLContext *context;
};

void keyDown(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);
void specialDown(int key, int x, int y);
void specialUp(int key, int x, int y);
#endif