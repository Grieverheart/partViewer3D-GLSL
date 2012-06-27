#version 330 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec4 in_Color;

flat out vec4 pass_Color;

void main(void){
	pass_Color = in_Color;
	gl_Position = vec4(in_Position, 1.0);
}