#ifndef __SHADER_H
#define __SHADER_H

#include <map>
#include <string>
#include <glm/fwd.hpp>

class Shader{
public:
	Shader(void);
	Shader(const char *vsText, const char *fsText = NULL, const char *gsText = NULL);
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
	void setUniform(const char * name, int count, glm::mat2 const &value)const;
	void setUniform(const char * name, int count, glm::mat3 const &value)const;
	void setUniform(const char * name, int count, glm::mat4 const &value)const;
	void setUniform(const char * name, int value)const;
	void setUniform(const char * name, float value)const;
	void setUniform(const char * name, float value1, float value2, float value3)const;
	
private:
	unsigned int shader_id;
	
	std::map<std::string, int> mUniformLocations;
};

#endif
