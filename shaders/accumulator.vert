#version 330 core

uniform mat4 invProjMatrix;

noperspective out vec2 TexCoord;
smooth out vec3 viewRay;

void main(void){
	TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;
	TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;

    vec4 position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);

	viewRay = (invProjMatrix * position).xyz;
	gl_Position = position;
}
