#include "PathTracerApp.hpp"
#include "input/Input.hpp"
#include "system/System.hpp"
#include "graphics/GLUtilities.hpp"
#include "graphics/ScreenQuad.hpp"
#include "resources/Texture.hpp"

PathTracerApp::PathTracerApp(RenderingConfig & config, const std::shared_ptr<Scene> & scene) : CameraApp(config) {
	
	const glm::vec2 renderRes = _config.renderingResolution();
	_bvhRenderer.reset(new BVHRenderer(renderRes));
	_passthrough = Resources::manager().getProgram2D("passthrough");
	
	// Initial setup for rendering image.
	_renderTex.shape  = TextureShape::D2;
	_renderTex.levels = 1;
	_renderTex.depth  = 1;
	_renderTex.width  = int(renderRes[0]);
	_renderTex.height = int(renderRes[1]);
	GLUtilities::setupTexture(_renderTex, {Layout::SRGB8, Filter::LINEAR, Wrap::CLAMP});
	checkGLError();
	
	_scene = scene;
	if(!scene) {
		return;
	}
	// Camera setup.
	_userCamera.apply(_scene->viewpoint());
	const BoundingBox & bbox = _scene->boundingBox();
	const float range		 = glm::length(bbox.getSize());
	_userCamera.frustum(0.01f * range, 5.0f * range);
	_userCamera.speed() = 0.2f * range;
	_userCamera.ratio(config.screenResolution[0] / config.screenResolution[1]);
	_cameraFOV			= _userCamera.fov() * 180.0f / glm::pi<float>();
	
	// Create the path tracer and raycaster.
	_pathTracer.reset(new PathTracer(_scene));
	// Setup the renderer data.
	_bvhRenderer->setScene(_scene, _pathTracer->raycaster());
	
}

void PathTracerApp::draw() {

	// If no scene, just clear.
	if(!_scene) {
		GLUtilities::clearColorAndDepth({0.2f, 0.2f, 0.2f, 1.0f}, 1.0f);
		return;
	}

	// Directly render the result texture without drawing the scene.
	if(_showRender) {
		glEnable(GL_FRAMEBUFFER_SRGB);
		GLUtilities::setViewport(0, 0, int(_config.screenResolution[0]), int(_config.screenResolution[1]));
		_passthrough->use();
		_passthrough->uniform("flip", 1);
		ScreenQuad::draw(_renderTex);
		glDisable(GL_FRAMEBUFFER_SRGB);
		return;
	}
	
	// Draw the real time visualization.
	_bvhRenderer->draw(_userCamera);
	// We now render a full screen quad in the default framebuffer, using sRGB space.
	glEnable(GL_FRAMEBUFFER_SRGB);
	GLUtilities::setViewport(0, 0, int(_config.screenResolution[0]), int(_config.screenResolution[1]));
	_passthrough->use();
	_passthrough->uniform("flip", 0);
	ScreenQuad::draw(_bvhRenderer->result());
	glDisable(GL_FRAMEBUFFER_SRGB);
}

void PathTracerApp::update() {
	CameraApp::update();
	
	// If no scene, no need to udpate the camera or the scene-specific UI.
	if(!_scene) {
		return;
	}

	if(ImGui::Begin("Path tracer")) {

		ImGui::Text("Rendering size: %d x %d", _renderTex.width, _renderTex.height);

		// Tracing options
		ImGui::PushItemWidth(100);
		if(ImGui::InputInt("Samples", &_samples, 1, 4)) {
			_samples = std::max(1, _samples);
		}
		if(ImGui::InputInt("Depth", &_depth, 1, 2)) {
			_depth = std::max(1, _depth);
		}
		if(ImGui::InputScalar("Output height", ImGuiDataType_U32, static_cast<void *>(&_renderTex.height))) {
			_renderTex.height = std::max(uint(1), _renderTex.height);
			_renderTex.width  = uint(std::round(_config.screenResolution[0] / _config.screenResolution[1] * float(_renderTex.height)));
		}
		ImGui::PopItemWidth();

		// Perform rendering.
		if(ImGui::Button("Render")) {
			// Clean the texture.
			_renderTex.clean();
			GLUtilities::setupTexture(_renderTex, {Layout::SRGB8, Filter::LINEAR, Wrap::CLAMP});
			// Render.
			_renderTex.images.emplace_back(_renderTex.width, _renderTex.height, 3);
			Image & render = _renderTex.images.back();
			_pathTracer->render(_userCamera, _samples, _depth, render);
			// Upload to the GPU.
			GLUtilities::uploadTexture(_renderTex);
			_showRender = true;
		}
		ImGui::SameLine();
		// Save the render to disk.
		const bool hasImage = !_renderTex.images.empty();
		if(hasImage && ImGui::Button("Save...")) {
			std::string outPath;
			if(System::showPicker(System::Picker::Save, "", outPath) && !outPath.empty()) {
				_renderTex.images[0].save(outPath, false);
			}
		}

		ImGui::Checkbox("Show rendered image", &_showRender);
		if(!_showRender) {
			// Mesh and BVH display.
			ImGui::Separator();
			ImGui::Checkbox("Show BVH", &_bvhRenderer->showBVH());
			ImGui::SameLine();
			
			int & minRange = _bvhRenderer->range()[0];
			int & maxRange = _bvhRenderer->range()[1];
			// Keep both ends of the range equal.
			if(ImGui::Checkbox("Lock", &_lockLevel)) {
				maxRange = minRange;
			}
			// Display a subset of the BVH.
			const int maxLevel = _bvhRenderer->maxLevel();
			const bool mod1	= ImGui::SliderInt("Range min.", &minRange, 0, maxLevel);
			const bool mod2	= ImGui::SliderInt("Range max.", &maxRange, 0, maxLevel);
			if(mod1 || mod2) {
				// Enforce synchronisation.
				maxRange = glm::clamp(maxRange, minRange, maxLevel);
				minRange = glm::clamp(minRange, 0, maxRange);
				if(_lockLevel) {
					maxRange = minRange;
				}
			}
		}

		if(Input::manager().released(Input::Mouse::Left) && Input::manager().pressed(Input::Key::Space)) {
			const glm::vec2 position = Input::manager().mouse();
			// Compute incremental pixel shifts.
			glm::vec3 corner, dx, dy;
			_userCamera.pixelShifts(corner, dx, dy);
			const glm::vec3 worldPos = corner + position.x * dx + position.y * dy;
			const glm::vec3 rayPos   = _userCamera.position();
			const glm::vec3 rayDir   = glm::normalize(worldPos - rayPos);
			_bvhRenderer->castRay(rayPos, rayDir);
		}

		if(ImGui::Button("Clear ray")) {
			_bvhRenderer->clearRay();
		}

		// Camera settings.
		if(ImGui::CollapsingHeader("Camera settings")) {
			ImGui::PushItemWidth(100);
			ImGui::Combo("Camera mode", reinterpret_cast<int *>(&_userCamera.mode()), "FPS\0Turntable\0Joystick\0\0", 3);
			ImGui::InputFloat("Camera speed", &_userCamera.speed(), 0.1f, 1.0f);
			if(ImGui::InputFloat("Camera FOV", &_cameraFOV, 1.0f, 10.0f)) {
				_userCamera.fov(_cameraFOV * glm::pi<float>() / 180.0f);
			}
			ImGui::PopItemWidth();

			// Copy/paste camera to clipboard.
			if(ImGui::Button("Copy camera")) {
				const std::string camDesc = _userCamera.encode();
				ImGui::SetClipboardText(camDesc.c_str());
			}
			ImGui::SameLine();
			if(ImGui::Button("Paste camera")) {
				const std::string camDesc(ImGui::GetClipboardText());
				const auto cameraCode = Codable::parse(camDesc);
				if(!cameraCode.empty()) {
					_userCamera.decode(cameraCode[0]);
					_cameraFOV = _userCamera.fov() * 180.0f / glm::pi<float>();
				}
			}
			// Reset to the scene reference viewpoint.
			if(ImGui::Button("Reset")) {
				_userCamera.apply(_scene->viewpoint());
				_userCamera.ratio(_config.screenResolution[0] / _config.screenResolution[1]);
				_cameraFOV = _userCamera.fov() * 180.0f / glm::pi<float>();
			}
		}
	}
	ImGui::End();
}

void PathTracerApp::physics(double, double) {
	// If there is any interaction, exit the 'show render' mode.
	if(Input::manager().interacted()) {
		_showRender = false;
	}
}

void PathTracerApp::clean() {
	_bvhRenderer->clean();
	_renderTex.clean();
}

void PathTracerApp::resize() {
	// Same aspect ratio as the display resolution
	const glm::vec2 renderRes = _config.renderingResolution();
	// Resize the renderer.
	_bvhRenderer->resize(renderRes[0], renderRes[1]);
	// Udpate the image resolution, using the new aspect ratio.
	_renderTex.width = uint(std::round(_config.screenResolution[0] / _config.screenResolution[1] * float(_renderTex.height)));
	checkGLError();
}
