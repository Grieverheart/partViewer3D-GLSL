#version 330 core

uniform mat4 projection_matrix;
uniform mat4 mv_matrix;
uniform float scale;

layout(location = 0) in vec3 in_Position;

flat out vec2 depth;
smooth out vec2 TexCoord;

vec2 TexCoords[] = vec2[](
    vec2(0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0)
);

void main(void){

	gl_Position = mv_matrix * vec4(in_Position, 1.0);
    gl_Position += scale * vec4(TexCoords[gl_VertexID] - vec2(0.5), vec2(0.0));
	gl_Position = projection_matrix * gl_Position;

    TexCoord = TexCoords[gl_VertexID];

    depth = gl_Position.zw;
}
