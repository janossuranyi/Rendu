#ifndef TestRenderer_h
#define TestRenderer_h
#include "../../Common.hpp"
#include "../../Framebuffer.hpp"
#include "../../input/Camera.hpp"
#include "../../ScreenQuad.hpp"

#include "../../processing/GaussianBlur.hpp"
#include "../../processing/BoxBlur.hpp"

#include "../Renderer.hpp"

#include "../deferred/Gbuffer.hpp"
#include "../deferred/AmbientQuad.hpp"



class TestRenderer : public Renderer {

public:

	~TestRenderer();

	/// Init function
	TestRenderer(Config & config);

	/// Draw function
	void draw();
	
	void update();
	
	void physics(double fullTime, double frameTime);

	/// Clean function
	void clean() const;

	/// Handle screen resizing
	void resize(unsigned int width, int unsigned height);
	
	
private:
	
	Camera _camera;

	std::shared_ptr<Framebuffer> _framebuffer;
	std::shared_ptr<ProgramInfos> _program;
	
};

#endif