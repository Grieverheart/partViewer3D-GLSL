#include "include/opengl_3.h"
#include "include/keyboard.h"
#include "include/mouse.h"
#include <GLFW/glfw3.h>
#include <AntTweakBar.h>

OpenGLContext openglContext; // Our OpenGL Context Object
//CKeyboard keyboard(&openglContext);
//CMouse mouse(&openglContext);
////////////GLUT Keyboard Function Wrappers/////////////
//void keyDown(unsigned char key, int x, int y){
//	if(!TwEventKeyboardGLUT(key, x, y)){
//		keyboard.keyIsDown(key);
//		keyboard.keyOps();
//	}
//}
//
//void keyUp(unsigned char key, int x, int y){
//	keyboard.keyIsUp(key);
//}
//
//void specialDown(int key, int x, int y){
//	if(!TwEventSpecialGLUT(key, x, y)){
//		keyboard.keySpIsDown(key);
//		keyboard.keySpOps();
//	}
//}
//
//void specialUp(int key, int x, int y){
//	keyboard.keySpIsUp(key);
//}
//
////////////////GLUT Mouse Function Wrappers////////////////
//
//void onMouse(int button, int state, int x, int y){
//	mouse.onMouse(button,state,x,y);
//}
//
//void onMotion(int x, int y){
//	mouse.onMotion(x,y);
//}
//void onPassiveMotion(int x, int y){
//	mouse.onPassiveMotion(x, y);
//}
//////////////////////////////////////////////////////////

void reshape(GLFWwindow* window, int width, int height){
	openglContext.reshapeWindow(width,height);
}

void init(int argc, char *argv[]){
	openglContext.create30Context();
	openglContext.setupScene(argc,argv);
}

void glfw_error_callback(int error, const char* description){
    fputs(description, stderr);
}

int main(int argc,char *argv[] ){

    glfwSetErrorCallback(glfw_error_callback);

	if(!glfwInit()) return 1;

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(600, 600, "partviewerGLSL", NULL, NULL);
    if(!window){
        glfwTerminate();
        return 1;
    }

	glfwSetFramebufferSizeCallback(window, reshape);

    glfwMakeContextCurrent(window);
	
	init(argc,argv);

    while(!glfwWindowShouldClose(window)){
        //mouse.idleArcball();
        openglContext.processScene();
        openglContext.renderScene();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
	
	//glutKeyboardFunc(keyDown);
	//glutKeyboardUpFunc(keyUp);
	//glutSpecialFunc(specialDown);
	//glutSpecialUpFunc(specialUp);
	//glutMouseFunc(onMouse);
	//glutMotionFunc(onMotion);
	//glutPassiveMotionFunc(onPassiveMotion);

    glfwTerminate();
	
	return 1;
}
