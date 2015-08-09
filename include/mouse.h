#ifndef __CMOUSE_H
#define __CMOUSE_H

#include <glm/glm.hpp>
#include "event_manager.h"

class OpenGLContext;

class CMouse{
public:
	CMouse(EventManager* manager, int width, int height);
	
	void onButton(int button, int state, int x, int y);
	void onMotion(int x, int y);
	void onScroll(double y);

    void wsize_changed(int width, int height);

private:
    EventManager* evt_mgr;
	int last_mx, last_my, cur_mx, cur_my;
	bool dragging;
	bool pressed;
	int windowWidth;
	int windowHeight;
	
	glm::vec3 getArcballVec3(int x, int y);
};
#endif
