#pragma once

#include "scene/Scene.hpp"
#include "renderers/Renderer.hpp"

#include "graphics/Framebuffer.hpp"
#include "input/ControllableCamera.hpp"

#include "Common.hpp"

/**
 \brief Renders a scene with an alternating black and white region style, using the stencil buffer to count primitives covering each pixel.
 \ingroup StencilDemo
 */
class StenciledRenderer final : public Renderer {

public:
	/** Constructor.
	 \param resolution the initial rendering resolution
	 */
	explicit StenciledRenderer(const glm::vec2 & resolution);

	/** Set the scene to render.
	 \param scene the new scene
	 */
	void setScene(const std::shared_ptr<Scene> & scene);

	/** \copydoc Renderer::draw */
	void draw(const Camera & camera, Framebuffer & framebuffer, uint layer = 0) override;

	/** \copydoc Renderer::resize
	 */
	void resize(uint width, uint height) override;
	
private:

	std::unique_ptr<Framebuffer> _sceneFramebuffer; ///< Scene framebuffer
	std::shared_ptr<Scene> _scene; ///< The scene to render
	
	Program * _objectProgram; ///< Basic stencil program
	Program * _fillProgram; ///< Final screen filling.

};
