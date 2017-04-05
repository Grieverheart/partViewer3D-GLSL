static const char color_frag[] = R"(
#version 330 core

uniform vec3 color;
layout(location = 0) out vec3 outColor;

void main(void){
	outColor = color;
}
)";
static const char color_vert[] = R"(
#version 330 core

uniform mat4 mvp_matrix;

layout(location = 0) in vec3 in_Position;

void main(void){
	gl_Position = mvp_matrix * vec4(in_Position, 1.0);
}
)";
