#version 330 core

struct Light{
	vec3 direction;
};

uniform Light light;
uniform vec3 diffColor;

smooth in vec3 pass_Normal;
smooth in vec3 pass_Position;

out vec4 out_Color;

void main(void){

	vec3 Normal = normalize(pass_Normal);
	vec3 light_Direction = -normalize(light.direction);
	vec3 camera_Direction = normalize(-pass_Position);
	vec3 half_vector = normalize(camera_Direction + light_Direction);
	
	float nlddot = dot(Normal, light_Direction);
	float diffuse = max(0.0, nlddot) + 0.2;
	vec3 temp_Color = diffuse * vec3(1.0);
	
	if(nlddot > 0.01){
		float specular = max( 0.0, dot( Normal, half_vector) );
		float fspecular = pow(specular, 128.0);
		temp_Color += fspecular;
	}
	
	out_Color = vec4(diffColor * temp_Color, 1.0);
}