#version 330 core

uniform mat4 ProjectionMatrix;
uniform mat4 MVMatrix;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Color;
layout(location = 2) in mat4 ModelMatrix;

smooth out vec2 TexCoord;
flat out vec2 depth;
flat out vec3 pass_Color;

vec2 TexCoords[] = vec2[](
    vec2(0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0)
);

void main(void){

	gl_Position = MVMatrix * ModelMatrix * vec4(in_Position, 1.0);
    gl_Position += vec4(TexCoords[gl_VertexID] - vec2(0.5), vec2(0.0));
	gl_Position = ProjectionMatrix * gl_Position;

    depth = gl_Position.zw;
    pass_Color = in_Color;

    TexCoord = TexCoords[gl_VertexID];
}
