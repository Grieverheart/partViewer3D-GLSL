#version 330 core

uniform mat3 NormalMatrix;
uniform mat4 MVPMatrix;

uniform float scale;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

smooth out vec3 pass_Normal;

void main(void){

	pass_Normal = NormalMatrix * in_Normal; 
	
	gl_Position = MVPMatrix * vec4(scale * in_Position, 1.0);
}