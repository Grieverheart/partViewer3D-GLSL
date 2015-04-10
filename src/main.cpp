#include "include/opengl_3.h"
#include "include/mouse.h"
#include <GLFW/glfw3.h>
#include <AntTweakBar.h>
#include <cstdio>

bool running = true;
OpenGLContext* openglContext; // Our OpenGL Context Object
CMouse* mouse;
////////////GLUT Keyboard Function Wrappers/////////////
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if(!TwEventKeyGLFW(key, action)){
        if((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)){
            running = false;
        }
	}
}

////////////////GLUT Mouse Function Wrappers////////////////

void on_mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    double x, y;
    glfwGetCursorPos(window, &x, &y);
	if(!TwEventMouseButtonGLFW(button, action)){
        mouse->onButton(button, action, x, y);
    }
}

void on_mouse_motion_callback(GLFWwindow* window, double x, double y){
	if(!TwEventMousePosGLFW(x, y)) mouse->onMotion(x, y);
}

void scroll_callback(GLFWwindow* window, double x, double y){
	mouse->onScroll(y);
}

////////////////////////////////////////////////////////

void reshape(GLFWwindow* window, int width, int height){
	openglContext->reshapeWindow(width,height);
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
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, on_mouse_motion_callback);
    glfwSetMouseButtonCallback(window, on_mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwMakeContextCurrent(window);
	
    openglContext = new OpenGLContext();
    mouse = new CMouse(openglContext);

	openglContext->create30Context();
	openglContext->load_scene(parse_config(argv[1]));

    while(!glfwWindowShouldClose(window) && running){
        mouse->idleArcball();
        openglContext->processScene();
        openglContext->renderScene();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete mouse;
    delete openglContext;

    glfwDestroyWindow(window);

    glfwTerminate();
	
	return 1;
}
