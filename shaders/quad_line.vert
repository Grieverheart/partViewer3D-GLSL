#version 330 core

uniform mat4 MVPMatrix;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_Barycentric;

smooth out vec2 pass_Barycentric;

void main(void){

	pass_Barycentric = in_Barycentric;
	
	gl_Position = MVPMatrix * vec4(in_Position, 1.0);
}
