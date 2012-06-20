#version 330 core

layout(location = 0) in vec3 in_Position;
layout(location = 2) in vec2 in_TexCoord;

noperspective out vec2 TexCoord;

void main(void){

	TexCoord = in_TexCoord;
	
	gl_Position = vec4(in_Position, 1.0);
}