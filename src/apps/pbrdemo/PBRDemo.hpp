#pragma once

#include "DeferredRenderer.hpp"
#include "PostProcessStack.hpp"

#include "Application.hpp"
#include "scene/Scene.hpp"
#include "renderers/Renderer.hpp"
#include "input/ControllableCamera.hpp"

#include "Common.hpp"

/**
 \brief PBR rendering demonstration and interactions.
 \ingroup DeferredRendering
 */
class PBRDemo final : public CameraApp {

public:
	/** Constructor.
	 \param config the configuration to apply when setting up
	 */
	explicit PBRDemo(RenderingConfig & config);

	/** \copydoc CameraApp::draw */
	void draw() override;

	/** \copydoc CameraApp::update */
	void update() override;

	/** \copydoc CameraApp::physics */
	void physics(double fullTime, double frameTime) override;

	/** \copydoc CameraApp::clean */
	void clean() override;

	/** \copydoc CameraApp::resize */
	void resize() override;

private:
	
	/** Select the scene to display.
	 \param scene the scene to use
	 */
	void setScene(const std::shared_ptr<Scene> & scene);
	
	std::unique_ptr<DeferredRenderer> _renderer; ///< Active PBR renderer.
	std::unique_ptr<PostProcessStack> _postprocess; ///< Post-process renderer.
	const Program * _finalProgram; ///< Final display program.
	
	std::vector<std::shared_ptr<Scene>> _scenes; ///< The existing scenes.
	std::vector<std::string> _sceneNames; ///< The associated scene names.
	
	size_t _currentScene = 0; ///< Currently selected scene.
	glm::vec2 _cplanes  = glm::vec2(0.01f, 100.0f); ///< Camera clipping planes.
	float _cameraFOV	= 50.0f; ///< Camera field of view in degrees.
	bool _paused		= false; ///< Pause animations.
};
