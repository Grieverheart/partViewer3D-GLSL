#ifndef __SHADER_H
#define __SHADER_H

#include <map>
#include <string>
#include <exception>
#include <glm/fwd.hpp>

#ifdef _MSC_VER
#define noexcept
#define snprintf sprintf_s
#endif

class Shader{
public:
    class InitializationException: public std::exception {
    public:
        explicit InitializationException(const char* shader_type, const char* shader_file){
            snprintf(buffer, 256, "Error when reading %s shader: %s", shader_type, shader_file);
        }

        virtual const char* what(void)const noexcept {
            return buffer;
        }
    private:
        char buffer[256];
    };

	Shader(void);
	Shader(const char *vsFile, const char *fsFile = NULL, const char *gsFile = NULL);
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
