
// heliangliang_ustb@126.com
#include "gl_transformation_state.h"
#include "gl_info.h"
#include "gl_staff.h"

#define GL_GLEXT_PROTOTYPES // used in <GL/glext.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>

#define GLM_FORCE_RADIANS // need not for glm 9.6
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <assert.h>
#include <vector>
#include <map>


// ------------------------ variables
static TransformationState trans_state;

static int screen_w, screen_h;
static bool camera_coor_marker = false;
static bool draw_fps = true;
static int  mouse_key_pressed = -1;
static int  modifier_key_pressed = -1;

// draw function
static void (*draw_func)() = NULL;

// user key functions
static std::map<int,void(*)()> key_funcs;

// ------------------------ declarations
static void initGL();
static void cb_display();
static void cb_reshape(int w, int h);
static void cb_keyboard(unsigned char key, int x, int y);
static void cb_specialkey(int key, int x, int y);
static void cb_mouseclick(int button, int trans_state, int x, int y);
static void cb_mousemotion(int x, int y);


// ------------------------ definitions
void GlStaff::init(int win_width, int win_height, const char* win_tile)
{
	int argc = 0;
	glutInit(&argc, NULL);
	screen_w = glutGet(GLUT_SCREEN_WIDTH);
	screen_h = glutGet(GLUT_SCREEN_HEIGHT);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE);
	glutInitWindowPosition(std::max((screen_w-win_width)/2,0), std::max((screen_h-win_height)/2,0)); // set window to center of screen
	glutInitWindowSize(win_width, win_height);
	cb_reshape(win_width, win_height);

	glutCreateWindow(win_tile);

	//  The callbacks, called after glutCreateWindow()
	glutDisplayFunc(cb_display);
	glutReshapeFunc(cb_reshape);
	glutKeyboardFunc(cb_keyboard);
	glutSpecialFunc(cb_specialkey);
	glutMouseFunc(cb_mouseclick);
	glutMotionFunc(cb_mousemotion);

	initGL();
}

static void initGL()
{
	// matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// color
	glClearColor(0, 0, 0, 1);
	glColor4f(.5f, .5f, .5f, 1);
	glShadeModel(GL_SMOOTH);

	// material
	GLfloat c[]={.7f, .7f, .7f, 1};
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c); // front, gray
	c[0]=.4f; c[1]=.4f; c[2]=.4f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, c);
	glMaterialf(GL_FRONT, GL_SHININESS, 50);
	c[0]=0; c[1]=0; c[2]=0;
	glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, c); // back, black
	//	glMaterialfv(GL_BACK, GL_SPECULAR, c);

	// lighting, light0
	GLfloat vec4f[]={1, 1, 1, 1};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, vec4f); // white DIFFUSE, SPECULAR
	glLightfv(GL_LIGHT0, GL_SPECULAR, vec4f);
	vec4f[0]=.0f; vec4f[1]=.0f; vec4f[2]=.0f;
	glLightfv(GL_LIGHT0, GL_AMBIENT, vec4f); // black AMBIENT
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // LOCAL_VIEWER
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE); // single side
	vec4f[0]=0.25f; vec4f[1]=0.25f; vec4f[2]=0.25f;
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, vec4f); // global AMBIENT lighting, gray

	//glEnable(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
	glDisable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	// blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_NORMALIZE);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);

	printGlVersion();
	printGlError("initGL()");
}

void GlStaff::add_key_func( int key, void (*f)() )
{
	assert(f);

	key_funcs[key] = f;
}

void GlStaff::display_loop( void (*draw)() )
{
	assert(draw);

	draw_func = draw;
	glutMainLoop();
}

bool GlStaff::toggle_proj_orth()
{
	return trans_state.toggle_orth();
}

static void cb_display()
{
	// matrix
	trans_state.load_gl_matrix();

	// call user's draw()
	draw_func();

	// fps and trans_state
	if(mouse_key_pressed==-1)
		trans_state.draw_trackball(0.3f, 1);
	else
		trans_state.draw_trackball(0.5f, 2);

	printGlError("cb_display()");
	glutSwapBuffers();
}

static void cb_reshape(int w, int h)
{
	trans_state.win_size(w, h);
	glViewport(0, 0, w, h);
}

static void cb_keyboard(unsigned char key, int x, int y)
{
	switch(key){
	case 27: // esc
		exit(0);
		break;
	default:
		if(key_funcs.find(key)!=key_funcs.end()){
			if(key_funcs[key])
				(*key_funcs[key])();
		}
	};
}

static void cb_specialkey(int key, int x, int y)
{
	switch(key){
	case GLUT_KEY_F1:
		break;
	};
}

#ifndef GLUT_WHEEL_UP
#define GLUT_WHEEL_UP 3
#endif
#ifndef GLUT_WHEEL_DOWN
#define GLUT_WHEEL_DOWN 4
#endif

static void cb_mouseclick(int button, int bstate, int x, int y)
{
	if( bstate == GLUT_UP ){
		mouse_key_pressed = -1;
		modifier_key_pressed = -1;
		return;
	}

	switch(button) {
	case GLUT_LEFT_BUTTON:
	case GLUT_RIGHT_BUTTON:
	case GLUT_MIDDLE_BUTTON:
		mouse_key_pressed = button;
		modifier_key_pressed = glutGetModifiers();
		break;
	case GLUT_WHEEL_UP:
		trans_state.scale(1.05f);
		break;
	case GLUT_WHEEL_DOWN:
		trans_state.scale(0.95f);
		break;
	}

	trans_state.save_mouse_pos(x, y);
}

static void cb_mousemotion(int x, int y)
{
	if(mouse_key_pressed==GLUT_LEFT_BUTTON)
	{ // mouse left, trackball
		if(modifier_key_pressed & GLUT_ACTIVE_CTRL){
			trans_state.rotate_ground(x,y,glm::vec3(0,1,0));
		}else{
			trans_state.rotate_trackball(x, y);
		}
	}
	else if(mouse_key_pressed==GLUT_RIGHT_BUTTON)
	{ // mouse right, change camera direction
		trans_state.rotate_grab(x, y);
	}
	else if(mouse_key_pressed==GLUT_MIDDLE_BUTTON)
	{ // mouse middle, translate
		trans_state.translate(x, y);
	}

	trans_state.save_mouse_pos(x, y);
}

void GlStaff::xyz_frame(float xlen, float ylen, float zlen, bool solid)
{
	if(solid){ // yz frame as solid arrows
		GLfloat color_gls[4]; glGetMaterialfv(GL_FRONT, GL_SPECULAR, color_gls);
		GLfloat color_gla[4]; glGetMaterialfv(GL_FRONT, GL_AMBIENT, color_gla);
		GLfloat color_gld[4]; glGetMaterialfv(GL_FRONT, GL_DIFFUSE, color_gld);

		GLfloat color[]={0, 0, 0, 1};
		glMaterialfv(GL_FRONT, GL_SPECULAR, color);

		glMatrixMode(GL_MODELVIEW);
		color[0]=1; color[1]=0.5f; color[2]=0; // o
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
		glPushMatrix();
			glutSolidSphere(std::max(std::max(xlen,ylen),zlen)/40, 50, 50);
		glPopMatrix();
		color[0]=1; color[1]=0; color[2]=0; // x
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
		glPushMatrix(); glTranslatef(xlen/2, 0, 0); glScalef(1.0f, 0.01f, 0.01f);
			glutSolidCube(xlen);
		glPopMatrix();
		glPushMatrix(); glTranslatef(xlen, 0, 0); glRotatef(90, 0, 1, 0);
			glutSolidCone(xlen/30, xlen/8, 50, 50);
		glPopMatrix();
		color[0]=0; color[1]=1; color[2]=0; // y
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
		glPushMatrix(); glTranslatef(0, ylen/2, 0); glScalef(0.01f, 1.0f, 0.01f);
			glutSolidCube(ylen);
		glPopMatrix();
		glPushMatrix(); glTranslatef(0, ylen, 0); glRotatef(-90, 1, 0, 0);
			glutSolidCone(ylen/30, ylen/8, 50, 50);
		glPopMatrix();
		color[0]=0; color[1]=0; color[2]=1; // z
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
		glPushMatrix(); glTranslatef(0, 0, zlen/2); glScalef(0.01f, 0.01f, 1.0f);
			glutSolidCube(zlen);
		glPopMatrix();
		glPushMatrix(); glTranslatef(0, 0, zlen);
			glutSolidCone(zlen/30, zlen/8, 50, 50);
		glPopMatrix();
		color[0]=1; color[1]=0; color[2]=1; // unit
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
		glPushMatrix(); glTranslatef(1,0,0); glutSolidCube(xlen/50); glPopMatrix();
		glPushMatrix(); glTranslatef(0,1,0); glutSolidCube(ylen/50); glPopMatrix();
		glPushMatrix(); glTranslatef(0,0,1); glutSolidCube(zlen/50); glPopMatrix();

		glMaterialfv(GL_FRONT, GL_SPECULAR, color_gls);
		glMaterialfv(GL_FRONT, GL_AMBIENT, color_gla);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, color_gld);
	}else{ // xyz frame as lines
		GLfloat psize; glGetFloatv(GL_POINT_SIZE, &psize);
		GLfloat lsize; glGetFloatv(GL_LINE_WIDTH, &lsize);
		GLfloat cgl[4]; glGetFloatv(GL_CURRENT_COLOR, cgl);
		GLboolean light = glIsEnabled(GL_LIGHTING);
		if(GL_TRUE==light) glDisable(GL_LIGHTING);
		glPointSize(8); glLineWidth(2);
		GLfloat color[]={0, 0, 0, 1};
		glBegin(GL_LINES);
			color[0]=1; color[1]=0; color[2]=0; // x
			glColor3fv(color);
			glVertex3f(0, 0, 0);
			glVertex3f(xlen, 0, 0);
			color[0]=0; color[1]=1; color[2]=0; // y
			glColor3fv(color);
			glVertex3f(0, 0, 0);
			glVertex3f(0, ylen, 0);
			color[0]=0; color[1]=0; color[2]=1; // z
			glColor3fv(color);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, zlen);
		glEnd();
		glBegin(GL_POINTS);
			color[0]=1; color[1]=0.5f; color[2]=0; // o
			glColor3fv(color);
			glVertex3f(0,0,0);
		glEnd();
		glPointSize(psize); glLineWidth(lsize);
		glColor4fv(cgl);
		if(GL_TRUE==light) glEnable(GL_LIGHTING);
	}
}


/* hue:0-360; saturation:0-1; lightness:0-1
 * hue:        red(0) -> green(120) -> blue(240) -> red(360)
 * saturation: gray(0) -> perfect colorful(1)
 * lightness:  black(0) -> perfect colorful(0.5) -> white(1)
 * http://blog.csdn.net/idfaya/article/details/6770414
*/
void GlStaff::hsl_to_rgb( float h, float s, float l, float* rgb )
{
	if( s==0 ) { rgb[0] = rgb[1] = rgb[2] = l; return; }
	float q, p, hk, t[3];
	if( l<0.5f ) { q = l * ( 1 + s ); }
	else		 { q = l + s - l * s; }
	p = 2 * l - q;
	hk = h / 360;
	t[0] = hk + 1/3.0f;
	t[1] = hk;
	t[2] = hk - 1/3.0f;
	for( int i=0; i<3; ++i ) {
		if     ( t[i]<0 ) { t[i] += 1; }
		else if( t[i]>1 ) { t[i] -= 1; }
	}
	for( int i=0; i<3; ++i ) {
		if     ( t[i] < 1/6.0f ){ rgb[i] = p + (q-p)*6*t[i]; }
		else if( t[i] < 1/2.0f ){ rgb[i] = q; }
		else if( t[i] < 2/3.0f ){ rgb[i] = p + (q-p)*6*(2/3.0f-t[i]); }
		else					{ rgb[i] = p; }
	}
}

float GlStaff::rgb_to_gray( float r, float g, float b )
{
	return r*0.299f + g*0.587f + b*0.114f;
}

// only use the Red channel to format a gray image
GLuint GlStaff::load_tex(const char* file, bool only_red2alpha)
{
	static std::map<std::string, GLuint> s_files;
	// for the file occurred, don't create new textures
	if( s_files.find(std::string(file)) != s_files.end() ){
		return s_files.find(std::string(file))->second;
	}

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	cv::Mat img_rgb;
	cv::Mat img_rgba;
	img_rgb = cv::imread(file, cv::IMREAD_UNCHANGED);
	if( img_rgb.empty() ){
		printf("load_tex(): load image \"%s\" failed!\n", file);
		return 0;
	}
	cv::Mat img2;
	cv::flip(img_rgb, img2, 0);
	img_rgb = img2;
	img_rgba.create(img_rgb.rows, img_rgb.cols, CV_32FC4);
	int channels = img_rgb.channels();

	if(channels>=4){
		if(only_red2alpha){
			for(int i=0; i<img_rgb.rows; ++i){
				for(int j=0; j<img_rgb.cols; ++j){
					img_rgba.at<cv::Vec4f>(i,j)[0] = img_rgba.at<cv::Vec4f>(i,j)[1] = img_rgba.at<cv::Vec4f>(i,j)[2] = 1;
					img_rgba.at<cv::Vec4f>(i,j)[3] = img_rgb.at<cv::Vec4b>(i,j)[0] / 255.0f;
				}
			}
		}else{
			for(int i=0; i<img_rgb.rows; ++i){
				for(int j=0; j<img_rgb.cols; ++j){
					img_rgba.at<cv::Vec4f>(i,j)[0] = img_rgb.at<cv::Vec4b>(i,j)[0] / 255.0f;
					img_rgba.at<cv::Vec4f>(i,j)[1] = img_rgb.at<cv::Vec4b>(i,j)[1] / 255.0f;
					img_rgba.at<cv::Vec4f>(i,j)[2] = img_rgb.at<cv::Vec4b>(i,j)[2] / 255.0f;
					img_rgba.at<cv::Vec4f>(i,j)[3] = img_rgb.at<cv::Vec4b>(i,j)[3] / 255.0f;
				}
			}
		}
	}else{
		if(only_red2alpha){
			for(int i=0; i<img_rgb.rows; ++i){
				for(int j=0; j<img_rgb.cols; ++j){
					img_rgba.at<cv::Vec4f>(i,j)[0] = img_rgba.at<cv::Vec4f>(i,j)[1] = img_rgba.at<cv::Vec4f>(i,j)[2] = 1;
					img_rgba.at<cv::Vec4f>(i,j)[3] = img_rgb.at<cv::Vec3b>(i,j)[0] / 255.0f;
				}
			}
		}else{
			for(int i=0; i<img_rgb.rows; ++i){
				for(int j=0; j<img_rgb.cols; ++j){
					img_rgba.at<cv::Vec4f>(i,j)[0] = img_rgb.at<cv::Vec3b>(i,j)[0] / 255.0f;
					img_rgba.at<cv::Vec4f>(i,j)[1] = img_rgb.at<cv::Vec3b>(i,j)[1] / 255.0f;
					img_rgba.at<cv::Vec4f>(i,j)[2] = img_rgb.at<cv::Vec3b>(i,j)[2] / 255.0f;
					img_rgba.at<cv::Vec4f>(i,j)[3] = 1;
				}
			}
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_rgba.cols, img_rgba.rows, 0, GL_BGRA, GL_FLOAT, img_rgba.data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBindTexture(GL_TEXTURE_2D, 0); // release bind

	s_files[std::string(file)] = tex;
	return tex;
}

int GlStaff::load_tex_from_video(const char* file, std::vector<GLuint>& textures, bool only_red2alpha)
{
	static std::map<std::string, std::vector<GLuint>> s_files;
	// for the file occurred, don't create new textures
	if( s_files.find(std::string(file)) != s_files.end() ){
		textures = s_files.find(std::string(file))->second;
		return textures.size();
	}

	cv::VideoCapture video;
	video.open(file);
	if(!video.isOpened()){
		printf("load_tex_from_video(\"%s\") failed!\n", file);
		return 0;
	}
	textures.clear();
	cv::Mat frame;
	cv::Mat img_rgba(frame.rows, frame.cols, CV_8UC4);
	while( video.read(frame) ){
		cv::Mat img2;
		cv::flip(frame, img2, 0);
		frame = img2;

		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		if(only_red2alpha){
			for(int i=0; i<frame.rows; ++i){
				for(int j=0; j<frame.cols; ++j){
					img_rgba.at<cv::Vec4b>(i,j)[0] = img_rgba.at<cv::Vec4b>(i,j)[1] = img_rgba.at<cv::Vec4b>(i,j)[2] = 255u;
					img_rgba.at<cv::Vec4b>(i,j)[3] = frame.at<cv::Vec3b>(i,j)[0];
				}
			}
		}else{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, frame.cols, frame.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, frame.data);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, 0); // release bind

		textures.push_back(tex);
	}

	s_files[std::string(file)] = textures;
	return textures.size();
}

