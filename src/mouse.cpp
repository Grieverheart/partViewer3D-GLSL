#include "../include/mouse.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "include/opengl_3.h"
#include "include/events.h"

CMouse::CMouse(OpenGLContext *context_, EventManager* manager):
    context(context_), evt_mgr(manager),
    last_mx(0), last_my(0), cur_mx(0), cur_my(0),
    dragging(false)
{}

CMouse::~CMouse(void){
}

glm::vec3 CMouse::getArcballVec3(int x, int y){
    int screen_min = windowHeight;
    if(windowWidth < screen_min) screen_min = windowWidth;

    glm::vec3 P = glm::vec3((2.0f * x - windowWidth) / screen_min,
                            (windowHeight - 2.0f * y) / screen_min,
                             0.0f);

    float OP_squared = P.x * P.x + P.y * P.y;
    if(OP_squared <= 0.5f) P.z = sqrt(1.0f - OP_squared);
    else P.z = 0.5f / sqrt(OP_squared);
    P = glm::normalize(P);

    return P;
}

void CMouse::onButton(int button, int state, int x, int y){
    if(button == GLFW_MOUSE_BUTTON_LEFT){
        if(state == GLFW_PRESS){
            evt_mgr->queueEvent(new ArcballStartEvent());
            dragging = true;
            cur_mx = x;
            cur_my = y;
            last_mx = cur_mx;
            last_my = cur_my;
        }
        else if(dragging){
            dragging = false;
            evt_mgr->queueEvent(new ArcballEndEvent());
            if((last_mx == x) && (last_my == y)){
                evt_mgr->queueEvent(new SelectionEvent(x, y));
            }
        }
    }
}

void CMouse::onMotion(int x, int y){
    if(dragging){
        cur_mx = x;
        cur_my = y;

        glm::ivec2 screen = context->getScreen();
        windowWidth  = screen.x;
        windowHeight = screen.y;

        if((cur_mx != last_mx) || (cur_my != last_my)){
            glm::vec3 a, b;

            a = getArcballVec3(last_mx, last_my);
            b = getArcballVec3(cur_mx, cur_my);

            float dot = glm::dot(a, b);
            if(dot > 1.0f) dot = 1.0f;
            else if(dot < -1.0f) dot = -1.0f;

            float angle = acos(dot);
            glm::vec3 axis = glm::cross(a, b);

            evt_mgr->queueEvent(new ArcballRotateEvent(glm::rotate(glm::mat4(1.0), angle, axis)));
        }
    }
}

void CMouse::onScroll(double yoffset){
    evt_mgr->queueEvent(new ZoomEvent(yoffset));
}
