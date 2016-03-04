#include "include/scene.h"

#include <GL/glew.h>

#ifdef _WIN32
#include <GL/wglew.h>
#elif __linux
#include <GL/glxew.h>
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
    window_width_(width), window_height_(height),
    fov_(60.0f), zoom_(0.0f),
    model_matrix_(1.0),
    background_color_(glm::vec3(44, 114, 220) / 255.0f),
    sky_color_(0.529, 0.808, 0.921),
    point_radius_(0.2f), point_outline_radius_(0.14f),
    point_color_(0.05, 0.05, 0.05, 0.5), point_outline_color_(0.0, 0.0, 0.0, 0.5),
    line_width_(0.01),
    clip_plane_{0.0, 0.0, -1.0, 0.0},
    shape_vaos_(nullptr), shape_vbos_(nullptr),
    particle_flags_(nullptr),
    config_(nullptr),
    box_vao_(0), box_vbo_(0), fullscreen_triangle_vao_(0),
    is_clip_plane_active_(false), is_box_drawing_active_(false), is_blur_active_(true),
    projection_type_(Projection::PERSPECTIVE),
    light_(glm::vec3(-0.27, -0.91, -0.33)),
    sh_gbuffer_(nullptr), sh_gbuffer_instanced_(nullptr), sh_ssao_(nullptr),
    sh_shadowmap_instanced_(nullptr), sh_blur_(nullptr), sh_accumulator_(nullptr),
    sh_edge_detection_(nullptr), sh_blend_weights_(nullptr), sh_blend_(nullptr),
    sh_spheres_(nullptr), sh_shadowmap_spheres_(nullptr), sh_points_(nullptr),
    sh_text_(nullptr), sh_quad_line_(nullptr),
    grid_(nullptr),
    fontManager_(new Text::FontManager())
{
    glewExperimental = GL_TRUE;
    GLenum error = glewInit(); //Enable GLEW
    if(error != GLEW_OK) throw InitializationException("Error initializing GLEW.\n");
    glError(__FILE__, __LINE__);

    int glVersion[2] = {-1, 1};
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

    sh_gbuffer_             = new Shader(gbuffer_vert, gbuffer_frag);
    sh_gbuffer_instanced_   = new Shader(gbuffer_instanced_vert, gbuffer_instanced_frag);
    sh_ssao_                = new Shader(ssao_vert, ssao_frag);
    sh_shadowmap_instanced_ = new Shader(shadowmap_instanced_vert, shadowmap_instanced_frag);
    sh_shadowmap_spheres_   = new Shader(shadowmap_spheres_vert, shadowmap_spheres_frag);
    sh_blur_                = new Shader(blur_vert, blur_frag);
    sh_accumulator_         = new Shader(accumulator_vert, accumulator_frag);
    sh_edge_detection_      = new Shader(edge_detection_vert, edge_detection_frag);
    sh_blend_weights_       = new Shader(blend_weights_vert, blend_weights_frag);
    sh_blend_               = new Shader(blend_vert, blend_frag);
    sh_spheres_             = new Shader(spheres_vert, spheres_frag);
    sh_color_               = new Shader(color_vert, color_frag);
    sh_color_sphere_        = new Shader(color_sphere_vert, color_sphere_frag);
    sh_points_              = new Shader(points_vert, points_frag);
    sh_text_                = new Shader(text_vert, text_frag);
    sh_quad_line_           = new Shader(quad_line_vert, quad_line_frag);

    if(!m_ssao_.Init(window_width_, window_height_)         ||
       !m_shadowmap_.Init(window_width_, window_height_)    ||
       !m_gbuffer_.Init(window_width_, window_height_)      ||
       !m_accumulator_.Init(window_width_, window_height_)  ||
       !m_edge_buffer.Init(window_width_, window_height_)   ||
       !m_blend_buffer_.Init(window_width_, window_height_)
    ){
        throw InitializationException("Couldn't initialize FBO!\n");
    }

    glGenVertexArrays(1, &fullscreen_triangle_vao_);

    glGenVertexArrays(1, &box_vao_);
    glBindVertexArray(box_vao_);

    glGenBuffers(1, &box_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, box_vbo_);
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

        glGenVertexArrays(1, &plane_vao_);
        glBindVertexArray(plane_vao_);

        glGenBuffers(1, &plane_vbo_);
        glBindBuffer(GL_ARRAY_BUFFER, plane_vbo_);

        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray((GLuint)0);
        glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

        glEnableVertexAttribArray((GLuint)1);
        glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)sizeof(glm::vec3));

        glBindVertexArray(0);
    }

    //Generate quad
    {
        glGenVertexArrays(1, &quad_vao_);
        glGenBuffers(1, &quad_vbo_);

        glBindVertexArray(quad_vao_);

        glm::vec3 vertices[] = {
            glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.0)
        };
        glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    }

    //Generate and load smaa textures
    glGenTextures(1, &area_texture_);
    glBindTexture(GL_TEXTURE_2D, area_texture_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, smaa_area_tex_xlen, smaa_area_tex_ylen, 0, GL_RG, GL_UNSIGNED_BYTE, smaa_area_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &search_texture_);
    glBindTexture(GL_TEXTURE_2D, search_texture_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, smaa_search_tex_xlen, smaa_search_tex_ylen, 0, GL_RED, GL_UNSIGNED_BYTE, smaa_search_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Scene::~Scene(void){
    delete sh_gbuffer_; // GLSL Shader
    delete sh_gbuffer_instanced_; // GLSL Shader
    delete sh_ssao_;
    delete sh_shadowmap_instanced_;
    delete sh_blur_;
    delete sh_accumulator_;
    delete sh_edge_detection_;
    delete sh_blend_weights_;
    delete sh_blend_;
    delete sh_spheres_;
    delete sh_shadowmap_spheres_;
    delete sh_color_;
    delete sh_color_sphere_;
    delete sh_points_;
    delete sh_text_;
    delete sh_quad_line_;

    if(config_){
        glDeleteVertexArrays(config_->shapes.size(), shape_vaos_);
        glDeleteBuffers(config_->shapes.size(), shape_vbos_);
    }

    delete[] shape_vaos_;
    delete[] shape_vbos_;
    delete[] model_matrices_;
    delete[] particle_flags_;
    delete[] particle_colors_;

    delete grid_;
    delete config_;

    glDeleteVertexArrays(1, &box_vao_);
    glDeleteBuffers(1, &box_vbo_);

    glDeleteVertexArrays(1, &fullscreen_triangle_vao_);
}

void Scene::load_scene(const SimConfig& config){
    if(config_){
        glDeleteBuffers(config_->shapes.size(), shape_vbos_);
        glDeleteVertexArrays(config_->shapes.size(), shape_vaos_);
        delete config_;
        delete grid_;
        delete[] shape_vaos_;
        delete[] shape_vbos_;
        delete[] model_matrices_;
        delete[] particle_flags_;
        delete[] particle_colors_;
    }

    config_ = new SimConfig(config);

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

        glBindBuffer(GL_ARRAY_BUFFER, box_vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 120, vertex_data, GL_STATIC_DRAW);
        delete[] vertex_data;
    }

    std::vector<double> shape_outradii(config.shapes.size());
    for(size_t sid = 0; sid < config.shapes.size(); ++sid){
        if(config.shapes[sid].type == Shape::MESH){
            const Shape::Mesh& mesh = config.shapes[sid].mesh;
            double max_vertex = 0.0;
            for(size_t vid = 0; vid < mesh.n_vertices; ++vid){
                double dist = glm::length(mesh.vertices[vid]._coord);
                if(dist > max_vertex) max_vertex = dist;
            }
            shape_outradii[sid] = max_vertex;
        }
        else shape_outradii[sid] = 1.0;
    }

    double max_outradius = 0.0;
    for(auto particle: config.particles){
        double outradius = particle.size * shape_outradii[particle.shape_id];
        if(outradius > max_outradius) max_outradius = outradius;
    }

    out_radius_ = glm::length(glm::vec3(
        (config.box[0][0] + config.box[0][1] + config.box[0][2]) / 2.0,
        (config.box[1][1] + config.box[1][2]) / 2.0,
        config.box[2][2] / 2.0
    )) + max_outradius;

    float init_zoom = -4.0;
    for(int i = 0; i < 3; i++){
        GLfloat max_zoom = -3.5f*glm::length(glm::transpose(config.box)[i]);
        if(max_zoom < init_zoom) init_zoom = max_zoom;
    }

    znear_ = -init_zoom - out_radius_;
    zfar_  = -init_zoom + 2.0 * out_radius_;


    //TODO: Move these to constructor with some other default values
    view_pos_ = glm::vec3(0.0, 0.0, -init_zoom);
    view_dir_ = -view_pos_;

    set_projection();

    light_projection_matrix_ = glm::ortho(-out_radius_, out_radius_, -out_radius_, out_radius_, 0.0f, 2.0f * out_radius_);
    view_matrix_             = glm::lookAt(view_pos_, view_pos_ + view_dir_, glm::vec3(0.0, 1.0, 0.0));
    inv_view_matrix_         = glm::inverse(view_matrix_);
    light_view_matrix_       = glm::lookAt(-out_radius_ * light_.direction_, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    size_t n_particles = config.particles.size();
    size_t n_shapes = config.shapes.size();

    shape_vaos_ = new unsigned int[n_shapes]{};
    shape_vbos_ = new unsigned int[n_shapes]{};

    particle_flags_  = new unsigned int[n_particles]{};
    particle_colors_ = new glm::vec3[n_particles];
    model_matrices_  = new glm::mat4[n_particles];

    glm::mat4 tMatrix = glm::translate(
        glm::mat4(1.0),
        glm::vec3(
            -(config.box[0][0] + config.box[0][1] + config.box[0][2]) / 2.0,
            -(config.box[1][1] + config.box[1][2]) / 2.0,
            -config.box[2][2] / 2.0
        )
    );

    //Count shape instances and copy particles_
    draw_pids_.clear();
    for(size_t i = 0; i < n_particles; ++i){
        particle_colors_[i] = glm::vec3(77, 27, 147) / 255.0f;
        draw_pids_.push_back(i);

        glm::mat4 tLocalMatrix = glm::translate(tMatrix, config.particles[i].pos);
        glm::mat4 rLocalMatrix = glm::rotate(
            glm::mat4(1.0),
            config.particles[i].rot.x,
            glm::vec3(config.particles[i].rot.y, config.particles[i].rot.z, config.particles[i].rot.w)
        );

        model_matrices_[i] = tLocalMatrix * rLocalMatrix;
    }

    glGenVertexArrays(n_shapes, shape_vaos_);
    glGenBuffers(n_shapes, shape_vbos_);

    for(size_t shape_id = 0; shape_id < n_shapes; ++shape_id){
        glBindVertexArray(shape_vaos_[shape_id]);
        if(config.shapes[shape_id].type == Shape::MESH){
            const Shape::Mesh& mesh = config.shapes[shape_id].mesh;

            glBindBuffer(GL_ARRAY_BUFFER, shape_vbos_[shape_id]);
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
            glBindBuffer(GL_ARRAY_BUFFER, shape_vbos_[shape_id]);
            glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        }
    }
    glBindVertexArray(0);

    //TODO: Move shader uniform initialization to constructor

    // SSAO Uniforms
    glm::mat2 iproj = glm::mat2(inv_projection_matrix_[2][2], inv_projection_matrix_[2][3],
                                inv_projection_matrix_[3][2], inv_projection_matrix_[3][3]);

    sh_ssao_->bind();
    {
        m_ssao_.UploadUniforms(*sh_ssao_);
        sh_ssao_->setUniform("NormalMap", 0);
        sh_ssao_->setUniform("DepthMap", 1);
        sh_ssao_->setUniform("depth_iproj", 1, iproj);
        sh_ssao_->setUniform("projectionMatrix", 1, projection_matrix_);
        sh_ssao_->setUniform("invProjMatrix", 1, inv_projection_matrix_);
    }

    // Blur Uniforms
    sh_blur_->bind();
    {
        sh_blur_->setUniform("aoSampler", 0);
        sh_blur_->setUniform("use_blur", int(is_blur_active_));
    }

    // Edge Detection Uniforms
    sh_edge_detection_->bind();
    {
        sh_edge_detection_->setUniform("colorTex", 0);
        sh_edge_detection_->setUniform("texel_size", 1, glm::vec2(1.0f / window_width_, 1.0f / window_height_));
    }

    // Blend weights Uniforms
    sh_blend_weights_->bind();
    {
        sh_blend_weights_->setUniform("edgesTex", 0);
        sh_blend_weights_->setUniform("areaTex", 1);
        sh_blend_weights_->setUniform("searchTex", 2);
        sh_blend_weights_->setUniform("texel_size", 1, glm::vec2(1.0f / window_width_, 1.0f / window_height_));
    }

    // Blend Uniforms
    sh_blend_->bind();
    {
        sh_blend_->setUniform("colorTex", 0);
        sh_blend_->setUniform("blendTex", 1);
        sh_blend_->setUniform("texel_size", 1, glm::vec2(1.0f / window_width_, 1.0f / window_height_));
    }

    // Accumulator Uniforms
    sh_accumulator_->bind();
    {
        sh_accumulator_->setUniform("ColorMap", 0);
        sh_accumulator_->setUniform("NormalMap", 1);
        sh_accumulator_->setUniform("DepthMap", 2);
        sh_accumulator_->setUniform("LightDepthMap", 3);
        sh_accumulator_->setUniform("depth_iproj", 1, iproj);
        sh_accumulator_->setUniform("skyColor", 1, sky_color_);
        sh_accumulator_->setUniform("invProjMatrix", 1, inv_projection_matrix_);
    }

    sh_text_->bind();
    {
        glm::mat4 projection_matrix_ = glm::ortho(0.0f, (float)window_width_, 0.0f, (float)window_height_);
        sh_text_->setUniform("projectionMatrix", 1, projection_matrix_);
        sh_text_->setUniform("inSampler", 0);
        sh_text_->setUniform("inColor", 1, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
    }

    sh_points_->bind();
    {
        sh_points_->setUniform("radius", point_radius_);
        sh_points_->setUniform("outline_radius", point_outline_radius_);
        sh_points_->setUniform("color", 1, point_color_);
        sh_points_->setUniform("outline_color", 1, point_outline_color_);
    }
}

//TODO: Handle Orthographic projection correctly.
bool Scene::raytrace(int x, int y, int& pid){
    if(!grid_) grid_ = new Grid(*config_);

    glm::vec3 ray_origin, ray_dir;
    if(projection_type_ == Projection::PERSPECTIVE){
        glm::mat4 imodel_matrix = glm::inverse(model_matrix_);
        glm::vec3 o = glm::vec3(imodel_matrix * inv_view_matrix_ * glm::vec4(glm::vec3(0.0), 1.0));
        glm::vec4 mouse_clip = glm::vec4(2.0f * x / window_width_ - 1.0f, 1.0f - 2.0f * y / window_height_, 0.0f, 1.0f);
        glm::vec4 dir = imodel_matrix * inv_view_matrix_ * inv_projection_matrix_ * mouse_clip;
        dir /= dir.w;

        ray_origin = o;
        ray_dir    = glm::normalize(glm::vec3(dir) - o);
    }
    else{
        glm::mat4 imodel_matrix = glm::inverse(model_matrix_);
        glm::vec4 mouse_clip = glm::vec4(2.0f * x / window_width_ - 1.0f, 1.0f - 2.0f * y / window_height_, -1.0f, 1.0f);
        glm::vec4 o = imodel_matrix * inv_view_matrix_ * inv_projection_matrix_ * mouse_clip;

        ray_origin = glm::vec3(o) / o.w;
        ray_dir    = glm::mat3(imodel_matrix) * glm::mat3(inv_view_matrix_) * glm::vec3(0.0, 0.0, -1.0);
    }

    if(is_clip_plane_active_){
        float t = -(glm::dot(ray_origin, glm::vec3(clip_plane_)) + clip_plane_.w) / glm::dot(ray_dir, glm::vec3(clip_plane_));
        ray_origin += ray_dir * t;
    }

    float t = FLT_MAX;
    return grid_->raycast(ray_origin, ray_dir, t, pid);
}

void Scene::select_particle(int pid){
    auto iter = std::find(selected_pids_.begin(), selected_pids_.end(), pid);
    if(iter != selected_pids_.end()){
        selected_pids_.erase(iter);
    }
    else selected_pids_.push_back(pid);
}

bool Scene::is_selected(int pid)const{
    return std::any_of(selected_pids_.begin(), selected_pids_.end(), [pid](int id) -> bool {return id == pid;});
}

void Scene::clear_selection(void){
    selected_pids_.clear();
}

void Scene::hide_particle(int pid){
    if(!(particle_flags_[pid] & ParticleFlags::Hidden)){
        particle_flags_[pid] |= ParticleFlags::Hidden;
        grid_->ignore_id(pid);
    }
}

void Scene::unhide_particle(int pid){
    if(particle_flags_[pid] & ParticleFlags::Hidden){
        particle_flags_[pid] &= ~ParticleFlags::Hidden;
        grid_->unignore_id(pid);
    }
}

void Scene::set_particle_color(int pid, const glm::vec3& color){
    particle_colors_[pid] = color;
}

void Scene::toggle_particle_point_drawing(int pid){
    particle_flags_[pid] ^= ParticleFlags::Point;
}

//TODO: Perhaps add a should_update_point_uniforms.
void Scene::set_point_radius(float radius){
    point_radius_ = radius;
    sh_points_->bind();
    sh_points_->setUniform("radius", point_radius_);
}

void Scene::set_point_outline_radius(float radius){
    point_outline_radius_ = radius;
    sh_points_->bind();
    sh_points_->setUniform("outline_radius", point_outline_radius_);
}

void Scene::set_point_color(const glm::vec4& color){
    point_color_ = color;
    sh_points_->bind();
    sh_points_->setUniform("color", 1, point_color_);
}

void Scene::set_point_outline_color(const glm::vec4& color){
    point_outline_color_ = color;
    sh_points_->bind();
    sh_points_->setUniform("outline_color", 1, point_outline_color_);
}

void Scene::wsize_changed(int w, int h){
    window_width_ = w;
    window_height_ = h;
    glViewport(0, 0, window_width_, window_height_);

    set_projection();

    m_gbuffer_.Resize(window_width_, window_height_);
    m_edge_buffer.Resize(window_width_, window_height_);
    m_blend_buffer_.Resize(window_width_, window_height_);
    m_accumulator_.Resize(window_width_, window_height_);
    m_shadowmap_.Resize(window_width_, window_height_);

    sh_ssao_->bind();
    m_ssao_.Resize(window_width_, window_height_, sh_ssao_);

    // Edge Detection Uniforms
    glm::vec2 texel_size = glm::vec2(1.0f / window_width_, 1.0f / window_height_);

    sh_edge_detection_->bind();
    sh_edge_detection_->setUniform("texel_size", 1, texel_size);

    sh_blend_weights_->bind();
    sh_blend_weights_->setUniform("texel_size", 1, texel_size);

    sh_blend_->bind();
    sh_blend_->setUniform("texel_size", 1, texel_size);

    sh_text_->bind();
    glm::mat4 projection_matrix = glm::ortho(0.0f, (float)window_width_, 0.0f, (float)window_height_);
    sh_text_->setUniform("projectionMatrix", 1, projection_matrix);
}

void Scene::process(void){
    set_projection();
    light_view_matrix_ = glm::lookAt(-out_radius_ * light_.direction_, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    glm::mat2 iproj = glm::mat2(inv_projection_matrix_[2][2], inv_projection_matrix_[2][3],
                                inv_projection_matrix_[3][2], inv_projection_matrix_[3][3]);

    sh_ssao_->bind();
    {
        sh_ssao_->setUniform("depth_iproj", 1, iproj);
        sh_ssao_->setUniform("projectionMatrix", 1, projection_matrix_);
        sh_ssao_->setUniform("invProjMatrix", 1, inv_projection_matrix_);
    }

    sh_accumulator_->bind();
    {
        sh_accumulator_->setUniform("depth_iproj", 1, iproj);
        sh_accumulator_->setUniform("invProjMatrix", 1, inv_projection_matrix_);
    }

    draw_points_end_ = std::partition(draw_pids_.begin(), draw_pids_.end(),
        [=](int i) -> bool {
            return (particle_flags_[i] & ParticleFlags::Point);
        }
    );

    double* depths = new double[config_->particles.size()];
    auto mv_matrix = view_matrix_ * model_matrix_;
    for(size_t i = 0; i < config_->particles.size(); ++i){
        auto pos = mv_matrix * model_matrices_[i] * glm::vec4(0.0, 0.0, 0.0, 1.0);
        depths[i] = pos.z / pos.w;
    }

    std::sort(draw_pids_.begin(), draw_points_end_,
        [depths](int i, int j) -> bool {
            return (depths[i] < depths[j]);
        }
    );

    std::sort(draw_points_end_, draw_pids_.end(),
        [depths](int i, int j) -> bool {
            return (depths[i] > depths[j]);
        }
    );

    delete[] depths;
}

void Scene::set_projection(void){
    if(projection_type_ == Projection::ORTHOGRAPHIC){
        float half_length = glm::length(view_pos_) * tan(0.5f * glm::radians(fov_ + zoom_));
        float aspect      = float(window_width_) / window_height_;

        projection_matrix_ = glm::ortho(-half_length * aspect, half_length * aspect, -half_length, half_length, znear_, zfar_);
        inv_projection_matrix_    = glm::inverse(projection_matrix_);
    }
    else{
        projection_matrix_ = glm::perspective(glm::radians(fov_ + zoom_), (float)window_width_/window_height_, znear_, zfar_);
        inv_projection_matrix_    = glm::inverse(projection_matrix_);
    }
}

void Scene::set_box_line_width(float linewidth){
    line_width_ = linewidth;
}

void Scene::set_projection_type(Projection ptype){
    projection_type_ = ptype;
}

//TODO: Improve line rendering!!!
void Scene::drawConfigurationBox(void)const{

    glm::mat4 MVPMatrix = projection_matrix_ * view_matrix_ * model_matrix_;
    sh_quad_line_->setUniform("MVPMatrix", 1, MVPMatrix);
    sh_quad_line_->setUniform("line_width", line_width_);

    glBindVertexArray(box_vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 12);
    glDrawArrays(GL_TRIANGLE_STRIP, 12, 12);
}

void Scene::render(void){
    if(!config_) return;
    glUseProgram(0);

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    //Pre-clear the accumulator.
    m_accumulator_.Bind();
    glClearColor(background_color_.x, background_color_.y, background_color_.z, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);


    if(draw_points_end_ != draw_pids_.end()){
        //"Shadow Pass"
        {
            m_shadowmap_.Bind();

            glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            glViewport(0, 0, window_width_ * 2, window_height_ * 2);

            for(size_t shape_id = 0; shape_id < config_->shapes.size(); ++shape_id){
                glBindVertexArray(shape_vaos_[shape_id]);

                if(config_->shapes[shape_id].type == Shape::MESH){
                    size_t n_vertices = config_->shapes[shape_id].mesh.n_vertices;
                    if(is_clip_plane_active_){
                        glEnable(GL_STENCIL_TEST);
                        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR_WRAP, GL_INCR_WRAP);
                        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR_WRAP, GL_DECR_WRAP);
                        glStencilFunc(GL_ALWAYS, 0, 0);
                        glStencilMask(0xFF);
                        glEnable(GL_CLIP_DISTANCE0);
                        glDisable(GL_CULL_FACE);
                    }

                    sh_shadowmap_instanced_->bind();

                    sh_shadowmap_instanced_->setUniform("MVPMatrix", 1, light_projection_matrix_ * light_view_matrix_ * model_matrix_);
                    sh_shadowmap_instanced_->setUniform("clip_plane", 1, clip_plane_);

                    for(auto pid_itr = draw_points_end_; pid_itr != draw_pids_.end(); ++pid_itr){
                        auto pid = *pid_itr;
                        if((config_->particles[pid].shape_id != shape_id) ||
                           (particle_flags_[pid] & ParticleFlags::Hidden)) continue;
                        sh_shadowmap_instanced_->setUniform("ModelMatrix", 1, model_matrices_[pid]);
                        glDrawArrays(GL_TRIANGLES, 0, n_vertices);
                    }

                    if(is_clip_plane_active_){
                        sh_gbuffer_->bind();

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
                        glm::mat4 MVPMatrix = light_projection_matrix_ * light_view_matrix_ * model_matrix_ * plane_model_matrix;
                        sh_gbuffer_->setUniform("MVPMatrix", 1, MVPMatrix);

                        glBindVertexArray(plane_vao_);
                        glDrawArrays(GL_TRIANGLES, 0, 6);

                        glDisable(GL_STENCIL_TEST);
                        glDisable(GL_CLIP_DISTANCE0);
                    }
                }
                else{
                    sh_shadowmap_spheres_->bind();

                    sh_shadowmap_spheres_->setUniform("clip", is_clip_plane_active_);
                    sh_shadowmap_spheres_->setUniform("radius", 0.5f);
                    sh_shadowmap_spheres_->setUniform("clip_plane", 1, clip_plane_);
                    sh_shadowmap_spheres_->setUniform("MVMatrix", 1, light_view_matrix_ * model_matrix_);
                    sh_shadowmap_spheres_->setUniform("ProjectionMatrix", 1, light_projection_matrix_);

                    for(auto pid_itr = draw_points_end_; pid_itr != draw_pids_.end(); ++pid_itr){
                        auto pid = *pid_itr;
                        if((config_->particles[pid].shape_id != shape_id) ||
                           (particle_flags_[pid] & ParticleFlags::Hidden)) continue;
                        sh_shadowmap_spheres_->setUniform("ModelMatrix", 1, model_matrices_[pid]);
                        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    }
                }
            }

            glViewport(0, 0, window_width_, window_height_);
        }

        //"FBO Pass"
        {
            m_gbuffer_.Bind();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            for(size_t shape_id = 0; shape_id < config_->shapes.size(); ++shape_id){
                glBindVertexArray(shape_vaos_[shape_id]);
                if(config_->shapes[shape_id].type == Shape::MESH){
                    int n_vertices = config_->shapes[shape_id].mesh.n_vertices;
                    if(is_clip_plane_active_){
                        glEnable(GL_STENCIL_TEST);
                        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR_WRAP, GL_INCR_WRAP);
                        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR_WRAP, GL_DECR_WRAP);
                        glStencilFunc(GL_ALWAYS, 0, 0);
                        glStencilMask(0xFF);
                        glEnable(GL_CLIP_DISTANCE0);
                        glDisable(GL_CULL_FACE);
                    }

                    sh_gbuffer_instanced_->bind();

                    sh_gbuffer_instanced_->setUniform("clip_plane", 1, clip_plane_);
                    sh_gbuffer_instanced_->setUniform("MVMatrix", 1, view_matrix_ * model_matrix_);
                    sh_gbuffer_instanced_->setUniform("ProjectionMatrix", 1, projection_matrix_);

                    for(auto pid_itr = draw_points_end_; pid_itr != draw_pids_.end(); ++pid_itr){
                        auto pid = *pid_itr;
                        if((config_->particles[pid].shape_id != shape_id) ||
                           (particle_flags_[pid] & ParticleFlags::Hidden)) continue;
                        sh_gbuffer_instanced_->setUniform("ModelMatrix", 1, model_matrices_[pid]);
                        sh_gbuffer_instanced_->setUniform("in_Color", 1, particle_colors_[pid]);
                        glDrawArrays(GL_TRIANGLES, 0, n_vertices);
                    }

                    if(is_clip_plane_active_){
                        sh_gbuffer_->bind();

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
                        glm::mat4 MVPMatrix = projection_matrix_ * view_matrix_ * model_matrix_ * plane_model_matrix;
                        glm::mat3 NormalMatrix = glm::mat3(glm::transpose(glm::inverse(view_matrix_ * model_matrix_ * plane_model_matrix)));
                        sh_gbuffer_->setUniform("NormalMatrix", 1, NormalMatrix);
                        sh_gbuffer_->setUniform("MVPMatrix", 1, MVPMatrix);

                        glBindVertexArray(plane_vao_);
                        glDrawArrays(GL_TRIANGLES, 0, 6);

                        glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                        glDisable(GL_STENCIL_TEST);
                        glDisable(GL_CLIP_DISTANCE0);
                        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                    }
                }
                else{
                    sh_spheres_->bind();
                    sh_spheres_->setUniform("clip", is_clip_plane_active_);
                    sh_spheres_->setUniform("clip_plane", 1, clip_plane_);

                    //TODO: Move this to a better place
                    if(projection_type_ == Projection::PERSPECTIVE){
                        sh_spheres_->setUniform("perspective_scale", 1.0f / cosf(0.5f * glm::radians(fov_ + zoom_)));
                    }
                    else{
                        sh_spheres_->setUniform("perspective_scale", 1.0f);
                    }
                    sh_spheres_->setUniform("radius", 0.5f);
                    sh_spheres_->setUniform("MVMatrix", 1, view_matrix_ * model_matrix_);
                    sh_spheres_->setUniform("ProjectionMatrix", 1, projection_matrix_);
                    sh_spheres_->setUniform("InvProjectionMatrix", 1, inv_projection_matrix_);

                    for(auto pid_itr = draw_points_end_; pid_itr != draw_pids_.end(); ++pid_itr){
                        auto pid = *pid_itr;
                        if((config_->particles[pid].shape_id != shape_id) ||
                           (particle_flags_[pid] & ParticleFlags::Hidden)) continue;
                        sh_spheres_->setUniform("in_Color", 1, particle_colors_[pid]);
                        sh_spheres_->setUniform("ModelMatrix", 1, model_matrices_[pid]);
                        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    }
                }
            }
        }

        //TODO: What's happening here?!
        glBindVertexArray(fullscreen_triangle_vao_);
        //"SSAO Pass"
        {
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);

            //"SSAO Calc Pass"
            m_ssao_.Bind();
            glClear(GL_COLOR_BUFFER_BIT);

            m_gbuffer_.BindTexture(CGBuffer::GBUFF_TEXTURE_TYPE_NORMAL, 0);
            m_gbuffer_.BindTexture(CGBuffer::GBUFF_TEXTURE_TYPE_DEPTH, 1);
            m_ssao_.BindTexture(Cssao::TEXTURE_TYPE_NOISE, 2);

            sh_ssao_->bind();
            {
                m_ssao_.UpdateUniforms(*sh_ssao_);

                glDrawArrays(GL_TRIANGLES, 0, 3);
            }


            //"SSAO Blur Pass"

            m_gbuffer_.Bind();
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
            m_ssao_.BindTexture(Cssao::TEXTURE_TYPE_SSAO, 0);

            sh_blur_->bind();
            {
                sh_blur_->setUniform("use_blur", int(is_blur_active_));

                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }

        //"Gather Pass"
        {
            //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            if(is_box_drawing_active_){
                glDepthMask(GL_TRUE);
                glEnable(GL_DEPTH_TEST);
            }
            m_accumulator_.Bind();

            m_gbuffer_.BindTexture(CGBuffer::GBUFF_TEXTURE_TYPE_DIFFUSE, 0);
            m_gbuffer_.BindTexture(CGBuffer::GBUFF_TEXTURE_TYPE_NORMAL, 1);
            m_gbuffer_.BindTexture(CGBuffer::GBUFF_TEXTURE_TYPE_DEPTH, 2);
            m_shadowmap_.BindTexture(3);

            sh_accumulator_->bind();
            {
                sh_accumulator_->setUniform("invProjMatrix", 1, inv_projection_matrix_);
                sh_accumulator_->setUniform("skyColor", 1, sky_color_);

                glm::mat4 depth_matrix = biasMatrix * light_projection_matrix_ * light_view_matrix_ * inv_view_matrix_;
                sh_accumulator_->setUniform("depth_matrix", 1, depth_matrix);

                glm::vec3 lightViewDirection = glm::mat3(view_matrix_) * light_.direction_;
                sh_accumulator_->setUniform("light.direction", 1, lightViewDirection);
                sh_accumulator_->setUniform("light.Si", light_.specular_);
                sh_accumulator_->setUniform("light.Di", light_.diffuse_);
                sh_accumulator_->setUniform("light.Ai", light_.ambient_);
                sh_accumulator_->setUniform("light.Intensity", light_.intensity_);

                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        }
    }

    if(is_box_drawing_active_){
        sh_quad_line_->bind();
        {
            glEnable(GL_BLEND);
            glDepthMask(GL_TRUE);
            glDisable(GL_CULL_FACE);
            drawConfigurationBox();
            glDisable(GL_BLEND);
        }
    }

    //"Point Drawing Pass"
    if(draw_points_end_ != draw_pids_.begin()){
        glEnable(GL_BLEND);
        //glDisable(GL_DEPTH_TEST);
        glBindVertexArray(fullscreen_triangle_vao_);

        sh_points_->bind();

        sh_points_->setUniform("MVMatrix", 1, view_matrix_ * model_matrix_);
        sh_points_->setUniform("ProjectionMatrix", 1, projection_matrix_);

        if(is_clip_plane_active_){
            glEnable(GL_CLIP_DISTANCE0);
            sh_points_->setUniform("clip_plane", 1, clip_plane_);
        }

        for(auto pid_itr = draw_pids_.begin(); pid_itr != draw_points_end_; ++pid_itr){
            auto pid = *pid_itr;
            if(particle_flags_[pid] & ParticleFlags::Hidden) continue;
            sh_points_->setUniform("ModelMatrix", 1, model_matrices_[pid]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        if(is_clip_plane_active_) glDisable(GL_CLIP_DISTANCE0);
        glDisable(GL_BLEND);
    }

    //"Selection Pass"
    glEnable(GL_STENCIL_TEST);
    for(auto selected_pid: selected_pids_){
        int shape_id = config_->particles[selected_pid].shape_id;

        glBindVertexArray(shape_vaos_[shape_id]);
        glEnable(GL_DEPTH_TEST);
        glStencilMask(0xFF);
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        if(config_->shapes[shape_id].type == Shape::MESH){
            int n_vertices = config_->shapes[shape_id].mesh.n_vertices;
            sh_color_->bind();

            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glm::mat4 mvp_matrix = projection_matrix_ * view_matrix_ * model_matrix_ * model_matrices_[selected_pid];
            sh_color_->setUniform("mvp_matrix", 1, mvp_matrix);
            glDrawArrays(GL_TRIANGLES, 0, n_vertices);

            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0x00);
            glDisable(GL_DEPTH_TEST);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            mvp_matrix = mvp_matrix * glm::scale(glm::mat4(1.0), glm::vec3(1.1));
            sh_color_->setUniform("mvp_matrix", 1, mvp_matrix);
            glDrawArrays(GL_TRIANGLES, 0, n_vertices);
        }
        else{
            sh_color_sphere_->bind();

            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            float perspective_scale = 1.0f / cosf(0.5f * glm::radians(fov_ + zoom_));

            glm::mat4 mv_matrix = view_matrix_ * model_matrix_ * model_matrices_[selected_pid];
            sh_color_sphere_->setUniform("mv_matrix", 1, mv_matrix);
            sh_color_sphere_->setUniform("projection_matrix", 1, projection_matrix_);
            sh_color_sphere_->setUniform("iprojection_matrix", 1, inv_projection_matrix_);
            sh_color_sphere_->setUniform("perspective_scale", perspective_scale);
            sh_color_sphere_->setUniform("radius", 0.5f);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0x00);
            glDisable(GL_DEPTH_TEST);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            sh_color_sphere_->setUniform("radius", 1.1f * 0.5f);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

    }
    glDisable(GL_STENCIL_TEST);

    //"SMAA"
    {
        glBindVertexArray(fullscreen_triangle_vao_);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        //"Edge Pass"
        {
            m_edge_buffer.Bind();
            glClear(GL_COLOR_BUFFER_BIT);

            m_accumulator_.BindTexture(0);

            sh_edge_detection_->bind();
            {
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        }

        //"Blend weight Pass"
        {
            m_blend_buffer_.Bind();
            glClear(GL_COLOR_BUFFER_BIT);

            m_edge_buffer.BindTexture(0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, area_texture_);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, search_texture_);

            sh_blend_weights_->bind();
            {
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        }

        //"Blend Pass"
        {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            m_accumulator_.BindTexture(0);
            m_blend_buffer_.BindTexture(1);

            glEnable(GL_FRAMEBUFFER_SRGB);

            sh_blend_->bind();
            {
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        }
    }

    glDisable(GL_FRAMEBUFFER_SRGB);
}

void Scene::rotate(float angle, const glm::vec3& axis){
    model_matrix_ = glm::rotate(glm::mat4(1.0), angle, axis) * model_matrix_;
}

//TODO: Move to camera
void Scene::zoom(float dz){
    zoom_ = zoom_ - 2.0f * dz; // put wheel up and down in one
    if(zoom_ < -58) zoom_ = -58.0f;
    else if(zoom_ > 90) zoom_ = 90.0f;
}

glm::mat4 Scene::get_view_matrix(void)const{
    return view_matrix_;
}

glm::mat4 Scene::get_projection_matrix(void)const{
    return projection_matrix_;
}

glm::mat4 Scene::get_model_matrix(void)const{
    return model_matrix_;
}

void Scene::set_clip_plane(const glm::vec4& clip_plane){
    clip_plane_ = clip_plane;
}

void Scene::enable_clip_plane(void){
    is_clip_plane_active_ = true;
}

void Scene::disable_clip_plane(void){
    is_clip_plane_active_ = false;
}

void Scene::toggle_box(void){
    is_box_drawing_active_ = !is_box_drawing_active_;
}

void Scene::set_view_position(const glm::vec3& pos){
    view_pos_ = pos;
    view_matrix_    = glm::lookAt(view_pos_, view_pos_ + view_dir_, glm::vec3(0.0, 1.0, 0.0));
    inv_view_matrix_ = glm::inverse(view_matrix_);
}

const glm::vec3& Scene::get_view_position(void)const{
    return view_pos_;
}

void Scene::set_view_direction(const glm::vec3& dir){
    view_dir_ = dir;
    view_matrix_    = glm::lookAt(view_pos_, view_pos_ + view_dir_, glm::vec3(0.0, 1.0, 0.0));
    inv_view_matrix_ = glm::inverse(view_matrix_);
}

const glm::vec3& Scene::get_view_direction(void)const{
    return view_dir_;
}

glm::vec3 Scene::get_light_direction(void)const{
    return light_.direction_;
}

void Scene::set_light_direction(const glm::vec3& direction){
    light_.direction_ = direction;
}

float Scene::get_light_specular_intensity(void)const{
    return light_.specular_;
}

void Scene::set_light_specular_intensity(float specular){
    light_.specular_ = specular;
}

float Scene::get_light_diffuse_intensity(void)const{
    return light_.diffuse_;
}

void Scene::set_light_diffuse_intensity(float diffuse){
    light_.diffuse_ = diffuse;
}

float Scene::get_light_ambient_intensity(void)const{
    return light_.ambient_;
}

void Scene::set_light_ambient_intensity(float ambient){
    light_.ambient_ = ambient;
}

float Scene::get_light_intensity(void)const{
    return light_.intensity_;
}

void Scene::set_light_intensity(float intensity){
    light_.intensity_ = intensity;
}

void Scene::set_ssao_blur(bool value){
    is_blur_active_ = value;
}

bool Scene::is_ssao_blur_enabled(void)const{
    return is_blur_active_;
}

void Scene::set_ssao_radius(float radius){
    m_ssao_.setRadius(radius);
}

float Scene::get_ssao_radius(void)const{
    return m_ssao_.getRadius();
}

void Scene::set_ssao_num_samples(unsigned int n_samples){
    m_ssao_.setSamples(n_samples);
}

unsigned int Scene::get_ssao_num_samples(void)const{
    return m_ssao_.getSamples();
}

void Scene::set_sky_color(const glm::vec3& color){
    sky_color_ = color;
}

glm::vec3 Scene::get_sky_color(void)const{
    return sky_color_;
}

void Scene::set_background_color(const glm::vec3& color){
    background_color_ = color;
}

glm::vec3 Scene::get_background_color(void)const{
    return background_color_;
}

void Scene::draw_text(const char* text, const Text::Properties& props){

    float scale = props.width_ / fontManager_->get_default_size();
    float dx = 0;
    float dy = 0;

    Text::Font* font = fontManager_->get_font(props.font_);
    if(!font) return;

    glEnable(GL_BLEND);
    glBindVertexArray(quad_vao_);
    glActiveTexture(GL_TEXTURE0);

    sh_text_->bind();
    for(const char* char_ptr = text; *char_ptr != '\0'; ++char_ptr){
        char character = *char_ptr;
        const Text::Glyph* glyph = font->get_char_glyph(character);
        if(!glyph){
            glDisable(GL_BLEND);
            return;
        }

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
        float y    = window_height_ - props.y_ - dy + glyph->top_bearing() * scale;

        glm::mat4 model_matrix_ = glm::scale(glm::mat4(1.0f), glm::vec3(width, -height, 1.0f));
        model_matrix_ = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f)) * model_matrix_;
        sh_text_->setUniform("modelMatrix", 1, model_matrix_);
        sh_text_->setUniform("inColor", 1, props.color_);
        glBindTexture(GL_TEXTURE_2D, (GLuint)glyph->tex_id());
    
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        dx += glyph->advance_width() * scale;
    }

    glDisable(GL_BLEND);
}


