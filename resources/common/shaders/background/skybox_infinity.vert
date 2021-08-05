
// Attributes
layout(location = 0) in vec3 v; ///< Position.

// Uniform
layout(set = 0, binding = 1) uniform UniformBlock {
	mat4 mvp; ///< MVP transformation matrix.
};

layout(location = 0) out INTERFACE {
	vec3 pos; ///< Position in model space.
} Out;

/** Apply the transformation to the input vertex, treating it as a vector to ignore the translation part and keep it centered.
 We also ensure the vertex will be set to the maximum depth by tweaking gl_Position.z.
 */
void main(){
	// We multiply the coordinates by the MVP matrix, and ouput the result.
	// To keep the skybox centered on the camera, we treat its vertices as directions (no translation)
	gl_Position = mvp * vec4(v, 0.0);
	// Ensure the skybox is sent to the maximum depth.
	gl_Position.z = gl_Position.w; 
	Out.pos = v;
	
}
