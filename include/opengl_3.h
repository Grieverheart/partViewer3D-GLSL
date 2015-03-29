#ifndef __OPENGL_3_H
#define __OPENGL_3_H

#include "obj_parser.h"
#include "coord_parser.h"
#include "mesh.h"
#include "light.h"
#include "g-buffer.h"
#include "ssao.h"
#include "shadowmap.h"

class Shader;

class OpenGLContext{
public:
	OpenGLContext(void); 				// Default constructor  
	~OpenGLContext(void); 				// Destructor for cleaning up our application  
	bool create30Context(void); 	// Creation of our OpenGL 3.x context  
	void setupScene(int argc, char *argv[]); 				// All scene information can be setup here  
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
	
	glm::mat4 *MVPArray;
	glm::mat3 *NormalArray;
	
	glm::vec3 m_bgColor;
	glm::vec3 diffcolor;
	glm::vec3 skycolor;
	
	unsigned int mNInstances;
	
	unsigned int vaoBox;
	unsigned int vboBox;
	unsigned int iboBox;
	
	bool use_dat;
	bool m_fboInit;
	bool m_blur;
	bool m_rotating;
	
	CLight light;
	CObjParser objparser;
	CCoordParser coordparser;
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
	
	TwBar *bar;
	
	void drawPass(void)const;
	void fboPass(void)const;
	void ssaoPass(void);
	void shadowPass(void);
	
	void drawConfiguration(const glm::mat4& vMatrix, const glm::mat4& pMatrix)const;
	void drawConfigurationBox(void)const;
	void initConfigurationBox(void);
	
	void createGui(void);
};


#endif
