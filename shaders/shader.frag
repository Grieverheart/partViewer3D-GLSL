#version 330 core

struct Light{
	vec3 direction;
};

uniform Light light;
uniform vec3 diffColor;
uniform sampler2DShadow inShadowMap;

smooth in vec3 pass_Normal;
smooth in vec3 pass_Position;
smooth in vec4 LSTexCoord;

out vec4 out_Color;

float CalcShadowFactor(vec4 LSTexCoord){

	float sum = 0;
	float bias = 0.0;
	sum += textureProjOffset(inShadowMap, LSTexCoord, ivec2(-1, -1));
	sum += textureProjOffset(inShadowMap, LSTexCoord, ivec2(1, 1));
	sum += textureProjOffset(inShadowMap, LSTexCoord, ivec2(-1, 1));
	sum += textureProjOffset(inShadowMap, LSTexCoord, ivec2(1, -1));

	float shadow = sum * 0.25;
	if(shadow < 0.3) shadow = 0.3;
	return shadow;
}

void main(void){

	vec3 Normal = normalize(pass_Normal);
	vec3 light_Direction = -normalize(light.direction);
	vec3 camera_Direction = normalize(-pass_Position);
	vec3 half_vector = normalize(camera_Direction + light_Direction);
	
	float nlddot = dot(Normal, light_Direction);
	float diffuse = max(0.0, nlddot) + 0.2;
	vec3 temp_Color = diffuse * vec3(1.0);
	
	float shadowFactor = 1.0;
	if(LSTexCoord.w > 0.0 && nlddot > 0.0001) shadowFactor = CalcShadowFactor(LSTexCoord);
	
	if(shadowFactor > 0.5){
		float specular = max( 0.0, dot( Normal, half_vector) );
		float fspecular = pow(specular, 128.0);
		temp_Color += fspecular;
	}
	
	out_Color = vec4(shadowFactor * diffColor * temp_Color, 1.0);
}