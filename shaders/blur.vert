#version 330 core

noperspective out vec2 TexCoord;

void main(void){
	TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;
	TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;
    vec4 position = vec4(TexCoord * 2.0 - 1.0, 0.0, 1.0);
	
	gl_Position = position;
}
