
in INTERFACE {
	vec3 dir; ///< View world direction.
	vec2 uv; ///< Texture coordinates.
} In ;

uniform float iTime; ///< Time since beginning of playback.
uniform float iTimeDelta; ///< Time since last frame.
uniform float iFrame; ///< Frame count since beginning of playback.
uniform vec3 iResolution; ///< Screen resolution.
uniform vec4 iMouse; ///< xy: mouse position if left button pressed, zw: left/right button clicked?
uniform mat4 iView; ///< View matrix.
uniform mat4 iProj; ///< Projection matrix.
uniform mat4 iViewProj; ///< View projection matrix.
uniform mat4 iViewInv; ///< Inverse view matrix.
uniform mat4 iProjInv; ///< Inverse projection matrix.
uniform mat4 iViewProjInv; ///< Inverse view projection matrix.
uniform mat4 iNormalMat; ///< Normal transformation matrix.
uniform vec3 iCamPos; ///< Camera position.
uniform vec3 iCamUp; ///< Camera up vector.
uniform vec3 iCamCenter; ///< Camera lookat position.
uniform float iCamFov; ///< Camera field of view.


layout(binding = 0) uniform sampler2D previousFrame; ///< Previous frame.
layout(binding = 1) uniform sampler2D sdfFont; ///< Font SDF texture.
layout(binding = 2) uniform sampler2D gridMap; ///< Debug grid texture.
layout(binding = 3) uniform sampler2D noise2DMap; ///< RGBA uniform noise in [0,1], uncorrelated.
layout(binding = 4) uniform sampler2D perlin2DMap; ///< RGBA tiling perlin noise in [0,1], four different scales and offsets.
layout(binding = 5) uniform sampler2D directionsMap; ///< Random 3D directions on the unit sphere.
layout(binding = 6) uniform sampler3D noise3DMap; ///< RGBA 3D uniform noise in [0,1], uncorrelated.
layout(binding = 7) uniform sampler3D perlin3DMap; ///< RGBA 3D tiling perlin noise in [0,1], four different scales and offsets.

layout(location = 0) out vec4 fragColor; ///< Output color.

uniform float gamma = 2.2; ///< Gamma correction.
uniform float specExponent = 128.0; ///< Specular exponent.
uniform float radius = 0.5; ///< Sphere radius.
uniform float epsilon = 0.001; ///< Raymarching tolerance.

uniform vec3 skyBottom = vec3(0.035, 0.090, 0.159); ///< Sky color bottom.
uniform vec3 skyLight = vec3(0, 0.254, 0.654); ///< Light color bottom.
uniform vec3 skyTop = vec3(0, 0.681, 1.0); ///< Sky color.
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0); ///< Light color.
uniform vec3 sphereColor = vec3(0.8, 0.5, 0.2); ///< Sphere color.
uniform vec3 ground0 = vec3(0.025, 0.390, 0.473); ///< Ground color 0.
uniform vec3 ground1 = vec3(0.123, 0.462, 0.527); ///< Ground color 1.

uniform vec4 lightDirection = vec4(-1.8, 1.6, 1.7, 0.0); ///< Light direction.

uniform int stepCount = 128; ///< Maximum step count.
uniform bool showPlane = true; ///< Show the moving plane.

/** Scene signed distance function.
\param pos the 3D world position
\return the distance to the surface and the material ID.
*/
vec2 map(vec3 pos){
	// Sphere 0.
	float dist0 = length(pos - vec3(0.0,0.1,-0.4)) - radius;
	float dist1 = abs(pos.y + 0.5);
	// Skip the plane if asked.
	return (dist0 < dist1 || !showPlane) ? vec2(dist0, 1.0) : vec2(dist1, 2.0);
}

/** Raymarch until hitting the scene surface or reaching the max number of steps.
\param orig the ray position
\param dir the ray direction
\param t will contain the distance along the ray to the intersection
\param res will contain final distance to the surface and material ID
\return true if there was an intersection
*/
bool raymarch(vec3 orig, vec3 dir, out float t, out vec2 res){
	// Reset.
	t = 0.0;
	res = vec2(0.0);
	// Step through the scene.
	for(int i = 0; i < stepCount; ++i){
		// Current position.
		vec3 pos = orig + t * dir;
		// Query the distance to the closest surface in the scene.
		res = map(pos);
		// Move by this distance.
		t += res.x;
		// If the distance to the scene is small, we have reached the surface.
		if(res.x < epsilon){
			return true;
		}
	}
	return false;
}

/** Compute the normal to the surface of the scene at a given world point.
\param p the point to evaluate the normal at
\return the normal
*/
vec3 normal(vec3 p){
	const vec2 epsilon = vec2(0.02, 0.0); //...bit agressive.
	float dP = map(p).x;
	// Forward differences scheme, cheaper.
	return normalize(vec3(
						  map(p + epsilon.xyy).x - dP,
						  map(p + epsilon.yxy).x - dP,
						  map(p + epsilon.yyx).x - dP
						  ));
}

/// Main render function.
void main(){
	vec3 dir = normalize(In.dir);
	vec3 eye = iCamPos;
	// Light parameters.
	vec3 lightDir = normalize(lightDirection.xyz);

	// Background color: 
	// 4-directions gradient, centered on the light.
	float lightFacing = dot(dir,lightDir)*0.5+0.5;
	vec3 backgroundColor = mix(
		mix(skyBottom, skyLight, lightFacing), 
		mix(skyTop, lightColor, lightFacing), 
		smoothstep(-0.1, 0.1, dir.y));
	vec3 color = backgroundColor;

	// Foreground:
	// Check if we intersect something along the ray.
	float t; vec2 res;
	bool didHit = raymarch(eye, dir, t, res);
	// If we hit a surface, compute its appearance.
	if(didHit){
		// Compute position and normal.
		vec3 hit = eye + t * dir;
		vec3 n = normal(hit);
		// Diffuse lighting.
		float diffuse = max(dot(n, lightDir), 0.0);
		// Specular lighting.
		float specular = max(dot(reflect(-lightDir, n), -dir), 0.0);
		// Adjust parameters based on surface hit.
		vec3 baseColor = vec3(0.0);
		if(res.y < 1.5){
			baseColor = sphereColor;
			specular = pow(specular, specExponent);
		} else if(res.y < 2.5){
			vec2 movingUV = hit.xz + vec2(0.5*iTime, 0.0);
			float noise = texture(noise2DMap, 0.005*movingUV).r;
			// Smooth transition.
			float intensity = smoothstep(0.45, 0.55, noise);
			// Mix two colors, diffuse material.
			baseColor = mix(ground0, ground1, intensity);
			specular = 0.0;
		}
		// Final appearance.
		vec3 objColor = 0.4 * baseColor + lightColor * (diffuse * baseColor + specular);
		// Apply a fading fog effect based on depth.
		color = mix(objColor, color, clamp(t-2.0,0.0,1.0));
	}
	
	/// Exposure tweak, output.
	fragColor = vec4(pow(color, vec3(gamma)),1.0);
}
