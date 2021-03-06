static const char gbuffer_instanced_frag[] = R"(
#version 330 core

smooth in vec3 pass_Normal;
flat in vec3 pass_Color;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec4 outNormal;

void main(void){
	outNormal = vec4(normalize(pass_Normal), 0.0);
	outColor = pass_Color;
}
)";
static const char gbuffer_instanced_vert[] = R"(
#version 330 core

uniform mat4 ProjectionMatrix;
uniform mat4 MVMatrix;
uniform vec4 clip_plane;
uniform mat4 ModelMatrix;
uniform vec3 in_Color;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

smooth out vec3 pass_Normal;
flat out vec3 pass_Color;

void main(void){

	pass_Normal = mat3(transpose(inverse(MVMatrix * ModelMatrix))) * in_Normal; 
	pass_Color = in_Color;
	
    gl_ClipDistance[0] = dot(clip_plane, ModelMatrix * vec4(in_Position, 1.0));
	gl_Position = ProjectionMatrix * MVMatrix * ModelMatrix * vec4(in_Position, 1.0);
}
)";
