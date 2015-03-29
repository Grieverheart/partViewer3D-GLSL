#include "include/opengl_3.h"
#include <GL/glew.h>

#ifdef _WIN32
#include <GL/wglew.h>
#elif __linux
#include <GL/glxew.h>
#endif

#include <GL/freeglut.h>
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

OpenGLContext::OpenGLContext(void):
	m_fboInit(false),
	mesh(1.0f),
	full_quad(1.0f)
{
	/////////////////////////////////////////////////
	// Default Constructor for OpenGLContext class //
	/////////////////////////////////////////////////
	zoom = 0.0f;
	fov = 60.0f;
	znear = 1.0f; //In the Future, calculate the optimal znear and zfar
	zfar = 100.0f;
	m_bgColor = glm::vec3(44, 114, 220) / 255.0f;
	redisplay = false;
	trackballMatrix = glm::mat4(1.0);
	use_dat = false;
	drawBox = false;
	m_blur = true;
	m_rotating = false;
	light = CLight(glm::vec3(-0.27, -0.91, -0.33));
	diffcolor = glm::vec3(77, 27, 147) / 255.0f;
	skycolor  = glm::vec3(0.529, 0.808, 0.921);
}

OpenGLContext::~OpenGLContext(void) { 
	if(vaoBox != 0) glDeleteBuffers(1, &vaoBox);
	if(vboBox != 0) glDeleteBuffers(1, &vboBox);
	if(iboBox != 0) glDeleteBuffers(1, &iboBox);
	if(sh_gbuffer) delete sh_gbuffer; // GLSL Shader
	if(sh_gbuffer_instanced) delete sh_gbuffer_instanced; // GLSL Shader
	if(sh_ssao) delete sh_ssao;
	if(sh_shadowmap_instanced) delete sh_shadowmap_instanced;
	if(sh_blur) delete sh_blur;
	if(sh_accumulator) delete sh_accumulator;
	if(MVPArray) delete[] MVPArray;
	if(NormalArray) delete[] NormalArray;
	TwTerminate();
}  

bool OpenGLContext::create30Context(void){
	////////////////////////////////////////////////////
	// Create an OpenGL 3.2 context. Fall back to a   //
	// OpenGL 2.1 Context if it fails.				  //
	////////////////////////////////////////////////////
	
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_SRGB | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(600,600);
	windowWidth=windowHeight = 600;
	// glutInitWindowPosition(100,100);
	glutCreateWindow("partViewer GLSL");
	
	glewExperimental = GL_TRUE;
	GLenum error = glewInit(); //Enable GLEW
	if(error != GLEW_OK) return false; //Failure!
	glError(__FILE__,__LINE__);
	
	int glVersion[2] = {-1,1};
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
	
	printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);
	printf("Renderer used: %s\n", glGetString(GL_RENDERER));
	printf("Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	
	createGui();
	
	return true; // Success, return true
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

void OpenGLContext::setupScene(int argc, char *argv[]){
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
    
	float init_zoom = -4.0f;
	if(argc>1){
	
		use_dat = true;
		std::ifstream file(argv[1], std::ios::in);
		if(!file){
			printf("Cannot open %s.\n", argv[1]);
			exit(1);
		}
		printf("Parsing %s ...\n", argv[1]);
		coordparser.parse(file);
		printf("Done Parsing %s.\n", argv[1]);
		printf("Found %d particles.\n", coordparser.npart);
		
		for(int i=0;i<3;i++){
			GLfloat max_zoom = -3.5f*glm::length(glm::transpose(coordparser.boxMatrix)[i]);
			if(max_zoom < init_zoom) init_zoom = max_zoom;
		}
		mNInstances = coordparser.npart;
	}
	else mNInstances = 1;
	
	MVPArray = new glm::mat4[mNInstances];
	NormalArray = new glm::mat3[mNInstances];
	
	glDisable(GL_BLEND);
	//glBlendEquation(GL_FUNC_ADD);
	//glBlendFunc(GL_ONE, GL_ONE);
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	initConfigurationBox();
	
	sh_gbuffer = new Shader("shaders/gbuffer.vert", "shaders/gbuffer.frag");
	sh_gbuffer_instanced = new Shader("shaders/gbuffer_instanced.vert", "shaders/gbuffer_instanced.frag");
	sh_ssao = new Shader("shaders/ssao.vert", "shaders/ssao.frag");
	sh_shadowmap_instanced = new Shader("shaders/shadowmap_instanced.vert");
	sh_blur = new Shader("shaders/blur.vert", "shaders/blur.frag");
	sh_accumulator = new Shader("shaders/accumulator.vert", "shaders/accumulator.frag");
	
	if(!m_ssao.Init(windowWidth, windowHeight)) printf("Couldn't initialize SSAO!");
	if(!m_shadowmap.Init(windowWidth, windowHeight)) printf("Couldn't initialize Shadowmap!");
	if(!light.Init(sh_accumulator->id())) printf("Cannot bind light uniform");

    out_radius = glm::length(glm::vec3(
        (coordparser.boxMatrix[0][0]+coordparser.boxMatrix[0][1]+coordparser.boxMatrix[0][2])/2.0,
        (coordparser.boxMatrix[1][1]+coordparser.boxMatrix[1][2])/2.0,
        coordparser.boxMatrix[2][2]/2.0
    )) + 1.0;

    znear = -init_zoom - out_radius;
    zfar  = -init_zoom + 2.0 * out_radius;

	
	projectionMatrix = glm::perspective(glm::radians(fov+zoom), (float)windowWidth/(float)windowHeight, znear, zfar);
    invProjMatrix = glm::inverse(projectionMatrix);
    lightProjectionMatrix = glm::ortho(-out_radius, out_radius, -out_radius, out_radius, 0.0f, 2.0f * out_radius);
	viewMatrix = glm::lookAt(glm::vec3(0.0, 0.0, -init_zoom), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    invViewMatrix = glm::inverse(viewMatrix);
    lightViewMatrix = glm::lookAt(-out_radius * light.getDirection(), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	modelMatrix = glm::mat4(1.0);

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
	sh_ssao->unbind();
	
	// Blur Uniforms
	sh_blur->bind();
	{
		sh_blur->setUniform("aoSampler", 0);
		sh_blur->setUniform("use_blur", int(m_blur));
	}
	sh_blur->unbind();
	
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
	sh_accumulator->unbind();
	
	objparser.parse("obj/octahedron.obj",&mesh, "flat");
	mesh.uploadInstanced(sh_gbuffer_instanced->id());
	objparser.parse("obj/full_quad.obj", &full_quad, "flat");
	full_quad.upload(sh_gbuffer->id());
	m_fboInit = m_gbuffer.Init(windowWidth, windowHeight);
	if(!m_fboInit) printf("Couldn't initialize FBO!");
}

void OpenGLContext::reshapeWindow(int w, int h){
	windowWidth = w;
	windowHeight = h;
	TwWindowSize(w, h);
	glViewport(0, 0, windowWidth, windowHeight);
	projectionMatrix = glm::perspective(glm::radians(fov+zoom), (float)windowWidth/(float)windowHeight, znear, zfar);
    invProjMatrix = glm::inverse(projectionMatrix);
	if(m_fboInit){
		m_gbuffer.Resize(windowWidth, windowHeight);
		m_shadowmap.Resize(windowWidth, windowHeight);
		sh_ssao->bind();
		{
			m_ssao.Resize(windowWidth, windowHeight, sh_ssao);
		}
		sh_ssao->unbind();
	}
}

void OpenGLContext::processScene(void){
	static float last_time = 0.0;
	float this_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	// if(this_time-last_time > 1.0f/61.0f){
		if(m_rotating){
			glm::mat4 rLocalMatrix = glm::rotate(
				glm::mat4(1.0),
				glm::radians((this_time-last_time) * 30.0f),
				glm::vec3(0.0, 1.0, 0.0)
			);
			trackballMatrix = rLocalMatrix * trackballMatrix;
		}
		redisplay = true;
		last_time = this_time;
		projectionMatrix = glm::perspective(glm::radians(fov+zoom), (float)windowWidth/(float)windowHeight, znear, zfar);
		invProjMatrix = glm::inverse(projectionMatrix);
        lightViewMatrix = glm::lookAt(-out_radius * light.getDirection(), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

	// }
}

void OpenGLContext::initConfigurationBox(void){
	GLfloat vertices[]={
		0.0f,0.0f,0.0f,
		coordparser.boxMatrix[0][0], coordparser.boxMatrix[1][0], coordparser.boxMatrix[2][0],
		coordparser.boxMatrix[0][1], coordparser.boxMatrix[1][1], coordparser.boxMatrix[2][1],
		coordparser.boxMatrix[0][2], coordparser.boxMatrix[1][2], coordparser.boxMatrix[2][2],
		coordparser.boxMatrix[0][0] + coordparser.boxMatrix[0][1], coordparser.boxMatrix[1][0] + coordparser.boxMatrix[1][1], coordparser.boxMatrix[2][0] + coordparser.boxMatrix[2][1],
		coordparser.boxMatrix[0][0] + coordparser.boxMatrix[0][2], coordparser.boxMatrix[1][0] + coordparser.boxMatrix[1][2], coordparser.boxMatrix[2][0] + coordparser.boxMatrix[2][2],
		coordparser.boxMatrix[0][1] + coordparser.boxMatrix[0][2], coordparser.boxMatrix[1][1] + coordparser.boxMatrix[1][2], coordparser.boxMatrix[2][1] + coordparser.boxMatrix[2][2],
		
		coordparser.boxMatrix[0][0] + coordparser.boxMatrix[0][1] + coordparser.boxMatrix[0][2],
		coordparser.boxMatrix[1][0] + coordparser.boxMatrix[1][1] + coordparser.boxMatrix[1][2],
		coordparser.boxMatrix[2][0] + coordparser.boxMatrix[2][1] + coordparser.boxMatrix[2][2]
	};
	
	GLushort elements[]={
		3,6,7,5,
		0,2,4,1,
		3,0,2,6,4,7,1,5
	};
	
	glGenVertexArrays(1, &vaoBox);
	glBindVertexArray(vaoBox);
	
	glGenBuffers(1, &vboBox);
	glBindBuffer(GL_ARRAY_BUFFER, vboBox);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0,3,GL_FLOAT,GL_FALSE,0,0);
	glEnableVertexAttribArray((GLuint)0);
	
	glGenBuffers(1,&iboBox);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboBox);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW); 
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
}

void OpenGLContext::drawConfigurationBox(void)const{
	
	glm::mat4 vMatrix;
    glm::mat4 pMatrix;
	
	vMatrix = viewMatrix;
	pMatrix = projectionMatrix;
	
	glm::mat4 tMatrix = glm::translate(
		modelMatrix,
		glm::vec3(
			-(coordparser.boxMatrix[0][0]+coordparser.boxMatrix[0][1]+coordparser.boxMatrix[0][2])/2.0,
			-(coordparser.boxMatrix[1][1]+coordparser.boxMatrix[1][2])/2.0,
			-coordparser.boxMatrix[2][2]/2.0
		)
	);
	glm::mat4 ModelViewMatrix = vMatrix * tMatrix;
	glm::mat4 MVPMatrix = pMatrix * ModelViewMatrix;
	
	sh_gbuffer->setUniform("MVPMatrix", 1, MVPMatrix);
	
	glBindVertexArray(vaoBox);
	
	glLineWidth(fabs(-0.067f * zoom + 4.0f));
	
	sh_gbuffer->setUniform("diffColor", 0.01f, 0.01f, 0.01f);
	sh_gbuffer->setUniform("scale", 1.0f);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4*sizeof(GLushort)));
	glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8*sizeof(GLushort)));
	
	glBindVertexArray(0);
}

void OpenGLContext::drawConfiguration(const glm::mat4& vMatrix, const glm::mat4& pMatrix)const{
	glm::mat4 tMatrix = glm::translate(
		modelMatrix,
		glm::vec3(
			-(coordparser.boxMatrix[0][0]+coordparser.boxMatrix[0][1]+coordparser.boxMatrix[0][2])/2.0,
			-(coordparser.boxMatrix[1][1]+coordparser.boxMatrix[1][2])/2.0,
			-coordparser.boxMatrix[2][2]/2.0
		)
	);
	glm::mat4 ModelViewMatrix = vMatrix * tMatrix;
	
	
	for(unsigned int i = 0; i < mNInstances; i++){
		glm::mat4 tLocalMatrix = glm::translate(ModelViewMatrix, coordparser.centers[i]);
		glm::mat4 rLocalMatrix = glm::rotate(
			glm::mat4(1.0),
			coordparser.rotations[i].x,
			glm::vec3(coordparser.rotations[i].y, coordparser.rotations[i].z, coordparser.rotations[i].w)
		);
		
		glm::mat4 tempModelViewMatrix = tLocalMatrix * rLocalMatrix ;
		glm::mat3 tempNormalMatrix = glm::mat3(glm::transpose(glm::inverse(tempModelViewMatrix)));
		glm::mat4 tempMVPMatrix = pMatrix * tempModelViewMatrix;

		MVPArray[i] = tempMVPMatrix;
		NormalArray[i] = tempNormalMatrix;
		
	}
	mesh.drawInstanced(mNInstances, MVPArray, NormalArray);
}


void OpenGLContext::fboPass(void)const{

	m_gbuffer.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	sh_gbuffer_instanced->bind();
	{	
        sh_gbuffer_instanced->setUniform("diffColor", 1, diffcolor);

		if(use_dat) drawConfiguration(viewMatrix, projectionMatrix);
		else{
			glm::mat3 tempNormalMatrix = glm::mat3(glm::transpose(glm::inverse(viewMatrix)));
			glm::mat4 tempMVPMatrix = projectionMatrix * viewMatrix * modelMatrix;

			MVPArray[0] = tempMVPMatrix;
			NormalArray[0] = tempNormalMatrix;
			
			sh_gbuffer_instanced->setUniform("diffColor", 1, diffcolor);
			
			mesh.drawInstanced(mNInstances, MVPArray, NormalArray);
		}
	}
	sh_gbuffer_instanced->unbind();
	
	if(!use_dat || drawBox){
		sh_gbuffer->bind();
		{	
            sh_gbuffer_instanced->setUniform("diffColor", 1, diffcolor);
			if(!use_dat) mesh.draw();
			else drawConfigurationBox();
		}
		sh_gbuffer->unbind();
	}
}

void OpenGLContext::ssaoPass(void){

	m_ssao.Bind();
	glClear(GL_COLOR_BUFFER_BIT);

	m_gbuffer.BindTexture(CGBuffer::GBUFF_TEXTURE_TYPE_NORMAL, 0);
	m_gbuffer.BindTexture(CGBuffer::GBUFF_TEXTURE_TYPE_DEPTH, 1);
	m_ssao.BindTexture(Cssao::TEXTURE_TYPE_NOISE, 2);
	
	sh_ssao->bind();
	{
		m_ssao.UpdateUniforms(*sh_ssao);
		full_quad.draw();
	}
	sh_ssao->unbind();
	
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	m_gbuffer.Bind();
	m_ssao.BindTexture(Cssao::TEXTURE_TYPE_SSAO, 0);
	
	sh_blur->bind();
	{
		sh_blur->setUniform("use_blur", int(m_blur));
		full_quad.draw();
	}
	sh_blur->unbind();
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void OpenGLContext::shadowPass(void){
	m_shadowmap.Bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	
	sh_shadowmap_instanced->bind();
	{	
        glm::mat4 vMatrix = lightViewMatrix;
        glm::mat4 pMatrix = lightProjectionMatrix;

		if(use_dat){
            glm::mat4 tMatrix = glm::translate(
                modelMatrix,
                glm::vec3(
                    -(coordparser.boxMatrix[0][0]+coordparser.boxMatrix[0][1]+coordparser.boxMatrix[0][2])/2.0,
                    -(coordparser.boxMatrix[1][1]+coordparser.boxMatrix[1][2])/2.0,
                    -coordparser.boxMatrix[2][2]/2.0
                )
            );
            glm::mat4 ModelViewMatrix = vMatrix * tMatrix;
            
            
            for(unsigned int i = 0; i < mNInstances; i++){
                glm::mat4 tLocalMatrix = glm::translate(ModelViewMatrix, coordparser.centers[i]);
                glm::mat4 rLocalMatrix = glm::rotate(
                    glm::mat4(1.0),
                    coordparser.rotations[i].x,
                    glm::vec3(coordparser.rotations[i].y, coordparser.rotations[i].z, coordparser.rotations[i].w)
                );
                
                glm::mat4 tempModelViewMatrix = tLocalMatrix * rLocalMatrix ;
                glm::mat4 tempMVPMatrix = pMatrix * tempModelViewMatrix;

                MVPArray[i] = tempMVPMatrix;
            }
            mesh.drawInstanced(mNInstances, MVPArray);
        }
		else{
			glm::mat4 tempMVPMatrix = pMatrix * vMatrix * modelMatrix;

			MVPArray[0] = tempMVPMatrix;
			
			sh_shadowmap_instanced->setUniform("diffColor", 1, diffcolor);
			
			mesh.drawInstanced(mNInstances, MVPArray);
		}
	}
	sh_shadowmap_instanced->unbind();
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void OpenGLContext::drawPass(void)const{

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
    glEnable(GL_FRAMEBUFFER_SRGB);
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
		full_quad.draw();
	}
	sh_accumulator->unbind();
	
    glDisable(GL_FRAMEBUFFER_SRGB);
}

void OpenGLContext::renderScene(void){	
    static int times = 0;
    static double avg = 0.0;
    ++times;
    perf_mon.push_query();

	modelMatrix = trackballMatrix;
	
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
    shadowPass();
	fboPass();
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	ssaoPass();
	drawPass();
	glEnable(GL_CULL_FACE);
	TwDraw();

	glutSwapBuffers();

    double dt = perf_mon.pop_query();
    avg += dt;
    if((times + 1) % 60 == 0){
        printf("%fms\n", avg / times);
        avg = 0.0;
        times = 0;
    }
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
