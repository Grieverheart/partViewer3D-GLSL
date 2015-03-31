#include "../include/mouse.h"
#include <AntTweakBar.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "include/opengl_3.h"

CMouse::CMouse(OpenGLContext *context){
	this->context = context;
	last_mx=last_my=cur_mx=cur_my=0;
	dragging=false;
	LastRotMatrix = glm::mat4(1.0);
}

CMouse::~CMouse(void){
}

glm::vec3 CMouse::getArcballVec3(int x, int y){
	int screen_min=windowHeight;
	if(windowWidth<screen_min)screen_min=windowWidth;
	glm::vec3 P = glm::vec3((2.0f*x-windowWidth)/screen_min,
							(2.0f*y-windowHeight)/screen_min,
							0.0f);
	P.y = -P.y;
	float OP_squared = P.x * P.x + P.y * P.y;
	if(OP_squared <= 0.5f) P.z = sqrt(1.0f - OP_squared);
	else P.z=0.5f/sqrt(OP_squared);
	P = glm::normalize(P);
	
	return P;
}

void CMouse::idleArcball(void){

	glm::ivec2 screen = context->getScreen();
	windowWidth = screen.x;
	windowHeight = screen.y;
	
	if(cur_mx!=last_mx||cur_my!=last_my){
		glm::vec3 a,b;
		float dot=0.0f;
		
		a = getArcballVec3(last_mx,last_my);
		b = getArcballVec3(cur_mx,cur_my);
		
		dot = glm::dot(a,b);
		glm::vec3 axis = glm::cross(a,b);
		
		if(dot>1.0f)dot=1.0f;
		else if(dot<-1.0f)dot=-1.0f;
		float angle = acos(dot);
		
		context->trackballMatrix = glm::rotate(glm::mat4(1.0),angle,axis);
		context->trackballMatrix = context->trackballMatrix * LastRotMatrix;
	}
}

void CMouse::onMouse(int button, int state, int x, int y){
	if(!TwEventMouseButtonGLUT(button, state, x, y)){
		if(button==GLUT_LEFT_BUTTON){
			if(state==GLUT_DOWN){
				dragging=true;
				cur_mx=x;
				last_mx=cur_mx;
				cur_my=y;
				last_my=cur_my;
				LastRotMatrix = context->trackballMatrix;
			}
			else if(dragging){
				dragging=false;
				if(last_mx==x&&last_my==y){
				//Regular Clicking goes here//
				}
				last_mx=cur_mx;
				last_my=cur_my;
			}
		}
		if(button==3||button==4){
			float zoom = context->getZoom();
			zoom=zoom+(button*2-7)*1.5f; // put wheel up and down in one
			if(zoom<-58)zoom=-58.0f;
			else if(zoom>90)zoom=90.0f;
			context->setZoom(zoom);
		}
	}
}

void CMouse::onMotion(int x, int y){
	if(!TwEventMouseMotionGLUT(x, y)){
		if(dragging){
			cur_mx=x;
			cur_my=y;
		}
	}
}

void CMouse::onPassiveMotion(int x, int y){
	if(!TwEventMouseMotionGLUT(x, y)){
	}
}
