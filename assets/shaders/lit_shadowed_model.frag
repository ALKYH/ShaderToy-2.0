#version 330 core

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform sampler2D uShadowMap;
uniform vec3 uViewPos;
uniform int uPointLightCount;

uniform vec3 uDirLightDirection;
uniform vec3 uDirLightColor;

const int MAX_POINT_LIGHTS = 4;
uniform vec3 uPointLightPosition[MAX_POINT_LIGHTS];
uniform vec3 uPointLightColor[MAX_POINT_LIGHTS];
uniform float uPointLightLinear[MAX_POINT_LIGHTS];
uniform float uPointLightQuadratic[MAX_POINT_LIGHTS];

uniform vec3 uSpotLightPosition;
uniform vec3 uSpotLightDirection;
uniform vec3 uSpotLightColor;
uniform float uSpotLightInnerCutoff;
uniform float uSpotLightOuterCutoff;

uniform bool uShadowsEnabled;
uniform float uShadowBias;

float computeShadow(vec4 fragPosLightSpace, vec3 normal)
{
    if (!uShadowsEnabled)
    {
        return 0.0;
    }

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
    {
        return 0.0;
    }

    float closestDepth = texture(uShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    vec3 lightDir = normalize(-uDirLightDirection);
    float bias = max(uShadowBias * (1.0 - dot(normal, lightDir)), uShadowBias);
    return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

vec3 applyDirectionalLight(vec3 normal, vec3 viewDir, vec3 albedo, float shadow)
{
    vec3 lightDir = normalize(-uDirLightDirection);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 ambient = 0.15 * uDirLightColor * albedo;
    vec3 diffuse = diff * uDirLightColor * albedo;
    vec3 specular = spec * uDirLightColor * 0.25;
    return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 applyPointLight(int index, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo)
{
    vec3 lightDir = normalize(uPointLightPosition[index] - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    float distance = length(uPointLightPosition[index] - fragPos);
    float attenuation = 1.0 / (1.0 + uPointLightLinear[index] * distance
        + uPointLightQuadratic[index] * distance * distance);

    vec3 ambient = 0.05 * uPointLightColor[index] * albedo;
    vec3 diffuse = diff * uPointLightColor[index] * albedo;
    vec3 specular = spec * uPointLightColor[index] * 0.25;
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
    float shadow = computeShadow(FragPosLightSpace, normal);

    vec3 result = vec3(0.0);
    result += applyDirectionalLight(normal, viewDir, albedo, shadow);
    for (int i = 0; i < uPointLightCount; ++i)
    {
        result += applyPointLight(i, normal, FragPos, viewDir, albedo);
    }
    result += applySpotLight(normal, FragPos, viewDir, albedo);

    FragColor = vec4(result, 1.0);
}
