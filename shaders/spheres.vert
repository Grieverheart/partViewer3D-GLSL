#version 330 core

uniform mat4 ProjectionMatrix;
uniform mat4 MVMatrix;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in mat4 ModelMatrix;

//smooth out vec3 Position;
smooth out vec2 TexCoord;
//smooth out vec3 pass_Normal;

vec2 TexCoords[] = vec2[](
    vec2(0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0)
);

void main(void){

	//pass_Normal = mat3(transpose(inverse(MVMatrix * ModelMatrix))) * vec3(0.0, 0.0, 1.0); 
	gl_Position = MVMatrix * ModelMatrix * vec4(in_Position, 1.0);
    gl_Position += vec4(TexCoords[gl_VertexID] - vec2(0.5), vec2(0.0));
	gl_Position = ProjectionMatrix * gl_Position;

    TexCoord = TexCoords[gl_VertexID];
}
