#version 330 core

uniform mat4 invProjMatrix;

noperspective out vec2 TexCoord;
smooth out vec3 ray_direction;
smooth out vec3 ray_origin;

void main(void){
	TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;
	TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;

    //Position on far and near planes
    vec4 p_near = invProjMatrix * vec4(2.0 * TexCoord - 1.0, -1.0, 1.0);
    vec4 p_far  = invProjMatrix * vec4(2.0 * TexCoord - 1.0, 1.0, 1.0);
    p_near /= p_near.w;
    p_far  /= p_far.w;

    //The difference of the point on the far and near planes, gives us the ray direction
    //We find the ray origin by going backwards from the near plane in the ray direction.
    //We move by an amount z_near. To do that we use the z-normalized ray direciton.
    ray_direction = p_far.xyz - p_near.xyz;
    ray_origin    = p_near.xyz - p_near.z * (ray_direction.xyz / ray_direction.z);

	gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);
}
