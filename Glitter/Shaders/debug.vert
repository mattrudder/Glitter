#version 400

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

uniform sampler2D uDiffuse;

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 vColor;

void main() {
  vColor = aColor;
	gl_Position = uProj * uView * uModel * vec4(aPosition, 1.0);
}
