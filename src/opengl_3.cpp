#include "../include/opengl_3.h"

OpenGLContext::OpenGLContext(void):mesh(1.0f){
	/////////////////////////////////////////////////
	// Default Constructor for OpenGLContext class //
	/////////////////////////////////////////////////
	zoom = 0.0f;
	fov = 60.0f;
	redisplay = false;
	trackballMatrix = glm::mat4(1.0);
	use_dat = false;
	drawBox = false;
	light = CLight(glm::vec3(-7.0, 7.0, 0.0), glm::vec3(1.0, -1.0, -1.0));
}

OpenGLContext::~OpenGLContext(void) {  
}  

bool OpenGLContext::create30Context(void){
	////////////////////////////////////////////////////
	// Create an OpenGL 3.2 context. Fall back to a   //
	// OpenGL 2.1 Context if it fails.				  //
	////////////////////////////////////////////////////
	
	glutInitContextVersion(3,3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA | GLUT_MULTISAMPLE);
	glutInitWindowSize(600,600);
	windowWidth=windowHeight=600;
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
	
	return true; // Success, return true
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
	glClearColor(0.4f,0.6f,0.9f,1.0f);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	shader = new Shader("shaders/shader.vert", "shaders/shader.frag");
	
	ModelViewMatrixLocation = glGetUniformLocation(shader->id(),"ModelViewMatrix");
	MVPMatrixLocation = glGetUniformLocation(shader->id(),"MVPMatrix");
	NormalMatrixLocation = glGetUniformLocation(shader->id(),"NormalMatrix");
	ScaleLocation = glGetUniformLocation(shader->id(),"scale");
	if(!light.Init(shader->id())) std::cout << "Could not bind light uniforms" << std::endl;
	
	if(
		ModelViewMatrixLocation == -1 || MVPMatrixLocation == -1 || NormalMatrixLocation == -1 ||
		ScaleLocation == -1
	) std::cout<<"Unable to bind uniform"<<std::endl;
	
	projectionMatrix = glm::perspective(fov+zoom, (float)windowWidth/(float)windowHeight, 0.1f, 100.0f);
	viewMatrix = glm::lookAt(glm::vec3(0.0, 2.0, 0.0), glm::vec3(0.0, 0.0, init_zoom), glm::vec3(0.0, 1.0, 0.0));
	viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0, 0.0, init_zoom));
	modelMatrix = glm::mat4(1.0);
	
	objparser.parse("obj/Octahedron.obj",&mesh, "flat");
	mesh.upload(shader->id());
}

void OpenGLContext::reshapeWindow(int w, int h){
	windowWidth = w;
	windowHeight = h;
	glViewport(0, 0, windowWidth, windowHeight);
	projectionMatrix = glm::perspective(fov+zoom, (float)windowWidth/(float)windowHeight, 0.1f, 100.0f);
}

void OpenGLContext::processScene(void){
	static float last_time = 0.0;
	float this_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	if(this_time-last_time > 1.0f/61.0f){
		redisplay = true;
		last_time = this_time;
		projectionMatrix = glm::perspective(fov+zoom, (float)windowWidth/(float)windowHeight, 0.1f, 100.0f);
	}
}

void OpenGLContext::drawConfigurationBox(void){
	GLfloat vertices[]={
		0.0f,0.0f,0.0f,
		coordparser.boxMatrix[0][0],coordparser.boxMatrix[1][0],coordparser.boxMatrix[2][0],
		coordparser.boxMatrix[0][1],coordparser.boxMatrix[1][1],coordparser.boxMatrix[2][1],
		coordparser.boxMatrix[0][2],coordparser.boxMatrix[1][2],coordparser.boxMatrix[2][2],
		coordparser.boxMatrix[0][0]+coordparser.boxMatrix[0][1],coordparser.boxMatrix[1][0]+coordparser.boxMatrix[1][1],coordparser.boxMatrix[2][0]+coordparser.boxMatrix[2][1],
		coordparser.boxMatrix[0][0]+coordparser.boxMatrix[0][2],coordparser.boxMatrix[1][0]+coordparser.boxMatrix[1][2],coordparser.boxMatrix[2][0]+coordparser.boxMatrix[2][2],
		coordparser.boxMatrix[0][1]+coordparser.boxMatrix[0][2],coordparser.boxMatrix[1][1]+coordparser.boxMatrix[1][2],coordparser.boxMatrix[2][1]+coordparser.boxMatrix[2][2],
		
		coordparser.boxMatrix[0][0]+coordparser.boxMatrix[0][1]+coordparser.boxMatrix[0][2],
		coordparser.boxMatrix[1][0]+coordparser.boxMatrix[1][1]+coordparser.boxMatrix[1][2],
		coordparser.boxMatrix[2][0]+coordparser.boxMatrix[2][1]+coordparser.boxMatrix[2][2]
	};
	
	GLushort elements[]={
		3,6,7,5,
		0,2,4,1,
		3,0,2,6,4,7,1,5
	};
	
	GLuint vaoBox;
	GLuint vboBox;
	GLuint iboBox;
	
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
	glm::mat4 tMatrix = glm::translate(modelMatrix, glm::vec3(-(coordparser.boxMatrix[0][0]+coordparser.boxMatrix[0][1]+coordparser.boxMatrix[0][2])/2.0,
															  -(coordparser.boxMatrix[1][1]+coordparser.boxMatrix[1][2])/2.0,
															  -coordparser.boxMatrix[2][2]/2.0));
	
	glm::mat4 ModelViewMatrix = viewMatrix * tMatrix;
	glm::mat4 MVPMatrix = projectionMatrix * ModelViewMatrix;
	
	glUniformMatrix4fv(ModelViewMatrixLocation, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, &MVPMatrix[0][0]);
	
	if(drawBox) drawConfigurationBox();
	
	for(int i = 0; i < coordparser.npart; i++){
		glm::mat4 tLocalMatrix = glm::translate(ModelViewMatrix, coordparser.centers[i]);
		glm::mat4 rLocalMatrix = glm::rotate(
			glm::mat4(1.0),
			coordparser.rotations[i].x,
			glm::vec3(coordparser.rotations[i].y, coordparser.rotations[i].z, coordparser.rotations[i].w)
		);
		
		glm::mat4 tempModelViewMatrix = tLocalMatrix * rLocalMatrix ;
		glm::mat3 tempNormalMatrix = glm::mat3(glm::transpose(glm::inverse(tempModelViewMatrix)));
		glm::mat4 tempMVPMatrix = projectionMatrix * tempModelViewMatrix;
		
		glUniformMatrix4fv(ModelViewMatrixLocation, 1, GL_FALSE, &tempModelViewMatrix[0][0]);
		glUniformMatrix3fv(NormalMatrixLocation, 1, GL_FALSE, &tempNormalMatrix[0][0]);
		glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, &tempMVPMatrix[0][0]);
		
		mesh.draw();
	}
}

void OpenGLContext::renderScene(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	shader->bind();
		
		modelMatrix = trackballMatrix;
		glm::mat4 ModelViewMatrix = viewMatrix * modelMatrix;
		glm::mat3 NormalMatrix = glm::mat3(glm::transpose(glm::inverse(ModelViewMatrix)));
		glm::mat4 MVPMatrix = projectionMatrix * ModelViewMatrix;
		
		glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, &MVPMatrix[0][0]);
		glUniformMatrix4fv(ModelViewMatrixLocation, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(NormalMatrixLocation, 1, GL_FALSE, &NormalMatrix[0][0]);
		light.uploadDirection(viewMatrix);
		
		if(use_dat)	drawConfiguration();
		else mesh.draw();
	
	shader->unbind();
	
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