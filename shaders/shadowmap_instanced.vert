#version 330 core

uniform mat4 MVPMatrix;

layout(location = 0) in vec3 in_Position;
layout(location = 2) in mat4 ModelMatrix;

smooth out vec2 depth;

void main(void){
	gl_Position = MVPMatrix * ModelMatrix * vec4(in_Position, 1.0);
    depth = gl_Position.zw;
}
