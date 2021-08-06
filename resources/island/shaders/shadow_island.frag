
layout(location = 0) in INTERFACE {
	vec2 uv; ///< UV coordinates.
} In ;

layout(set = 0, binding = 0) uniform UniformBlock {
	vec3 lDir; ///< Light direction.
	uint stepCount; ///< Raymarching step count.
	float texelSize; ///< Size of a texel in world space.
};

layout(set = 1, binding = 0) uniform sampler2D heightMap; ///< Height map.

layout(location = 0) out vec2 shadow; ///< Shadowing factors.

/** Ray-march against the height map to determine shadows at sea and ground level.*/
void main(){
	
	if(lDir.y >= 0.999){
		// Vertical sun, no shadowing.
		shadow = vec2(1.0);
		return;
	} else if(lDir.y < -0.06){
		// Below horizon, all in shadows.
		shadow = vec2(0.0);
		return;
	}

	vec2 wh = textureSize(heightMap, 0).xy;
	float hStart = textureLod(heightMap, In.uv, 0.0).r;

	// Compute ray height.
	bool occGround = false;
	bool occWater = false;
	vec2 texSize = wh * texelSize;
	vec2 initPosPlane = texSize * (In.uv - 0.5);
	vec3 initPos = vec3(initPosPlane.x, hStart, initPosPlane.y);

	for(uint i = 0; i < stepCount; ++i){
		// Jut step based on 3D dir.
		vec3 pos = initPos + (float(i)) * texelSize * lDir;
		// Reproject onto the texture plane.
		vec2 grPos = pos.xz / texSize + 0.5;
		if(any(lessThan(grPos, vec2(0.0))) || any(greaterThanEqual(grPos, vec2(1.0)))){
			break;
		}
		// Read corresponding height and compare, for both the ground and water height.
		float hRef = textureLod(heightMap, grPos, 0.0).r;
		if(pos.y < hRef){
			occGround = true;
		}
		if(pos.y - hStart < hRef){
			occWater = true;
		}
		if(occGround && occWater){
			break;
		}
	}
	// Modulate when getting close to the horizon.
	float modu = min((lDir.y - -0.06)/0.06, 1.0);
	shadow.x = modu * float(!occGround);
	shadow.y = modu * float(!occWater);
}
