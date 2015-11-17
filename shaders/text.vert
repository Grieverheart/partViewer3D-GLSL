#version 330 core

uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

layout(location = 0) in vec3 in_Position;

smooth out vec2 TexCoord;

vec2 TexCoords[] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0)
);

void main(void){
    TexCoord = TexCoords[gl_VertexID];
    //TexCoord.y = 1.0 - TexCoord.y;
	gl_Position = projectionMatrix * modelMatrix * vec4(in_Position + vec3(TexCoords[gl_VertexID], 0.0), 1.0);
}
