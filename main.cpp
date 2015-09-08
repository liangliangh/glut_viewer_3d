
#include "gl_staff.h"

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GlStaff::xyz_frame( 25, 25, 25, false );
	//glutSolidTeapot(10);
	glutSolidCube(20);

	glutPostRedisplay();
}

void key_a()
{
	static int count=0;
	printf("key a pressed %d.\n", count++);
	printf("acos(1): %f\n", acos(-1.00000f));
	printf("%d\n", GlStaff::toggle_proj_orth());
}

int main()
{
	GlStaff::init(800, 600, "RGB-D viewer");
	GlStaff::add_key_func('a', key_a);

	GlStaff::display_loop(draw);

	return 0;
}
