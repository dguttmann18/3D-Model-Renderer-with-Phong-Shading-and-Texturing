#version 330 core
in vec3 FragPos;       // Fragment position
in vec3 Normal;        // Normal vector of the fragment
in vec2 TexCoord;      // Texture coordinates

out vec4 FragColor;    // Output fragment color

struct Light {
    vec3 position;
    vec3 color;
};

uniform sampler2D texture0;
uniform Light light1;
uniform Light light2;

uniform vec3 viewPos;  // Camera position

void main()
{
    vec3 texColor = texture(texture0, TexCoord).rgb;

    // Ambient lighting
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * texColor;

    // Diffuse lighting
    vec3 lightDir1 = normalize(light1.position - FragPos);
    vec3 lightDir2 = normalize(light2.position - FragPos);
    float diffuseStrength = 0.7;
    float diff1 = max(dot(Normal, lightDir1), 0.0);
    float diff2 = max(dot(Normal, lightDir2), 0.0);
    vec3 diffuse1 = diff1 * diffuseStrength * light1.color;
    vec3 diffuse2 = diff2 * diffuseStrength * light2.color;

    // Specular lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir1 = reflect(-lightDir1, Normal);
    vec3 reflectDir2 = reflect(-lightDir2, Normal);
    float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), 20.0);
    float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), 10.0);
    vec3 specular1 = spec1 * specularStrength * light1.color;
    vec3 specular2 = spec2 * specularStrength * light2.color;

    vec3 result = (ambient + diffuse1 + diffuse2) * texColor + specular1 + specular2;

    FragColor = vec4(result, 1.0);
}