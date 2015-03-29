#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <cstring>
#include <cstdio>

inline void glError(const char *file, int line) {
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR){
		char error[128];

		switch(err) {
			case GL_INVALID_ENUM:                   strncpy(error, "INVALID_ENUM", sizeof(error));                   break;
			case GL_INVALID_VALUE:                  strncpy(error, "INVALID_VALUE", sizeof(error));                  break;
			case GL_INVALID_OPERATION:              strncpy(error, "INVALID_OPERATION", sizeof(error));              break;
			case GL_OUT_OF_MEMORY:                  strncpy(error, "OUT_OF_MEMORY", sizeof(error));                  break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:  strncpy(error, "INVALID_FRAMEBUFFER_OPERATION", sizeof(error));  break;
            default:                                strncpy(error, "UNKNOWN_ERROR", sizeof(error));                  break;
		}

        fprintf(stderr, "GL%s - %s: %d", error, file, line);
	}
}


#endif
