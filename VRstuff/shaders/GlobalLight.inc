
layout(std140) uniform GlobalLight
{
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
} globalLight;


vec3 CalculateGlobalLighting(GlobalLight light, vec3 normal, vec3 viewDir,vec3 diffuse,vec3 specular,float shininess)
{
    vec3 lightDir = normalize(-light.direction.xyz);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // combine results
    vec3 ambient = light.ambient.xyz * diffuse;
    vec3 diffuse = light.diffuse.xyz * diff * diffuse;
    vec3 specular = light.specular.xyz * spec * specuar;
    return (ambient + diffuse + specular);
}
