#version 330

in vec3 worldPos; ///< The world space position of the fragment.
in vec2 uv; ///< UV coordinates.
uniform bool hasMask = false; ///< Should the object alpha mask be applied.
layout(binding = 0) uniform sampler2D mask;  ///< RGBA texture.
uniform vec3 lightPositionWorld; ///< The world space position of the light.
uniform float lightFarPlane; ///< The light projection matrix far plane.
layout(location = 0) out vec2 fragColor; ///< World space depth and depth squared.

/** Compute the depth in world space, normalized by the far plane distance */
void main(){
	// Mask cutout.
	if(hasMask){
		float a = texture(mask, uv).a;
		if(a <= 0.01){
			discard;
		}
	}
	// We compute the distance in world space (or equivalently view space).
	// We normalize it by the far plane distance to obtain a [0,1] value.
	float dist = clamp(length(worldPos - lightPositionWorld) / lightFarPlane, 0.0, 1.0);
	fragColor = vec2(dist, dist*dist);
}
