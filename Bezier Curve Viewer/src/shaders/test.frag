#version 330 core
out vec4 color;
out vec4 FragColor;

in vec3 fragColor;

void main() {
	color = vec4(fragColor, 1.0);
	FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
