
uniform sampler2D shadowMap;

float shadow_calculation(vec4 fragPosLightSpace,sampler2D _shadowMap)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	float closestDepth = texture(_shadowMap, projCoords.xy).r;   
	float currentDepth = projCoords.z; 
	float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;  

	return shadow;
}
