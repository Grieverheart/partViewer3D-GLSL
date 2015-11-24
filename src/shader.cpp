#include "include/shader.h"
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#include <glm/glm.hpp>
#include <cstdio>

static inline void validateShader(GLuint shader, const char *file = 0){
	static const unsigned int BUFFER_SIZE = 512;
	char buffer[BUFFER_SIZE];
	GLsizei length = 0;
	
	glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);
	
	if(length>0){
		printf("Shader %d(%s) compile error: %s\n", shader, (file? file: ""), buffer);
	}
}

static inline bool validateProgram(GLuint program){
	static const GLsizei BUFFER_SIZE = 512;
	GLchar buffer[BUFFER_SIZE];
	GLsizei length = 0;
	
	glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);
	
	if(length>0){
		printf("Program %d link error: %s\n", program, buffer);
        return false;
	}

	return true;
}

Shader::Shader(void):
    shader_id(0)
{}

Shader::Shader(const char *vertexText, const char *fragmentText, const char *geometryText):
    shader_id(0)
{
	bool isGS = (geometryText != NULL);
	bool isFS = (fragmentText != NULL);
	
	shader_id = glCreateProgram();
	
    {
        unsigned int shader_vp = glCreateShader(GL_VERTEX_SHADER);

        glShaderSource(shader_vp, 1, &vertexText, 0);
        glCompileShader(shader_vp);
        validateShader(shader_vp, vertexText);
        glAttachShader(shader_id, shader_vp);

        glDeleteShader(shader_vp);
    }

	if(isFS){
        unsigned int shader_fp = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(shader_fp, 1, &fragmentText, 0);
        glCompileShader(shader_fp);
        validateShader(shader_fp, fragmentText);
        glAttachShader(shader_id, shader_fp);

        glDeleteShader(shader_fp);
    }

	if(isGS){
        unsigned int shader_gp = glCreateShader(GL_GEOMETRY_SHADER);

		glShaderSource(shader_gp, 1, &geometryText, 0);
		glCompileShader(shader_gp);
		validateShader(shader_gp, geometryText);
        glAttachShader(shader_id, shader_gp);

        glDeleteShader(shader_gp);
    }
	
	glLinkProgram(shader_id);

    //TODO: Add exception
	if(validateProgram(shader_id)){
        GLint nUniforms;
        glGetProgramiv(shader_id, GL_ACTIVE_UNIFORMS, &nUniforms);
        for(GLint i = 0; i < nUniforms; i++){
            GLchar name[64];
            GLint size;
            GLenum type;
            glGetActiveUniform(shader_id, i, 64, NULL, &size, &type, name);
            GLint location = glGetUniformLocation(shader_id, name);
            mUniformLocations[std::string(name)] = location;
            if(location == -1) printf("Couldn't bind uniform %s in program %d.\n", name, shader_id);
        }
    }
}

//TODO: Properly delete shaders!
Shader::~Shader(void){
	glDeleteProgram(shader_id);
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

void Shader::setUniform(const char * name, int count, glm::vec2 const &value)const{
	GLint location = mUniformLocations.find(std::string(name))->second;
	glUniform2fv(location, count, &value[0]);
}

void Shader::setUniform(const char * name, int count, glm::vec3 const &value)const{
	GLint location = mUniformLocations.find(std::string(name))->second;
	glUniform3fv(location, count, &value[0]);
}

void Shader::setUniform(const char * name, int count, glm::vec4 const &value)const{
	GLint location = mUniformLocations.find(std::string(name))->second;
	glUniform4fv(location, count, &value[0]);
}

void Shader::setUniform(const char * name, int count, glm::ivec2 const &value)const{
	GLint location = mUniformLocations.find(std::string(name))->second;
	glUniform2iv(location, count, &value[0]);
}

void Shader::setUniform(const char * name, int count, glm::ivec3 const &value)const{
	GLint location = mUniformLocations.find(std::string(name))->second;
	glUniform3iv(location, count, &value[0]);
}
void Shader::setUniform(const char * name, int count, glm::ivec4 const &value)const{
	GLint location = mUniformLocations.find(std::string(name))->second;
	glUniform4iv(location, count, &value[0]);
}

void Shader::setUniform(const char * name, int count, glm::mat2 const &value)const{
	GLint location = mUniformLocations.find(std::string(name))->second;
	glUniformMatrix2fv(location, count, GL_FALSE, &value[0][0]);
}

void Shader::setUniform(const char * name, int count, glm::mat3 const &value)const{
	GLint location = mUniformLocations.find(std::string(name))->second;
	glUniformMatrix3fv(location, count, GL_FALSE, &value[0][0]);
}

void Shader::setUniform(const char * name, int count, glm::mat4 const &value)const{
	GLint location = mUniformLocations.find(std::string(name))->second;
	glUniformMatrix4fv(location, count, GL_FALSE, &value[0][0]);
}

void Shader::setUniform(const char * name, int value)const{
	GLint location = mUniformLocations.find(std::string(name))->second;
	glUniform1i(location, value);
}

void Shader::setUniform(const char * name, float value)const{
	GLint location = mUniformLocations.find(std::string(name))->second;
	glUniform1f(location, value);
}

void Shader::setUniform(const char * name, float value1, float value2, float value3)const{
	GLint location = mUniformLocations.find(std::string(name))->second;
	glUniform3f(location, value1, value2, value3);
}
