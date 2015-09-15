
#ifndef _GL_INFO_H_
#define _GL_INFO_H_

#define GL_GLEXT_PROTOTYPES // used in <GL/glext.h>
#include <GL/gl.h>
#include <GL/glext.h>


void printGlError(const char* ss, bool always=false);
void printShaderInfoLog(GLuint obj, bool always=false);
void printProgramInfoLog(GLuint obj, bool always=false);
void printGlVersion();


static const char* glerrorstring(GLenum code)
{
	switch(code){
		case GL_NO_ERROR:
			return "GL_NO_ERROR";
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
		default:
			return "undefined error code";
	}
}

void printGlError(const char* ss, bool always)
{
	GLenum code = glGetError();
	if( always || code != GL_NO_ERROR ){
		printf("%s: %s\n", ss, glerrorstring(code));
	}
}

// http://www.lighthouse3d.com/tutorials/glsl-tutorial/
void printShaderInfoLog(GLuint obj, bool always)
{
	GLint ret; glGetShaderiv(obj, GL_COMPILE_STATUS, &ret);
	if(!always && ret==GL_TRUE) return;
	if(ret==GL_TRUE) printf("compile shader %d success.\n", obj);
	else 			 printf("compile shader %d failed.\n", obj);

	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 0) {
		infoLog = (char*)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("compile log:\n%s\n",infoLog);
		free(infoLog);
	}
}

void printProgramInfoLog(GLuint obj, bool always)
{
	GLint ret; glGetProgramiv(obj, GL_LINK_STATUS, &ret);
	if(!always && ret==GL_TRUE) return;
	if(ret==GL_TRUE) printf("link program %d success.\n", obj);
	else 			 printf("link program %d failed.\n", obj);

	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 0) {
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("link log:\n%s\n",infoLog);
		free(infoLog);
	}
}

void printGlVersion()
{
	printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
	printf("OpenGL Vendor : %s\n", glGetString(GL_VENDOR));
	//printf(" OpenGL Render: %s\n", glGetString(GL_RENDER));
	printf("GLSL Version  : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	int n;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &n);
	printf("max supported texture units: %d\n", n);
	glGetIntegerv(GL_MAX_IMAGE_UNITS, &n);
	printf("max supported image units  : %d\n", n);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &n);
	printf("max length of texture array: %d\n", n);
}

#endif // #ifndef _GL_INFO_H_
