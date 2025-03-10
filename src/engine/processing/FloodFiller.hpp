#pragma once
#include "graphics/Framebuffer.hpp"
#include "resources/ResourcesManager.hpp"
#include "Common.hpp"

/**
 \brief Perform an approximate flood fill on the GPU, outputing a color filled image or a distance map.
 Implement the method described in Jump Flooding in GPU with Applications to Voronoi Diagram and Distance Transform, Rong et al., 2006.
 \ingroup Processing
 */
class FloodFiller {

public:
	/** \brief Output mode: either the color of the input seeds propagated, or the normalized distance to the closest seed at each pixel.
	 */
	enum class Output {
		COLOR,
		DISTANCE
	};

	/** Constructor.
	 \param width internal processing width
	 \param height internal processing height
	 */
	FloodFiller(unsigned int width, unsigned int height);

	/** Fill a given input texture.
	 \param texture the GPU ID of the texture
	 \param mode the output mode (color or distance)
	 */
	void process(const Texture * texture, Output mode);

	/** Resize the internal buffers.
	 \param width the new width
	 \param height the new height
	 */
	void resize(unsigned int width, unsigned int height);

	/** The GPU ID of the filter result.
	 \return the ID of the result texture
	 */
	const Texture * texture() const { return _final->texture(); }

private:
	/** Extract seeds from the input texture and propagate them so that each pixel contains the coordinates of the closest seed (approximately). The result will be stored in _ping.
	 \param texture the input texture
	 */
	void extractAndPropagate(const Texture * texture);

	Program * _extract;		 ///< Extract the flood fill seeds.
	Program * _floodfill;		 ///< Perform one pass of the flood fill.
	Program * _compositeColor; ///< Generate the color image from the flood-fill seed map.
	Program * _compositeDist;  ///< Generate the normalized distance map from the flood-fill seed map.

	std::unique_ptr<Framebuffer> _ping;  ///< First flooding buffer.
	std::unique_ptr<Framebuffer> _pong;  ///< Second flooding buffer.
	std::unique_ptr<Framebuffer> _final; ///< Buffer containing the result.

	int _iterations; ///< Number of iterations to perform (derived from input size).
};
