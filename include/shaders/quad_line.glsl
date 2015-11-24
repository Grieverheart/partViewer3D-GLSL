static const char quad_line_frag[] = R"(
#version 330 core

smooth in vec2 pass_Barycentric;
layout(location = 0) out vec4 outColor;

void main(void){
    vec2 abs_coord = abs(pass_Barycentric - vec2(0.5));
    float offset = step(0.5 - 0.04, max(abs_coord.x, abs_coord.y));
    if(offset < 0.01) discard;
	outColor = vec4(vec3(0.0), offset);
}
)";
static const char quad_line_vert[] = R"(
#version 330 core

uniform mat4 MVPMatrix;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_Barycentric;

smooth out vec2 pass_Barycentric;

void main(void){

	pass_Barycentric = in_Barycentric;
	
	gl_Position = MVPMatrix * vec4(in_Position, 1.0);
}
)";
