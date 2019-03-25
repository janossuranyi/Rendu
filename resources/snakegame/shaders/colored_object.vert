#version 330

// Attributes
layout(location = 0) in vec3 v;///< Position.
layout(location = 1) in vec3 n; ///< Normal.

uniform mat4 mvp; ///< The transformation matrix.
uniform mat3 normalMat; ///< Model to view space for normals.

out INTERFACE {
	vec3 n;
} Out; ///< The view space normal.

/** Apply the MVP transformation to the input vertex. */
void main(){
	// We multiply the coordinates by the MVP matrix, and ouput the result.
	gl_Position = mvp * vec4(v, 1.0);
	Out.n = normalMat * n;
}
