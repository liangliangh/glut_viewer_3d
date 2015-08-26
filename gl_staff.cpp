
// heliangliang_ustb@126.com

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <vector>
#include <map>
#include "gl_staff.h"


namespace GlStaff {


// state variables
static struct State {
	// window
	int win_w, win_h;
	int screen_w, screen_h;

	// trackball
	glm::mat4 mat_modelview = glm::lookAt(glm::vec3(0,0,100), glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 mat_projection;
	bool  proj_orth = false; // true for orthogonal projection, false for perspective projection
	float frustum_fovy = glm::radians(30.0);
	float clip_n = 0.1f, clip_f = 1000;
	int   mouse_key_pressed = -1;
	float xpos, ypos;
	float trackball_r = 0.4f; // radius of track ball relative to min(WIN_H, WIN_W), should less than 0.5
	float trackball_center_z = -100; // center of trackball, i.e., in camera coor
	bool  camera_coor_marker = false;

	// draw function
	void (*draw_func)() = NULL;

	// user key functions
	std::map<int,void(*)()> key_funcs;

} state;



void initGL()
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

void cb_display();
void cb_reshape(int w, int h);
void cb_keyboard(unsigned char key, int x, int y);
void cb_specialkey(int key, int x, int y);
void cb_mouseclick(int button, int state, int x, int y);
void cb_mousemotion(int x, int y);

void init(int win_width, int win_height, const char* win_tile)
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

void add_key_func( int key, void (*f)() )
{
	assert(f);

	state.key_funcs[key] = f;
}

void display_loop( void (*draw)() )
{
	assert(draw);

	state.draw_func = draw;
	glutMainLoop();
}

void trackball_draw(float transparency);

void cb_display()
{
	// matrix
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&state.mat_projection[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&state.mat_modelview[0][0]);

	// call user's draw()
	state.draw_func();

	// fps and state
	if(state.mouse_key_pressed==-1)
		;//trackball_draw(0.2f);
	else
		trackball_draw(0.5f);

	glutSwapBuffers();
}

void cb_reshape(int w, int h)
{
	state.win_w = w;
	state.win_h = h;
	if(state.proj_orth){
		float hh = -state.trackball_center_z * tan(state.frustum_fovy/2) * 2;
		float ww = hh * w / h;
		state.mat_projection = glm::ortho(-ww/2, ww/2, -hh/2, hh/2, 0.0f, state.trackball_center_z);
	}else{
		state.mat_projection = glm::perspective(state.frustum_fovy, float(w)/h, state.clip_n, state.clip_f);
	}
	glViewport(0, 0, w, h);
}

void cb_keyboard(unsigned char key, int x, int y)
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

void cb_specialkey(int key, int x, int y)
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

void cb_mouseclick(int button, int bstate, int x, int y)
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

			break;
		case GLUT_WHEEL_DOWN:

			break;
	}

	state.xpos = x;
	state.ypos = state.win_h - y;
}

void trackball(float* theta, glm::vec3* normal, float ax, float ay, float bx, float by, float r);

void cb_mousemotion(int x, int y)
{
	float xpos = x, ypos = state.win_h - y;
	float dx = xpos-state.xpos, dy = ypos-state.ypos;

	if(dx==0 && dy==0) return;

	if(state.mouse_key_pressed==GLUT_LEFT_BUTTON){ // mouse left, trackball

		float theta; glm::vec3 n;
		trackball( &theta, &n,
			state.xpos-state.win_w/2.0f, state.ypos-state.win_h/2.0f,
			xpos-state.win_w/2.0f, ypos-state.win_h/2.0f,
			std::min(state.win_w,state.win_h)*state.trackball_r );

		glm::mat4 inv_modelview = glm::affineInverse(state.mat_modelview);
		glm::vec3 normal = glm::vec3( inv_modelview * glm::vec4(n, 0) );
		glm::vec3 center = glm::vec3( inv_modelview * glm::vec4(0, 0, state.trackball_center_z, 1) );
		state.mat_modelview *= glm::translate(center) * glm::rotate(theta, normal) * glm::translate(-center);

	}else if(state.mouse_key_pressed==GLUT_RIGHT_BUTTON){ // mouse right, change camera direction

	}else if(state.mouse_key_pressed==GLUT_MIDDLE_BUTTON){ // mouse middle, translate

		float scale = tan(state.frustum_fovy/2)*(-state.trackball_center_z) / (state.win_h/2);
		state.mat_modelview = glm::translate( glm::vec3(scale*dx, scale*dy, 0) ) * state.mat_modelview;

	}

	state.xpos = xpos;
	state.ypos = ypos;
}

void trackball(float* theta, glm::vec3* normal, float ax, float ay, float bx, float by, float r)
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
	*theta = std::acos( glm::dot(a,b) );
	*normal = glm::cross( a, b );
}


void trackball_draw(float transparency)
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
	glColor4f(1, 0, 0, transparency);
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<circle_r1_xy.size(); i+=2){
		glVertex3f(0, circle_r1_xy[i], circle_r1_xy[i+1]);
	}
	glEnd();
	// y
	glColor4f(0, 1, 0, transparency);
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<circle_r1_xy.size(); i+=2){
		glVertex3f(circle_r1_xy[i], 0, circle_r1_xy[i+1]);
	}
	glEnd();
	// z
	glColor4f(0, 0, 1, transparency);
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

void xyz_frame(float xlen, float ylen, float zlen, bool solid)
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
void hsl_to_rgb( float h, float s, float l, float* rgb )
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

float rgb_to_gray( float r, float g, float b )
{
	return r*0.299f + g*0.587f + b*0.114f;
}

} // namespace GlStaff
