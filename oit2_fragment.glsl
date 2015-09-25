// fragment shader
#version 440 core

layout(r32ui)    uniform uimage2DRect head_pointer; // value 0 reserved for end of list
layout(rgba32ui) uniform uimageBuffer fragment_list; // (next, rgba4x8, depth, nouse)

uniform sampler2D texture0; // the origin rgba

uniform ivec2  resolution;
const int MAX_N = 20;

layout(location=0) out vec4 frag_color;

void main()
{
	ivec2 invocationid = ivec2(gl_FragCoord.xy);
	if(invocationid.x<resolution.x && invocationid.y<resolution.y){
		vec3 rgb = texelFetch(texture0, invocationid, 0).rgb;
		uint idx; int N1, N2;
		N1=0; while( (idx=imageLoad(head_pointer,invocationid).r) != 0 ){ if(++N1>MAX_N) break;
			uint maxz=0, maxy=0, maxx=0, prex=0, pre=0;
			N2=0; while(idx != 0){ if(++N2>MAX_N) break;
				uvec4 item = imageLoad(fragment_list, int(idx));
				if(item.z>maxz) { maxz = item.z; maxy = item.y; maxx = item.x; prex = pre; }
				pre = idx;
				idx = item.x;
			}
			// delete the list node
			if(prex==0){
				imageStore(head_pointer, invocationid, uvec4(maxx,0,0,0));
			}else{
				uvec4 item = imageLoad(fragment_list, int(prex));
				item.x = maxx;
				imageStore(fragment_list, int(prex), item);
			}
			// blend the color
			vec4 rgb_new = unpackUnorm4x8(maxy);
			rgb = mix(rgb, rgb_new.rgb, rgb_new.a);
		}
		frag_color = vec4(rgb, 1);
		//frag_color = texelFetch(texture0, invocationid, 0);
		//frag_color = vec4(float(N1)/20, 0,0, 1);
	}

	
}
