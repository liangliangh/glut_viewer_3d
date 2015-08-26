
#include "gl_staff.h"

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GlStaff::xyz_frame( 25, 25, 25, false );
	glutSolidTeapot(10);

	glutPostRedisplay();
}


int main()
{
	GlStaff::init(800, 600, "RGB-D viewer");

	GlStaff::display_loop(draw);

	return 0;
}






