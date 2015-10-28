#include "include/scene.h"
#include <GL/glew.h>

#ifdef _WIN32
#include <GL/wglew.h>
#elif __linux
#include <GL/glxew.h>
#endif

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "include/gl_utils.h"
#include "include/shader.h"
#include "include/grid.h"

static const glm::mat4 biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
);

enum ParticleFlags{
    Hidden = 0x01
};

Scene::Scene(int width, int height):
    windowWidth(width), windowHeight(height),
    fov_(60.0f), zoom_(0.0f),
	modelMatrix(1.0),
	m_bgColor(glm::vec3(44, 114, 220) / 255.0f),
	diffcolor(glm::vec3(77, 27, 147) / 255.0f),
	skycolor(0.529, 0.808, 0.921),
    clip_plane_{0.0, 0.0, -1.0, 0.0},
	shape_vaos(nullptr), shape_vbos(nullptr),
    particle_flags(nullptr),
    particles(nullptr), shapes(nullptr), n_shapes(0), n_particles(0),
    vaoBox(0), vboBox(0), iboBox(0), fullscreen_triangle_vao(0),
    is_scene_loaded(false), is_clip_plane_activated_(false), drawBox(false), m_blur(true),
    projection_type(Projection::PERSPECTIVE),
	light(glm::vec3(-0.27, -0.91, -0.33)),
    sh_gbuffer(nullptr), sh_gbuffer_instanced(nullptr), sh_ssao(nullptr),
    sh_shadowmap_instanced(nullptr), sh_blur(nullptr), sh_accumulator(nullptr),
    sh_edge_detection(nullptr), sh_blend_weights(nullptr), sh_blend(nullptr),
    sh_spheres(nullptr), sh_shadowmap_spheres(nullptr),
    grid(nullptr)

{
	glewExperimental = GL_TRUE;
	GLenum error = glewInit(); //Enable GLEW
	if(error != GLEW_OK) throw GlewInitializationException();
	glError(__FILE__,__LINE__);

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
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    try{
        sh_gbuffer = new Shader("shaders/gbuffer.vert", "shaders/gbuffer.frag");
        sh_gbuffer_instanced = new Shader("shaders/gbuffer_instanced.vert", "shaders/gbuffer_instanced.frag");
        sh_ssao = new Shader("shaders/ssao.vert", "shaders/ssao.frag");
        sh_shadowmap_instanced = new Shader("shaders/shadowmap_instanced.vert");
        sh_shadowmap_spheres = new Shader("shaders/shadowmap_spheres.vert", "shaders/shadowmap_spheres.frag");
        sh_blur = new Shader("shaders/blur.vert", "shaders/blur.frag");
        sh_accumulator = new Shader("shaders/accumulator.vert", "shaders/accumulator.frag");
        sh_edge_detection = new Shader("shaders/smaa/edge_detection.vert", "shaders/smaa/edge_detection.frag");
        sh_blend_weights = new Shader("shaders/smaa/blend_weights.vert", "shaders/smaa/blend_weights.frag");
        sh_blend = new Shader("shaders/smaa/blend.vert", "shaders/smaa/blend.frag");
        sh_spheres = new Shader("shaders/spheres.vert", "shaders/spheres.frag");
        sh_color = new Shader("shaders/color.vert", "shaders/color.frag");
        sh_color_sphere = new Shader("shaders/color_sphere.vert", "shaders/color_sphere.frag");
    }
    catch(Shader::InitializationException){
        delete sh_gbuffer;
        delete sh_gbuffer_instanced;
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
        throw;
    }

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
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray((GLuint)0);

    GLushort elements[] = {
        3, 6, 7, 5,
        1, 4, 2, 0,
        7, 4, 2, 6, 1, 0, 3, 5
    };

    glGenBuffers(1, &iboBox);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboBox);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

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

    //Generate and load smaa textures
    //TODO: Handle read errors!!!
    FILE* fp = fopen("res/smaa_area.raw", "rb");
    if(fp){
        char* buffer = new char[160 * 560 * 2];

        fread(buffer, 160 * 560 * 2, 1, fp);

        glGenTextures(1, &area_texture);
        glBindTexture(GL_TEXTURE_2D, area_texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, 160, 560, 0, GL_RG, GL_UNSIGNED_BYTE, buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        delete[] buffer;

        fclose(fp);
    }

    fp = fopen("res/smaa_search.raw", "rb");
    if(fp){
        char* buffer = new char[66 * 33];

        fread(buffer, 66 * 33, 1, fp);

        glGenTextures(1, &search_texture);
        glBindTexture(GL_TEXTURE_2D, search_texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 66, 33, 0, GL_RED, GL_UNSIGNED_BYTE, buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        delete[] buffer;

        fclose(fp);
    }
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
	glDeleteBuffers(1, &iboBox);

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

        glm::vec3 vertices[] = {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(config.box[0][0], config.box[1][0], config.box[2][0]),
            glm::vec3(config.box[0][1], config.box[1][1], config.box[2][1]),
            glm::vec3(config.box[0][2], config.box[1][2], config.box[2][2]),
            glm::vec3(config.box[0][0] + config.box[0][1], config.box[1][0] + config.box[1][1], config.box[2][0] + config.box[2][1]),
            glm::vec3(config.box[0][0] + config.box[0][2], config.box[1][0] + config.box[1][2], config.box[2][0] + config.box[2][2]),
            glm::vec3(config.box[0][1] + config.box[0][2], config.box[1][1] + config.box[1][2], config.box[2][1] + config.box[2][2]),
            glm::vec3(config.box[0][0] + config.box[0][1] + config.box[0][2],
                      config.box[1][0] + config.box[1][1] + config.box[1][2],
                      config.box[2][0] + config.box[2][1] + config.box[2][2])
        };

        for(size_t i = 0; i < 8; ++i) vertices[i] += offset;

        glBindBuffer(GL_ARRAY_BUFFER, vboBox);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 8, vertices, GL_STATIC_DRAW);
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
}

bool Scene::raytrace(int x, int y, int& pid){
    glm::mat4 imodel_matrix = glm::inverse(modelMatrix);
    glm::vec3 o = glm::vec3(imodel_matrix * invViewMatrix * glm::vec4(glm::vec3(0.0), 1.0));
    glm::vec4 mouse_clip = glm::vec4(2.0f * x / windowWidth - 1.0f, 1.0f - 2.0f * y / windowHeight, 0.0, 1.0);
    glm::vec4 dir = imodel_matrix * invViewMatrix * invProjMatrix * mouse_clip;
    dir /= dir.w;

    glm::vec3 ray_origin = o;
    glm::vec3 ray_dir = glm::normalize(glm::vec3(dir) - o);

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

    //TODO: Cache model_matrices[i] * glm::vec4(particles[i].pos, 1.0)
    double* depths = new double[n_particles];
    auto mv_matrix = viewMatrix * modelMatrix;
    for(int i = 0; i < n_particles; ++i){
        auto pos = mv_matrix * model_matrices[i] * glm::vec4(particles[i].pos, 1.0);
        depths[i] = pos.z / pos.w;
    }

    std::sort(draw_pids.begin(), draw_pids.end(),
        [depths](int i, int j) -> bool {
            return (depths[i] > depths[j]);
        }
    );
    delete[] depths;
}

void Scene::set_projection(void){
    if(projection_type == Projection::ORTHOGRAPHIC){
        float half_length = glm::length(view_pos) * tan(0.5f * glm::radians(fov_ + zoom_));
        projectionMatrix = glm::ortho(-half_length, half_length, -half_length, half_length, znear_, zfar_);
        invProjMatrix    = glm::inverse(projectionMatrix);
    }
    else{
        projectionMatrix = glm::perspective(glm::radians(fov_ + zoom_), (float)windowWidth/windowHeight, znear_, zfar_);
        invProjMatrix    = glm::inverse(projectionMatrix);
    }
}

void Scene::set_projection_type(Projection ptype){
    projection_type = ptype;
}

//TODO: Improve line rendering!!!
void Scene::drawConfigurationBox(void)const{

	glLineWidth(fabs(-0.067f * zoom_ + 4.0f));

	glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * modelMatrix;
	sh_gbuffer->setUniform("MVPMatrix", 1, MVPMatrix);
	sh_gbuffer->setUniform("diffColor", 0.01f, 0.01f, 0.01f);

	glBindVertexArray(vaoBox);
	glDrawElements(GL_LINE_LOOP, 8, GL_UNSIGNED_SHORT, 0);
	glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8*sizeof(GLushort)));

	glBindVertexArray(0);
}

void Scene::render(void){
    if(!is_scene_loaded) return;

	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

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

                for(auto pid: draw_pids){
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

                for(auto pid: draw_pids){
                    if((particles[pid].shape_id != shape_id) ||
                       (particle_flags[pid] & ParticleFlags::Hidden)) continue;
                    sh_shadowmap_instanced->setUniform("ModelMatrix", 1, model_matrices[pid]);
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

                for(auto pid: draw_pids){
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

                for(auto pid: draw_pids){
                    if((particles[pid].shape_id != shape_id) ||
                       (particle_flags[pid] & ParticleFlags::Hidden)) continue;
                    sh_shadowmap_instanced->setUniform("ModelMatrix", 1, model_matrices[pid]);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                }
            }
        }

        if(drawBox){
            sh_gbuffer->bind();
            {
                drawConfigurationBox();
            }
        }
    }

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

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
        m_gbuffer.Bind();
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
        m_accumulator.Bind();

        glClearColor(m_bgColor.x, m_bgColor.y, m_bgColor.z, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0, 0.0, 0.0, 1.0);

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

    //"Selection Pass"
    for(auto selected_pid: selected_pids){
        int shape_id = particles[selected_pid].shape_id;

        m_accumulator.Bind();

        glBindVertexArray(shape_vaos[shape_id]);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
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

        glDisable(GL_STENCIL_TEST);
        glDepthMask(GL_FALSE);
    }

    //"SMAA"
    {
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

