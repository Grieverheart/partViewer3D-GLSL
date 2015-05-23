#version 330 core

smooth in vec2 depth;

layout(location = 0) out vec2 variance;

void main(void){
    float moment1 = 0.5 * (depth.x / depth.y) + 0.5;
    float moment2 = moment1 * moment1;

    float dx = dFdx(moment1);
    float dy = dFdy(moment1);
    moment2 += 0.25 * (dx * dx + dy * dy);

    variance = vec2(moment1, moment2);
}
