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

static const glm::mat4 biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
);

OpenGLContext::OpenGLContext(int width, int height):
    drawBox(false),
	trackballMatrix(1.0),
    windowWidth(width), windowHeight(height),
    fov(60.0f), zoom(0.0f),
	m_bgColor(glm::vec3(44, 114, 220) / 255.0f),
	diffcolor(glm::vec3(77, 27, 147) / 255.0f),
	skycolor(0.529, 0.808, 0.921),
    vao_instanced(0), vbo_instanced(0), vaoBox(0), vboBox(0), iboBox(0), fullscreen_triangle_vao(0),
    is_scene_loaded(false), m_blur(true), m_rotating(false),
	light(glm::vec3(-0.27, -0.91, -0.33)),
    sh_gbuffer(nullptr), sh_gbuffer_instanced(nullptr), sh_ssao(nullptr),
    sh_shadowmap_instanced(nullptr), sh_blur(nullptr), sh_accumulator(nullptr)

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
        sh_blur = new Shader("shaders/blur.vert", "shaders/blur.frag");
        sh_accumulator = new Shader("shaders/accumulator.vert", "shaders/accumulator.frag");
    }
    catch(Shader::InitializationException){
        delete sh_gbuffer;
        delete sh_gbuffer_instanced;
        delete sh_ssao;
        delete sh_shadowmap_instanced;
        delete sh_blur;
        delete sh_accumulator;
        throw;
    }
	
    //TODO: Add exceptions
	if(!m_ssao.Init(windowWidth, windowHeight)) printf("Couldn't initialize SSAO!");
	if(!m_shadowmap.Init(windowWidth, windowHeight)) printf("Couldn't initialize Shadowmap!");
	if(!light.Init(sh_accumulator->id())) printf("Cannot bind light uniform");
	if(!m_gbuffer.Init(windowWidth, windowHeight)) printf("Couldn't initialize FBO!");

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
	
	createGui();
}

OpenGLContext::~OpenGLContext(void) { 
	delete sh_gbuffer; // GLSL Shader
	delete sh_gbuffer_instanced; // GLSL Shader
	delete sh_ssao;
	delete sh_shadowmap_instanced;
	delete sh_blur;
	delete sh_accumulator;

	glDeleteBuffers(1, &vbo_instanced);
	glDeleteBuffers(1, &vboBox);
	glDeleteBuffers(1, &iboBox);
	glDeleteVertexArrays(1, &vao_instanced);
	glDeleteVertexArrays(1, &vaoBox);
	glDeleteVertexArrays(1, &fullscreen_triangle_vao);

	TwTerminate();
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
	mNInstances = config.n_part;
	
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

	
	projectionMatrix = glm::perspective(glm::radians(fov+zoom), (float)windowWidth/(float)windowHeight, znear, zfar);
    invProjMatrix = glm::inverse(projectionMatrix);
    lightProjectionMatrix = glm::ortho(-out_radius, out_radius, -out_radius, out_radius, 0.0f, 2.0f * out_radius);
	viewMatrix = glm::lookAt(glm::vec3(0.0, 0.0, -init_zoom), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    invViewMatrix = glm::inverse(viewMatrix);
    lightViewMatrix = glm::lookAt(-out_radius * light.getDirection(), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	modelMatrix = glm::mat4(1.0);
	
    mesh.set(config.meshes[0]);
    {
        glm::mat4* ModelArray = new glm::mat4[mNInstances];

        glm::mat4 tMatrix = glm::translate(
            glm::mat4(1.0),
            glm::vec3(
                -(config.box[0][0] + config.box[0][1] + config.box[0][2]) / 2.0,
                -(config.box[1][1] + config.box[1][2]) / 2.0,
                -config.box[2][2] / 2.0
            )
        );
        
        
        for(unsigned int i = 0; i < mNInstances; i++){
            glm::mat4 tLocalMatrix = glm::translate(tMatrix, config.pos[i]);
            glm::mat4 rLocalMatrix = glm::rotate(
                glm::mat4(1.0),
                config.rot[i].x,
                glm::vec3(config.rot[i].y, config.rot[i].z, config.rot[i].w)
            );
            
            ModelArray[i] = tLocalMatrix * rLocalMatrix ;
        }

        //TODO: This should be done for each mesh that was loaded. Additionally, we should
        //only add vertex attributes to the vao and move the ModelArray to a separate vertex
        //buffer bound at index 1.
        
        //Build instanced vao
        glGenVertexArrays(1, &vao_instanced);
        glGenBuffers(1, &vbo_instanced);

        glBindVertexArray(vao_instanced);
        
        glEnableVertexAttribArray((GLuint)0);
        glVertexAttribFormat((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexAttribBinding(0, 0);
        glEnableVertexAttribArray((GLuint)1);
        glVertexAttribFormat((GLuint)1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3));
        glVertexAttribBinding(1, 0);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo_instanced);
        for(int i = 0; i < 4; i++){ //MVP Matrices
            glEnableVertexAttribArray(2 + i);
            glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const GLvoid*)(sizeof(float) * i * 4));
            glVertexAttribDivisor(2 + i, 1);
        }
        glBufferData(GL_ARRAY_BUFFER, mNInstances * sizeof(glm::mat4), ModelArray, GL_STATIC_DRAW);
        
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        delete[] ModelArray;
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

void OpenGLContext::reshapeWindow(int w, int h){
	windowWidth = w;
	windowHeight = h;
	TwWindowSize(w, h);
	glViewport(0, 0, windowWidth, windowHeight);
	projectionMatrix = glm::perspective(glm::radians(fov+zoom), (float)windowWidth/(float)windowHeight, znear, zfar);
    invProjMatrix = glm::inverse(projectionMatrix);
    m_gbuffer.Resize(windowWidth, windowHeight);
    m_shadowmap.Resize(windowWidth, windowHeight);
    sh_ssao->bind();
    {
        m_ssao.Resize(windowWidth, windowHeight, sh_ssao);
    }
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
        trackballMatrix = rLocalMatrix * trackballMatrix;
    }
    last_time = this_time;
    projectionMatrix = glm::perspective(glm::radians(fov+zoom), (float)windowWidth/(float)windowHeight, znear, zfar);
    invProjMatrix = glm::inverse(projectionMatrix);
    lightViewMatrix = glm::lookAt(-out_radius * light.getDirection(), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
}

//TODO: Improve line rendering!!!
void OpenGLContext::drawConfigurationBox(void)const{
	
	glLineWidth(fabs(-0.067f * zoom + 4.0f));
	
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

	modelMatrix = trackballMatrix;
	
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

    glBindVertexArray(vao_instanced);

    perf_mon.push_query("Shadow Pass");
    {
        m_shadowmap.Bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        
        sh_shadowmap_instanced->bind();
        {	
            glm::mat4 vMatrix = lightViewMatrix;
            glm::mat4 pMatrix = lightProjectionMatrix;

            sh_shadowmap_instanced->setUniform("MVPMatrix", 1, lightProjectionMatrix * lightViewMatrix * modelMatrix);

            mesh.draw_instanced(mNInstances);
        }
    }
    perf_mon.pop_query();

    perf_mon.push_query("FBO Pass");
    {
        m_gbuffer.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        sh_gbuffer_instanced->bind();
        {	
            sh_gbuffer_instanced->setUniform("diffColor", 1, diffcolor);
            sh_gbuffer_instanced->setUniform("MVMatrix", 1, viewMatrix * modelMatrix);
            sh_gbuffer_instanced->setUniform("ProjectionMatrix", 1, projectionMatrix);
            mesh.draw_instanced(mNInstances);
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

    glEnable(GL_FRAMEBUFFER_SRGB);
    perf_mon.push_query("Gather Pass");
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        
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

    perf_mon.push_query("TwDraw Pass");
	TwDraw();
    perf_mon.pop_query();
    glDisable(GL_FRAMEBUFFER_SRGB);
}

float OpenGLContext::getZoom(void)const{
	return zoom;
}

void OpenGLContext::setZoom(float zoom){
	this->zoom = zoom;
}

glm::ivec2 OpenGLContext::getScreen(void)const{
	glm::ivec2 screen = glm::ivec2(windowWidth, windowHeight);
	return screen;
}
