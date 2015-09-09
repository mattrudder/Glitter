#version 400

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform sampler2D uDiffuse;
uniform sampler2D uShadowTexture;
uniform mat4 uShadowProj;
uniform mat4 uShadowView;
uniform vec4 uMainTexSB;
uniform vec3 uLightDir;
uniform vec4 uLightColor;
uniform vec4 uColor;
uniform vec3 uPointLightPosition;
uniform vec4 uPointLightColor;

uniform sampler2D uBoneTexture;
//uniform int uBoneTextureWidth;
//uniform mat4 uBones[120];

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColor;
layout (location = 3) in vec4 aBoneWeights;
layout (location = 4) in ivec4 aBoneIndices;
layout (location = 5) in vec2 aUV;

//in vec4 aPosition;
//in vec4 aUV;
//in vec4 aColor;
//in vec3 aNormal;
//in vec4 aBoneWeights;
//in ivec4 aBoneIndices;

out vec4 vPos;
out vec3 vNormal;
out vec2 vUV;
out vec4 vShadowCoord;
out vec3 vPointLightDir;
out vec4 vColor;
out float vPointLightAttenuation;

const int pixelsPerMatrix = 4;

// TODO: Why doesn't this operation work when the texture width is passed in?
ivec2 lookupMatrixCoord(int offset)
{
	return ivec2(offset % 512, offset / 512);
}

mat4 lookupMatrix(sampler2D tex, int index)
{
  vec4 col0 = texelFetch(tex, lookupMatrixCoord(index * pixelsPerMatrix + 0), 0);
  vec4 col1 = texelFetch(tex, lookupMatrixCoord(index * pixelsPerMatrix + 1), 0);
  vec4 col2 = texelFetch(tex, lookupMatrixCoord(index * pixelsPerMatrix + 2), 0);
  vec4 col3 = texelFetch(tex, lookupMatrixCoord(index * pixelsPerMatrix + 3), 0);
  return mat4(col0, col1, col2, col3);
}

mat4 biasMatrix = mat4(
                    0.5, 0.0, 0.0, 0.0,
                    0.0, 0.5, 0.0, 0.0,
                    0.0, 0.0, 0.5, 0.0,
                    0.5, 0.5, 0.5, 1.0
                    );

mat4 id= mat4(
                    1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, 1.0, 0.0,
                    0.0, 0.0, 0.0, 1.0
                    );

void main ()
{
  mat4 matBone = lookupMatrix(uBoneTexture, aBoneIndices.x) * aBoneWeights.x;
	if (aBoneWeights.y > 0.0)
		matBone += lookupMatrix(uBoneTexture, aBoneIndices.y) * aBoneWeights.y;
	if (aBoneWeights.z > 0.0)
		matBone += lookupMatrix(uBoneTexture, aBoneIndices.z) * aBoneWeights.z;
	if (aBoneWeights.w > 0.0)
		matBone += lookupMatrix(uBoneTexture, aBoneIndices.w) * aBoneWeights.w;

  vec4 posLocal = matBone * vec4(aPosition, 1.0);
  vec4 normLocal = matBone * vec4(aNormal, 0.0);

	vNormal = normLocal.xyz;
	vColor = aColor;
	vUV = aUV.xy;


	gl_Position = uProj * uView * uModel * posLocal;

  vPointLightDir = normalize(uPointLightPosition - vPos.xyz);
  //vPointLightAttenuation = 10.0 / length(uPointLightPosition - vPos.xyz);
  float dist = length(uPointLightPosition - vPos.xyz);
  vPointLightAttenuation = 1.0 / ((0.022 * dist) + (0.0019 * dist * dist));

  vShadowCoord = biasMatrix * uShadowProj * uShadowView * vPos;
}
