#version 330 core

out vec4 FragColor;

uniform vec3 uParticleColor;

void main()
{
    FragColor = vec4(uParticleColor, 1.0);
}
