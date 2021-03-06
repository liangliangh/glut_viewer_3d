
// heliangliang_ustb@126.com

#ifndef _GL_STAFF_H_
#define _GL_STAFF_H_

#define GL_GLEXT_PROTOTYPES // used in <GL/glext.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>

#define GLM_FORCE_RADIANS // need not for glm 9.6
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "omp.h"
#include <vector>


namespace GlStaff {


void init( int win_width, int win_height, const char* win_tile );
void add_key_func( int key, void (*f)() );
void display_loop( void (*draw)() ); // do not swapbuffer in draw()
void xyz_frame(float xlen, float ylen, float zlen, bool solid);
void toggle_fps();
bool toggle_proj_orth();

void hsl_to_rgb( float h, float s, float l, float* rgb );
float rgb_to_gray( float r, float g, float b );
GLuint load_tex(const char* file, bool only_red2alpha=false);
int load_tex_from_video(const char* file, std::vector<GLuint>& textures, bool only_red2alpha=false);


} // namespace GlStaff


#endif // #ifndef _GL_STAFF_H_
