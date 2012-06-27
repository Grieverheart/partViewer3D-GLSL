#version 330 core

uniform sampler2D g_Sampler;
smooth in vec2 pass_TexCoord;

out vec4 outColor;

void main(void){
	outColor = texture(g_Sampler, pass_TexCoord);
}