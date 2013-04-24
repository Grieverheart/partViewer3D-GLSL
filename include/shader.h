#ifndef __SHADER_H
#define __SHADER_H

#include "main.h"
#include <map>
#include <string>

class Shader{
public:
	Shader(void);
	Shader(const char *vsFile, const char *fsFile, const char *gsFile = NULL);
	~Shader(void);
	
	void bind(void);
	void unbind(void);
	unsigned int id(void);
	
	void setUniform(const char * name, int count, glm::vec2 const &value)const;
	void setUniform(const char * name, int count, glm::vec3 const &value)const;
	void setUniform(const char * name, int count, glm::vec4 const &value)const;
	void setUniform(const char * name, int count, glm::ivec2 const &value)const;
	void setUniform(const char * name, int count, glm::ivec3 const &value)const;
	void setUniform(const char * name, int count, glm::ivec4 const &value)const;
	void setUniform(const char * name, int count, glm::mat3 const &value)const;
	void setUniform(const char * name, int count, glm::mat4 const &value)const;
	void setUniform(const char * name, int value)const;
	void setUniform(const char * name, float value)const;
	void setUniform(const char * name, float value1, float value2, float value3)const;
	
private:
	void init(void);
	
	unsigned int shader_id;
	unsigned int shader_vp;
	unsigned int shader_gp;
	unsigned int shader_fp;
	
	std::map<std::string, GLint> mUniformLocations;
};

#endif