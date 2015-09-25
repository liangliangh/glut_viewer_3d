// fragment shader
#version 440 core

uniform mat4   mat_projection = mat4(1);
uniform mat4   mat_modelview = mat4(1);
uniform mat4   mat_transformation = mat4(1); // projection and model-view

uniform vec4   tri_color = vec4(1,1,1, 1);
uniform sampler2D texture0;

in vec2 tex_coord; // name based matching

//layout(location=0) out vec4 frag_color;

// Order-Independent Transparency
layout(r32ui)    uniform uimage2DRect head_pointer; // value 0 reserved for end of list
layout(rgba32ui) uniform uimageBuffer fragment_list; // (next, rgba4x8, depth, nouse)
uniform int  oi_transp_list_max_n = 0; // sizeof fragment_list
layout(binding=0, offset=0) uniform atomic_uint index_counter; // init to 1, 0 for NULL next

layout(early_fragment_tests) in;

void main()
{
	vec4 t = texture(texture0, tex_coord);
	if(t.a==0)
		discard;
	vec4 frag_color = vec4(tri_color.rgb, t.a*tri_color.a);

	uint index = atomicCounterIncrement(index_counter);
	if(index < oi_transp_list_max_n){
		uint next = imageAtomicExchange(head_pointer, ivec2(gl_FragCoord.xy), index);
		uvec4 item;
		item.x = next;
		item.y = packUnorm4x8( frag_color );
		item.z = floatBitsToUint(gl_FragCoord.z);
		item.w = 0;
		imageStore(fragment_list, int(index), item);
	}

	discard;
}
