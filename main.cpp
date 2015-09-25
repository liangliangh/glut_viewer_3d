
#include "gl_info.h"
#include "gl_staff.h"

#include <string.h>
#include <fstream>
#include <unistd.h>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"


void ShaderBegin(GLint* transformation, GLint* tri_color);
void ShaderEnd();
void OitBegin(GLint* transformation, GLint* tri_color);
void OitEnd();
void DrawVaoSquare();


void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	GlStaff::xyz_frame( 25, 25, 25, false );
	//glutSolidTeapot(10);
	glutSolidCube(12);


	GLint gl_uin_trans, gl_uin_tricolor;
	//ShaderBegin(&gl_uin_trans, &gl_uin_tricolor);
	OitBegin(&gl_uin_trans, &gl_uin_tricolor);

	glm::mat4 mat_modelview(1), mat_projection(1);
	glGetFloatv(GL_MODELVIEW_MATRIX, &mat_modelview[0][0]);
	glGetFloatv(GL_PROJECTION_MATRIX, &mat_projection[0][0]);
	for(int i=0; i<5; ++i){
		glm::mat4 mat = mat_projection * mat_modelview * glm::translate(glm::vec3(0,0,i*5-25)) * glm::scale(glm::vec3(20));
		glUniformMatrix4fv(gl_uin_trans, 1, GL_FALSE, &mat[0][0]);
		float rgba[4] = {0,0,0, 0.5f};
		GlStaff::hsl_to_rgb((i*23%20)*18, 1, 0.5f, rgba);
		glUniform4fv(gl_uin_tricolor, 1, rgba);
		DrawVaoSquare();
	}
//	for(int i=5; i>0; --i){
//		glm::mat4 mat = mat_projection * mat_modelview * glm::translate(glm::vec3(0,0,i*5)) * glm::scale(glm::vec3(20));
//		glUniformMatrix4fv(gl_uin_trans, 1, GL_FALSE, &mat[0][0]);
//		float rgba[4] = {0,0,0, 0.5f};
//		GlStaff::hsl_to_rgb((i*23%20)*18, 1, 0.5f, rgba);
//		glUniform4fv(gl_uin_tricolor, 1, rgba);
//		DrawVaoSquare();
//	}

	//ShaderEnd();
	OitEnd();


	glutPostRedisplay();
	usleep(1000 * 30);
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








void DrawVaoSquare()
{
	static GLuint gl_tex = 0;
	static GLuint gl_vao_square_ = 0;
	if(!gl_vao_square_){
		gl_tex = GlStaff::load_tex("ring_tex.png", true);

		GLuint buffer;
		float poscoor[] = {1,1,0, 1,1, -1,1,0, 0,1, -1,-1,0, 0,0, 1,-1,0, 1,0, };
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(poscoor), poscoor, GL_STATIC_DRAW);
		glGenVertexArrays(1, &gl_vao_square_);
		glBindVertexArray(gl_vao_square_);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, 0, 5*sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, 0, 5*sizeof(float), (void*)(3*sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0); // release binding
		glBindBuffer(GL_ARRAY_BUFFER, 0); // release binding
		glBindVertexArray(0);
	}

	glBindTexture(GL_TEXTURE_2D, gl_tex);
	glBindVertexArray(gl_vao_square_);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void ShaderBegin(GLint* transformation, GLint* tri_color)
{
	static GLuint gl_prog = 0;
	static GLint  gl_uin_trans;
	static GLint  gl_uin_tricolor;
	if(!gl_prog){
		printf("create shader.\n");
		gl_prog = setupGLPipeline("orig_vertex.glsl", "orig_fragment.glsl");
		glUseProgram(gl_prog);
		gl_uin_trans = glGetUniformLocation(gl_prog, "mat_transformation");
		gl_uin_tricolor = glGetUniformLocation(gl_prog, "tri_color");
		glUniform1i( glGetUniformLocation(gl_prog,"texture0"), 0 );
		glUseProgram(0);
	}

	*transformation = gl_uin_trans;
	*tri_color = gl_uin_tricolor;

	// draw using shader
	glUseProgram(gl_prog);
	glDepthMask(GL_FALSE);
}
void ShaderEnd()
{
	glDepthMask(GL_TRUE);
	glUseProgram(0);
}

static GLuint buff_atomic_counter_;
static GLuint gl_tex_frame_;
void OitBegin(GLint* transformation, GLint* tri_color)
{
	static GLuint gl_program = 0;
	static GLint gl_uni_mat_trans;
	static GLint gl_tri_color;
	static GLuint tex_list_head_;

	if(!gl_program){
		printf("OitBegin() create shader.\n");
		gl_program = setupGLPipeline("oit1_vertex.glsl", "oit1_fragment.glsl");
		glUseProgram(gl_program);
		gl_uni_mat_trans = glGetUniformLocation(gl_program, "mat_transformation");
		gl_tri_color = glGetUniformLocation(gl_program, "tri_color");
		glUniform1i( glGetUniformLocation(gl_program,"texture0"), 0 );

		glActiveTexture(GL_TEXTURE0 + 5);
		glGenTextures(1, &tex_list_head_);
		glBindTexture(GL_TEXTURE_RECTANGLE, tex_list_head_);
		//glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32UI, width_, height_, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
		// use int data=0; glClearTexImage(tex_list_head_, 0, GL_R32UI, GL_UNSIGNED_INT, &data); to init
		glBindImageTexture(0, tex_list_head_, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
		glUniform1i(glGetUniformLocation(gl_program, "head_pointer"), 0); // image unit 0, texture unit 5

		GLuint list_buffer;
		glGenBuffers(1, &list_buffer);
		glBindBuffer(GL_TEXTURE_BUFFER, list_buffer);
		glBufferData(GL_TEXTURE_BUFFER, sizeof(GLuint)*4 * 1920*1080*5, NULL, GL_STATIC_DRAW);
		int data=0; glClearBufferData(GL_TEXTURE_BUFFER, GL_RGBA32UI, GL_RGBA, GL_UNSIGNED_INT, &data);
		glUniform1i(glGetUniformLocation(gl_program, "oi_transp_list_max_n"), 1920*1080*5 );
		glBindBuffer(GL_TEXTURE_BUFFER, 0);
		GLuint list_tex;
		glActiveTexture(GL_TEXTURE0 + 6);
		glGenTextures(1, &list_tex);
		glBindTexture(GL_TEXTURE_BUFFER, list_tex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, list_buffer);
		glBindImageTexture(1, list_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32UI);
		glUniform1i(glGetUniformLocation(gl_program, "fragment_list"), 1); // image unit 1, texture unit 6

		glGenBuffers(1, &buff_atomic_counter_);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, buff_atomic_counter_);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, buff_atomic_counter_);
		// set, GLuint data=1; glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &data, GL_STATIC_DRAW);
		// get, GLuint data; glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &data);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

		glActiveTexture(GL_TEXTURE0 + 0);
		glUseProgram(0);

		glGenTextures(1, &gl_tex_frame_);
		glBindTexture(GL_TEXTURE_2D, gl_tex_frame_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		printGlError("OitBegin() create shader");
	}

	GLint vret[4]; glGetIntegerv(GL_VIEWPORT, vret);
	static int width_=0, height_=0;
	if( !(width_==vret[2] && height_==vret[3]) ){
		width_ = vret[2];
		height_ = vret[3];
		glActiveTexture(GL_TEXTURE0 + 5);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32UI, width_, height_, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
		glActiveTexture(GL_TEXTURE0 + 0);
	}
	glActiveTexture(GL_TEXTURE0 + 0);
	//GLfloat* pixdata = new GLfloat[width_*height_*4];
	cv::Mat pix(height_, width_, CV_32FC4);
	glReadPixels(vret[0], vret[1], width_, height_, GL_RGBA, GL_FLOAT, pix.data);//cv::imshow("a", pix);cv::waitKey(1);
	glBindTexture(GL_TEXTURE_2D, gl_tex_frame_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_FLOAT, pix.data);
	//delete[] pixdata;
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE0 + 5);
	GLuint data=0; glClearTexImage(tex_list_head_, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &data); // head pointer to 0
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, buff_atomic_counter_);
	data=1; glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &data, GL_STATIC_DRAW); // index counter to 1
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	// disable write to depth and color buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_TEST); // early test for depth

	glActiveTexture(GL_TEXTURE0 + 0);
	glUseProgram(gl_program);
	*transformation = gl_uni_mat_trans;
	*tri_color = gl_tri_color;

	printGlError("OitBegin()");
}

void OitEnd()
{
	GLuint c;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, buff_atomic_counter_);
	glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &c);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	printf("total fragments number: %d / %d\n", c, 1920*1080*5);

	static GLuint gl_program = 0;
	static GLuint gl_vao_square_ = 0;
	static GLint  gl_uni_res;
	if(!gl_program){
		// shader
		printf("OitEnd() create shader.\n");
		gl_program = setupGLPipeline("oit2_vertex.glsl", "oit2_fragment.glsl");
		glUseProgram(gl_program);
		glUniform1i( glGetUniformLocation(gl_program,"texture0"), 0 );
		glUniform1i(glGetUniformLocation(gl_program, "head_pointer"), 0);
		glUniform1i(glGetUniformLocation(gl_program, "fragment_list"), 1);
		gl_uni_res = glGetUniformLocation(gl_program, "resolution");
		glUseProgram(0);
		// vao
		GLuint vert_poscoor_buffer;
		float poscoor[] = {1,1,0, -1,1,0, -1,-1,0, 1,-1,0, };
		glGenBuffers(1, &vert_poscoor_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vert_poscoor_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(poscoor), poscoor, GL_STATIC_DRAW);

		glGenVertexArrays(1, &gl_vao_square_);
		glBindVertexArray(gl_vao_square_);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vert_poscoor_buffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0); // index 0(see the shader code): pos

		glBindBuffer(GL_ARRAY_BUFFER, 0); // release binding
		glBindVertexArray(0);

		printGlError("OitEnd() create shader");
	}

	// enable write to depth and color buffer
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glUseProgram(gl_program);
	GLint vret[4]; glGetIntegerv(GL_VIEWPORT, vret);
	int width_=vret[2], height_=vret[3];
	glUniform2i(gl_uni_res, width_, height_);

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, gl_tex_frame_);

	glBindVertexArray(gl_vao_square_);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	printGlError("OitEnd()");
}
