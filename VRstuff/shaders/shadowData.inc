
uniform sampler2D shadowSampler;

float calculate_shadow_bias(vec3 normal,vec3 lightDir)
{
	return max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
}

float shadow_calculation(vec4 fragPosLightSpace,sampler2D _shadowMap,float bias)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	float closestDepth = texture(_shadowMap, projCoords.xy).r;   
	float currentDepth = projCoords.z; 
	//float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  
	
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(_shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(_shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;
	
	
	
	if(projCoords.z > 1.0)
			shadow = 0.0;
	return shadow;
}
