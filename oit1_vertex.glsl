// vertex shader
#version 440 core

uniform mat4   mat_projection = mat4(1);
uniform mat4   mat_modelview = mat4(1);
uniform mat4   mat_transformation = mat4(1); // projection and model-view

layout(location=0) in vec3  position;
layout(location=1) in vec2  tex_coord_in;

out vec2 tex_coord; // name based matching

void main()
{
	gl_Position = mat_transformation * vec4(position, 1);
	tex_coord = tex_coord_in;
}
