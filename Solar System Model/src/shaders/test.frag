#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoord; // Texture coordinates from the vertex shader

uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;

uniform sampler2D objectTexture; // Texture sampler

void main()
{
    vec3 objectColor = texture(objectTexture, TexCoord).rgb;
    float distance = length(lightPos - FragPos);

    //Attenuation
    float maxDistance = 400.0;
    float attenuation = clamp(1.0 - (distance / maxDistance), 0.0, 1.0);

    //Ambient 
    float ambientStrength = 0.02;
    vec3 ambient = ambientStrength * lightColor;

    //Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    //Specular 
    float specularStrength = 1.25;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor * attenuation;
    FragColor = vec4(result, 1.0);

    // Debugging
    // FragColor = vec4(Normal, 1.0);
    // FragColor = vec4(lightDir, 1.0);
    // FragColor = vec4(reflectDir, 1.0);
    // FragColor = vec4(viewDir, 1.0);
}
