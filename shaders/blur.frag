#version 330 core

uniform sampler2D aoSampler;
uniform vec2 TEXEL_SIZE; // x = 1/res x, y = 1/res y
uniform bool use_blur;

noperspective in vec2 TexCoord;

layout(location = 0) out vec4 out_AO;

void main(void){
	if(use_blur){
		float result = 0.0;
		for(int i = -1; i < 2; i++){
			for(int j = -1; j < 2; j++){
				vec2 offset = vec2(TEXEL_SIZE.x * i, TEXEL_SIZE.y * j);
				result += texture(aoSampler, TexCoord + offset).r; // -0.004 because the texture seems to be a bit displaced
			}
		}
		
		out_AO = vec4(vec3(0.0), result / 9);
	}
	else out_AO = vec4(vec3(0.0), texture(aoSampler, TexCoord).r);
}