#include "include/opengl_3.h"
#include <GL/glew.h>

#ifdef _WIN32
#include <GL/wglew.h>
#elif __linux
#include <GL/glxew.h>
#endif

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include "include/gl_utils.h"
#include "include/shader.h"
#include "include/grid.h"

//#define DRAW_SPHERES

static const glm::mat4 biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
);

OpenGLContext::OpenGLContext(int width, int height):
    drawBox(false),
    windowWidth(width), windowHeight(height),
    fov(60.0f), zoom_(0.0f),
	modelMatrix(1.0),
	m_bgColor(glm::vec3(44, 114, 220) / 255.0f),
	diffcolor(glm::vec3(77, 27, 147) / 255.0f),
	skycolor(0.529, 0.808, 0.921),
	shape_instances(nullptr), shape_vaos(nullptr), shape_vertex_vbos(nullptr),
	shape_model_matrix_vbos(nullptr), shape_colors_vbos(nullptr), shape_num_vertices(nullptr),
    shape_single_vaos(nullptr),
    particles(nullptr), shape_types(nullptr), n_shapes(0), n_particles(0),
    vaoBox(0), vboBox(0), iboBox(0), fullscreen_triangle_vao(0),
    selected_pid(-1),
    is_scene_loaded(false), m_blur(true), m_rotating(false),
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
	if(!light.Init(sh_accumulator->id())) printf("Cannot bind light uniform");
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

	//createGui();
}

OpenGLContext::~OpenGLContext(void){
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
	if(n_shapes) glDeleteBuffers(n_shapes, shape_vertex_vbos);
	if(n_shapes) glDeleteBuffers(n_shapes, shape_model_matrix_vbos);
	if(n_shapes) glDeleteBuffers(n_shapes, shape_colors_vbos);
	if(n_shapes) glDeleteVertexArrays(n_shapes, shape_single_vaos);

    delete[] shape_vaos;
    delete[] shape_single_vaos;
    delete[] shape_vertex_vbos;
    delete[] shape_model_matrix_vbos;
    delete[] shape_colors_vbos;
    delete[] shape_instances;
    delete[] shape_types;
    delete[] shape_num_vertices;
    delete[] model_matrices;

    delete grid;

	glDeleteVertexArrays(1, &vaoBox);
	glDeleteBuffers(1, &vboBox);
	glDeleteBuffers(1, &iboBox);

	glDeleteVertexArrays(1, &fullscreen_triangle_vao);

	//TwTerminate();
}  

void OpenGLContext::createGui(void){
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(windowWidth, windowHeight);
	bar = TwNewBar("Parameters");
	
	TwDefine(
		"Parameters position='0 0' size='200 250' iconified=true");
	
	TwAddVarRW(bar, "Rotation", TW_TYPE_BOOLCPP, &m_rotating, "");	
	
	TwAddVarCB(bar, "Direction", TW_TYPE_DIR3F, CLight::SetDirCallback, CLight::GetDirCallback, &light,"\
		group=Light");
	
	TwAddVarCB(bar, "Specular", TW_TYPE_FLOAT, CLight::SetSpecIntCallback, CLight::GetSpecIntCallback, &light,"\
		min=0.0 max=2.0	step=0.01 group=Light");
	
	TwAddVarCB(bar, "Diffuse", TW_TYPE_FLOAT, CLight::SetDiffIntCallback, CLight::GetDiffIntCallback, &light,"\
		min=0.0 max=2.0	step=0.01 group=Light");
	
	TwAddVarCB(bar, "Ambient", TW_TYPE_FLOAT, CLight::SetAmbIntCallback, CLight::GetAmbIntCallback, &light,"\
		min=0.0 max=2.0	step=0.01 group=Light");
	
	TwAddVarCB(bar, "Intensity", TW_TYPE_FLOAT, CLight::SetIntCallback, CLight::GetIntCallback, &light,"\
		min=0.0 max=2.0	step=0.01 group=Light");	
		
	TwAddVarRW(bar, "Blur", TW_TYPE_BOOLCPP, &m_blur, "\
		group=AO");	
	
	TwAddVarCB(bar, "Radius", TW_TYPE_FLOAT, Cssao::SetRadiusCallback, Cssao::GetRadiusCallback, &m_ssao,"\
		help='The radius of the ambient occlusion sampling kernel.'\
		min=0.1 max=30.0 step=0.1 group=AO");
	
	TwAddVarCB(bar, "Samples", TW_TYPE_UINT32, Cssao::SetSamplesCallback, Cssao::GetSamplesCallback, &m_ssao,"\
		help='Number of samples for ambient occlusion. Increase for higher quality.'\
		min=4 max=256 step=2 group=AO");

	TwAddVarRW(bar, "Sky Color", TW_TYPE_COLOR3F, &skycolor," colormode=hls ");
	TwAddVarRW(bar, "Particle Color", TW_TYPE_COLOR3F, &diffcolor," colormode=hls ");
	TwAddVarRW(bar, "Background Color", TW_TYPE_COLOR3F, &m_bgColor," colormode=hls ");
}

void OpenGLContext::load_scene(const SimConfig& config){
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

    out_radius = glm::length(glm::vec3(
        (config.box[0][0] + config.box[0][1] + config.box[0][2]) / 2.0,
        (config.box[1][1] + config.box[1][2]) / 2.0,
        config.box[2][2] / 2.0
    )) + 1.0;

    //TODO: Fix
    float init_zoom = -4.0;
    for(int i=0;i<3;i++){
        GLfloat max_zoom = -3.5f*glm::length(glm::transpose(config.box)[i]);
        if(max_zoom < init_zoom) init_zoom = max_zoom;
    }

    znear = -init_zoom - out_radius;
    zfar  = -init_zoom + 2.0 * out_radius;

	
	projectionMatrix      = glm::perspective(glm::radians(fov + zoom_), (float)windowWidth/(float)windowHeight, znear, zfar);
    invProjMatrix         = glm::inverse(projectionMatrix);
    lightProjectionMatrix = glm::ortho(-out_radius, out_radius, -out_radius, out_radius, 0.0f, 2.0f * out_radius);
	viewMatrix            = glm::lookAt(glm::vec3(0.0, 0.0, -init_zoom), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    invViewMatrix         = glm::inverse(viewMatrix);
    lightViewMatrix       = glm::lookAt(-out_radius * light.getDirection(), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	
	shape_instances         = new unsigned int[config.n_shapes]{};
	shape_vaos              = new unsigned int[config.n_shapes]{};
	shape_single_vaos       = new unsigned int[config.n_shapes]{};
	shape_vertex_vbos       = new unsigned int[config.n_shapes]{};
	shape_model_matrix_vbos = new unsigned int[config.n_shapes]{};
	shape_colors_vbos       = new unsigned int[config.n_shapes]{};
	shape_num_vertices      = new unsigned int[config.n_shapes]{};

    n_shapes = config.n_shapes;
    shape_types = new Shape::Type[n_shapes];

    glGenVertexArrays(config.n_shapes, shape_vaos);
    glGenBuffers(config.n_shapes, shape_vertex_vbos);
    glGenBuffers(config.n_shapes, shape_model_matrix_vbos);
    glGenBuffers(config.n_shapes, shape_colors_vbos);
    glGenVertexArrays(config.n_shapes, shape_single_vaos);

    model_matrices = new glm::mat4[config.n_part];

    //Count shape instances and copy particles
    for(int i = 0; i < config.n_part; ++i){
        particles[i] = config.particles[i];
        ++shape_instances[config.particles[i].shape_id];
    }

    for(int shape_id = 0; shape_id < config.n_shapes; ++shape_id){
        shape_types[shape_id] = config.shapes[shape_id].type;

        glm::mat4* ModelArray = new glm::mat4[shape_instances[shape_id]];

        glm::mat4 tMatrix = glm::translate(
            glm::mat4(1.0),
            glm::vec3(
                -(config.box[0][0] + config.box[0][1] + config.box[0][2]) / 2.0,
                -(config.box[1][1] + config.box[1][2]) / 2.0,
                -config.box[2][2] / 2.0
            )
        );
        
        for(int pid = 0, i = 0; pid < config.n_part; ++pid){
            if(config.particles[pid].shape_id == shape_id){
                glm::mat4 tLocalMatrix = glm::translate(tMatrix, config.particles[pid].pos);
                glm::mat4 rLocalMatrix = glm::rotate(
                    glm::mat4(1.0),
                    config.particles[pid].rot.x,
                    glm::vec3(config.particles[pid].rot.y, config.particles[pid].rot.z, config.particles[pid].rot.w)
                );
                
                ModelArray[i++] = tLocalMatrix * rLocalMatrix;
                model_matrices[pid] = ModelArray[i - 1];
            }
        }

        glm::vec3* shape_colors = new glm::vec3[shape_instances[shape_id]];
        for(unsigned int pid = 0; pid < shape_instances[shape_id]; ++pid){
            shape_colors[pid] = diffcolor;
        }
        
        //Build instanced vao
        //TODO: Buffer data filling will be deferred until rendering.
        //We will have a needs_update kind of variable that will be checked
        //each frame.
        glBindVertexArray(shape_vaos[shape_id]);
        if(config.shapes[shape_id].type == Shape::MESH){
            //TODO: Find the maximal vertex here. This will be used for
            //constructing the cell list.
            const Shape::Mesh& mesh = config.shapes[shape_id].mesh;
            shape_num_vertices[shape_id] = mesh.n_vertices;

            glBindBuffer(GL_ARRAY_BUFFER, shape_vertex_vbos[shape_id]);
            glBufferData(GL_ARRAY_BUFFER, mesh.n_vertices * sizeof(Vertex), mesh.vertices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)0);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)sizeof(glm::vec3));

            glBindBuffer(GL_ARRAY_BUFFER, shape_colors_vbos[shape_id]);
            glBufferData(GL_ARRAY_BUFFER, shape_instances[shape_id] * sizeof(glm::vec3), shape_colors, GL_STATIC_DRAW);

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const GLvoid*)0);
            glVertexAttribDivisor(2, 1);

            glBindBuffer(GL_ARRAY_BUFFER, shape_model_matrix_vbos[shape_id]);
            glBufferData(GL_ARRAY_BUFFER, shape_instances[shape_id] * sizeof(glm::mat4), ModelArray, GL_STATIC_DRAW);

            for(int i = 0; i < 4; i++){ //MVP Matrices
                glEnableVertexAttribArray(3 + i);
                glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const GLvoid*)(sizeof(float) * i * 4));
                glVertexAttribDivisor(3 + i, 1);
            }
        }
        else if(config.shapes[shape_id].type == Shape::SPHERE){
            glm::vec3 vertices[] = {
                glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.0)
            };
            glBindBuffer(GL_ARRAY_BUFFER, shape_vertex_vbos[shape_id]);
            glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

            glBindBuffer(GL_ARRAY_BUFFER, shape_colors_vbos[shape_id]);
            glBufferData(GL_ARRAY_BUFFER, shape_instances[shape_id] * sizeof(glm::vec3), shape_colors, GL_STATIC_DRAW);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const GLvoid*)0);
            glVertexAttribDivisor(1, 1);

            glBindBuffer(GL_ARRAY_BUFFER, shape_model_matrix_vbos[shape_id]);
            glBufferData(GL_ARRAY_BUFFER, shape_instances[shape_id] * sizeof(glm::mat4), ModelArray, GL_STATIC_DRAW);

            for(int i = 0; i < 4; i++){ //MVP Matrices
                glEnableVertexAttribArray(2 + i);
                glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const GLvoid*)(sizeof(float) * i * 4));
                glVertexAttribDivisor(2 + i, 1);
            }
        }
        glBindVertexArray(0);

        delete[] shape_colors;
        delete[] ModelArray;

        //Generate single instance vaos
        glBindVertexArray(shape_single_vaos[shape_id]);

        glBindBuffer(GL_ARRAY_BUFFER, shape_vertex_vbos[shape_id]);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)0);

        glBindVertexArray(0);
    }

    //TODO: Move shader uniform initialization to constructor

	// SSAO Uniforms
	glm::vec2 projAB;
	sh_ssao->bind();
	{
		float projA = zfar / (zfar - znear);
		float projB = 2.0 * zfar * znear / (zfar - znear);
		projAB = glm::vec2(projA, projB);
		m_ssao.UploadUniforms(*sh_ssao);
		sh_ssao->setUniform("NormalMap", 0);
		sh_ssao->setUniform("DepthMap", 1);
		sh_ssao->setUniform("projAB", 1, projAB);
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
		
		float projA = (zfar + znear) / (zfar - znear);
		float projB = 2.0 * zfar * znear / (zfar - znear);
		projAB = glm::vec2(projA, projB);
		sh_accumulator->setUniform("projAB", 1, projAB);
		sh_accumulator->setUniform("skyColor", 1, skycolor);
		sh_accumulator->setUniform("invProjMatrix", 1, invProjMatrix);
	}
}

void OpenGLContext::select_particle(int x, int y){
    glm::vec3 o = glm::vec3(glm::inverse(viewMatrix * modelMatrix) * glm::vec4(glm::vec3(0.0), 1.0));

    glm::vec4 mouse_clip = glm::vec4(2.0f * x / windowWidth - 1.0f, 1.0f - 2.0f * y / windowHeight, 0.0, 1.0);
    glm::vec4 dir = glm::inverse(projectionMatrix * viewMatrix * modelMatrix) * mouse_clip;
    dir /= dir.w;

    int pid;
    if(grid->raycast(o, glm::normalize(glm::vec3(dir) - o), pid)){
        if(selected_pid == pid) selected_pid = -1;
        else selected_pid = pid;
    }
    else selected_pid = -1;
}

void OpenGLContext::wsize_changed(int w, int h){
	windowWidth = w;
	windowHeight = h;
	TwWindowSize(w, h);
	glViewport(0, 0, windowWidth, windowHeight);
	projectionMatrix = glm::perspective(glm::radians(fov + zoom_), (float)windowWidth/(float)windowHeight, znear, zfar);
    invProjMatrix = glm::inverse(projectionMatrix);
    m_gbuffer.Resize(windowWidth, windowHeight);
    m_edge_buffer.Resize(windowWidth, windowHeight);
    m_blend_buffer.Resize(windowWidth, windowHeight);
    m_accumulator.Resize(windowWidth, windowHeight);
    m_shadowmap.Resize(windowWidth, windowHeight);
    sh_ssao->bind();
    {
        m_ssao.Resize(windowWidth, windowHeight, sh_ssao);
    }

	// Edge Detection Uniforms
	sh_edge_detection->bind();
    sh_edge_detection->setUniform("texel_size", 1, glm::vec2(1.0f / windowWidth, 1.0f / windowHeight));
	sh_blend_weights->bind();
    sh_blend_weights->setUniform("texel_size", 1, glm::vec2(1.0f / windowWidth, 1.0f / windowHeight));
	sh_blend->bind();
    sh_blend->setUniform("texel_size", 1, glm::vec2(1.0f / windowWidth, 1.0f / windowHeight));
}

void OpenGLContext::processScene(void){
	static uint64_t last_time = 0;
	uint64_t this_time = perf_mon.get_time_ns();
    if(m_rotating){
        glm::mat4 rLocalMatrix = glm::rotate(
            glm::mat4(1.0),
            glm::radians(float(((this_time-last_time) / 1000000000.0) * 30.0)),
            glm::vec3(0.0, 1.0, 0.0)
        );
        modelMatrix = rLocalMatrix * modelMatrix;
    }
    last_time = this_time;
    projectionMatrix = glm::perspective(glm::radians(fov + zoom_), (float)windowWidth/(float)windowHeight, znear, zfar);
    invProjMatrix = glm::inverse(projectionMatrix);
    lightViewMatrix = glm::lookAt(-out_radius * light.getDirection(), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
}

//TODO: Improve line rendering!!!
void OpenGLContext::drawConfigurationBox(void)const{
	
	glLineWidth(fabs(-0.067f * zoom_ + 4.0f));
	
	glm::mat4 MVPMatrix = projectionMatrix * viewMatrix * modelMatrix;
	sh_gbuffer->setUniform("MVPMatrix", 1, MVPMatrix);
	sh_gbuffer->setUniform("diffColor", 0.01f, 0.01f, 0.01f);
	
	glBindVertexArray(vaoBox);
	glDrawElements(GL_LINE_LOOP, 8, GL_UNSIGNED_SHORT, 0);
	glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8*sizeof(GLushort)));
	
	glBindVertexArray(0);
}

void OpenGLContext::renderScene(void){	
    perf_mon.sync();

	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

    perf_mon.push_query("Shadow Pass");
    {
        m_shadowmap.Bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, windowWidth * 2, windowHeight * 2);
        
        for(unsigned int shape_id = 0; shape_id < n_shapes; ++shape_id){
            glBindVertexArray(shape_vaos[shape_id]);
            if(shape_types[shape_id] == Shape::MESH){
                sh_shadowmap_instanced->bind();

                glm::mat4 vMatrix = lightViewMatrix;
                glm::mat4 pMatrix = lightProjectionMatrix;

                sh_shadowmap_instanced->setUniform("MVPMatrix", 1, lightProjectionMatrix * lightViewMatrix * modelMatrix);

                glDrawArraysInstanced(GL_TRIANGLES, 0, shape_num_vertices[shape_id], shape_instances[shape_id]);
            }
            else{
                sh_shadowmap_spheres->bind();

                glm::mat4 vMatrix = lightViewMatrix;
                glm::mat4 pMatrix = lightProjectionMatrix;

                sh_shadowmap_spheres->setUniform("MVMatrix", 1, lightViewMatrix * modelMatrix);
                sh_shadowmap_spheres->setUniform("ProjectionMatrix", 1, lightProjectionMatrix);

                glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, shape_instances[shape_id]);
            }
        }

        glViewport(0, 0, windowWidth, windowHeight);
    }
    perf_mon.pop_query();

    perf_mon.push_query("FBO Pass");
    {
        m_gbuffer.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for(unsigned int shape_id = 0; shape_id < n_shapes; ++shape_id){
            glBindVertexArray(shape_vaos[shape_id]);
            if(shape_types[shape_id] == Shape::MESH){
                sh_gbuffer_instanced->bind();

                //sh_gbuffer_instanced->setUniform("diffColor", 1, diffcolor);
                sh_gbuffer_instanced->setUniform("MVMatrix", 1, viewMatrix * modelMatrix);
                sh_gbuffer_instanced->setUniform("ProjectionMatrix", 1, projectionMatrix);

                glDrawArraysInstanced(GL_TRIANGLES, 0, shape_num_vertices[shape_id], shape_instances[shape_id]);
            }
            else{
                sh_spheres->bind();

                //sh_spheres->setUniform("diffColor", 1, diffcolor);
                sh_spheres->setUniform("MVMatrix", 1, viewMatrix * modelMatrix);
                sh_spheres->setUniform("ProjectionMatrix", 1, projectionMatrix);

                glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, shape_instances[shape_id]);
            }
        }
        
        if(drawBox){
            sh_gbuffer->bind();
            {	
                drawConfigurationBox();
            }
        }
    }
    perf_mon.pop_query();

    glBindVertexArray(fullscreen_triangle_vao);

    perf_mon.push_query("SSAO Pass");
    {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        perf_mon.push_query("SSAO Calc Pass");
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

        perf_mon.pop_query();
        
        perf_mon.push_query("SSAO Blur Pass");

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
        m_gbuffer.Bind();
        m_ssao.BindTexture(Cssao::TEXTURE_TYPE_SSAO, 0);
        
        sh_blur->bind();
        {
            sh_blur->setUniform("use_blur", int(m_blur));

            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        perf_mon.pop_query();
    }
    perf_mon.pop_query();

    perf_mon.push_query("Gather Pass");
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

            light.uploadDirection(viewMatrix);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        
    }
    perf_mon.pop_query();

    if(selected_pid >= 0){

        perf_mon.push_query("Selection Pass");
        {
            int shape_id = particles[selected_pid].shape_id;

            m_accumulator.Bind();

            glBindVertexArray(shape_single_vaos[shape_id]);
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_STENCIL_TEST);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilMask(0xFF);
            glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            if(shape_types[shape_id] == Shape::MESH){
                sh_color->bind();

                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                glm::mat4 mvp_matrix = projectionMatrix * viewMatrix * modelMatrix * model_matrices[selected_pid];
                sh_color->setUniform("mvp_matrix", 1, mvp_matrix);
                glDrawArrays(GL_TRIANGLES, 0, shape_num_vertices[shape_id]);

                glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                glStencilMask(0x00);
                glDisable(GL_DEPTH_TEST);
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                mvp_matrix = mvp_matrix * glm::scale(glm::mat4(1.0), glm::vec3(1.1));
                sh_color->setUniform("mvp_matrix", 1, mvp_matrix);
                glDrawArrays(GL_TRIANGLES, 0, shape_num_vertices[shape_id]);
            }
            else{
                sh_color_sphere->bind();

                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                glm::mat4 mv_matrix = viewMatrix * modelMatrix * model_matrices[selected_pid];
                sh_color_sphere->setUniform("mv_matrix", 1, mv_matrix);
                sh_color_sphere->setUniform("projection_matrix", 1, projectionMatrix);
                sh_color_sphere->setUniform("scale", 1.0f);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                glStencilMask(0x00);
                glDisable(GL_DEPTH_TEST);
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                sh_color_sphere->setUniform("scale", 1.1f);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }

            glDisable(GL_STENCIL_TEST);
            glDepthMask(GL_FALSE);
        }
    }

    perf_mon.push_query("SMAA");
    {
        perf_mon.push_query("Edge Pass");
        {
            m_edge_buffer.Bind();
            glClear(GL_COLOR_BUFFER_BIT);

            m_accumulator.BindTexture(0);

            sh_edge_detection->bind();
            {
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        }
        perf_mon.pop_query();

        perf_mon.push_query("Blend weight Pass");
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
        perf_mon.pop_query();

        perf_mon.push_query("Blend Pass");
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
        perf_mon.pop_query();
    }
    perf_mon.pop_query();

    perf_mon.push_query("TwDraw Pass");
	//TwDraw();
    perf_mon.pop_query();
    glDisable(GL_FRAMEBUFFER_SRGB);
}

void OpenGLContext::rotate(float angle, const glm::vec3& axis){
    modelMatrix = glm::rotate(glm::mat4(1.0), angle, axis) * modelMatrix;
}

//TODO: Move to camera
void OpenGLContext::zoom(float dz){
    zoom_ = zoom_ - 2.0f * dz; // put wheel up and down in one
    if(zoom_ < -58) zoom_ = -58.0f;
    else if(zoom_ > 90) zoom_ = 90.0f;
}

