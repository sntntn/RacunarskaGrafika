#version 330 core
layout (location = 0) in vec3 aPos;             //pozicija
layout (location = 1) in vec3 aNormal;          //atribut normale                          //ova tri uvek saljemo kada crtamo model
layout (location = 2) in vec2 aTexCoords;       //kordinate tekstura

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));                            //odredjujemo poziciju fragmenta preko pozicije vertex-a u svetu
    Normal = aNormal;
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}