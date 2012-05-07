#ifndef __SHADER_H
#define __SHADER_H

#include "main.h"

class Shader{
public:
	Shader(void);
	Shader(const char *vsFile, const char *fsFile);
	~Shader(void);
	
	void init(const char *vsFile, const char *fsFile);
	
	void bind(void);
	void unbind(void);
	unsigned int id(void);
	
private:
	unsigned int shader_id;
	unsigned int shader_vp;
	unsigned int shader_fp;
	
	bool initialized;
};

#endif