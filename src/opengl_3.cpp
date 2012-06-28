#include "../include/opengl_3.h"

OpenGLContext::OpenGLContext(void):
	m_fboInit(false),
	mesh(1.0f),
	full_quad(1.0)
{
	/////////////////////////////////////////////////
	// Default Constructor for OpenGLContext class //
	/////////////////////////////////////////////////
	zoom = 0.0f;
	fov = 60.0f;
	znear = 1.0f;
	zfar = 50.0f;
	m_bgColor = glm::vec3(0.4, 0.6, 0.9);
	redisplay = false;
	trackballMatrix = glm::mat4(1.0);
	use_dat = false;
	drawBox = false;
	m_blur = true;
	light = CLight(glm::vec3(-10.0, 10.0, 10.0), glm::vec3(1.0, -1.0, -1.0));
	IdentityMatrix = glm::mat4(1.0);
	diffcolor = glm::vec3(0.282, 0.239, 0.545);
	
}

OpenGLContext::~OpenGLContext(void) { 
	TwTerminate();
}  

bool OpenGLContext::create30Context(void){
	////////////////////////////////////////////////////
	// Create an OpenGL 3.2 context. Fall back to a   //
	// OpenGL 2.1 Context if it fails.				  //
	////////////////////////////////////////////////////
	
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA);
	glutInitWindowSize(600,600);
	windowWidth=windowHeight = 600;
	// glutInitWindowPosition(100,100);
	glutCreateWindow("Project");
	
	glewExperimental = GL_TRUE;
	GLenum error = glewInit(); //Enable GLEW
	if(error != GLEW_OK) return false; //Failure!
	glError(__FILE__,__LINE__);
	
	int glVersion[2] = {-1,1};
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
	
	std::cout << "Using OpenGL: " << glVersion[0] << "." << glVersion[1] << std::endl;
	std::cout << "Renderer used: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	
	createGui();
	
	return true; // Success, return true
}

void OpenGLContext::createGui(void){
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(windowWidth, windowHeight);
	bar = TwNewBar("Parameters");
	
	TwDefine(
		"Parameters position='0 0' size='200 200' iconified=true");
	
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
		min=0.1 max=10.0 step=0.1 group=AO");
	
	TwAddVarCB(bar, "Samples", TW_TYPE_UINT32, Cssao::SetSamplesCallback, Cssao::GetSamplesCallback, &m_ssao,"\
		help='Number of samples for ambient occlusion. Increase for higher quality.'\
		min=4 max=256 step=2 group=AO");
}

void OpenGLContext::setupScene(int argc, char *argv[]){
	float init_zoom = -4.0f;
	if(argc>1){
		use_dat = true;
		coordparser.parse(argv[1]);
		for(int i=0;i<3;i++){
			GLfloat max_zoom = -3.5f*glm::length(glm::transpose(coordparser.boxMatrix)[i]);
			if(max_zoom < init_zoom) init_zoom = max_zoom;
		}
	}
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	sh_gbuffer = new Shader("shaders/gbuffer.vert", "shaders/gbuffer.frag");
	sh_ssao = new Shader("shaders/ssao.vert", "shaders/ssao.frag");
	sh_blur = new Shader("shaders/blur.vert", "shaders/blur.frag");
	sh_accumulator = new Shader("shaders/accumulator.vert", "shaders/accumulator.frag");
	
	// Gbuffer Uniform Locations
	MVPMatrixLocation = glGetUniformLocation(sh_gbuffer->id(),"MVPMatrix");
	NormalMatrixLocation = glGetUniformLocation(sh_gbuffer->id(),"NormalMatrix");
	diffColorLocation = glGetUniformLocation(sh_gbuffer->id(),"diffColor");
	
	if(
		MVPMatrixLocation == -1	||	NormalMatrixLocation == -1
	) std::cout<<"Unable to bind GBuffer uniforms"<<std::endl;
	
	// SSAO Uniform Locations
	ssaoProjMatrixLocation = glGetUniformLocation(sh_ssao->id(), "projectionMatrix");
	ssaoDepthMapLocation = glGetUniformLocation(sh_ssao->id(), "DepthMap");
	ssaoNormalMapLocation = glGetUniformLocation(sh_ssao->id(), "NormalMap");
	ssaoprojABLocation = glGetUniformLocation(sh_ssao->id(), "projAB");
	ssaoinvProjMatrixLocation = glGetUniformLocation(sh_ssao->id(), "invProjMatrix");
	if(!m_ssao.Init(windowWidth, windowHeight, sh_ssao->id())) std::cout << "Couldn't initialize SSAO!" << std::endl;
	
	if(
		ssaoProjMatrixLocation == -1	||	ssaoDepthMapLocation == -1	||
		ssaoNormalMapLocation == -1		||	ssaoprojABLocation == -1	||
		ssaoinvProjMatrixLocation == -1
	){ std::cout << "Unable to bind SSAO main uniforms" << std::endl; }
	
	// Blur Uniform Locations
	aoSamplerLocation = glGetUniformLocation(sh_blur->id(), "aoSampler");
	texelSizeLocation = glGetUniformLocation(sh_blur->id(), "TEXEL_SIZE");
	m_blurLocation = glGetUniformLocation(sh_blur->id(), "use_blur");
	
	if(
		aoSamplerLocation == -1	||	texelSizeLocation == -1
	){ std::cout << "Unable to bind blur uniforms" << std::endl; }
	
	// Accumulator Uniform Locations
	DepthMapLocation = glGetUniformLocation(sh_accumulator->id(), "DepthMap");
	NormalMapLocation = glGetUniformLocation(sh_accumulator->id(), "NormalMap");
	ColorMapLocation = glGetUniformLocation(sh_accumulator->id(), "ColorMap");
	if(!light.Init(sh_accumulator->id())) std::cout << "Cannot bind light uniform" << std::endl;
	projABLocation = glGetUniformLocation(sh_accumulator->id(), "projAB");
	invProjMatrixLocation = glGetUniformLocation(sh_accumulator->id(), "invProjMatrix");
	bgColorLocation = glGetUniformLocation(sh_accumulator->id(), "bgColor");
	
	if(
		invProjMatrixLocation == -1	||	DepthMapLocation == -1	||
		NormalMapLocation == -1		||	projABLocation == -1	||
		ColorMapLocation == -1		||	bgColorLocation == -1
	){ std::cout << "Unable to bind Accumulator uniforms" << std::endl; }
	
	
	projectionMatrix = glm::perspective(fov+zoom, (float)windowWidth/(float)windowHeight, znear, zfar);
	viewMatrix = glm::lookAt(glm::vec3(0.0, 2.0, 0.0), glm::vec3(0.0, 0.0, init_zoom), glm::vec3(0.0, 1.0, 0.0));
	viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0, 0.0, init_zoom));
	modelMatrix = glm::mat4(1.0);
	
	// SSAO Uniforms
	glm::vec2 projAB;
	glm::mat4 invProjMatrix;
	sh_ssao->bind();
	{
		float projA = zfar / (zfar - znear);
		float projB = zfar * znear / (zfar - znear);
		projAB = glm::vec2(projA, projB);
		invProjMatrix = glm::inverse(projectionMatrix);
		m_ssao.UploadUniforms();
		glUniform1i(ssaoNormalMapLocation, 0);
		glUniform1i(ssaoDepthMapLocation, 1);
		glUniform2fv(ssaoprojABLocation, 1, &projAB[0]);
		glUniformMatrix4fv(ssaoProjMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		glUniformMatrix4fv(ssaoinvProjMatrixLocation, 1, GL_FALSE, &invProjMatrix[0][0]);
		
	}
	sh_ssao->unbind();
	
	// Blur Uniforms
	sh_blur->bind();
	{
		glm::vec2 texel_size = glm::vec2(1.0 / windowWidth, 1.0 / windowHeight);
		glUniform2fv(texelSizeLocation, 1, &texel_size[0]);
		glUniform1i(aoSamplerLocation, 0);
		glUniform1i(m_blurLocation, int(m_blur));
	}
	sh_blur->unbind();
	
	// Accumulator Uniforms
	
	sh_accumulator->bind();
	{
		glUniform1i(ColorMapLocation, 0);
		glUniform1i(NormalMapLocation, 1);
		glUniform1i(DepthMapLocation, 2);
		float projA = (zfar + znear)/ (zfar - znear);
		float projB = 2.0 * zfar * znear / (zfar - znear);
		projAB = glm::vec2(projA, projB);
		glUniform2fv(projABLocation, 1, &projAB[0]);
		glUniform3fv(bgColorLocation, 1, &m_bgColor[0]);
		invProjMatrix = glm::inverse(projectionMatrix);
		glUniformMatrix4fv(invProjMatrixLocation, 1, GL_FALSE, &invProjMatrix[0][0]);
	}
	sh_accumulator->unbind();
	
	
	objparser.parse("obj/Octahedron.obj",&mesh, "flat");
	mesh.upload(sh_gbuffer->id());
	objparser.parse("obj/full_quad.obj", &full_quad, "flat");
	full_quad.upload(sh_gbuffer->id());
	m_fboInit = m_gbuffer.Init(windowWidth, windowHeight);
	if(!m_fboInit) std::cout << "Couldn't initialize FBO!" << std::endl;
}

void OpenGLContext::reshapeWindow(int w, int h){
	windowWidth = w;
	windowHeight = h;
	TwWindowSize(w, h);
	glViewport(0, 0, windowWidth, windowHeight);
	projectionMatrix = glm::perspective(fov+zoom, (float)windowWidth/(float)windowHeight, znear, zfar);
	if(m_fboInit){
		m_gbuffer.Resize(windowWidth, windowHeight);
		sh_ssao->bind();
		{
			m_ssao.Resize(windowWidth, windowHeight);
		}
		sh_ssao->unbind();
		sh_blur->bind();
		{
			glm::vec2 texel_size = glm::vec2(1.0 / windowWidth, 1.0 / windowHeight);
			glUniform2fv(texelSizeLocation, 1, &texel_size[0]);
		}
		sh_blur->unbind();
	}
}

void OpenGLContext::processScene(void){
	static float last_time = 0.0;
	float this_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	if(this_time-last_time > 1.0f/61.0f){
		redisplay = true;
		last_time = this_time;
		projectionMatrix = glm::perspective(fov+zoom, (float)windowWidth/(float)windowHeight, znear, zfar);
	}
}

void OpenGLContext::drawConfigurationBox(void){
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
	
	GLuint vaoBox;
	GLuint vboBox;
	GLuint iboBox;
	
	glUniform3f(diffColorLocation, 0.1f, 0.1f, 0.1f);
	
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
	
	glLineWidth(4);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4*sizeof(GLushort)));
	glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8*sizeof(GLushort)));
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glDeleteBuffers(1, &vaoBox);
	glDeleteBuffers(1, &vboBox);
	glDeleteBuffers(1, &iboBox);
}

void OpenGLContext::drawConfiguration(void){
	
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
	
	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, &MVPMatrix[0][0]);
	
	if(drawBox) drawConfigurationBox();
	
	glUniform3fv(diffColorLocation, 1, &diffcolor[0]);
	
	for(int i = 0; i < coordparser.npart; i++){
		glm::mat4 tLocalMatrix = glm::translate(ModelViewMatrix, coordparser.centers[i]);
		glm::mat4 rLocalMatrix = glm::rotate(
			glm::mat4(1.0),
			coordparser.rotations[i].x,
			glm::vec3(coordparser.rotations[i].y, coordparser.rotations[i].z, coordparser.rotations[i].w)
		);
		
		glm::mat4 tempModelViewMatrix = tLocalMatrix * rLocalMatrix ;
		glm::mat3 tempNormalMatrix = glm::mat3(glm::transpose(glm::inverse(tempModelViewMatrix)));
		glm::mat4 tempMVPMatrix = pMatrix * tempModelViewMatrix;
		
		glUniformMatrix3fv(NormalMatrixLocation, 1, GL_FALSE, &tempNormalMatrix[0][0]);
		glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, &tempMVPMatrix[0][0]);
		
		mesh.draw();
	}
}


void OpenGLContext::fboPass(void){

	glDisable(GL_BLEND);
	
	m_gbuffer.BindForWriting();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	sh_gbuffer->bind();
	{	
		glm::mat4 ModelViewMatrix = viewMatrix * modelMatrix;
		glm::mat3 NormalMatrix = glm::mat3(glm::transpose(glm::inverse(ModelViewMatrix)));
		glm::mat4 MVPMatrix = projectionMatrix * ModelViewMatrix;

		glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, &MVPMatrix[0][0]);
		glUniformMatrix3fv(NormalMatrixLocation, 1, GL_FALSE, &NormalMatrix[0][0]);

		if(use_dat)	drawConfiguration();
		else{
			glUniform3fv(diffColorLocation, 1, &diffcolor[0]);
			mesh.draw();
		}
	}
	sh_gbuffer->unbind();
	
	glEnable(GL_BLEND);
}

void OpenGLContext::ssaoPass(void){
	
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	m_gbuffer.BindForSSAO();
	m_ssao.BindNoise();
	m_ssao.BindForWriting();
	glClear(GL_COLOR_BUFFER_BIT);
	
	sh_ssao->bind();
	{
		m_ssao.UpdateUniforms();
		full_quad.draw();
	}
	sh_ssao->unbind();
	
	m_ssao.BindForReading();
	m_gbuffer.BindForWriting();
	
	sh_blur->bind();
	{
		glUniform1i(m_blurLocation, int(m_blur));
		full_quad.draw();
	}
	sh_blur->unbind();
	
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

void OpenGLContext::drawPass(void){
	
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);
	m_gbuffer.BindForReading();
	
	sh_accumulator->bind();
	{
		light.uploadDirection(viewMatrix);
		full_quad.draw();
	}
	sh_accumulator->unbind();
	
	glDepthMask(GL_TRUE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void OpenGLContext::renderScene(void){
	modelMatrix = trackballMatrix;
	
	fboPass();
	glDisable(GL_CULL_FACE);
	ssaoPass();
	drawPass();
	glEnable(GL_CULL_FACE);
	TwDraw();
	
	glutSwapBuffers();
}

float OpenGLContext::getZoom(void){
	return zoom;
}

void OpenGLContext::setZoom(float zoom){
	this->zoom = zoom;
}


glm::ivec2 OpenGLContext::getScreen(void){
	glm::ivec2 screen = glm::ivec2(windowWidth, windowHeight);
	return screen;
}