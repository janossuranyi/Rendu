#include "DeferredRenderer.hpp"
#include "helpers/Random.hpp"
#include "input/Input.hpp"
#include "helpers/Interface.hpp"
#include "scene/Scene.hpp"
#include "Common.hpp"

/**
 \defgroup DeferredRendering Deferred rendering
 \brief Render a scene using a PBR approach, image-based ambient lighting, ambient occlusion, HDR and tonemapping, antialiasing, and deferred rendering.
 \details All scene informations (albedo, normals, material ID, roughness) are rendered to a G-Buffer before being used to render each light contribution using simple geometric proxies. The scene is rendered using the GGX BRDF model.
 \see GLSL::Frag::Bloom, GLSL::Frag::Tonemap, GLSL::Frag::Fxaa, GLSL::Frag::Final_screenquad, GLSL::Frag::Ambient, GLSL::Frag::Ssao, GLSL::Frag::Point_light, GLSL::Frag::Directional_light, GLSL::Frag::Spot_light
 \ingroup Applications
 */

/**
 The main function of the physically-based rendering demo. Handles the setup and main loop.
 \param argc the number of input arguments.
 \param argv a pointer to the raw input arguments.
 \return a general error code.
 \ingroup DeferredRendering
 */
int main(int argc, char** argv) {
	
	// First, init/parse/load configuration.
	RenderingConfig config(std::vector<std::string>(argv, argv+argc));
	
	GLFWwindow* window = Interface::initWindow("PBR demo", config);
	if(!window){
		return -1;
	}
	
	Resources::manager().addResources("../../../resources/pbrdemo");
	// Initialize random generator;
	Random::seed();
	// Query the renderer identifier, and the supported OpenGL version.
	const GLubyte* rendererString = glGetString(GL_RENDERER);
	const GLubyte* versionString = glGetString(GL_VERSION);
	Log::Info() << Log::OpenGL << "Internal renderer: " << rendererString << "." << std::endl;
	Log::Info() << Log::OpenGL << "Version supported: " << versionString << "." << std::endl;
	
	
	
	// Create the renderer.
	std::unique_ptr<DeferredRenderer> renderer(new DeferredRenderer(config));
	
	double timer = glfwGetTime();
	double fullTime = 0.0;
	double remainingTime = 0.0;
	const double dt = 1.0/120.0; // Small physics timestep.
	
	
	std::map<std::string, std::string> sceneInfos;
	Resources::manager().getFiles("scene", sceneInfos);
	std::vector<std::string> sceneNames;
	sceneNames.push_back("None");
	for(const auto & info : sceneInfos){
		sceneNames.push_back(info.first);
	}
	
	std::vector<std::shared_ptr<Scene>> scenes;
	scenes.push_back(nullptr);
	for(int i = 1; i < sceneNames.size(); ++i){
		const auto & sceneName = sceneNames[i];
		scenes.emplace_back(new Scene(sceneName));
	}
	
	
	// Load the first scene by default.
	int selected_scene = 0;
	renderer->setScene(scenes[selected_scene]);
	
	// Start the display/interaction loop.
	while (!glfwWindowShouldClose(window)) {
		// Update events (inputs,...).
		Input::manager().update();
		// Handle quitting.
		if(Input::manager().pressed(Input::KeyEscape)){
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		// Reload resources.
		if(Input::manager().triggered(Input::KeyP)){
			Resources::manager().reload();
			// Reload the scene.
			if(scenes[selected_scene]){
				scenes[selected_scene].reset(new Scene(sceneNames[selected_scene]));
				renderer->setScene(scenes[selected_scene]);
			}
		}
		
		
		// Start a new frame for the interface.
		Interface::beginFrame();
		
		// Handle scene switching.
		if(ImGui::Begin("Renderer")){
			ImGui::Text("%.1f ms, %.1f fps", ImGui::GetIO().DeltaTime*1000.0f, ImGui::GetIO().Framerate);
			
			const std::string & currentName = sceneNames[selected_scene];
			if(ImGui::BeginCombo("Scene", currentName.c_str(), ImGuiComboFlags_None)){
				for(int i = 0; i < sceneNames.size(); ++i){
					if(ImGui::Selectable(sceneNames[i].c_str(), (i == selected_scene))) {
						selected_scene = i;
						Log::Info() << Log::Resources << "Loading scene " << sceneNames[selected_scene] << "." << std::endl;
						renderer->setScene(scenes[selected_scene]);
						
					}
					if(selected_scene == i){
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			ImGui::Separator();
		}
		ImGui::End();
		
		// We separate punctual events from the main physics/movement update loop.
		renderer->update();
		
		// Compute the time elapsed since last frame
		double currentTime = glfwGetTime();
		double frameTime = currentTime - timer;
		timer = currentTime;
		
		// Physics simulation
		// First avoid super high frametime by clamping.
		if(frameTime > 0.2){ frameTime = 0.2; }
		// Accumulate new frame time.
		remainingTime += frameTime;
		// Instead of bounding at dt, we lower our requirement (1 order of magnitude).
		while(remainingTime > 0.2*dt){
			double deltaTime = fmin(remainingTime, dt);
			// Update physics and camera.
			renderer->physics(fullTime, deltaTime);
			// Update timers.
			fullTime += deltaTime;
			remainingTime -= deltaTime;
		}
		
		// Update the content of the window.
		renderer->draw();
		// Then render the interface.
		Interface::endFrame();
		//Display the result for the current rendering loop.
		glfwSwapBuffers(window);

	}
	
	// Clean the interface.
	Interface::clean();
	// Clean other resources
	renderer->clean();
	Resources::manager().clean();
	// Close GL context and any other GLFW resources.
	glfwDestroyWindow(window);
	glfwTerminate();
	
	return 0;
}


