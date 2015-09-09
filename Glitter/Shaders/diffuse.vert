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

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColor;
layout (location = 5) in vec2 aUV;

//out vec4 vPos;
out vec3 vNormal;
out vec4 vColor;
out vec2 vUV;
out vec4 vShadowCoord;
out vec3 vPointLightDir;

#define TRANSFORM_TEX(xy,tiling,offset) (xy * tiling + offset)

mat4 biasMatrix = mat4(
                    0.5, 0.0, 0.0, 0.0,
                    0.0, 0.5, 0.0, 0.0,
                    0.0, 0.0, 0.5, 0.0,
                    0.5, 0.5, 0.5, 1.0
                    );

void main ()
{
  mat4 mvp = uProj * uView * uModel;
  vec4 vPos = mvp * vec4(aPosition, 1.0f);
  vColor = aColor;
  gl_Position = vPos;

  vUV = TRANSFORM_TEX(aUV, uMainTexSB.zw, uMainTexSB.xy);

  vec4 v4Norm = mvp * vec4(aNormal, 0.0f);
  vNormal = v4Norm.xyz;

  vPointLightDir = normalize(uPointLightPosition - vPos.xyz);
  vShadowCoord = biasMatrix * uShadowProj * uShadowView * uModel * vec4(aPosition, 1.0f);
}
