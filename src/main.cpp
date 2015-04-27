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
        switch(key){
        case GLFW_KEY_ESCAPE:
            if(action == GLFW_PRESS) running = false;
            break;
        case 'B':
            if(action == GLFW_PRESS) openglContext->drawBox = !openglContext->drawBox;
            break;
        default:
            break;
        }
	}
}

////////////////GLUT Mouse Function Wrappers////////////////

void on_mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    double x, y;
    glfwGetCursorPos(window, &x, &y);
	if(!TwEventMouseButtonGLFW(button, action)){
        mouse->onButton(button, action, (int)x, (int)y);
    }
}

void on_mouse_motion_callback(GLFWwindow* window, double x, double y){
	if(!TwEventMousePosGLFW(x, y)) mouse->onMotion((int)x, (int)y);
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

    int width  = 600;
    int height = 600;

    glfwSetErrorCallback(glfw_error_callback);

	if(!glfwInit()) return 1;

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(width, height, "partviewerGLSL", NULL, NULL);
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
	
    //TODO: Add try catch for OpenGLContext construction
    openglContext = new OpenGLContext(width, height);
    mouse = new CMouse(openglContext);

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
