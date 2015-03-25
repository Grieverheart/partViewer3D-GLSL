#version 330 core

uniform sampler2D aoSampler;
uniform bool use_blur;

noperspective in vec2 TexCoord;

layout(location = 0) out vec4 out_AO;

void main(void){
	if(use_blur){
		float result = texture(aoSampler, TexCoord).r;
        result += textureOffset(aoSampler, TexCoord, ivec2(-1, -1)).r;
        result += textureOffset(aoSampler, TexCoord, ivec2(-1, 0)).r;
        result += textureOffset(aoSampler, TexCoord, ivec2(-1, 1)).r;
        result += textureOffset(aoSampler, TexCoord, ivec2(0, -1)).r;
        result += textureOffset(aoSampler, TexCoord, ivec2(0, 1)).r;
        result += textureOffset(aoSampler, TexCoord, ivec2(1, -1)).r;
        result += textureOffset(aoSampler, TexCoord, ivec2(1, 0)).r;
        result += textureOffset(aoSampler, TexCoord, ivec2(1, 1)).r;
		
		out_AO = vec4(vec3(0.0), result / 9);
	}
	else out_AO = vec4(vec3(0.0), texture(aoSampler, TexCoord).r);
}
