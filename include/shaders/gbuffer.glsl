static const char gbuffer_frag[] = R"(
#version 330 core

uniform vec3 diffColor;

smooth in vec3 pass_Normal;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;

void main(void){
	outNormal = normalize(pass_Normal);
	outColor = diffColor;
}
)";
static const char gbuffer_vert[] = R"(
#version 330 core

uniform mat3 NormalMatrix;
uniform mat4 MVPMatrix;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

smooth out vec3 pass_Normal;

void main(void){

	pass_Normal = NormalMatrix * in_Normal; 
	
	gl_Position = MVPMatrix * vec4(in_Position, 1.0);
}
)";
