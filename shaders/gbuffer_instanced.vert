#version 330 core

uniform float scale;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 3) in mat4 MVPMatrix;
layout(location = 7) in mat3 NormalMatrix;

smooth out vec3 pass_Normal;
// flat out uint instanceID;

void main(void){

	pass_Normal = NormalMatrix * in_Normal; 
	// instanceID = uint(gl_InstanceID);
	
	gl_Position = MVPMatrix * vec4(scale * in_Position, 1.0);
}