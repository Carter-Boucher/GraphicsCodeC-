#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord; // Texture coordinates input

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord; // Pass texture coordinates to the fragment shader

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{
    FragPos = vec3(M * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(M))) * aNormal; // Transform normals
    TexCoord = aTexCoord; // Pass texture coordinates
    gl_Position = P * V * vec4(FragPos, 1.0);
}
