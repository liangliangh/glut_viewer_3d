
#ifndef _TRANSFORMATION_STATE_H_
#define _TRANSFORMATION_STATE_H_

#define GL_GLEXT_PROTOTYPES // used in <GL/glext.h>
#include <GL/gl.h>
#include <GL/glext.h>
//#include <GL/glut.h>

#define GLM_FORCE_RADIANS // need not for glm 9.6
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include <vector>
#include <cmath>


// class to implement OpenGL view transformation from mouse
class TransformationState
{
public:
	int   win_w=1, win_h=1, o_x=0, o_y=0, origin_upper_left_ylen=0;
	glm::mat4 mat_modelview = glm::lookAt(glm::vec3(0,0,100), glm::vec3(0), glm::vec3(0,1,0));
	glm::mat4 mat_projection;
	float frustum_fovy = glm::radians(30.0f);
	float trackball_r = 0.4f; // radius of track ball relative to min(WIN_H, WIN_W), should less than 0.5
	float trackball_center_z = -100; // center of trackball, i.e., in camera coor
	bool  proj_orth = false; // true for orthogonal projection, false for perspective projection
	float origin_upper_left = true; // true if origin is upper-left, false lower-left
	float xpos=0, ypos=0;

	// rotate_*, translate use the last mouse pos saved by save_mouse_pos
	void save_mouse_pos(float x, float y);
	void rotate_grab(float x, float y);
	void rotate_trackball(float x, float y); // O at left-bottom of window, to left is X+, up is Y+
	void rotate_ground(float x, float y, const glm::vec3& ground_normal=glm::vec3(0,1,0));
	void rotate_ground(float x, float y, const float ground_normal[3]);
	void translate(float x, float y);
	// s=1, not change, >1 bigger, <1 smaller
	void scale(float s);
	void win_size(int width, int height, int ox=0, int oy=0, int ylen=0);
	bool toggle_orth();
	void load_gl_matrix() const;
	void draw_trackball(float transparency, float linewidth) const;


private:

	void normal_xy(float& x, float& y);
	static float angle(float d, float r);
	static void grab(float* theta, glm::vec3* normal, float ax, float ay, float bx, float by, float z);
	static void trackball(float* theta, glm::vec3* normal, float ax, float ay, float bx, float by, float r);
	// color_xox[4] is linewidth
	static void draw_3axis_unite_circle(float color_yoz[5], float color_zox[5], float color_xoy[5]);

}; // class TransformationState


inline void TransformationState::normal_xy(float& x, float& y)
{
	x = x - o_x;
	y = (origin_upper_left ? origin_upper_left_ylen-y : y) - o_y;
}

// rotate_*, translate use the last mouse pos saved by save_mouse_pos
inline void TransformationState::save_mouse_pos(float x, float y)
{
	normal_xy(x, y);
	xpos = x;
	ypos = y;
}

inline void TransformationState::rotate_grab(float x, float y)
{
	normal_xy(x, y);
	if(x==xpos && y==ypos) return;
	float s = -trackball_center_z*std::tan(frustum_fovy/2) / (win_h/2.0f);
	float theta; glm::vec3 n;
	grab( &theta, &n,
		(xpos-win_w/2.0f)*s, (ypos-win_h/2.0f)*s, (x-win_w/2.0f)*s, (y-win_h/2.0f)*s,
		trackball_center_z);
	mat_modelview = glm::rotate(theta, n) * mat_modelview;
}

inline void TransformationState::rotate_trackball(float x, float y) // O at left-bottom of window, to left is X+, up is Y+
{
	normal_xy(x, y);
	if(x==xpos && y==ypos) return;
	float theta; glm::vec3 n;
	trackball( &theta, &n,
		xpos-win_w/2.0f, ypos-win_h/2.0f, x-win_w/2.0f, y-win_h/2.0f,
		std::min(win_w,win_h)*trackball_r );
	glm::mat4 inv_modelview = glm::affineInverse(mat_modelview);
	glm::vec3 normal = glm::vec3( inv_modelview * glm::vec4(n, 0) );
	glm::vec3 center = glm::vec3( inv_modelview * glm::vec4(0, 0, trackball_center_z, 1) );
	mat_modelview *= glm::translate(center) * glm::rotate(theta, normal) * glm::translate(-center);
}

inline void TransformationState::rotate_ground(float x, float y, const glm::vec3& ground_normal)
{
	normal_xy(x, y);
	if(x==xpos && y==ypos) return;
	float r = std::min(win_w,win_h)*trackball_r;
	if(x!=xpos){
		float a = angle(x-xpos, r);
		glm::vec3 center = glm::vec3( glm::affineInverse(mat_modelview) * glm::vec4(0, 0, trackball_center_z, 1) );
		mat_modelview *= glm::translate(center) * glm::rotate(a, ground_normal) * glm::translate(-center);
	}
	if(y!=ypos){
		float a = angle(y-ypos, r);
		glm::vec3 v = glm::vec3(glm::affineInverse(mat_modelview)*glm::vec4(1,0,0, 0));
		glm::vec3 center = glm::vec3( glm::affineInverse(mat_modelview) * glm::vec4(0, 0, trackball_center_z, 1) );
		mat_modelview *= glm::translate(center) * glm::rotate(-a, v) * glm::translate(-center);
	}
}

inline void TransformationState::rotate_ground(float x, float y, const float ground_normal[3])
{
	if(ground_normal)
		rotate_ground(x, y, *(const glm::vec3*)ground_normal);
	else
		rotate_ground(x, y);
}

inline void TransformationState::translate(float x, float y)
{
	normal_xy(x, y);
	if(x==xpos && y==ypos) return;
	float dx = x-xpos, dy = y-ypos;
	float scale = std::tan(frustum_fovy/2)*(-trackball_center_z) / (win_h/2);
	mat_modelview = glm::translate( glm::vec3(scale*dx, scale*dy, 0) ) * mat_modelview;
}

// s=1, not change, >1 bigger, <1 smaller
inline void TransformationState::scale(float s)
{
	if(s==1 || s<=0) return;
	float z = trackball_center_z;
	trackball_center_z *= s;
	win_size(win_w, win_h, o_x, o_y, origin_upper_left_ylen); // recompute projection matrix
	mat_modelview = glm::translate( glm::vec3(0, 0, trackball_center_z-z) ) * mat_modelview;
}

inline void TransformationState::win_size(int width, int height, int ox, int oy, int ylen)
{
	win_w = width;
	win_h = height;
	o_x = ox;
	o_y = oy;
	if(ylen) origin_upper_left_ylen = ylen;
	else     origin_upper_left_ylen = height; //printf("%d  %d  %d  %d  %d\n", o_x, o_y, win_w, win_h, origin_upper_left_ylen);
	if(proj_orth){
		float hh = -trackball_center_z * std::tan(frustum_fovy/2) * 2;
		float ww = hh * win_w / win_h;
		mat_projection = glm::ortho(-ww/2, ww/2, -hh/2, hh/2, trackball_center_z*2, -trackball_center_z*4);
	}else{
		mat_projection = glm::perspective(
			frustum_fovy, float(win_w)/win_h, -trackball_center_z*0.1f, -trackball_center_z*10000);
	}
}

inline bool TransformationState::toggle_orth()
{
	proj_orth = !proj_orth;
	win_size(win_w, win_h, o_x, o_y, origin_upper_left_ylen);
	return proj_orth;
}

inline void TransformationState::load_gl_matrix() const
{
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&mat_projection[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&mat_modelview[0][0]);
}

inline void TransformationState::draw_trackball(float transparency, float linewidth) const
{
	float r = -trackball_center_z * std::tan( frustum_fovy/2 ) * trackball_r * 2;
	glm::vec3 center = glm::vec3(
			glm::affineInverse(mat_modelview) * glm::vec4(0, 0, trackball_center_z, 1) );

	glMatrixMode(GL_PROJECTION); glPushMatrix();
	glMatrixMode(GL_MODELVIEW); glPushMatrix();
	load_gl_matrix();
	glTranslatef(center.x, center.y, center.z);
	glScalef(r, r, r);

	float color_x[] = { 1,0,0, transparency, linewidth };
	float color_y[] = { 0,1,0, transparency, linewidth };
	float color_z[] = { 0,0,1, transparency, linewidth };
	draw_3axis_unite_circle(color_x, color_y, color_z);

	glMatrixMode(GL_PROJECTION); glPopMatrix();
	glMatrixMode(GL_MODELVIEW);glPopMatrix();
}

inline float TransformationState::angle(float d, float r)
{
	return d/r;
}

inline void TransformationState::grab(float* theta, glm::vec3* normal, float ax, float ay, float bx, float by, float z)
{
	glm::vec3 a = glm::normalize( glm::vec3(ax,ay,z) );
	glm::vec3 b = glm::normalize( glm::vec3(bx,by,z) );
	float d = glm::dot(a,b);
	if( d <= -1 )   *theta = std::acos(-1 ); // acoss(a<-1 || a>1) return NaN
	else if(d >= 1) *theta = std::acos( 1 );
	else            *theta = std::acos( d );
	glm::vec3 c = glm::cross( a, b );
	*normal = glm::length2(c)==0 ? glm::vec3(0,1,0) : c;
}

inline void TransformationState::trackball(float* theta, glm::vec3* normal, float ax, float ay, float bx, float by, float r)
{
	float az, bz, a2=ax*ax+ay*ay, b2=bx*bx+by*by, r2=r*r;
	if(a2 <= r2/2)
		az = std::sqrt( r2-a2 ); // https://www.opengl.org/wiki/Object_Mouse_Trackball
	else
		az = r2 / 2 / std::sqrt( a2 );
	if(b2 <= r2/2)
		bz = std::sqrt( r2-b2 );
	else
		bz = r2 / 2 / std::sqrt( b2 );
	glm::vec3 a = glm::normalize( glm::vec3(ax,ay,az) );
	glm::vec3 b = glm::normalize( glm::vec3(bx,by,bz) );
	float d = glm::dot(a,b);
	if( d <= -1 )   *theta = std::acos(-1 ); // acoss(a<-1 || a>1) return NaN
	else if(d >= 1) *theta = std::acos( 1 );
	else            *theta = std::acos( d );
	glm::vec3 c = glm::cross( a, b );
	*normal = glm::length2(c)==0 ? glm::vec3(0,1,0) : c;
}

// color_xox[4] is linewidth
inline void TransformationState::draw_3axis_unite_circle(float color_yoz[5], float color_zox[5], float color_xoy[5])
{
	static std::vector<float> circle_r1_xy;
	if( circle_r1_xy.size()==0 ){
		const int n = 100;
		for(int i=0; i<n; ++i){
			float radians = float(i)/n * 2 * 3.14159265f;
			circle_r1_xy.push_back( std::cos(radians) );
			circle_r1_xy.push_back( std::sin(radians) );
		}
		circle_r1_xy.push_back( 1 );
		circle_r1_xy.push_back( 0 );
	}

	GLboolean gl_li = glIsEnabled(GL_LIGHTING);
	GLboolean gl_dp = glIsEnabled(GL_DEPTH_TEST);
	GLboolean gl_bd = glIsEnabled(GL_BLEND);
	GLfloat   gl_lw;  glGetFloatv(GL_LINE_WIDTH, &gl_lw);
	GLfloat   gl_cl[4]; glGetFloatv(GL_CURRENT_COLOR, gl_cl);

	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// x
	glLineWidth(color_yoz[4]);
	glColor4fv(color_yoz);
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<circle_r1_xy.size(); i+=2){
		glVertex3f(0, circle_r1_xy[i], circle_r1_xy[i+1]);
	}
	glEnd();
	// y
	glLineWidth(color_zox[4]);
	glColor4fv(color_zox);
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<circle_r1_xy.size(); i+=2){
		glVertex3f(circle_r1_xy[i], 0, circle_r1_xy[i+1]);
	}
	glEnd();
	// z
	glLineWidth(color_xoy[4]);
	glColor4fv(color_xoy);
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<circle_r1_xy.size(); i+=2){
		glVertex3f(circle_r1_xy[i], circle_r1_xy[i+1], 0);
	}
	glEnd();

	if( gl_li ) glEnable(GL_LIGHTING);   else glDisable(GL_LIGHTING);
	if( gl_dp ) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
	if( gl_bd ) glEnable(GL_BLEND);      else glDisable(GL_BLEND);
	glLineWidth(gl_lw);
	glColor3fv(gl_cl);
}


#endif // #ifndef _TRANSFORMATION_STATE_H_
