#version 330 core

uniform mat4 MVPMatrix;

layout(location = 0) in vec3 in_Position;
layout(location = 3) in mat4 ModelMatrix;

void main(void){
    gl_ClipDistance[0] = dot(vec4(0.0, 0.0, -1.0, 0.0), ModelMatrix * vec4(in_Position, 1.0));
	gl_Position = MVPMatrix * ModelMatrix * vec4(in_Position, 1.0);
}
