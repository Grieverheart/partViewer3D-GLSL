#version 330 core

uniform vec3 diffColor;

smooth in vec3 pass_Normal;
// flat in uint instanceID;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;
// layout(location = 2) out uint outID;

void main(void){
	outNormal = normalize(pass_Normal);
	outColor = diffColor;
	// outID = instanceID;
}