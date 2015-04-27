#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <cstdio>

#ifdef _MSC_VER
#define snprintf sprintf_s
#endif

inline void glError(const char *file, int line) {
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR){
		char error[128];

		switch(err) {
			case GL_INVALID_ENUM:                   snprintf(error, sizeof(error), "%s", "INVALID_ENUM");                   break;
			case GL_INVALID_VALUE:                  snprintf(error, sizeof(error), "%s", "INVALID_VALUE");                  break;
			case GL_INVALID_OPERATION:              snprintf(error, sizeof(error), "%s", "INVALID_OPERATION");              break;
			case GL_OUT_OF_MEMORY:                  snprintf(error, sizeof(error), "%s", "OUT_OF_MEMORY");                  break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:  snprintf(error, sizeof(error), "%s", "INVALID_FRAMEBUFFER_OPERATION");  break;
            default:                                snprintf(error, sizeof(error), "%s", "UNKNOWN_ERROR");                  break;
		}

        fprintf(stderr, "GL%s - %s: %d\n", error, file, line);
	}
}


#endif
