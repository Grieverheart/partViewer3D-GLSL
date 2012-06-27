#version 330 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 TexCoord;

smooth out vec2 pass_TexCoord;

void main(void){
	pass_TexCoord = TexCoord;
	gl_Position = vec4(in_Position, 1.0);
}