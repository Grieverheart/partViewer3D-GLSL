#version 330 core

smooth in vec2 TexCoord;

void main(void){
    vec2 pos = vec2(TexCoord - vec2(0.5));
    //TODO: Change to square
    if(length(pos) > 0.5) discard;
}
