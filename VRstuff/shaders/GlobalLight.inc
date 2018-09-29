

struct GlobalLight
{
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


vec3 CalculateGlobalighting(GlobalLight light, vec3 normal, vec3 viewDir,vec3 diffuse,vec3 specular,float shininess)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * diffuse);
    vec3 diffuse = light.diffuse * diff * diffuse);
    vec3 specular = light.specular * spec * specular);
    return (ambient + diffuse + specular);
}
