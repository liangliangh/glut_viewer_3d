// fragment shader
#version 440 core

uniform mat4   mat_projection = mat4(1);
uniform mat4   mat_modelview = mat4(1);
uniform mat4   mat_transformation = mat4(1); // projection and model-view

uniform vec4   tri_color = vec4(1,1,1, 1);
uniform sampler2D texture0;

in vec2 tex_coord; // name based matching

layout(location=0) out vec4 frag_color;

layout(early_fragment_tests) in;

void main()
{
	vec4 t = texture(texture0, tex_coord);
	if(t.a==0)
		discard;
	frag_color = vec4(tri_color.rgb, t.a*tri_color.a);
}
