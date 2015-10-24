#version 330 core

uniform mat4 MVPMatrix;
uniform vec4 clip_plane;
uniform mat4 ModelMatrix;

layout(location = 0) in vec3 in_Position;

void main(void){
    gl_ClipDistance[0] = dot(clip_plane, ModelMatrix * vec4(in_Position, 1.0));
	gl_Position = MVPMatrix * ModelMatrix * vec4(in_Position, 1.0);
}
