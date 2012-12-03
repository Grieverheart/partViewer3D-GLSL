#ifndef __OPENGL_3_H
#define __OPENGL_3_H

#include <AntTweakBar.h>
#include "main.h"
#include "shader.h"
#include "obj_parser.h"
#include "coord_parser.h"
#include "mesh.h"
#include "light.h"
#include "g-buffer.h"
#include "ssao.h"

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
	
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 modelMatrix;
	glm::mat4 lightViewMatrix;
	glm::mat4 IdentityMatrix;
	glm::mat4 lightProjectionMatrix;
	
	glm::mat4 *MVPArray;
	glm::mat3 *NormalArray;
	
	glm::vec3 m_bgColor;
	glm::vec3 diffcolor;
	
	int MVPMatrixLocation;
	int ModelViewMatrixLocation;
	int NormalMatrixLocation;
	int DepthMapLocation;
	int NormalMapLocation;
	int ColorMapLocation;
	// int IDMapLocation;
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
	int diffColorLocation;
	int scaleLocation;
	int diffColorInstancedLocation;
	int m_blurLocation;
	unsigned int mNInstances;
	
	GLuint vaoBox;
	GLuint vboBox;
	GLuint iboBox;
	
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
	
	Shader *sh_gbuffer; // GLSL Shader
	Shader *sh_gbuffer_instanced; // GLSL Shader
	Shader *sh_ssao;
	Shader *sh_blur;
	Shader *sh_accumulator;
	
	TwBar *bar;
	
	void drawPass(void)const;
	void fboPass(void)const;
	void ssaoPass(void);
	
	void drawConfiguration(void)const;
	void drawConfigurationBox(void)const;
	void initConfigurationBox(void);
	
	void createGui(void);
};


#endif