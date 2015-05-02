#version 330 core

noperspective out vec2 TexCoord;

void main(void){
	TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;
	TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;

	gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);
}
