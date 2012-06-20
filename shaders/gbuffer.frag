#version 330 core

uniform sampler2D inSampler;
uniform vec3 diffColor;

smooth in vec3 pass_Normal;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;

void main(void){
	outNormal = normalize(pass_Normal);
	outColor = diffColor;
}