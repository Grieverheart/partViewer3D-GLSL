#version 330 core

uniform mat4 ProjectionMatrix;
uniform mat4 MVMatrix;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 3) in mat4 ModelMatrix;

smooth out vec3 pass_Normal;

void main(void){

	pass_Normal = mat3(transpose(inverse(MVMatrix * ModelMatrix))) * in_Normal; 
	
	gl_Position = ProjectionMatrix * MVMatrix * ModelMatrix * vec4(in_Position, 1.0);
}
