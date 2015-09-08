
// heliangliang_ustb@126.com

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <assert.h>
#include <vector>
#include <map>
#include "gl_staff.h"


static void initGL();
static void cb_display();
static void cb_reshape(int w, int h);
static void cb_keyboard(unsigned char key, int x, int y);
static void cb_specialkey(int key, int x, int y);
static void cb_mouseclick(int button, int state, int x, int y);
static void cb_mousemotion(int x, int y);
static void grab(float* theta, glm::vec3* normal, float ax, float ay, float bx, float by, float z);
static void trackball(float* theta, glm::vec3* normal, float ax, float ay, float bx, float by, float r);
static void trackball_draw(float transparency);


// state variables
static struct State {
	// window
	int win_w, win_h;
	int screen_w, screen_h;

	// transformation
	glm::mat4 mat_modelview = glm::lookAt(glm::vec3(0,0,100), glm::vec3(0), glm::vec3(0,1,0));
	glm::mat4 mat_projection;
	bool  proj_orth = false; // true for orthogonal projection, false for perspective projection
	float frustum_fovy = glm::radians(30.0f);
	int   mouse_key_pressed = -1;
	float xpos, ypos;
	float trackball_r = 0.4f; // radius of track ball relative to min(WIN_H, WIN_W), should less than 0.5
	float trackball_center_z = -100; // center of trackball, i.e., in camera coor
	bool  camera_coor_marker = false;
	bool  draw_fps = true;

	// draw function
	void (*draw_func)() = NULL;

	// user key functions
	std::map<int,void(*)()> key_funcs;

	// functions
	void save_mouse_pos(float x, float y)
	{
		xpos = x;
		ypos = y;
	}
	void rotate_grab(float x, float y)
	{
		if(x==xpos && y==ypos) return;
		float s = -trackball_center_z*tan(frustum_fovy/2) / (win_h/2.0f);
		float theta; glm::vec3 n;
		grab( &theta, &n,
			(xpos-win_w/2.0f)*s, (ypos-win_h/2.0f)*s, (x-win_w/2.0f)*s, (y-win_h/2.0f)*s,
			trackball_center_z);
		mat_modelview = glm::rotate(theta, n) * mat_modelview;
	}
	void rotate_trackball_win(float x, float y) // O at left-bottom of window, to left is X+, up is Y+
	{
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
	void translate_win(float x, float y)
	{
		if(x==xpos && y==ypos) return;
		float dx = x-xpos, dy = y-ypos;
		float scale = tan(frustum_fovy/2)*(-trackball_center_z) / (win_h/2);
		mat_modelview = glm::translate( glm::vec3(scale*dx, scale*dy, 0) ) * mat_modelview;
	}
	void scale(float s) // s=1, not change, >1 bigger, <1 smaller
	{
		if(s==1) return;
		float z = trackball_center_z;
		trackball_center_z *= s;
		win_size(win_w, win_h); // for proj_orth
		mat_modelview = glm::translate( glm::vec3(0, 0, trackball_center_z-z) ) * mat_modelview;
	}
	void win_size(int w, int h)
	{
		win_w = w;
		win_h = h;
		if(proj_orth){
			float hh = -trackball_center_z * tan(frustum_fovy/2) * 2;
			float ww = hh * w / h;
			mat_projection = glm::ortho(-ww/2, ww/2, -hh/2, hh/2, trackball_center_z*2, -trackball_center_z*4);
		}else{
			mat_projection = glm::perspective(
				frustum_fovy, float(w)/h, -trackball_center_z*0.1f, -trackball_center_z*5);
		}
	}
	bool toggle_orth()
	{
		proj_orth = !proj_orth;
		win_size(win_w, win_h);
		return proj_orth;
	}
	void load_gl_matrix()
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(&mat_projection[0][0]);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(&mat_modelview[0][0]);
	}

} state;


void GlStaff::init(int win_width, int win_height, const char* win_tile)
{
	int argc = 0;
	glutInit(&argc, NULL);
	state.screen_w = glutGet(GLUT_SCREEN_WIDTH);
	state.screen_h = glutGet(GLUT_SCREEN_HEIGHT);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE);
	glutInitWindowPosition(std::max((state.screen_w-win_width)/2,0), std::max((state.screen_h-win_height)/2,0)); // set window to center of screen
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
}

void GlStaff::add_key_func( int key, void (*f)() )
{
	assert(f);

	state.key_funcs[key] = f;
}

void GlStaff::display_loop( void (*draw)() )
{
	assert(draw);

	state.draw_func = draw;
	glutMainLoop();
}

bool GlStaff::toggle_proj_orth()
{
	return state.toggle_orth();
}

static void cb_display()
{
	// matrix
	state.load_gl_matrix();

	// call user's draw()
	state.draw_func();

	// fps and state
	if(state.mouse_key_pressed==-1)
		trackball_draw(0.3f);
	else
		trackball_draw(0.5f);

	glutSwapBuffers();
}

static void cb_reshape(int w, int h)
{
	state.win_size(w, h);
	glViewport(0, 0, w, h);
}

static void cb_keyboard(unsigned char key, int x, int y)
{
	switch(key){
	case 27: // esc
		exit(0);
		break;
	default:
		if(state.key_funcs.find(key)!=state.key_funcs.end()){
			if(state.key_funcs[key])
				(*state.key_funcs[key])();
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
		state.mouse_key_pressed = -1;
		return;
	}

	switch(button) {
	case GLUT_LEFT_BUTTON:
	case GLUT_RIGHT_BUTTON:
	case GLUT_MIDDLE_BUTTON:
		state.mouse_key_pressed = button;
		break;
	case GLUT_WHEEL_UP:
		state.scale(1.05f);
		break;
	case GLUT_WHEEL_DOWN:
		state.scale(0.95f);
		break;
	}

	state.save_mouse_pos(x, state.win_h - y);
}

static void cb_mousemotion(int x, int y)
{
	if(state.mouse_key_pressed==GLUT_LEFT_BUTTON)
	{ // mouse left, trackball
		state.rotate_trackball_win(x, state.win_h - y);
	}
	else if(state.mouse_key_pressed==GLUT_RIGHT_BUTTON)
	{ // mouse right, change camera direction
		state.rotate_grab(x, state.win_h - y);
	}
	else if(state.mouse_key_pressed==GLUT_MIDDLE_BUTTON)
	{ // mouse middle, translate
		state.translate_win(x, state.win_h - y);
	}

	state.save_mouse_pos(x, state.win_h - y);
}

static void grab(float* theta, glm::vec3* normal, float ax, float ay, float bx, float by, float z)
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

static void trackball(float* theta, glm::vec3* normal, float ax, float ay, float bx, float by, float r)
{
	float az, bz, a2=ax*ax+ay*ay, b2=bx*bx+by*by, r2=r*r;
	if(a2 <= r2/2)
		az = sqrt( r2-a2 ); // https://www.opengl.org/wiki/Object_Mouse_Trackball
	else
		az = r2 / 2 / sqrt( a2 );
	if(b2 <= r2/2)
		bz = sqrt( r2-b2 );
	else
		bz = r2 / 2 / sqrt( b2 );
	glm::vec3 a = glm::normalize( glm::vec3(ax,ay,az) );
	glm::vec3 b = glm::normalize( glm::vec3(bx,by,bz) );
	float d = glm::dot(a,b);
	if( d <= -1 )   *theta = std::acos(-1 ); // acoss(a<-1 || a>1) return NaN
	else if(d >= 1) *theta = std::acos( 1 );
	else            *theta = std::acos( d );
	glm::vec3 c = glm::cross( a, b );
	*normal = glm::length2(c)==0 ? glm::vec3(0,1,0) : c;
}

static void trackball_draw(float transparency)
{
	static std::vector<float> circle_r1_xy;
	if( circle_r1_xy.size()==0 ){
		const int n = 100;
		for(int i=0; i<n; ++i){
			float radians = float(i)/n * 2 * 3.14159265f;
			circle_r1_xy.push_back( cos(radians) );
			circle_r1_xy.push_back( sin(radians) );
		}
		circle_r1_xy.push_back( 1 );
		circle_r1_xy.push_back( 0 );
	}

	float r = abs(state.trackball_center_z) * tan( state.frustum_fovy/2 ) * state.trackball_r * 2;
	glm::vec3 center = glm::vec3(
			glm::affineInverse(state.mat_modelview) * glm::vec4(0, 0, state.trackball_center_z, 1) );

	GLboolean gl_li = glIsEnabled(GL_LIGHTING);
	GLboolean gl_dp = glIsEnabled(GL_DEPTH_TEST);
	GLboolean gl_bd = glIsEnabled(GL_BLEND);
	GLfloat   gl_lw; glGetFloatv(GL_LINE_WIDTH, &gl_lw);
	GLfloat gl_cl[4]; glGetFloatv(GL_CURRENT_COLOR, gl_cl);

	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(2);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(center.x, center.y, center.z);
	glScalef(r, r, r);
	// x
	glColor4f(1, 0.5f, 0.5f, transparency);
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<circle_r1_xy.size(); i+=2){
		glVertex3f(0, circle_r1_xy[i], circle_r1_xy[i+1]);
	}
	glEnd();
	// y
	glColor4f(0.5f, 1, 0.5f, transparency);
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<circle_r1_xy.size(); i+=2){
		glVertex3f(circle_r1_xy[i], 0, circle_r1_xy[i+1]);
	}
	glEnd();
	// z
	glColor4f(0.5f, 0.5f, 1, transparency);
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<circle_r1_xy.size(); i+=2){
		glVertex3f(circle_r1_xy[i], circle_r1_xy[i+1], 0);
	}
	glEnd();
	glPopMatrix();

	if( gl_li ) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);
	if( gl_dp ) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
	if( gl_bd ) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	glLineWidth(gl_lw);
	glColor3fv(gl_cl);
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

