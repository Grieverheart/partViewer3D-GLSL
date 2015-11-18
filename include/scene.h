#ifndef __SCENE_H
#define __SCENE_H

#include "coord_parser.h"
#include "light.h"
#include "g-buffer.h"
#include "accumulator.h"
#include "ssao.h"
#include "shadowmap.h"
#include "Text/Properties.h"
#include <exception>
#include <vector>

#ifdef _MSC_VER
#define noexcept
#endif

class Shader;
class Grid;
namespace Text{
    class FontManager;
}

enum class Projection{
    PERSPECTIVE,
    ORTHOGRAPHIC,
};

class Scene{
public:
    class GlewInitializationException: public std::exception{
    public:
        virtual const char* what(void)const noexcept {
            return "Error when initializating GLEW.";
        }
    };

	Scene(int width, int height);
	~Scene(void);

	void load_scene(const SimConfig& config);
	void wsize_changed(int width, int height);
	void render(void);
	void process(void);

    void rotate(float angle, const glm::vec3& axis);

	void zoom(float dz);

    bool raytrace(int x, int y, int& pid);
    void select_particle(int pid);
    bool is_selected(int pid)const;
    void clear_selection(void);

    void hide_particle(int pid);
    void unhide_particle(int pid);

    void set_particle_color(int pid, const glm::vec3& color);

    void set_projection_type(Projection);

    glm::mat4 get_view_matrix(void)const;
    glm::mat4 get_projection_matrix(void)const;
    glm::mat4 get_model_matrix(void)const;

    void set_clip_plane(const glm::vec4&);
    void enable_clip_plane(void);
    void disable_clip_plane(void);
    void toggle_box(void);
    void toggle_point_drawing_mode(void);

    void set_view_position(const glm::vec3&);
    const glm::vec3& get_view_position(void)const;

    void set_view_direction(const glm::vec3&);
    const glm::vec3& get_view_direction(void)const;

    glm::vec3 get_light_direction(void)const;
    void set_light_direction(const glm::vec3&);
    float get_light_specular_intensity(void)const;
    void set_light_specular_intensity(float);
    float get_light_diffuse_intensity(void)const;
    void set_light_diffuse_intensity(float);
    float get_light_ambient_intensity(void)const;
    void set_light_ambient_intensity(float);
    float get_light_intensity(void)const;
    void set_light_intensity(float);
    void set_ssao_blur(bool);
    bool is_ssao_blur_enabled(void)const;
    void set_ssao_radius(float);
    float get_ssao_radius(void)const;
    void set_ssao_num_samples(unsigned int);
    unsigned int get_ssao_num_samples(void)const;
    void set_sky_color(const glm::vec3&);
    glm::vec3 get_sky_color(void)const;
    void set_background_color(const glm::vec3&);
    glm::vec3 get_background_color(void)const;

    void draw_text(const char* text, const Text::Properties& props);

private:
	int windowWidth;
	int windowHeight;
	float fov_, zoom_;
	float znear_, zfar_;
    float out_radius_;

	glm::mat4 projectionMatrix;
	glm::mat4 invProjMatrix;
	glm::mat4 viewMatrix;
    glm::mat4 invViewMatrix;
	glm::mat4 modelMatrix;
    glm::mat4 lightProjectionMatrix;
    glm::mat4 lightViewMatrix;
    glm::mat4* model_matrices;
    glm::vec3 view_pos;
    glm::vec3 view_dir;

	glm::vec3 m_bgColor;
	glm::vec3 diffcolor;
	glm::vec3 skycolor;

    glm::vec4 clip_plane_;

	unsigned int* shape_vaos;
	unsigned int* shape_vbos;

	unsigned int* particle_flags;
    glm::vec3* particle_colors;

    Particle* particles;
    Shape* shapes;
    int n_shapes;
    int n_particles;

	unsigned int vaoBox;
	unsigned int vboBox;
	unsigned int iboBox;
	unsigned int plane_vao;
	unsigned int plane_vbo;
	unsigned int quad_vao;
	unsigned int quad_vbo;
	unsigned int fullscreen_triangle_vao;

	unsigned int area_texture;
	unsigned int search_texture;

    std::vector<int> selected_pids;
    std::vector<int> draw_pids;

	bool is_scene_loaded;
    bool is_clip_plane_activated_;
	bool drawBox;
	bool draw_points_mode_;
	bool m_blur;

    Projection projection_type;

	CLight light;

	CGBuffer m_gbuffer;
    //TODO: Do something about these, make them more specific!
	Accumulator m_accumulator;
	Accumulator m_edge_buffer;
	Accumulator m_blend_buffer;
	Cssao m_ssao;
    CShadowmap m_shadowmap;

	Shader* sh_gbuffer;
	Shader* sh_gbuffer_instanced;
	Shader* sh_ssao;
	Shader* sh_shadowmap_instanced;
	Shader* sh_blur;
	Shader* sh_accumulator;
	Shader* sh_edge_detection;
	Shader* sh_blend_weights;
	Shader* sh_blend;
	Shader* sh_spheres;
	Shader* sh_shadowmap_spheres;
	Shader* sh_color;
	Shader* sh_color_sphere;
	Shader* sh_points;
	Shader* sh_text;

    Grid* grid;

    Text::FontManager* fontManager_;

    void set_projection(void);
	void drawConfigurationBox(void)const;
};


#endif
