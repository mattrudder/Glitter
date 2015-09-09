#version 400

uniform sampler2D uDiffuse;

in vec3 vColor;

out vec4 finalColor;

void main() {
	finalColor = vec4(vColor, 1.0);
}
