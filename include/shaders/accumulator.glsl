static const char accumulator_frag[] = R"(
#version 330 core

struct Light{
	vec3 direction;
	float Si;
	float Di;
	float Ai;
	float Intensity;
};

uniform Light light;

uniform sampler2D DepthMap;
uniform sampler2DShadow LightDepthMap;
uniform sampler2D ColorMap;
uniform sampler2D NormalMap;

uniform mat2 depth_iproj;
uniform mat4 depth_matrix;
uniform vec3 skyColor;

noperspective in vec2 TexCoord;
smooth in vec3 ray_origin;
smooth in vec3 ray_direction;

layout(location = 0) out vec4 out_Color;

//vec2 poissonDisk[4] = vec2[](
//    vec2( -0.94201624, -0.39906216 ),
//    vec2( 0.94558609, -0.76890725 ),
//    vec2( -0.094184101, -0.92938870 ),
//    vec2( 0.34495938, 0.29387760 )
//);

vec3 CalcPosition(float depth){
	vec2 linearDepth = depth_iproj * vec2(2.0 * depth - 1.0, 1.0);
	return ray_origin + ray_direction * (linearDepth.x / (linearDepth.y * ray_direction.z));
}

vec3 CalcLight(vec3 position, vec3 normal, float AO){
	
	vec3 DiffuseColor = vec3(0.0);
	vec3 SpecularColor = vec3(0.0);
	
	vec3 light_Direction = -normalize(light.direction);
	float diffuse = max(0.0, dot(normal, light_Direction));
	
	if(diffuse > 0.0){
		DiffuseColor = vec3(diffuse);
		
		vec3 half_vector = normalize(light_Direction - normalize(position));
		
		float specular = max(0.0, dot(normal, half_vector));
		float fspecular = pow(specular, 32.0);
		SpecularColor = vec3(fspecular);
	}

    vec3 proj = (depth_matrix * vec4(position, 1.0)).xyz;
    float visibility = 1.0 - texture(LightDepthMap, vec3(proj.xy, proj.z - 0.01));
    DiffuseColor  *= visibility;
    SpecularColor *= visibility;
    //if(AO < 0.5) AO *= 0.5;

	return light.Intensity * (light.Di * DiffuseColor + light.Si * SpecularColor + light.Ai * AO * skyColor);
}

bool isEdge(vec3 normal){
	vec3 normalRight = textureOffset(NormalMap, TexCoord, ivec2(1, 0)).xyz; //Assume normalized
	vec3 normalUp = textureOffset(NormalMap, TexCoord, ivec2(0, 1)).xyz; //Assume normalized
	
	float amountX = dot(normalRight, normal);
	float amountY = dot(normalUp, normal);
	
	if(amountX < 0.9 || amountY < 0.9 ) return true;
	else return false;
}

void main(void){

    vec4 NormalAO = texture(NormalMap, TexCoord); //Assume normalized
    if(NormalAO.xyz != vec3(0.0)){
        float Depth = texture(DepthMap, TexCoord).r;
        vec3 Position = CalcPosition(Depth);

        vec3 Color = texture(ColorMap, TexCoord).rgb;
        vec3 Normal = NormalAO.xyz;
        float AO = NormalAO.a;

        out_Color = vec4(Color * CalcLight(Position, Normal, AO), 1.0);
        //if(isEdge(Normal)) out_Color = out_Color * 0.6;
        gl_FragDepth = Depth;
    }
    else discard;
}
)";
static const char accumulator_vert[] = R"(
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
)";
