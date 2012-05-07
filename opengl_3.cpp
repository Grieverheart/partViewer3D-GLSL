#include "opengl_3.h"

OpenGLContext::OpenGLContext(void){
	/////////////////////////////////////////////////
	// Default Constructor for OpenGLContext class //
	/////////////////////////////////////////////////
	zoom = 0.0f;
	fov = 60.0f;
	redisplay = false;
	trackballMatrix = glm::mat4(1.0);
	use_dat = false;
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
	glutInitWindowPosition(100,100);
	glutCreateWindow("Project");
	
	glewExperimental = GL_TRUE;
	GLenum error = glewInit(); //Enable GLEW
	if(error != GLEW_OK) return false; //Failure!
	
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
		init_zoom = -2.0 * coordparser.boxMatrix[2][2];
	}
	glClearColor(0.4f,0.6f,0.9f,1.0f);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	
	shader = new Shader("shader.vert", "shader.frag");
	
	projectionMatrixLocation = glGetUniformLocation(shader->id(),"projectionMatrix");
	viewMatrixLocation = glGetUniformLocation(shader->id(),"viewMatrix");
	modelMatrixLocation = glGetUniformLocation(shader->id(),"modelMatrix");
	trackballMatrixLocation = glGetUniformLocation(shader->id(),"trackballMatrix");
	
	if(projectionMatrixLocation == -1 || viewMatrixLocation == -1 || modelMatrixLocation == -1){
		std::cout<<"Unable to bind uniform"<<std::endl;
	}
	
	projectionMatrix = glm::perspective(fov+zoom, (float)windowWidth/(float)windowHeight, 0.1f, 100.0f);
	viewMatrix = glm::lookAt(glm::vec3(0.0, 2.0, 0.0), glm::vec3(0.0, 0.0, init_zoom), glm::vec3(0.0, 1.0, 0.0));
	viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0, 0.0, init_zoom));
	modelMatrix = glm::mat4(1.0);
	
	objparser.parse("octahedron.obj",&monkey);
	monkey.upload(shader->id());
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

void OpenGLContext::renderScene(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	shader->bind();
	
		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
		glUniformMatrix4fv(trackballMatrixLocation, 1, GL_FALSE, &trackballMatrix[0][0]);
		if(use_dat){
			glm::mat4 tMatrix = glm::translate(modelMatrix, glm::vec3(-coordparser.boxMatrix[0][0]/2.0,-coordparser.boxMatrix[1][1]/2.0,-coordparser.boxMatrix[2][2]/2.0));
			for(int i = 0; i < coordparser.npart; i++){
				glm::mat4 tLocalMatrix = glm::translate(tMatrix, coordparser.centers[i]);
				glm::mat4 rLocalMatrix = glm::rotate(glm::mat4(1.0),
													coordparser.rotations[i].x,
													glm::vec3(coordparser.rotations[i].y, coordparser.rotations[i].z, coordparser.rotations[i].w));
				glm::mat4 tempMatrix = tLocalMatrix * rLocalMatrix ;
				glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &tempMatrix[0][0]);
				monkey.draw();
			}
		}
		else monkey.draw();
	
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