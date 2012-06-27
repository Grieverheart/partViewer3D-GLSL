#version 330 core

flat in vec4 pass_Color;

out vec4 outColor;

void main(void){
	outColor = pass_Color;
}