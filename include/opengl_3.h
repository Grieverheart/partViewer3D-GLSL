#ifndef __OPENGL_3_H
#define __OPENGL_3_H

#include "main.h"
#include "shader.h"
#include "obj_parser.h"
#include "coord_parser.h"
#include "mesh.h"

class OpenGLContext{
public:
	OpenGLContext(void); 				// Default constructor  
	~OpenGLContext(void); 				// Destructor for cleaning up our application  
	bool create30Context(void); 	// Creation of our OpenGL 3.x context  
	void setupScene(int argc, char *argv[]); 				// All scene information can be setup here  
	void reshapeWindow(int w, int h); 	// Method to get our window width and height on resize  
	void renderScene(void); 			// Render scene (display method from previous OpenGL tutorials)
	void processScene(void);
	
	void createSquare(void);
	
	float getZoom(void);
	void setZoom(float zoom);
	glm::ivec2 getScreen(void);
	
	bool redisplay;
	glm::mat4 trackballMatrix;
	
private:
	int windowWidth;	//Store the width of the window
	int windowHeight;	//Store the height of the window
	float fov, zoom;
	Shader *shader; // GLSL Shader
	
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 modelMatrix;
	
	int projectionMatrixLocation;
	int viewMatrixLocation;
	int modelMatrixLocation;
	int trackballMatrixLocation;
	
	bool use_dat;
	
	CObjParser objparser;
	CCoordParser coordparser;
	CMesh monkey;
};


#endif