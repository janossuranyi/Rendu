
#include "common_pbr.glsl"
#include "shadow_maps.glsl"
#include "utils.glsl"

// Uniforms
layout(set = 2, binding = 0) uniform texture2D albedoTexture; ///< Albedo.
layout(set = 2, binding = 1) uniform texture2D normalTexture; ///< Normal.
layout(set = 2, binding = 2) uniform texture2D depthTexture; ///< Depth.
layout(set = 2, binding = 3) uniform texture2D effectsTexture; ///< Effects.
layout(set = 2, binding = 4) uniform texture2DArray shadowMap; ///< Shadow map.

layout(set = 0, binding = 0) uniform UniformBlock {
	mat4 viewToLight; ///< View to light space matrix.
	vec4 projectionMatrix; ///< Camera projection matrix
	vec3 lightPosition; ///< Light position in view space.
	vec3 lightDirection; ///< Light direction in view space.
	vec3 lightColor; ///< Light intensity.
	vec2 intOutAnglesCos; ///< Angular attenuation inner and outer angles.
	float lightRadius; ///< Attenuation radius.
	float shadowBias; ///< shadow depth bias.
	int shadowMode; ///< The shadow map technique.
	int shadowLayer; ///< The shadow map layer.
};

layout(location = 0) out vec3 fragColor; ///< Color.


/** Compute the lighting contribution of a spot light using the GGX BRDF. */
void main(){
	
	vec2 uv = gl_FragCoord.xy/textureSize(albedoTexture, 0).xy;
	
	vec4 albedoInfo = textureLod(sampler2D(albedoTexture, sClampNear),uv, 0.0);
	// If emissive (skybox or object), don't shade.
	uint material = decodeMaterial(albedoInfo.a);
	if(material == MATERIAL_EMISSIVE){
		discard;
	}
	
	// Get all informations from textures.
	vec3 baseColor = albedoInfo.rgb;
	float depth = textureLod(sampler2D(depthTexture, sClampNear),uv, 0.0).r;
	vec3 position = positionFromDepth(depth, uv, projectionMatrix);


	vec3 n = decodeNormal(textureLod(sampler2D(normalTexture, sClampNear), uv, 0.0).rg);
	vec3 v = normalize(-position);
	vec3 deltaPosition = lightPosition - position;
	vec3 l = normalize(deltaPosition);
	
	// Early exit if we are outside the sphere of influence.
	if(length(deltaPosition) > lightRadius){
		discard;
	}
	// Compute the angle between the light direction and the (light, surface point) vector.
	float currentAngleCos = dot(-l, normalize(lightDirection));
	// If we are outside the spotlight cone, no lighting.
	if(currentAngleCos < intOutAnglesCos.y){
		discard;
	}
	// Compute the spotlight attenuation factor based on our angle compared to the inner and outer spotlight angles.
	float angleAttenuation = clamp((currentAngleCos - intOutAnglesCos.y)/(intOutAnglesCos.x - intOutAnglesCos.y), 0.0, 1.0);
	
	// Orientation: basic diffuse shadowing.
	float orientation = max(0.0, dot(l,n));
	// Shadowing
	float shadowing = 1.0;
	if(shadowMode != SHADOW_NONE){
		vec4 lightSpacePosition = viewToLight * vec4(position,1.0);
		lightSpacePosition /= lightSpacePosition.w;
		lightSpacePosition.xy = 0.5 * lightSpacePosition.xy + 0.5;
		shadowing = shadow(shadowMode, lightSpacePosition.xyz, shadowMap, shadowLayer, shadowBias);
	}
	// Attenuation with increasing distance to the light.
	float localRadius2 = dot(deltaPosition, deltaPosition);
	float radiusRatio2 = localRadius2/(lightRadius*lightRadius);
	float attenNum = clamp(1.0 - radiusRatio2, 0.0, 1.0);
	float attenuation = angleAttenuation * attenNum * attenNum;
	
	// Read effects infos.
	vec3 infos = texture(sampler2D(effectsTexture, sClampNear), uv).rgb;
	float roughness = max(0.0001, infos.r);
	float metallic = infos.g;
	
	// BRDF contributions.
	// Compute F0 (fresnel coeff).
	// Dielectrics have a constant low coeff, metals use the baseColor (ie reflections are tinted).
	vec3 F0 = mix(vec3(0.04), baseColor, metallic);
	
	// Normalized diffuse contribution. Metallic materials have no diffuse contribution.
	vec3 diffuse = INV_M_PI * (1.0 - metallic) * baseColor * (1.0 - F0);
	
	vec3 specular = ggx(n, v, l, F0, roughness);
	
	fragColor.rgb = shadowing * attenuation * orientation * (diffuse + specular) * lightColor;
	
}

