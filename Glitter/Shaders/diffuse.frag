#version 400

precision highp float;

uniform sampler2D uDiffuse;
uniform vec4 uMainTexSB;
uniform vec3 uLightPos;
uniform vec4 uLightColor;
uniform vec4 uColor;
uniform sampler2D uShadowTexture;
uniform mat4 uShadowProj;
uniform mat4 uShadowView;

uniform vec3 uLightDir;
uniform vec3 uPointLightPosition;
uniform vec4 uPointLightColor;

//in vec4 vPos;
//in vec4 vColorVarying;
in vec3 vNormal;
in vec2 vUV;
in vec4 vColor;
in vec4 vShadowCoord;
in vec3 vPointLightDir;
//in float vPointLightAttenuation;

//layout (origin_upper_left) in vec4 gl_FragCoord;

out vec4 finalColor;

//void lightFunc(vec3 lightDir, vec4 lightColor, vec3 normal, out vec4 lambertianColor, out float specular) {
//	float lambertian = max(dot(lightDir,normal), 0.0);
//	lambertianColor = lambertian * lightColor;
//	specular = 0.0;
//
//	if(lambertian > 0.0) {
//		vec3 viewDir = normalize(-vPos.xyz);
//		vec3 halfDir = normalize(lightDir + viewDir);
//		float specAngle = max(dot(halfDir, normal), 0.0);
//		specular = pow(specAngle, 16.0);
//	}
//}

vec4 lightFunc(vec3 lightDir, vec4 lightColor, vec3 normal, float atten) {
	float diff = max(0.0, dot(lightDir, normal));
	vec4 c;
	c = lightColor * diff * atten * 2.0;
	c.a = 1.0;
	return c;
}

void main ()
{
	//vec4 texColor = texture(uDiffuse, vUV);
	vec4 texColor = mix(uColor, vColor, 0.95);

	vec4 ambientColor = 0.3 * texColor;
	vec4 diffuseColor = texColor;

	vec3 normal = normalize(vNormal);

	vec4 lambertianColor;
	//lightFunc(vLightDir, uLightColor, normal, lambertianColor, specular);
	//lightFunc(vLightDir, uPointLightColor, normal, lambertianColor, specular);
	vec4 litC = lightFunc(uLightDir, uLightColor, normal, 0.075);
	litC += lightFunc(vPointLightDir, uPointLightColor, normal, 1.0);

	vec4 c = ambientColor + litC * diffuseColor;
	vec4 ac = ambientColor * diffuseColor;
	vec4 lc = ambientColor + litC;

    // Shadows
    float bias = 0.0355;
    vec4 depth = texture(uShadowTexture, vShadowCoord.xy);
    vec4 shadow = vec4(1.0, 0.0, 0.0, 1.0);

    float occluderDepth = depth.z;
    float pixelDepth = vShadowCoord.z + bias;

    if( occluderDepth < pixelDepth)
        shadow = vec4(vec3(1.0 - 0.689), 1.0);
    else
        shadow = vec4(vec3(1.0), 1.0);

    finalColor = shadow * lc;
    //finalColor = vec4(0, gl_FragCoord.y / 1600, 0, 1.0);
//		finalColor = c;
		//finalColor = vec4(vColor.xyz, 1);
}
