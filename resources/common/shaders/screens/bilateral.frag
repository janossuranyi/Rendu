
#include "utils.glsl"
#include "samplers.glsl"

layout(location = 0) in INTERFACE {
	vec2 uv; ///< UV coordinates.
} In ;

layout(set = 2, binding = 0) uniform texture2D source; ///< Image to blur.
layout(set = 2, binding = 1) uniform texture2D depth; ///< Depth information.
layout(set = 2, binding = 2) uniform texture2D normal; ///< Normal information.

layout(set = 0, binding = 0) uniform UniformBlock {
	vec2 invDstSize; ///< Inverse of the target resolution.
	vec2 projParams; ///< Projection depth-related coefficients.
	int axis; ///< Dimension to blur along.
};

layout(location = 0) out vec4 fragColor; ///< Blurred color.


// Fixed parameters of the Gaussian blur.
const int radius = 4; ///< Blur radius.
const int shift = 2; ///< Step between samples (introduce dithering artifacts).
const float weights[5] = float[](0.153170, 0.144893, 0.122649, 0.092902, 0.062970); ///< Gaussian weights.

/** Compute bilateral weight based on depth and normal differences.
 \param n normal at the center point
 \param z depth at the center point (in view space)
 \param nNeigh normal at the neighbor
 \param zNeigh depth at the neighbor (in view space)
 \return the geometry-guided weight
 */
float computeWeight(vec3 n, float z, vec3 nNeigh, float zNeigh){
	// Depth difference.
	float zWeight = max(1.0 - abs(z - zNeigh)*2.0, 0.0);
	// Normal difference.
	float angleDiff = dot(n, nNeigh);
	angleDiff *= angleDiff;
	angleDiff *= angleDiff;
	float nWeight = max(angleDiff, 0.0);
	// Final weight.
	return nWeight * zWeight;
}

/** Compute the bilateral weight of a neighboring texel and apply it.
 \param uvNeigh the texel UV coordinates
 \param n the reference texel surface normal
 \param z the reference texel surface depth (view space)
 \param blurWeight gaussian blur weight
 \param minWeight lowest weight uncountered yet (for monotonous enforcement)
 \param sum normalization sum
 \return the weighted color sample
 */
vec4 weightedColor(vec2 uvNeigh, vec3 n, float z, float blurWeight, inout float minWeight, inout float sum){
	vec3 nNeigh = normalize(2.0 * textureLod(sampler2D(normal, sClampLinear), uvNeigh, 0.0).rgb - 1.0);
	float zNeigh = linearizeDepth(textureLod(sampler2D(depth, sClampLinear), uvNeigh, 0.0).r, projParams);
	// Compute weight, keep them decreasing.
	float weight = computeWeight(n, z, nNeigh, zNeigh);
	minWeight = min(minWeight, weight);
	// Blend with Gaussian weight.
	float totalWeight = blurWeight * minWeight;
	sum += totalWeight;
	vec4 colNeigh = textureLod(sampler2D(source, sClampLinear), uvNeigh, 0.0);
	return totalWeight * colNeigh;
}

/** Apply an approximate bilateral filter combining a separable gaussian blur and a geometry-guided weight.
 */
void main(){

	// Center always contribute.
	vec4 col = textureLod(sampler2D(source, sClampLinear), In.uv, 0.0);
	float sum = weights[0];
	vec4 accum = sum * col;
	// Get reference geoemtric info.
	vec3 n = normalize(2.0 * textureLod(sampler2D(normal, sClampLinear), In.uv, 0.0).rgb - 1.0);
	float z = linearizeDepth(textureLod(sampler2D(depth, sClampLinear), In.uv, 0.0).r, projParams);

	// Delta is a one pixel shift of the target resolution.
	vec2 delta = axis == 0 ? vec2(shift * invDstSize.x, 0.0) : vec2(0.0, shift * invDstSize.y);

	// Explore on both sides, avoid non monotonous weights.
	float minWeight = 1000000.0;
	for(int dx = 1; dx <= radius; ++dx){
		vec2 uvNeigh = In.uv - dx * delta;
		accum += weightedColor(uvNeigh, n, z, weights[dx], minWeight, sum);
	}

	minWeight = 1000000.0;
	for(int dx = 1; dx <= radius; ++dx){
		// Shift UV and read geometric info.
		vec2 uvNeigh = In.uv + dx * delta;
		accum += weightedColor(uvNeigh, n, z, weights[dx], minWeight, sum);
	}

	// Final normalization.
	fragColor = accum;
	if(sum != 0.0){
		fragColor /= sum;
	}
}
