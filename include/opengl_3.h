#ifndef __OPENGL_3_H
#define __OPENGL_3_H

#include "obj_parser.h"
#include "coord_parser.h"
#include "mesh.h"
#include "light.h"
#include "g-buffer.h"
#include "ssao.h"
#include "shadowmap.h"
#include "perfmon.h"

class Shader;

class OpenGLContext{
public:
	OpenGLContext(void); 				// Default constructor  
	~OpenGLContext(void); 				// Destructor for cleaning up our application  
	bool create30Context(void); 	// Creation of our OpenGL 3.x context  
	void load_scene(const SimConfig& config);// All scene information can be setup here  
	void reshapeWindow(int w, int h); 	// Method to get our window width and height on resize  
	void renderScene(void); 			// Render scene (display method from previous OpenGL tutorials)
	void processScene(void);
	
	float getZoom(void)const;
	void setZoom(float zoom);
	glm::ivec2 getScreen(void)const;
	
	bool redisplay;
	bool drawBox;
	glm::mat4 trackballMatrix;
	
private:
	int windowWidth;	//Store the width of the window
	int windowHeight;	//Store the height of the window
	float fov, zoom;
	float znear,zfar;
    float out_radius;
	
	glm::mat4 projectionMatrix;
	glm::mat4 invProjMatrix;
	glm::mat4 viewMatrix;
    glm::mat4 invViewMatrix;
	glm::mat4 modelMatrix;
    glm::mat4 lightProjectionMatrix;
    glm::mat4 lightViewMatrix;
	
	glm::vec3 m_bgColor;
	glm::vec3 diffcolor;
	glm::vec3 skycolor;
	
	unsigned int mNInstances;
	
	unsigned int vaoBox;
	unsigned int vboBox;
	unsigned int iboBox;
	
	bool is_scene_loaded;
	bool m_fboInit;
	bool m_blur;
	bool m_rotating;
	
	CLight light;
	CObjParser objparser;
	CMesh mesh;
	CMesh full_quad;
	CGBuffer m_gbuffer;
	Cssao m_ssao;
    CShadowmap m_shadowmap;
	
	Shader *sh_gbuffer; // GLSL Shader
	Shader *sh_gbuffer_instanced; // GLSL Shader
	Shader *sh_ssao;
	Shader *sh_shadowmap_instanced;
	Shader *sh_blur;
	Shader *sh_accumulator;

    PerfMon perf_mon;
	
	TwBar *bar;
	
	void drawConfigurationBox(void)const;
	
	void createGui(void);
};


#endif
