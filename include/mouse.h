#ifndef __CMOUSE_H
#define __CMOUSE_H

#include <glm/glm.hpp>
#include "event_manager.h"

class OpenGLContext;

class CMouse{
public:
	CMouse(OpenGLContext *context, EventManager* manager);
	~CMouse(void);
	
	void onButton(int button, int state, int x, int y);
	void onMotion(int x, int y);
	void onScroll(double y);
private:
	OpenGLContext *context;
    EventManager* evt_mgr;
	int last_mx, last_my, cur_mx, cur_my;
	bool dragging;
	int windowWidth;
	int windowHeight;
	
	glm::vec3 getArcballVec3(int x, int y);
};
#endif
