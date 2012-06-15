#version 330 core

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVPMatrix;

uniform float scale;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

smooth out vec3 pass_Normal;
smooth out vec3 pass_Position;

const mat4 TexMatrix = mat4(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
);

void main(void){


	pass_Normal = NormalMatrix * in_Normal; 
	pass_Position = (ModelViewMatrix * vec4(scale * in_Position, 1.0)).xyz;
	
	gl_Position = MVPMatrix * vec4(scale * in_Position, 1.0);
}