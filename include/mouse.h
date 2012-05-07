#ifndef __CMOUSE_H
#define __CMOUSE_H
#include "opengl_3.h"
class CMouse{
public:
	CMouse(OpenGLContext *context);
	~CMouse(void);
	
	void onMouse(int button, int state, int x, int y);
	void onMotion(int x, int y);
	
	void idleArcball(void);
private:
	OpenGLContext *context;
	int last_mx,last_my,cur_mx,cur_my;
	bool dragging;
	int windowWidth;
	int windowHeight;
	
	glm::mat4 LastRotMatrix;
	
	glm::vec3 getArcballVec3(int x, int y);
};
#endif