#version 330 core

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec3 uViewPos;

uniform vec3 uDirLightDirection;
uniform vec3 uDirLightColor;

uniform vec3 uPointLightPosition;
uniform vec3 uPointLightColor;
uniform float uPointLightLinear;
uniform float uPointLightQuadratic;

uniform vec3 uSpotLightPosition;
uniform vec3 uSpotLightDirection;
uniform vec3 uSpotLightColor;
uniform float uSpotLightInnerCutoff;
uniform float uSpotLightOuterCutoff;

vec3 applyDirectionalLight(vec3 normal, vec3 viewDir, vec3 albedo)
{
    vec3 lightDir = normalize(-uDirLightDirection);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 ambient = 0.15 * uDirLightColor * albedo;
    vec3 diffuse = diff * uDirLightColor * albedo;
    vec3 specular = spec * uDirLightColor * 0.25;
    return ambient + diffuse + specular;
}

vec3 applyPointLight(vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo)
{
    vec3 lightDir = normalize(uPointLightPosition - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    float distance = length(uPointLightPosition - fragPos);
    float attenuation = 1.0 / (1.0 + uPointLightLinear * distance
        + uPointLightQuadratic * distance * distance);

    vec3 ambient = 0.05 * uPointLightColor * albedo;
    vec3 diffuse = diff * uPointLightColor * albedo;
    vec3 specular = spec * uPointLightColor * 0.25;
    return (ambient + diffuse + specular) * attenuation;
}

vec3 applySpotLight(vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo)
{
    vec3 lightDir = normalize(uSpotLightPosition - fragPos);
    float theta = dot(lightDir, normalize(-uSpotLightDirection));
    float epsilon = uSpotLightInnerCutoff - uSpotLightOuterCutoff;
    float intensity = clamp((theta - uSpotLightOuterCutoff) / epsilon, 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 ambient = 0.03 * uSpotLightColor * albedo;
    vec3 diffuse = diff * uSpotLightColor * albedo;
    vec3 specular = spec * uSpotLightColor * 0.3;
    return (ambient + diffuse + specular) * intensity;
}

void main()
{
    vec3 albedo = texture(uTexture, TexCoord).rgb;
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(uViewPos - FragPos);

    vec3 result = vec3(0.0);
    result += applyDirectionalLight(normal, viewDir, albedo);
    result += applyPointLight(normal, FragPos, viewDir, albedo);
    result += applySpotLight(normal, FragPos, viewDir, albedo);

    FragColor = vec4(result, 1.0);
}
