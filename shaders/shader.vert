#version 150 core

const vec3 lightPosition = vec3(-100.0,100.0,200.0);

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 trackballMatrix;

in vec3 in_Position;
in vec3 in_Normal;

smooth out vec3 pass_Normal;
out vec3 pass_Position;
smooth out vec3 lightDir;

void main(void){
	// In principle we should use mat4 normalMatrix = transpose(inverse(modelMatrix)); here.
	// we only want to re-orient the normal and the modelMatrix contains translations.
	mat4 tmMatrix = trackballMatrix * modelMatrix;
	pass_Normal = ( tmMatrix * vec4(in_Normal, 1.0)).xyz; 
	vec3 eyePosition = (viewMatrix * tmMatrix * vec4(in_Position, 1.0)).xyz;
	lightDir = normalize(lightPosition-eyePosition);
	pass_Position = eyePosition;
	
	gl_Position = projectionMatrix * viewMatrix * tmMatrix * vec4(in_Position, 1.0);
}