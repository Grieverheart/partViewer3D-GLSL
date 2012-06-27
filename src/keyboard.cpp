#include "../include/keyboard.h"

CKeyboard::CKeyboard(OpenGLContext *context){
	keyRepeat = false;
	fullscreen = false;
	this->context = context;
}

CKeyboard::~CKeyboard(void){
}

void CKeyboard::keyIsDown(int key){
	keyMap[key]=true;
}

void CKeyboard::keyIsUp(int key){
	keyMap[key]=false;
}

void CKeyboard::keySpIsDown(int key){
	keySpMap[key]=true;
}

void CKeyboard::keySpIsUp(int key){
	keySpMap[key]=false;
}

void CKeyboard::keyOps(void){
	if(keyMap[27])exit(0);//exit when escape
	
	if(keyMap[13]){ //Enter
		int mod=glutGetModifiers();
		if(mod==GLUT_ACTIVE_ALT){
			if(fullscreen){
				glutReshapeWindow(600,600);
				glutSetWindowTitle("Project");
			}
			else{
				glutFullScreen();
				glutSetWindowTitle("Project Fullscreen");
			}
			fullscreen=!fullscreen;
		}
	}
	
	if(keyMap['z']){ //Enter
		int mod=glutGetModifiers();
		if(mod==GLUT_ACTIVE_ALT){
			context->hideGui();
		}
	}
	
	if(keyMap['b'])context->drawBox = !context->drawBox;//draw Box
}

void CKeyboard::keySpOps(void){
	if(keySpMap[GLUT_KEY_F1]){
		if(!keyRepeat)printf("Key Repeat Enabled\n");
		else printf("Key Repeat Disabled\n");
		keyRepeat=!keyRepeat;
		glutSetKeyRepeat(keyRepeat);
	}
}