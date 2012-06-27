#ifndef __OPENGL_3_H
#define __OPENGL_3_H

#include "main.h"
#include "shader.h"
#include "obj_parser.h"
#include "coord_parser.h"
#include "mesh.h"
#include "light.h"
#include "g-buffer.h"
#include "ssao.h"
#include "gui.h"

class OpenGLContext{
public:
	OpenGLContext(void); 				// Default constructor  
	~OpenGLContext(void); 				// Destructor for cleaning up our application  
	bool create30Context(void); 	// Creation of our OpenGL 3.x context  
	void setupScene(int argc, char *argv[]); 				// All scene information can be setup here  
	void reshapeWindow(int w, int h); 	// Method to get our window width and height on resize  
	void renderScene(void); 			// Render scene (display method from previous OpenGL tutorials)
	void processScene(void);
	void drawPass(void);
	void fboPass(void);
	void ssaoPass(void);
	
	void drawConfiguration(void);
	void drawConfigurationBox(void);
	void mouseListener(int x, int y);
	void hideGui(void);
	
	float getZoom(void);
	void setZoom(float zoom);
	glm::ivec2 getScreen(void);
	
	bool redisplay;
	bool drawBox;
	glm::mat4 trackballMatrix;
	
private:
	int windowWidth;	//Store the width of the window
	int windowHeight;	//Store the height of the window
	float fov, zoom;
	float znear,zfar;
	int m_click_x, m_click_y;
	bool m_clicked;
	bool m_gui_functional;
	
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 modelMatrix;
	glm::mat4 lightViewMatrix;
	glm::mat4 IdentityMatrix;
	glm::mat4 lightProjectionMatrix;
	glm::vec3 m_bgColor;
	
	int MVPMatrixLocation;
	int ModelViewMatrixLocation;
	int NormalMatrixLocation;
	int DepthMapLocation;
	int NormalMapLocation;
	int ColorMapLocation;
	int projABLocation;	// projA and projB are two uniforms needed to convert the post projective depth to the linear depth
	int invProjMatrixLocation;
	int ssaoProjMatrixLocation;
	int ssaoDepthMapLocation;
	int ssaoNormalMapLocation;
	int ssaoprojABLocation;
	int ssaoinvProjMatrixLocation;
	int aoSamplerLocation;
	int texelSizeLocation;
	int bgColorLocation;
	
	bool use_dat;
	bool m_fboInit;
	
	CLight light;
	CObjParser objparser;
	CCoordParser coordparser;
	CMesh mesh;
	CMesh full_quad;
	CGBuffer m_gbuffer;
	Cssao m_ssao;
	Cgui m_gui;
	
	Shader *sh_gbuffer; // GLSL Shader
	Shader *sh_ssao;
	Shader *sh_blur;
	Shader *sh_accumulator;
};


#endif