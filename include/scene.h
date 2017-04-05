#ifndef __SCENE_H
#define __SCENE_H

#include "simconfig.h"
#include "light.h"
#include "g-buffer.h"
#include "accumulator.h"
#include "ssao.h"
#include "shadowmap.h"
#include "Text/Properties.h"
#include <exception>
#include <vector>

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
    class InitializationException: public std::exception{
    public:
        InitializationException(const char* message):
            message_(message)
        {}
        virtual const char* what(void)const noexcept {
            return message_;
        }
    private:
        const char* message_;
    };

	Scene(int width, int height);
	~Scene(void);

	void load_scene(const SimConfig& config);
	void wsize_changed(int width, int height);
	void render(void);
	void process(void);

    void rotate(float angle, const glm::vec3& axis);
	void set_fov_degrees(float fov);
	float get_fov_degrees(void);
    bool raytrace(int x, int y, int& pid);
    void select_particle(int pid);
    bool is_particle_selected(int pid)const;
    void clear_particle_selections(void);
    //TODO: Add function to change selection rendering color.
    //TODO: Add function to change selection rendering size.

    void hide_particle(int pid);
    void unhide_particle(int pid);

    void set_particle_color(int pid, const glm::vec3& color);

    void toggle_particle_point_drawing(int pid);
    void set_point_radius(float);
    void set_point_outline_radius(float);
    void set_point_color(const glm::vec4&);
    void set_point_outline_color(const glm::vec4&);

    void set_box_line_width(float);

    void set_projection_type(Projection);

    glm::mat4 get_view_matrix(void)const;
    glm::mat4 get_projection_matrix(void)const;
    glm::mat4 get_model_matrix(void)const;

    void set_clip_plane(const glm::vec4&);
    void enable_clip_plane(void);
    void disable_clip_plane(void);
    void toggle_box(void);

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
    void toggle_ssao_blur(void);
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

    void save_snapshot_png(const char* path)const;

private:
	int window_width_;
	int window_height_;
	float fov_;
	float znear_, zfar_;
    float out_radius_;

	glm::mat4 projection_matrix_;
	glm::mat4 inv_projection_matrix_;
	glm::mat4 view_matrix_;
    glm::mat4 inv_view_matrix_;
	glm::mat4 model_matrix_;
    glm::mat4 light_projection_matrix_;
    glm::mat4 light_view_matrix_;
    glm::mat4* model_matrices_;
    glm::vec3 view_pos_;
    glm::vec3 view_dir_;

	glm::vec3 background_color_;
	glm::vec3 sky_color_;

    float point_radius_;
    float point_outline_radius_;
	glm::vec4 point_color_;
	glm::vec4 point_outline_color_;

    float line_width_;

    glm::vec4 clip_plane_;

	unsigned int* shape_vaos_;
	unsigned int* shape_vbos_;

	unsigned int* particle_flags_;
    glm::vec3* particle_colors_;

    SimConfig* config_;

	unsigned int box_vao_;
	unsigned int box_vbo_;
	unsigned int plane_vao_;
	unsigned int plane_vbo_;
	unsigned int quad_vao_;
	unsigned int quad_vbo_;
	unsigned int fullscreen_triangle_vao_;

	unsigned int area_texture_;
	unsigned int search_texture_;

    std::vector<int> selected_pids_;
    std::vector<int> draw_pids_;
    std::vector<int>::iterator draw_points_end_;

    bool is_clip_plane_active_;
	bool is_box_drawing_active_;
	bool is_blur_active_;

    Projection projection_type_;

	CLight light_;

	CGBuffer m_gbuffer_;
    //TODO: Do something about these, make them more specific!
	Accumulator m_accumulator_;
	Accumulator m_edge_buffer;
	Accumulator m_blend_buffer_;
	Cssao m_ssao_;
    CShadowmap m_shadowmap_;

	Shader* sh_gbuffer_;
	Shader* sh_gbuffer_instanced_;
	Shader* sh_ssao_;
	Shader* sh_shadowmap_instanced_;
	Shader* sh_blur_;
	Shader* sh_accumulator_;
	Shader* sh_edge_detection_;
	Shader* sh_blend_weights_;
	Shader* sh_blend_;
	Shader* sh_spheres_;
	Shader* sh_shadowmap_spheres_;
	Shader* sh_color_;
	Shader* sh_color_sphere_;
	Shader* sh_points_;
	Shader* sh_text_;
	Shader* sh_quad_line_;

    Grid* grid_;

    Text::FontManager* fontManager_;

    void set_projection(void);
	void drawConfigurationBox(void)const;
};


#endif
