#include "../include/main.h"
#include "../include/opengl_3.h"
#include "../include/keyboard.h"
#include "../include/mouse.h"

OpenGLContext openglContext; // Our OpenGL Context Object
CKeyboard keyboard(&openglContext);
CMouse mouse(&openglContext);
////////////GLUT Keyboard Function Wrappers/////////////
void keyDown(unsigned char key, int x, int y){
	keyboard.keyIsDown(key);
	keyboard.keyOps();
}

void keyUp(unsigned char key, int x, int y){
	keyboard.keyIsUp(key);
}

void specialDown(int key, int x, int y){
	keyboard.keySpIsDown(key);
	keyboard.keySpOps();
}

void specialUp(int key, int x, int y){
	keyboard.keySpIsUp(key);
}

//////////////GLUT Mouse Function Wrappers////////////////

void onMouse(int button, int state, int x, int y){
	mouse.onMouse(button,state,x,y);
}

void onMotion(int x, int y){
	mouse.onMotion(x,y);
}
//////////////////////////////////////////////////////////

void display(void){
	openglContext.renderScene();
}

void idle(void){
	mouse.idleArcball();
	openglContext.processScene();
	if(openglContext.redisplay){
		openglContext.redisplay = false;
		glutPostRedisplay();
	}
}

void reshape(int width, int height){
	openglContext.reshapeWindow(width,height);
}

void init(int argc, char *argv[]){
	openglContext.create30Context();
	openglContext.setupScene(argc,argv);
}

int main(int argc,char *argv[] ){

	glutInit(&argc,argv);
	
	init(argc,argv);
	
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	
	
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutSpecialFunc(specialDown);
	glutSpecialUpFunc(specialUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	
	glutMainLoop();
	
	return 1;
}