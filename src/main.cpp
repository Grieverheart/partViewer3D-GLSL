#include "include/scene.h"
#include "include/mouse.h"
#include "include/event_manager.h"
#include "include/events.h"
#include <GLFW/glfw3.h>
#include <AntTweakBar.h>
#include <cstdio>

bool running = true;

Scene* scene          = nullptr;
EventManager* evt_mgr = nullptr;
CMouse* mouse         = nullptr;
////////////GLUT Keyboard Function Wrappers/////////////
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    static int mode = 0;
	if(!TwEventKeyGLFW(key, action)){
        switch(key){
        case GLFW_KEY_ESCAPE:
            if(action == GLFW_PRESS) running = false;
            break;
        case 'B':
            if(action == GLFW_PRESS) scene->drawBox = !scene->drawBox;
            break;
        case 'O':
            if(action == GLFW_PRESS){
                if(mode == 0) scene->set_projection_type(Projection::ORTHOGRAPHIC);
                else scene->set_projection_type(Projection::PERSPECTIVE);
                mode = !mode;
            }
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
    if(evt_mgr) evt_mgr->queueEvent(new WindowSizeEvent(width, height));
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

    evt_mgr = new EventManager();
    //TODO: Add try catch for Scene construction
    scene   = new Scene(width, height);
    mouse   = new CMouse(evt_mgr, width, height);

    evt_mgr->addHandler([=](const Event& event){
        auto rotation_event = static_cast<const ArcballRotateEvent&>(event);
        scene->rotate(rotation_event.angle, rotation_event.axis);
    }, EVT_ARCBALL_ROTATE);

    evt_mgr->addHandler([=](const Event& event){
        auto selection_event = static_cast<const SelectionEvent&>(event);
        scene->select_particle(selection_event.x, selection_event.y);
    }, EVT_SELECTION);

    evt_mgr->addHandler([=](const Event& event){
        float dz = static_cast<const ZoomEvent&>(event).dz;
        scene->zoom(dz);
    }, EVT_ZOOM);

    evt_mgr->addHandler([=](const Event& event){
        auto wsize_event = static_cast<const WindowSizeEvent&>(event);
        scene->wsize_changed(wsize_event.width, wsize_event.height);
        mouse->wsize_changed(wsize_event.width, wsize_event.height);
    }, EVT_WINDOW_SIZE_CHANGED);


	scene->load_scene(parse_config(argv[1]));

    while(!glfwWindowShouldClose(window) && running){
        evt_mgr->processQueue();
        scene->process();
        scene->render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete evt_mgr;
    delete mouse;
    delete scene;

    glfwDestroyWindow(window);

    glfwTerminate();
	
	return 1;
}
