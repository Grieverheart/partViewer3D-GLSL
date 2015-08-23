#version 330 core

uniform float radius;

noperspective in vec3 ray_origin;
noperspective in vec3 ray_dir;
flat in vec3 sphere_position;

layout(location = 0) out vec3 outColor;

//TODO: Improve this?
bool intersect_sphere(in vec3 position, in float radius, in vec3 ray_r0, in vec3 ray_dir){
	vec3 direction = position - ray_r0;
	float B = dot(ray_dir, direction);
	float det = B * B - dot(direction, direction) + radius * radius;

	if(det < 0.0) return false;

	return true;
}

void main(void){
    vec3 ray_dir_ = normalize(ray_dir);
    if(!intersect_sphere(sphere_position, radius, ray_origin, ray_dir_)) discard;
    outColor = vec3(1.0, 1.0, 0.0);
}
