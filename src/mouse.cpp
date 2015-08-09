#include "../include/mouse.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "include/events.h"

CMouse::CMouse(EventManager* manager, int width, int height):
    evt_mgr(manager),
    last_mx(0), last_my(0), cur_mx(0), cur_my(0),
    dragging(false),
    pressed(false),
    windowWidth(width), windowHeight(height)
{}

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
            pressed = true;
            last_mx = cur_mx;
            last_my = cur_my;
        }
        else{
            pressed = false;
            if(dragging) dragging = false;
            //Normal clicks go here
            else{
                evt_mgr->queueEvent(new SelectionEvent(x, y));
            }
        }
    }
}

void CMouse::onMotion(int x, int y){
    cur_mx = x;
    cur_my = y;

    if((cur_mx != last_mx) || (cur_my != last_my)){
        if(pressed && !dragging) dragging = true;
        if(dragging){
            glm::vec3 a, b;

            a = getArcballVec3(last_mx, last_my);
            b = getArcballVec3(cur_mx, cur_my);

            float dot = glm::dot(a, b);
            if(dot > 1.0f) dot = 1.0f;
            else if(dot < -1.0f) dot = -1.0f;

            float angle = acos(dot);
            glm::vec3 axis = glm::cross(a, b);

            evt_mgr->queueEvent(new ArcballRotateEvent(angle, axis));
            last_mx = cur_mx;
            last_my = cur_my;
        }
    }
}

void CMouse::onScroll(double yoffset){
    evt_mgr->queueEvent(new ZoomEvent(yoffset));
}

void CMouse::wsize_changed(int width, int height){
    windowWidth  = width;
    windowHeight = height;
}
