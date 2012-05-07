#include "../include/shader.h"
#include <string>
#include <fstream>

static void validateShader(GLuint shader, const char *file = 0){
	const unsigned int BUFFER_SIZE = 512;
	char buffer[BUFFER_SIZE];
	//memset(buffer, 0, BUFFER_SIZE);
	GLsizei length = 0;
	
	glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);
	
	if(length>0){
		std::cout << "Shader" << shader << "(" << (file?file:"") << ") compile errr:" << buffer << std::endl;
	}
}

static void validateProgram(GLuint program){
	const unsigned int BUFFER_SIZE = 512;
	char buffer[BUFFER_SIZE];
	//memset(buffer, 0, BUFFER_SIZE);
	GLsizei length = 0;
	
	glGetShaderInfoLog(program, BUFFER_SIZE, &length, buffer);
	
	if(length>0){
		std::cout << "Program" << program << "link error: " << buffer << std::endl;
	}
	
	glValidateProgram(program);
	GLint status;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	
	if(status == GL_FALSE){
		std::cout << "Error validating program " << program << std::endl;
	}
}

static std::string textFileRead(const char *fileName){
	std::string fileString = std::string(); // A string for storing the file contents
	std::string line = std::string(); // A string for holding the current line

	std::ifstream file(fileName); // Open an input stream with the selected file
	if (file.is_open()) { // If the file opened successfully
		while (!file.eof()) { // While we are not at the end of the file
			getline(file, line); // Get the current line
		  	fileString.append(line); // Append the line to our file string
			fileString.append("\n"); // Appand a new line character
		}
		file.close(); // Close the file
	}

    return fileString; // Return our string
}

Shader::Shader(void){
}

Shader::Shader(const char *vsFile, const char *fsFile){
	initialized = false;
	init(vsFile, fsFile);
}

Shader::~Shader(void){
	glDetachShader(shader_id, shader_fp);
	glDetachShader(shader_id, shader_vp);
	
	glDeleteShader(shader_fp);
	glDeleteShader(shader_vp);
	glDeleteShader(shader_id);
}

void Shader::init(const char *vsFile, const char *fsFile){
	shader_vp=glCreateShader(GL_VERTEX_SHADER);
	shader_fp=glCreateShader(GL_FRAGMENT_SHADER);
	
	std::string vsText = textFileRead(vsFile);
	std::string fsText = textFileRead(fsFile);
    
	const char *vertexText = vsText.c_str();
	const char *fragmentText = fsText.c_str();
	
	if(vertexText == NULL || fragmentText == NULL){
		std::cout << "Either vertex shader or fragment shader file not found" << std::endl;
		return;
	}
	
	glShaderSource(shader_vp, 1, &vertexText, 0);
	glCompileShader(shader_vp);
	validateShader(shader_vp, vsFile);
	
	glShaderSource(shader_fp, 1, &fragmentText, 0);
	glCompileShader(shader_fp);
	validateShader(shader_fp, fsFile);
	
	shader_id = glCreateProgram();
	
	glAttachShader(shader_id, shader_fp);
	glAttachShader(shader_id, shader_vp);
	
	glBindAttribLocation(shader_id, 0, "in_Position");
	glBindAttribLocation(shader_id, 1, "in_Normal");
	
	glLinkProgram(shader_id);
	validateProgram(shader_id);
}

unsigned int Shader::id(void){
	return shader_id;
}

void Shader::bind(void){
	glUseProgram(shader_id);
}

void Shader::unbind(void){
	glUseProgram(0);
}
