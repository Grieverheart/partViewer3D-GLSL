#version 330 core

uniform mat4 mvp_matrix;

layout(location = 0) in vec3 in_Position;

void main(void){
	gl_Position = mvp_matrix * vec4(in_Position, 1.0);
}
