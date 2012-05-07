#version 150 core

uniform vec3 diffColor;
// const vec3 ambientColor = vec3(0.0, 0.0, 0.0);

smooth in vec3 pass_Normal;
smooth in vec3 lightDir;
in vec3 pass_Position;

out vec4 out_Color;

void main(void){

	vec3 flat_normal = normalize(cross(dFdx(pass_Position),dFdy(pass_Position)));

	float diffuse = max(0.0, dot(normalize(pass_Normal), normalize(lightDir)));
	float diffuse2 = max(0.0, dot(normalize(flat_normal), normalize(lightDir)));
	
	vec3 temp_Color = 1.5 * diffuse2 * diffuse *  diffColor;
	// temp_Color += ambientColor;
	
	// vec3 reflection = normalize( reflect( -normalize(lightDir), normalize(flat_normal) ) );
	// float specular = max( 0.0, dot( normalize(flat_normal), reflection) );
	
	// if(diffuse != 0 && diffuse2 != 0){
		// float fspecular = pow(specular, 128.0);
		// temp_Color += vec3(fspecular);
	// }
	out_Color = vec4(temp_Color,1.0);
}