#ifndef __OPENGL_3_H
#define __OPENGL_3_H

#include "coord_parser.h"
#include "light.h"
#include "g-buffer.h"
#include "accumulator.h"
#include "ssao.h"
#include "shadowmap.h"
#include "perfmon.h"
#include <exception>

#ifdef _MSC_VER
#define noexcept
#endif

class Shader;

class OpenGLContext{
public:
    class GlewInitializationException: public std::exception{
    public:
        virtual const char* what(void)const noexcept {
            return "Error when initializating GLEW.";
        }
    };

	OpenGLContext(int width, int height);
	~OpenGLContext(void);
	void load_scene(const SimConfig& config);
	void reshapeWindow(int width, int height);
	void renderScene(void);
	void processScene(void);
	
	float getZoom(void)const;
	void setZoom(float zoom);
	glm::ivec2 getScreen(void)const;
	
	bool drawBox;
	glm::mat4 trackballMatrix;
	
private:
	int windowWidth;
	int windowHeight;
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
	
	unsigned int* shape_instances;
	unsigned int* shape_vaos;
	unsigned int* shape_vertex_vbos;
	unsigned int* shape_model_matrix_vbos;
	unsigned int* shape_colors_vbos;
	unsigned int* shape_num_vertices;

    Shape::Type* shape_types;
    unsigned int n_shapes;

	unsigned int vaoBox;
	unsigned int vboBox;
	unsigned int iboBox;
	unsigned int fullscreen_triangle_vao;

	unsigned int area_texture;
	unsigned int search_texture;
	
	bool is_scene_loaded;
	bool m_blur;
	bool m_rotating;
	
	CLight light;

	CGBuffer m_gbuffer;
	Accumulator m_accumulator;
	Accumulator m_edge_buffer;
	Accumulator m_blend_buffer;
	Cssao m_ssao;
    CShadowmap m_shadowmap;
	
	Shader *sh_gbuffer;
	Shader *sh_gbuffer_instanced;
	Shader *sh_ssao;
	Shader *sh_shadowmap_instanced;
	Shader *sh_blur;
	Shader *sh_accumulator;
	Shader *sh_edge_detection;
	Shader *sh_blend_weights;
	Shader *sh_blend;
	Shader *sh_spheres;
	Shader *sh_shadowmap_spheres;

    PerfMon perf_mon;
	
	TwBar *bar;
	
	void drawConfigurationBox(void)const;
	
	void createGui(void);
};


#endif
