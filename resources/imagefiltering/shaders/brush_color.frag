
layout(location = 0) in float dist; ///< Pseudo-distance to the brush center.

layout(set = 0, binding = 0) uniform UniformBlock {
	vec3 color; ///< Brush color.
	bool outline; ///< Are we in outline mode.
	float radiusPx; ///< Radius of the brush in pixels.
};

layout(location = 0) out vec3 fragColor; ///< Output color.

/** Draw the full brush or its outline. */
void main(){
	// For the outline case, we only want to display the 'edges' of the brush.
	// Those are detemrined by vertices with a dist equal to 1.
	// We work in pixels to be radius independent.
	if(outline){
		// Allow for a 10 pixels outline.
		if(radiusPx*(dist - 1.0) + 10 < 0){
			discard;
		}
	}
	fragColor = color;
}
