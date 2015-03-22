#version 330 core

layout(location = 0) in vec3 in_Position;
layout(location = 3) in mat4 MVPMatrix;

void main(void){
	gl_Position = MVPMatrix * vec4(in_Position, 1.0);
}
