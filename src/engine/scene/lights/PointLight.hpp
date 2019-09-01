#pragma once

#include "scene/lights/Light.hpp"
#include "scene/Object.hpp"
#include "graphics/FramebufferCube.hpp"

/**
 \brief An omnidirectional punctual light, where light is radiating in all directions from a single point in space. Implements distance attenuation.
 \details It can be associated with a shadow cubemap with six orthogonal projections, and is rendered as a sphere in deferred rendering.
 \see GPU::Frag::Point_light, GPU::Frag::Light_shadow_linear, GPU::Frag::Light_debug
 \ingroup Scene
 */
class PointLight final : public Light {

public:
	/** Default constructor. */
	PointLight() = default;

	/** Constructor.
	 \param worldPosition the light position in world space
	 \param color the colored intensity of the light
	 \param radius the distance at which the light is completely attenuated
	 */
	PointLight(const glm::vec3 & worldPosition, const glm::vec3 & color, float radius);

	/**
	 \copydoc Light::init
	 */
	void init(const Texture * albedo, const Texture * normal, const Texture * depth, const Texture * effects) override;

	/**
	 \copydoc Light::draw
	 */
	void draw(const glm::mat4 & viewMatrix, const glm::mat4 & projectionMatrix, const glm::vec2 & invScreenSize) const override;

	/**
	 \copydoc Light::drawShadow
	 */
	void drawShadow(const std::vector<Object> & objects) const override;

	/**
	 \copydoc Light::drawDebug
	 */
	void drawDebug(const glm::mat4 & viewMatrix, const glm::mat4 & projectionMatrix) const override;

	/**
	 \copydoc Light::update
	 */
	void update(double fullTime, double frameTime) override;

	/**
	 \copydoc Light::clean
	 */
	void clean() const override;

	/**
	 \copydoc Light::setScene
	 */
	void setScene(const BoundingBox & sceneBox) override;

	/**
	 \copydoc Light::visible
	 */
	bool visible(const glm::vec3 & position, const Raycaster & raycaster, glm::vec3 & direction, float & attenuation) const override;

	/** Setup a point light parameters from a list of key-value tuples. The following keywords will be searched for:
	 \verbatim
	 position: X,Y,Z
	 radius: radius
	 intensity: R,G,B
	 shadows: bool
	 animations:
	 	- animationtype: ...
	 	- ...
	 \endverbatim
	 \param params the parameters tuple
	 */
	void decode(const KeyValues & params);

private:
	std::unique_ptr<FramebufferCube> _shadowFramebuffer; ///< The shadow cubemap framebuffer.

	std::vector<glm::mat4> _mvps;				///< Light mvp matrices for each face.
	glm::vec3 _lightPosition = glm::vec3(1.0f); ///< Light position.
	float _radius			 = 1.0f;			///< The attenuation radius.
	float _farPlane			 = 1.0f;			///< The projection matrices far plane.

	const Mesh * _sphere = nullptr; ///< The supporting geometry.
};
