
layout(std140) uniform GlobalLight
{
    vec4 directionGL;
    vec4 ambientGL;
    vec4 diffuseGL;
    vec4 specularGL;
} globalLight;


vec3 CalculateGlobalLighting(vec3 normal, vec3 viewDir,
	vec3 diffuse,vec3 specular,float shininess)
{
    vec3 lightDir = normalize(-globalLight.directionGL.xyz);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // combine results
    vec3 ambientRet = globalLight.ambientGL.xyz * diffuse;
    vec3 diffuseRet = globalLight.diffuseGL.xyz * diff *
						diffuse;
    vec3 specularRet = globalLight.specularGL.xyz * spec * 
	specular;
    return (ambientRet + diffuseRet + specularRet);
}
