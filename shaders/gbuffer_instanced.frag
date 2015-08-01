#version 330 core

smooth in vec3 pass_Normal;
flat in vec3 pass_Color;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;

void main(void){
	outNormal = normalize(pass_Normal);
	outColor = pass_Color;
}
