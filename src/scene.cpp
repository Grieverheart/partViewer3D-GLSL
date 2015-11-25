#include "include/scene.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>

#ifdef _WIN32
#define WGL_WGLEXT_PROTOTYPES
//#include <GL/wgl.h>
#include <GL/wglext.h>
#elif __linux
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>
#endif

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include "include/Text/FontManager.h"
#include "include/gl_utils.h"
#include "include/shader.h"
#include "include/grid.h"

#include "include/smaa/smaa_area.h"
#include "include/smaa/smaa_search.h"

//Include shaders
#include "include/shaders/accumulator.glsl"
#include "include/shaders/blur.glsl"
#include "include/shaders/color.glsl"
#include "include/shaders/color_sphere.glsl"
#include "include/shaders/gbuffer.glsl"
#include "include/shaders/gbuffer_instanced.glsl"
#include "include/shaders/points.glsl"
#include "include/shaders/quad_line.glsl"
#include "include/shaders/shadowmap_instanced.glsl"
#include "include/shaders/shadowmap_spheres.glsl"
#include "include/shaders/spheres.glsl"
#include "include/shaders/ssao.glsl"
#include "include/shaders/text.glsl"
#include "include/shaders/smaa/blend.glsl"
#include "include/shaders/smaa/blend_weights.glsl"
#include "include/shaders/smaa/edge_detection.glsl"

static const glm::mat4 biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
);

enum ParticleFlags{
    Hidden = 0x01,
    Point  = 0x02
};

Scene::Scene(int width, int height):
    windowWidth(width), windowHeight(height),
    fov_(60.0f), zoom_(0.0f),
	modelMatrix(1.0),
	m_bgColor(glm::vec3(44, 114, 220) / 255.0f),
	diffcolor(glm::vec3(77, 27, 147) / 255.0f),
	skycolor(0.529, 0.808, 0.921),
    point_radius_(0.2f), point_outline_radius_(0.14f),
    point_color_(0.05, 0.05, 0.05, 0.5), point_outline_color_(0.0, 0.0, 0.0, 0.5),
    line_width_(0.01),
    clip_plane_{0.0, 0.0, -1.0, 0.0},
	shape_vaos(nullptr), shape_vbos(nullptr),
    particle_flags(nullptr),
    particles(nullptr), shapes(nullptr), n_shapes(0), n_particles(0),
    vaoBox(0), vboBox(0), fullscreen_triangle_vao(0),
    is_scene_loaded(false), is_clip_plane_activated_(false), drawBox(false), m_blur(true),
    projection_type(Projection::PERSPECTIVE),
	light(glm::vec3(-0.27, -0.91, -0.33)),
    sh_gbuffer(nullptr), sh_gbuffer_instanced(nullptr), sh_ssao(nullptr),
    sh_shadowmap_instanced(nullptr), sh_blur(nullptr), sh_accumulator(nullptr),
    sh_edge_detection(nullptr), sh_blend_weights(nullptr), sh_blend(nullptr),
    sh_spheres(nullptr), sh_shadowmap_spheres(nullptr), sh_points(nullptr),
    sh_text(nullptr), sh_quad_line(nullptr),
    grid(nullptr),
    fontManager_(new Text::FontManager())
{
	int glVersion[2] = {-1,1};
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

	printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);
	printf("Renderer used: %s\n", glGetString(GL_RENDERER));
	printf("Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

#ifdef _WIN32
    wglSwapIntervalEXT(0);
#elif __linux
    Display *dpy = glXGetCurrentDisplay();
    GLXDrawable drawable = glXGetCurrentDrawable();
    const int interval = 0;

    if (drawable) {
        glXSwapIntervalEXT(dpy, drawable, interval);
    }
#endif

	glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    sh_gbuffer             = new Shader(gbuffer_vert, gbuffer_frag);
    sh_gbuffer_instanced   = new Shader(gbuffer_instanced_vert, gbuffer_instanced_frag);
    sh_ssao                = new Shader(ssao_vert, ssao_frag);
    sh_shadowmap_instanced = new Shader(shadowmap_instanced_vert, shadowmap_instanced_frag);
    sh_shadowmap_spheres   = new Shader(shadowmap_spheres_vert, shadowmap_spheres_frag);
    sh_blur                = new Shader(blur_vert, blur_frag);
    sh_accumulator         = new Shader(accumulator_vert, accumulator_frag);
    sh_edge_detection      = new Shader(edge_detection_vert, edge_detection_frag);
    sh_blend_weights       = new Shader(blend_weights_vert, blend_weights_frag);
    sh_blend               = new Shader(blend_vert, blend_frag);
    sh_spheres             = new Shader(spheres_vert, spheres_frag);
    sh_color               = new Shader(color_vert, color_frag);
    sh_color_sphere        = new Shader(color_sphere_vert, color_sphere_frag);
    sh_points              = new Shader(points_vert, points_frag);
    sh_text                = new Shader(text_vert, text_frag);
    sh_quad_line           = new Shader(quad_line_vert, quad_line_frag);

    //TODO: Add exceptions
	if(!m_ssao.Init(windowWidth, windowHeight)) printf("Couldn't initialize SSAO!");
	if(!m_shadowmap.Init(windowWidth, windowHeight)) printf("Couldn't initialize Shadowmap!");
	if(!m_gbuffer.Init(windowWidth, windowHeight)) printf("Couldn't initialize FBO!");
	if(!m_accumulator.Init(windowWidth, windowHeight)) printf("Couldn't initialize FBO!");
	if(!m_edge_buffer.Init(windowWidth, windowHeight)) printf("Couldn't initialize FBO!");
	if(!m_blend_buffer.Init(windowWidth, windowHeight)) printf("Couldn't initialize FBO!");

    glGenVertexArrays(1, &fullscreen_triangle_vao);

    glGenVertexArrays(1, &vaoBox);
    glBindVertexArray(vaoBox);

    glGenBuffers(1, &vboBox);
    glBindBuffer(GL_ARRAY_BUFFER, vboBox);
    glEnableVertexAttribArray((GLuint)0);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray((GLuint)1);
    glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const GLvoid*)(3 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Generate plane
    {
        Vertex vertices[] = {
            {{-1.0, -1.0, 0.0}, {0.0, 0.0, 1.0}},
            {{ 1.0, -1.0, 0.0}, {0.0, 0.0, 1.0}},
            {{-1.0,  1.0, 0.0}, {0.0, 0.0, 1.0}},

            {{-1.0,  1.0, 0.0}, {0.0, 0.0, 1.0}},
            {{ 1.0, -1.0, 0.0}, {0.0, 0.0, 1.0}},
            {{ 1.0,  1.0, 0.0}, {0.0, 0.0, 1.0}}
        };

        glGenVertexArrays(1, &plane_vao);
        glBindVertexArray(plane_vao);

        glGenBuffers(1, &plane_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, plane_vbo);

        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray((GLuint)0);
        glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

        glEnableVertexAttribArray((GLuint)1);
        glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)sizeof(glm::vec3));

        glBindVertexArray(0);
    }

    //Generate quad
    {
        glGenVertexArrays(1, &quad_vao);
        glGenBuffers(1, &quad_vbo);

        glBindVertexArray(quad_vao);

        glm::vec3 vertices[] = {
            glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.0)
        };
        glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    }

    //Generate and load smaa textures
    glGenTextures(1, &area_texture);
    glBindTexture(GL_TEXTURE_2D, area_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, smaa_area_tex_xlen, smaa_area_tex_ylen, 0, GL_RG, GL_UNSIGNED_BYTE, smaa_area_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &search_texture);
    glBindTexture(GL_TEXTURE_2D, search_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, smaa_search_tex_xlen, smaa_search_tex_ylen, 0, GL_RED, GL_UNSIGNED_BYTE, smaa_search_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Scene::~Scene(void){
	delete sh_gbuffer; // GLSL Shader
	delete sh_gbuffer_instanced; // GLSL Shader
	delete sh_ssao;
	delete sh_shadowmap_instanced;
	delete sh_blur;
	delete sh_accumulator;
	delete sh_edge_detection;
	delete sh_blend_weights;
	delete sh_blend;
	delete sh_spheres;
	delete sh_shadowmap_spheres;
	delete sh_color;
	delete sh_color_sphere;
    delete sh_points;
	delete sh_text;
    delete sh_quad_line;

	if(n_shapes) glDeleteVertexArrays(n_shapes, shape_vaos);
	if(n_shapes) glDeleteBuffers(n_shapes, shape_vbos);

    delete[] shape_vaos;
    delete[] shape_vbos;
    delete[] shapes;
    delete[] model_matrices;
    delete[] particle_flags;
    delete[] particle_colors;

    delete grid;

	glDeleteVertexArrays(1, &vaoBox);
	glDeleteBuffers(1, &vboBox);

	glDeleteVertexArrays(1, &fullscreen_triangle_vao);
}

void Scene::load_scene(const SimConfig& config){
    if(is_scene_loaded){
        glDeleteBuffers(n_shapes, shape_vbos);
        glDeleteVertexArrays(n_shapes, shape_vaos);
        delete grid;
        delete[] particles;
        delete[] shape_vaos;
        delete[] shape_vbos;
        delete[] shapes;
        delete[] model_matrices;
        delete[] particle_flags;
        delete[] particle_colors;
    }

    is_scene_loaded = true;

    grid = new Grid(config);

    particles = new Particle[config.n_part];
    n_particles = config.n_part;

    //Configuration box
    {
        glm::vec3 offset(
            -(config.box[0][0] + config.box[0][1] + config.box[0][2]) / 2.0,
            -(config.box[1][1] + config.box[1][2]) / 2.0,
            -config.box[2][2] / 2.0
        );

        ////////////////////////
        //                    //
        //    6 +--------+ 7  //
        //     /|       /|    //
        //    / |    4 / |    //
        // 2 +--------+  |    //
        //   |  |     |  |    //
        //   |3 +-----|--+ 5  //
        //   | /      | /     //
        //   |/       |/      //
        // 0 +--------+ 1     //
        //                    //
        ////////////////////////

        glm::vec3 box_vertices[] = {
            /* 0 */ glm::vec3(0.0f, 0.0f, 0.0f),
            /* 1 */ glm::column(config.box, 0),
            /* 2 */ glm::column(config.box, 1),
            /* 3 */ glm::column(config.box, 2),
            /* 4 */ glm::column(config.box, 0) + glm::column(config.box, 1),
            /* 5 */ glm::column(config.box, 0) + glm::column(config.box, 2),
            /* 6 */ glm::column(config.box, 1) + glm::column(config.box, 2),
            /* 7 */ glm::column(config.box, 0) + glm::column(config.box, 1) + glm::column(config.box, 2),
        };

        for(size_t i = 0; i < 8; ++i) box_vertices[i] += offset;

        size_t vertex_ids[] = {
            0, 1, 3, 5, //bottom
            3, 5, 6, 7, //back
            6, 7, 2, 4, //top
            3, 6, 0, 2,//left
            0, 2, 1, 4, //front
            1, 4, 5, 7  //right
        };

        float* vertex_data = new float[24 * 5];
        for(size_t qid = 0; qid < 6; ++qid){
            for(size_t vid = 0; vid < 4; ++vid){
                std::copy(&box_vertices[vertex_ids[4 * qid + vid]].x, &box_vertices[vertex_ids[4 * qid + vid]].z + 1, &vertex_data[20 * qid + 5 * vid]);
                vertex_data[20 * qid + 5 * vid + 3] = float(vid & 1);
                vertex_data[20 * qid + 5 * vid + 4] = float((vid & 2) >> 1);
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, vboBox);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 120, vertex_data, GL_STATIC_DRAW);
    }

    out_radius_ = glm::length(glm::vec3(
        (config.box[0][0] + config.box[0][1] + config.box[0][2]) / 2.0,
        (config.box[1][1] + config.box[1][2]) / 2.0,
        config.box[2][2] / 2.0
    )) + 1.0;

    float init_zoom = -4.0;
    for(int i = 0; i < 3; i++){
        GLfloat max_zoom = -3.5f*glm::length(glm::transpose(config.box)[i]);
        if(max_zoom < init_zoom) init_zoom = max_zoom;
    }

    znear_ = -init_zoom - out_radius_;
    zfar_  = -init_zoom + 2.0 * out_radius_;


    //TODO: Move these to constructor with some other default values
    view_pos = glm::vec3(0.0, 0.0, -init_zoom);
    view_dir = -view_pos;

    set_projection();

    lightProjectionMatrix = glm::ortho(-out_radius_, out_radius_, -out_radius_, out_radius_, 0.0f, 2.0f * out_radius_);
	viewMatrix            = glm::lookAt(view_pos, view_pos + view_dir, glm::vec3(0.0, 1.0, 0.0));
    invViewMatrix         = glm::inverse(viewMatrix);
    lightViewMatrix       = glm::lookAt(-out_radius_ * light.direction_, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    n_shapes = config.n_shapes;

	shape_vaos = new unsigned int[config.n_shapes]{};
	shape_vbos = new unsigned int[config.n_shapes]{};

    shapes          = new Shape[config.n_shapes];
	particle_flags  = new unsigned int[config.n_part]{};
	particle_colors = new glm::vec3[config.n_part];
    model_matrices  = new glm::mat4[config.n_part];

    glm::mat4 tMatrix = glm::translate(
        glm::mat4(1.0),
        glm::vec3(
            -(config.box[0][0] + config.box[0][1] + config.box[0][2]) / 2.0,
            -(config.box[1][1] + config.box[1][2]) / 2.0,
            -config.box[2][2] / 2.0
        )
    );

    //Count shape instances and copy particles
    for(int i = 0; i < config.n_part; ++i){
        particle_colors[i] = diffcolor;
        draw_pids.push_back(i);
        particles[i] = config.particles[i];

        glm::mat4 tLocalMatrix = glm::translate(tMatrix, config.particles[i].pos);
        glm::mat4 rLocalMatrix = glm::rotate(
            glm::mat4(1.0),
            config.particles[i].rot.x,
            glm::vec3(config.particles[i].rot.y, config.particles[i].rot.z, config.particles[i].rot.w)
        );

        model_matrices[i] = tLocalMatrix * rLocalMatrix;
    }

    glGenVertexArrays(config.n_shapes, shape_vaos);
    glGenBuffers(config.n_shapes, shape_vbos);

    for(int shape_id = 0; shape_id < config.n_shapes; ++shape_id){
        shapes[shape_id] = config.shapes[shape_id];

        glBindVertexArray(shape_vaos[shape_id]);
        if(config.shapes[shape_id].type == Shape::MESH){
            const Shape::Mesh& mesh = config.shapes[shape_id].mesh;

            glBindBuffer(GL_ARRAY_BUFFER, shape_vbos[shape_id]);
            glBufferData(GL_ARRAY_BUFFER, mesh.n_vertices * sizeof(Vertex), mesh.vertices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)0);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)sizeof(glm::vec3));
        }
        else if(config.shapes[shape_id].type == Shape::SPHERE){
            glm::vec3 vertices[] = {
                glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.0)
            };
            glBindBuffer(GL_ARRAY_BUFFER, shape_vbos[shape_id]);
            glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        }
    }
    glBindVertexArray(0);

    //TODO: Move shader uniform initialization to constructor

	// SSAO Uniforms
    glm::mat2 iproj = glm::mat2(invProjMatrix[2][2], invProjMatrix[2][3],
                                invProjMatrix[3][2], invProjMatrix[3][3]);

	sh_ssao->bind();
	{
		m_ssao.UploadUniforms(*sh_ssao);
		sh_ssao->setUniform("NormalMap", 0);
		sh_ssao->setUniform("DepthMap", 1);
		sh_ssao->setUniform("depth_iproj", 1, iproj);
		sh_ssao->setUniform("projectionMatrix", 1, projectionMatrix);
		sh_ssao->setUniform("invProjMatrix", 1, invProjMatrix);
	}

	// Blur Uniforms
	sh_blur->bind();
	{
		sh_blur->setUniform("aoSampler", 0);
		sh_blur->setUniform("use_blur", int(m_blur));
	}

	// Edge Detection Uniforms
	sh_edge_detection->bind();
	{
		sh_edge_detection->setUniform("colorTex", 0);
		sh_edge_detection->setUniform("texel_size", 1, glm::vec2(1.0f / windowWidth, 1.0f / windowHeight));
	}

	// Blend weights Uniforms
	sh_blend_weights->bind();
	{
		sh_blend_weights->setUniform("edgesTex", 0);
		sh_blend_weights->setUniform("areaTex", 1);
		sh_blend_weights->setUniform("searchTex", 2);
		sh_blend_weights->setUniform("texel_size", 1, glm::vec2(1.0f / windowWidth, 1.0f / windowHeight));
	}

	// Blend Uniforms
	sh_blend->bind();
	{
		sh_blend->setUniform("colorTex", 0);
		sh_blend->setUniform("blendTex", 1);
		sh_blend->setUniform("texel_size", 1, glm::vec2(1.0f / windowWidth, 1.0f / windowHeight));
	}

	// Accumulator Uniforms
	sh_accumulator->bind();
	{
		sh_accumulator->setUniform("ColorMap", 0);
		sh_accumulator->setUniform("NormalMap", 1);
		sh_accumulator->setUniform("DepthMap", 2);
		sh_accumulator->setUniform("LightDepthMap", 3);
		sh_accumulator->setUniform("depth_iproj", 1, iproj);
		sh_accumulator->setUniform("skyColor", 1, skycolor);
		sh_accumulator->setUniform("invProjMatrix", 1, invProjMatrix);
	}

	sh_text->bind();
	{
		glm::mat4 projectionMatrix = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);
		sh_text->setUniform("projectionMatrix", 1, projectionMatrix);
		sh_text->setUniform("inSampler", 0);
		sh_text->setUniform("inColor", 1, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	}

    sh_points->bind();
    {
        sh_points->setUniform("radius", point_radius_);
        sh_points->setUniform("outline_radius", point_outline_radius_);
        sh_points->setUniform("color", 1, point_color_);
        sh_points->setUniform("outline_color", 1, point_outline_color_);
    }
}

//TODO: Handle Orthographic projection correctly.
bool Scene::raytrace(int x, int y, int& pid){
    glm::vec3 ray_origin, ray_dir;
    if(projection_type == Projection::PERSPECTIVE){
        glm::mat4 imodel_matrix = glm::inverse(modelMatrix);
        glm::vec3 o = glm::vec3(imodel_matrix * invViewMatrix * glm::vec4(glm::vec3(0.0), 1.0));
        glm::vec4 mouse_clip = glm::vec4(2.0f * x / windowWidth - 1.0f, 1.0f - 2.0f * y / windowHeight, 0.0f, 1.0f);
        glm::vec4 dir = imodel_matrix * invViewMatrix * invProjMatrix * mouse_clip;
        dir /= dir.w;

        ray_origin = o;
        ray_dir    = glm::normalize(glm::vec3(dir) - o);
    }
    else{
        glm::mat4 imodel_matrix = glm::inverse(modelMatrix);
        glm::vec4 mouse_clip = glm::vec4(2.0f * x / windowWidth - 1.0f, 1.0f - 2.0f * y / windowHeight, -1.0f, 1.0f);
        glm::vec4 o = imodel_matrix * invViewMatrix * invProjMatrix * mouse_clip;

        ray_origin = glm::vec3(o) / o.w;
        ray_dir    = glm::mat3(imodel_matrix) * glm::mat3(invViewMatrix) * glm::vec3(0.0, 0.0, -1.0);
    }

    if(is_clip_plane_activated_){
        float t = -(glm::dot(ray_origin, glm::vec3(clip_plane_)) + clip_plane_.w) / glm::dot(ray_dir, glm::vec3(clip_plane_));
        ray_origin += ray_dir * t;
    }

    float t = FLT_MAX;
    return grid->raycast(ray_origin, ray_dir, t, pid);
}

void Scene::select_particle(int pid){
    auto iter = std::find(selected_pids.begin(), selected_pids.end(), pid);
    if(iter != selected_pids.end()){
        selected_pids.erase(iter);
    }
    else selected_pids.push_back(pid);
}

bool Scene::is_selected(int pid)const{
    return std::any_of(selected_pids.begin(), selected_pids.end(), [pid](int id) -> bool {return id == pid;});
}

void Scene::clear_selection(void){
    selected_pids.clear();
}

void Scene::hide_particle(int pid){
    if(!(particle_flags[pid] & ParticleFlags::Hidden)){
        particle_flags[pid] |= ParticleFlags::Hidden;
        grid->ignore_id(pid);
    }
}

void Scene::unhide_particle(int pid){
    if(particle_flags[pid] & ParticleFlags::Hidden){
        particle_flags[pid] &= ~ParticleFlags::Hidden;
        grid->unignore_id(pid);
    }
}

void Scene::set_particle_color(int pid, const glm::vec3& color){
    particle_colors[pid] = color;
}

void Scene::toggle_particle_point_drawing(int pid){
    particle_flags[pid] ^= ParticleFlags::Point;
}

//TODO: Perhaps add a should_update_point_uniforms.
void Scene::set_point_radius(float radius){
    point_radius_ = radius;
    sh_points->bind();
    sh_points->setUniform("radius", point_radius_);
}

void Scene::set_point_outline_radius(float radius){
    point_outline_radius_ = radius;
    sh_points->bind();
    sh_points->setUniform("outline_radius", point_outline_radius_);
}

void Scene::set_point_color(const glm::vec4& color){
    point_color_ = color;
    sh_points->bind();
    sh_points->setUniform("color", 1, point_color_);
}

void Scene::set_point_outline_color(const glm::vec4& color){
    point_outline_color_ = color;
    sh_points->bind();
    sh_points->setUniform("outline_color", 1, point_outline_color_);
}

void Scene::wsize_changed(int w, int h){
	windowWidth = w;
	windowHeight = h;
	glViewport(0, 0, windowWidth, windowHeight);

    set_projection();

    m_gbuffer.Resize(windowWidth, windowHeight);
    m_edge_buffer.Resize(windowWidth, windowHeight);
    m_blend_buffer.Resize(windowWidth, windowHeight);
    m_accumulator.Resize(windowWidth, windowHeight);
    m_shadowmap.Resize(windowWidth, windowHeight);

    sh_ssao->bind();
    m_ssao.Resize(windowWidth, windowHeight, sh_ssao);

	// Edge Detection Uniforms
    glm::vec2 texel_size = glm::vec2(1.0f / windowWidth, 1.0f / windowHeight);

	sh_edge_detection->bind();
    sh_edge_detection->setUniform("texel_size", 1, texel_size);

	sh_blend_weights->bind();
    sh_blend_weights->setUniform("texel_size", 1, texel_size);

	sh_blend->bind();
    sh_blend->setUniform("texel_size", 1, texel_size);

	sh_text->bind();
    glm::mat4 projectionMatrix = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);
    sh_text->setUniform("projectionMatrix", 1, projectionMatrix);
}

void Scene::process(void){
    set_projection();
    lightViewMatrix = glm::lookAt(-out_radius_ * light.direction_, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    glm::mat2 iproj = glm::mat2(invProjMatrix[2][2], invProjMatrix[2][3],
                                invProjMatrix[3][2], invProjMatrix[3][3]);

	sh_ssao->bind();
	{
		sh_ssao->setUniform("depth_iproj", 1, iproj);
		sh_ssao->setUniform("projectionMatrix", 1, projectionMatrix);
		sh_ssao->setUniform("invProjMatrix", 1, invProjMatrix);
	}

	sh_accumulator->bind();
	{
		sh_accumulator->setUniform("depth_iproj", 1, iproj);
		sh_accumulator->setUniform("invProjMatrix", 1, invProjMatrix);
	}

    draw_points_end_ = std::partition(draw_pids.begin(), draw_pids.end(),
        [=](int i) -> bool {
            return (particle_flags[i] & ParticleFlags::Point);
        }
    );

    double* depths = new double[n_particles];
    auto mv_matrix = viewMatrix * modelMatrix;
    for(int i = 0; i < n_particles; ++i){
        auto pos = mv_matrix * model_matrices[i] * glm::vec4(0.0, 0.0, 0.0, 1.0);
        depths[i] = pos.z / pos.w;
    }

    std::sort(draw_pids.begin(), draw_points_end_,
        [depths](int i, int j) -> bool {
            return (depths[i] < depths[j]);
        }
    );

    std::sort(draw_points_end_, draw_pids.end(),
        [depths](int i, int j) -> bool {
            return (depths[i] > depths[j]);
        }
    );

    delete[] depths;
}

void Scene::set_projection(void){
    if(projection_type == Projection::ORTHOGRAPHIC){
        float half_length = glm::length(view_pos) * tan(0.5f * glm::radians(fov_ + zoom_));
        float aspect      = float(windowWidth) / windowHeight;

        projectionMatrix = glm::ortho(-half_length * aspect, half_length * aspect, -half_length, half_length, znear_, zfar_);
        invProjMatrix    = glm::inverse(projectionMatrix);
    }
    else{
        projectionMatrix = glm::perspective(glm::radians(fov_ + zoom_), (float)windowWidth/windowHeight, znear_, zfar_);
        invProjMatrix    = glm::inverse(projectionMatrix);
    }
}

void Scene::set_box_line_width(float linewidth){
    line_width_ = linewidth;
}

void Scene::set_projection_type(Projection ptype){
    projection_type = ptype;
}

//TODO: Improve line rendering!!!
void Scene::drawConfigurationBox(void)const{

	glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * modelMatrix;
	sh_quad_line->setUniform("MVPMatrix", 1, MVPMatrix);
	sh_quad_line->setUniform("line_width", line_width_);

	glBindVertexArray(vaoBox);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 12);
	glDrawArrays(GL_TRIANGLE_STRIP, 12, 12);
}

void Scene::render(void){
    if(!is_scene_loaded) return;

	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

    //Pre-clear the accumulator.
    m_accumulator.Bind();
    glClearColor(m_bgColor.x, m_bgColor.y, m_bgColor.z, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);


    if(draw_points_end_ != draw_pids.end()){
        //"Shadow Pass"
        {
            m_shadowmap.Bind();

            glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            glViewport(0, 0, windowWidth * 2, windowHeight * 2);

            for(int shape_id = 0; shape_id < n_shapes; ++shape_id){
                glBindVertexArray(shape_vaos[shape_id]);

                if(shapes[shape_id].type == Shape::MESH){
                    int n_vertices = shapes[shape_id].mesh.n_vertices;
                    if(is_clip_plane_activated_){
                        glEnable(GL_STENCIL_TEST);
                        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR_WRAP, GL_INCR_WRAP);
                        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR_WRAP, GL_DECR_WRAP);
                        glStencilFunc(GL_ALWAYS, 0, 0);
                        glStencilMask(0xFF);
                        glEnable(GL_CLIP_DISTANCE0);
                        glDisable(GL_CULL_FACE);
                    }

                    sh_shadowmap_instanced->bind();

                    sh_shadowmap_instanced->setUniform("MVPMatrix", 1, lightProjectionMatrix * lightViewMatrix * modelMatrix);
                    sh_shadowmap_instanced->setUniform("clip_plane", 1, clip_plane_);

                    for(auto pid_itr = draw_points_end_; pid_itr != draw_pids.end(); ++pid_itr){
                        auto pid = *pid_itr;
                        if((particles[pid].shape_id != shape_id) ||
                           (particle_flags[pid] & ParticleFlags::Hidden)) continue;
                        sh_shadowmap_instanced->setUniform("ModelMatrix", 1, model_matrices[pid]);
                        glDrawArrays(GL_TRIANGLES, 0, n_vertices);
                    }

                    if(is_clip_plane_activated_){
                        sh_gbuffer->bind();

                        glEnable(GL_STENCIL_TEST);
                        glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
                        glm::mat4 plane_model_matrix = glm::scale(glm::translate(glm::mat4(1.0), -clip_plane_.w * glm::vec3(clip_plane_)), glm::vec3(out_radius_));
                        {
                            glm::vec3 axis = -glm::cross(glm::vec3(clip_plane_), glm::vec3(0.0, 0.0, -1.0));
                            if(glm::dot(axis, axis) > 1e-12f){
                                axis = glm::normalize(axis);
                                float angle = acos(-clip_plane_.z);
                                plane_model_matrix = glm::rotate(plane_model_matrix, angle, axis);
                            }

                        }
                        glm::mat4 MVPMatrix = lightProjectionMatrix * lightViewMatrix * modelMatrix * plane_model_matrix;
                        sh_gbuffer->setUniform("MVPMatrix", 1, MVPMatrix);

                        glBindVertexArray(plane_vao);
                        glDrawArrays(GL_TRIANGLES, 0, 6);

                        glDisable(GL_STENCIL_TEST);
                        glDisable(GL_CLIP_DISTANCE0);
                    }
                }
                else{
                    sh_shadowmap_spheres->bind();

                    sh_shadowmap_spheres->setUniform("clip", is_clip_plane_activated_);
                    sh_shadowmap_spheres->setUniform("radius", 0.5f);
                    sh_shadowmap_spheres->setUniform("clip_plane", 1, clip_plane_);
                    sh_shadowmap_spheres->setUniform("MVMatrix", 1, lightViewMatrix * modelMatrix);
                    sh_shadowmap_spheres->setUniform("ProjectionMatrix", 1, lightProjectionMatrix);

                    for(auto pid_itr = draw_points_end_; pid_itr != draw_pids.end(); ++pid_itr){
                        auto pid = *pid_itr;
                        if((particles[pid].shape_id != shape_id) ||
                           (particle_flags[pid] & ParticleFlags::Hidden)) continue;
                        sh_shadowmap_spheres->setUniform("ModelMatrix", 1, model_matrices[pid]);
                        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    }
                }
            }

            glViewport(0, 0, windowWidth, windowHeight);
        }

        //"FBO Pass"
        {
            m_gbuffer.Bind();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            for(int shape_id = 0; shape_id < n_shapes; ++shape_id){
                glBindVertexArray(shape_vaos[shape_id]);
                if(shapes[shape_id].type == Shape::MESH){
                    int n_vertices = shapes[shape_id].mesh.n_vertices;
                    if(is_clip_plane_activated_){
                        glEnable(GL_STENCIL_TEST);
                        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR_WRAP, GL_INCR_WRAP);
                        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR_WRAP, GL_DECR_WRAP);
                        glStencilFunc(GL_ALWAYS, 0, 0);
                        glStencilMask(0xFF);
                        glEnable(GL_CLIP_DISTANCE0);
                        glDisable(GL_CULL_FACE);
                    }

                    sh_gbuffer_instanced->bind();

                    sh_gbuffer_instanced->setUniform("clip_plane", 1, clip_plane_);
                    sh_gbuffer_instanced->setUniform("MVMatrix", 1, viewMatrix * modelMatrix);
                    sh_gbuffer_instanced->setUniform("ProjectionMatrix", 1, projectionMatrix);

                    for(auto pid_itr = draw_points_end_; pid_itr != draw_pids.end(); ++pid_itr){
                        auto pid = *pid_itr;
                        if((particles[pid].shape_id != shape_id) ||
                           (particle_flags[pid] & ParticleFlags::Hidden)) continue;
                        sh_gbuffer_instanced->setUniform("ModelMatrix", 1, model_matrices[pid]);
                        sh_gbuffer_instanced->setUniform("in_Color", 1, particle_colors[pid]);
                        glDrawArrays(GL_TRIANGLES, 0, n_vertices);
                    }

                    if(is_clip_plane_activated_){
                        sh_gbuffer->bind();

                        glEnable(GL_STENCIL_TEST);
                        glColorMaski(0, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                        glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
                        glm::mat4 plane_model_matrix = glm::scale(glm::translate(glm::mat4(1.0), -clip_plane_.w * glm::vec3(clip_plane_)), glm::vec3(out_radius_));
                        {
                            glm::vec3 axis = -glm::cross(glm::vec3(clip_plane_), glm::vec3(0.0, 0.0, -1.0));
                            if(glm::dot(axis, axis) > 1e-12f){
                                axis = glm::normalize(axis);
                                float angle = acos(-clip_plane_.z);
                                plane_model_matrix = glm::rotate(plane_model_matrix, angle, axis);
                            }

                        }
                        glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * modelMatrix * plane_model_matrix;
                        glm::mat3 NormalMatrix = glm::mat3(glm::transpose(glm::inverse(viewMatrix * modelMatrix * plane_model_matrix)));
                        sh_gbuffer->setUniform("NormalMatrix", 1, NormalMatrix);
                        sh_gbuffer->setUniform("MVPMatrix", 1, MVPMatrix);

                        glBindVertexArray(plane_vao);
                        glDrawArrays(GL_TRIANGLES, 0, 6);

                        glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                        glDisable(GL_STENCIL_TEST);
                        glDisable(GL_CLIP_DISTANCE0);
                        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                    }
                }
                else{
                    sh_spheres->bind();
                    sh_spheres->setUniform("clip", is_clip_plane_activated_);
                    sh_spheres->setUniform("clip_plane", 1, clip_plane_);

                    //TODO: Move this to a better place
                    if(projection_type == Projection::PERSPECTIVE){
                        sh_spheres->setUniform("perspective_scale", 1.0f / cosf(0.5f * glm::radians(fov_ + zoom_)));
                    }
                    else{
                        sh_spheres->setUniform("perspective_scale", 1.0f);
                    }
                    sh_spheres->setUniform("radius", 0.5f);
                    sh_spheres->setUniform("MVMatrix", 1, viewMatrix * modelMatrix);
                    sh_spheres->setUniform("ProjectionMatrix", 1, projectionMatrix);
                    sh_spheres->setUniform("InvProjectionMatrix", 1, invProjMatrix);
                    sh_spheres->setUniform("in_Color", 1, diffcolor);

                    for(auto pid_itr = draw_points_end_; pid_itr != draw_pids.end(); ++pid_itr){
                        auto pid = *pid_itr;
                        if((particles[pid].shape_id != shape_id) ||
                           (particle_flags[pid] & ParticleFlags::Hidden)) continue;
                        sh_spheres->setUniform("ModelMatrix", 1, model_matrices[pid]);
                        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    }
                }
            }
        }

        //TODO: What's happening here?!
        glBindVertexArray(fullscreen_triangle_vao);
        //"SSAO Pass"
        {
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);

            //"SSAO Calc Pass"
            m_ssao.Bind();
            glClear(GL_COLOR_BUFFER_BIT);

            m_gbuffer.BindTexture(CGBuffer::GBUFF_TEXTURE_TYPE_NORMAL, 0);
            m_gbuffer.BindTexture(CGBuffer::GBUFF_TEXTURE_TYPE_DEPTH, 1);
            m_ssao.BindTexture(Cssao::TEXTURE_TYPE_NOISE, 2);

            sh_ssao->bind();
            {
                m_ssao.UpdateUniforms(*sh_ssao);

                glDrawArrays(GL_TRIANGLES, 0, 3);
            }


            //"SSAO Blur Pass"

            m_gbuffer.Bind();
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
            m_ssao.BindTexture(Cssao::TEXTURE_TYPE_SSAO, 0);

            sh_blur->bind();
            {
                sh_blur->setUniform("use_blur", int(m_blur));

                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }

        //"Gather Pass"
        {
            //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            if(drawBox){
                glDepthMask(GL_TRUE);
                glEnable(GL_DEPTH_TEST);
            }
            m_accumulator.Bind();

            m_gbuffer.BindTexture(CGBuffer::GBUFF_TEXTURE_TYPE_DIFFUSE, 0);
            m_gbuffer.BindTexture(CGBuffer::GBUFF_TEXTURE_TYPE_NORMAL, 1);
            m_gbuffer.BindTexture(CGBuffer::GBUFF_TEXTURE_TYPE_DEPTH, 2);
            m_shadowmap.BindTexture(3);

            sh_accumulator->bind();
            {
                sh_accumulator->setUniform("invProjMatrix", 1, invProjMatrix);
                sh_accumulator->setUniform("skyColor", 1, skycolor);

                glm::mat4 depth_matrix = biasMatrix * lightProjectionMatrix * lightViewMatrix * invViewMatrix;
                sh_accumulator->setUniform("depth_matrix", 1, depth_matrix);

                glm::vec3 lightViewDirection = glm::mat3(viewMatrix) * light.direction_;
                sh_accumulator->setUniform("light.direction", 1, lightViewDirection);
                sh_accumulator->setUniform("light.Si", light.specular_);
                sh_accumulator->setUniform("light.Di", light.diffuse_);
                sh_accumulator->setUniform("light.Ai", light.ambient_);
                sh_accumulator->setUniform("light.Intensity", light.intensity_);

                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        }
    }

    if(drawBox){
        sh_quad_line->bind();
        {
            glEnable(GL_BLEND);
            glDepthMask(GL_TRUE);
            glDisable(GL_CULL_FACE);
            drawConfigurationBox();
            glDisable(GL_BLEND);
        }
    }

    //"Point Drawing Pass"
    if(draw_points_end_ != draw_pids.begin()){
        glEnable(GL_BLEND);
        //glDisable(GL_DEPTH_TEST);
        glBindVertexArray(fullscreen_triangle_vao);

        sh_points->bind();

        sh_points->setUniform("MVMatrix", 1, viewMatrix * modelMatrix);
        sh_points->setUniform("ProjectionMatrix", 1, projectionMatrix);

        if(is_clip_plane_activated_){
            glEnable(GL_CLIP_DISTANCE0);
            sh_points->setUniform("clip_plane", 1, clip_plane_);
        }

        for(auto pid_itr = draw_pids.begin(); pid_itr != draw_points_end_; ++pid_itr){
            auto pid = *pid_itr;
            if(particle_flags[pid] & ParticleFlags::Hidden) continue;
            sh_points->setUniform("ModelMatrix", 1, model_matrices[pid]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        if(is_clip_plane_activated_) glDisable(GL_CLIP_DISTANCE0);
        glDisable(GL_BLEND);
    }

    //"Selection Pass"
    glEnable(GL_STENCIL_TEST);
    for(auto selected_pid: selected_pids){
        int shape_id = particles[selected_pid].shape_id;

        glBindVertexArray(shape_vaos[shape_id]);
        glEnable(GL_DEPTH_TEST);
        glStencilMask(0xFF);
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        if(shapes[shape_id].type == Shape::MESH){
            int n_vertices = shapes[shape_id].mesh.n_vertices;
            sh_color->bind();

            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glm::mat4 mvp_matrix = projectionMatrix * viewMatrix * modelMatrix * model_matrices[selected_pid];
            sh_color->setUniform("mvp_matrix", 1, mvp_matrix);
            glDrawArrays(GL_TRIANGLES, 0, n_vertices);

            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0x00);
            glDisable(GL_DEPTH_TEST);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            mvp_matrix = mvp_matrix * glm::scale(glm::mat4(1.0), glm::vec3(1.1));
            sh_color->setUniform("mvp_matrix", 1, mvp_matrix);
            glDrawArrays(GL_TRIANGLES, 0, n_vertices);
        }
        else{
            sh_color_sphere->bind();

            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            float perspective_scale = 1.0f / cosf(0.5f * glm::radians(fov_ + zoom_));

            glm::mat4 mv_matrix = viewMatrix * modelMatrix * model_matrices[selected_pid];
            sh_color_sphere->setUniform("mv_matrix", 1, mv_matrix);
            sh_color_sphere->setUniform("projection_matrix", 1, projectionMatrix);
            sh_color_sphere->setUniform("iprojection_matrix", 1, invProjMatrix);
            sh_color_sphere->setUniform("perspective_scale", perspective_scale);
            sh_color_sphere->setUniform("radius", 0.5f);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0x00);
            glDisable(GL_DEPTH_TEST);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            sh_color_sphere->setUniform("radius", 1.1f * 0.5f);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

    }
    glDisable(GL_STENCIL_TEST);

    //"SMAA"
    {
        glBindVertexArray(fullscreen_triangle_vao);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        //"Edge Pass"
        {
            m_edge_buffer.Bind();
            glClear(GL_COLOR_BUFFER_BIT);

            m_accumulator.BindTexture(0);

            sh_edge_detection->bind();
            {
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        }

        //"Blend weight Pass"
        {
            m_blend_buffer.Bind();
            glClear(GL_COLOR_BUFFER_BIT);

            m_edge_buffer.BindTexture(0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, area_texture);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, search_texture);

            sh_blend_weights->bind();
            {
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        }

        //"Blend Pass"
        {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            m_accumulator.BindTexture(0);
            m_blend_buffer.BindTexture(1);

            glEnable(GL_FRAMEBUFFER_SRGB);

            sh_blend->bind();
            {
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        }
    }

    glDisable(GL_FRAMEBUFFER_SRGB);
}

void Scene::rotate(float angle, const glm::vec3& axis){
    modelMatrix = glm::rotate(glm::mat4(1.0), angle, axis) * modelMatrix;
}

//TODO: Move to camera
void Scene::zoom(float dz){
    zoom_ = zoom_ - 2.0f * dz; // put wheel up and down in one
    if(zoom_ < -58) zoom_ = -58.0f;
    else if(zoom_ > 90) zoom_ = 90.0f;
}

glm::mat4 Scene::get_view_matrix(void)const{
    return viewMatrix;
}

glm::mat4 Scene::get_projection_matrix(void)const{
    return projectionMatrix;
}

glm::mat4 Scene::get_model_matrix(void)const{
    return modelMatrix;
}

void Scene::set_clip_plane(const glm::vec4& clip_plane){
    clip_plane_ = clip_plane;
}

void Scene::enable_clip_plane(void){
    is_clip_plane_activated_ = true;
}

void Scene::disable_clip_plane(void){
    is_clip_plane_activated_ = false;
}

void Scene::toggle_box(void){
    drawBox = !drawBox;
}

void Scene::set_view_position(const glm::vec3& pos){
    view_pos = pos;
	viewMatrix    = glm::lookAt(view_pos, view_pos + view_dir, glm::vec3(0.0, 1.0, 0.0));
    invViewMatrix = glm::inverse(viewMatrix);
}

const glm::vec3& Scene::get_view_position(void)const{
    return view_pos;
}

void Scene::set_view_direction(const glm::vec3& dir){
    view_dir = dir;
	viewMatrix    = glm::lookAt(view_pos, view_pos + view_dir, glm::vec3(0.0, 1.0, 0.0));
    invViewMatrix = glm::inverse(viewMatrix);
}

const glm::vec3& Scene::get_view_direction(void)const{
    return view_dir;
}

glm::vec3 Scene::get_light_direction(void)const{
    return light.direction_;
}

void Scene::set_light_direction(const glm::vec3& direction){
    light.direction_ = direction;
}

float Scene::get_light_specular_intensity(void)const{
    return light.specular_;
}

void Scene::set_light_specular_intensity(float specular){
    light.specular_ = specular;
}

float Scene::get_light_diffuse_intensity(void)const{
    return light.diffuse_;
}

void Scene::set_light_diffuse_intensity(float diffuse){
    light.diffuse_ = diffuse;
}

float Scene::get_light_ambient_intensity(void)const{
    return light.ambient_;
}

void Scene::set_light_ambient_intensity(float ambient){
    light.ambient_ = ambient;
}

float Scene::get_light_intensity(void)const{
    return light.intensity_;
}

void Scene::set_light_intensity(float intensity){
    light.intensity_ = intensity;
}

void Scene::set_ssao_blur(bool value){
    m_blur = value;
}

bool Scene::is_ssao_blur_enabled(void)const{
    return m_blur;
}

void Scene::set_ssao_radius(float radius){
    m_ssao.setRadius(radius);
}

float Scene::get_ssao_radius(void)const{
    return m_ssao.getRadius();
}

void Scene::set_ssao_num_samples(unsigned int n_samples){
    m_ssao.setSamples(n_samples);
}

unsigned int Scene::get_ssao_num_samples(void)const{
    return m_ssao.getSamples();
}

void Scene::set_sky_color(const glm::vec3& color){
    skycolor = color;
}

glm::vec3 Scene::get_sky_color(void)const{
    return skycolor;
}

void Scene::set_background_color(const glm::vec3& color){
    m_bgColor = color;
}

glm::vec3 Scene::get_background_color(void)const{
    return m_bgColor;
}

void Scene::draw_text(const char* text, const Text::Properties& props){

    float scale = props.width_ / fontManager_->get_default_size();
    float dx = 0;
    float dy = 0;

    glEnable(GL_BLEND);
    glBindVertexArray(quad_vao);
	glActiveTexture(GL_TEXTURE0);

    Text::Font* font = fontManager_->get_font(props.font_);

    sh_text->bind();
    for(const char* char_ptr = text; *char_ptr != '\0'; ++char_ptr){
        char character = *char_ptr;
        const Text::Glyph* glyph = font->get_char_glyph(character);
        if(!glyph) return;

        if(character == '\n'){
            dy += font->line_advance() * scale;
            dx = 0;
            continue;
        }

        if(char_ptr - text > 0){
            float kerning = font->kern_advance(character, *(char_ptr - 1));
            dx += kerning * scale;
        }

        int width  = glyph->width() * scale;
        int height = glyph->height() * scale;
        float x    = props.x_ + dx + glyph->left_bearing() * scale;
        float y    = windowHeight - props.y_ - dy + glyph->top_bearing() * scale;

        glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(width, -height, 1.0f));
        modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f)) * modelMatrix;
        sh_text->setUniform("modelMatrix", 1, modelMatrix);
        sh_text->setUniform("inColor", 1, props.color_);
        glBindTexture(GL_TEXTURE_2D, (GLuint)glyph->tex_id());
    
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        dx += glyph->advance_width() * scale;
    }

    glDisable(GL_BLEND);
}


