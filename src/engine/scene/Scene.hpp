#pragma once

#include "scene/Object.hpp"
#include "scene/Material.hpp"
#include "scene/LightProbe.hpp"
#include "lights/Light.hpp"

#include "resources/ResourcesManager.hpp"
#include "input/Camera.hpp"
#include "system/Codable.hpp"

#include "Common.hpp"

/**
 \brief Represents a 3D environment composed of objects, a background and additional environment lighting informations, along with serialization support.
 \ingroup Scene
 */
class Scene {

public:
	
	/** Constructor from a scene description file.
	 \param name the name of the scene description file
	 */
	explicit Scene(const std::string & name);

	/** Performs initialization against the graphics API, loading data.
	 \param options data loading and storage options.
	 \return the initialization success
	 */
	bool init(Storage options);

	/** Update the animations in the scene.
	 \param fullTime the time elapsed since the beginning of the render loop
	 \param frameTime the duration of the last frame
	 */
	void update(double fullTime, double frameTime);

	/** Convert a scene to a Codable-compliant list of key-values tuples. Can be used for serialization.
	 \return a list of tuples
	 */
	std::vector<KeyValues> encode() const;
	
	/** Get the scene bounding box.
	 \return the bounding box
	 */
	const BoundingBox & boundingBox() const { return _bbox; }

	/** Get the initial viewpoint on the scene.
	 \return the viewpoint camera
	 */
	const Camera & viewpoint() const { return _camera; }
	
	/** Set the initial viewpoint on the scene.
	 \param cam the new camera
	 */
	void setViewpoint(const Camera & cam) { _camera = cam; }

	/** \return true if the scene contains animations */
	bool animated() const { return _animated; }

	/** \return true if the scene contains transparent objects */
	bool transparent() const { return _transparent; }

	std::vector<Object> objects;				///< The objects in the scene.
	std::vector<Material> materials;			///< The materials in the scene.
	std::vector<std::shared_ptr<Light>> lights; ///< Lights present in the scene.

	/** \brief The background mode to use for a scene. */
	enum class Background {
		COLOR,	 ///< Use a unique color as background.
		IMAGE,	 ///< Use a 2D texture image as background (will be stretched).
		SKYBOX,	///< Use a skybox/cubemap as background.
		ATMOSPHERE ///< Use a realtime atmospheric scattering simulation.
	};
	Background backgroundMode = Background::COLOR; ///< The background mode (see enum).
	glm::vec3 backgroundColor = glm::vec3(0.0f);   ///< Color to use if the background mode is COLOR.
	std::unique_ptr<Object> background;			   ///< Background object, containing the geometry  to use.
	LightProbe environment;						   ///< Reflection probe.
	
	/** Copy constructor.*/
	Scene(const Scene &) = delete;
	
	/** Copy assignment.
	 \return a reference to the object assigned to
	 */
	Scene & operator=(const Scene &) = delete;
	
	/** Move constructor.*/
	Scene(Scene &&) = delete;
	
	/** Move assignment.
	 \return a reference to the object assigned to
	 */
	Scene & operator=(Scene &&) = delete;
	
private:
	/** Load an object in the scene from its serialized representation.
	 \param params the object parameters
	 \param options data loading and storage options
	 */
	void loadObject(const KeyValues & params, Storage options);

	/** Load a material in the scene from its serialized representation.
	 \param params the material parameters
	 \param options data loading and storage options
	 */
	void loadMaterial(const KeyValues & params, Storage options);

	/** Load a point light in the scene from its serialized representation.
	 \param params the point light parameters
	 \param options data loading and storage options (unused)
	 */
	void loadLight(const KeyValues & params, Storage options);

	/** Load the scene camera informations from its serialized representation.
	 \param params the camera parameters
	\param options data loading and storage options (unused)
	 */
	void loadCamera(const KeyValues & params, Storage options);

	/** Load the scene background informations from its serialized representation.
	 \param params the background parameters
	 \param options data loading and storage options
	 */
	void loadBackground(const KeyValues & params, Storage options);

	/** Load the scene environment informations from its serialized representation.
	 \param params the probe parameters
	 \param options data loading and storage options
	 */
	void loadProbe(const KeyValues & params, Storage options);

	/** Load the scene informations from its serialized representation.
	 \param params the scene parameters
	 \param options data loading and storage options
	 */
	void loadScene(const KeyValues & params, Storage options);

	/** Compute the bounding box of the scene, optionaly excluding objects that do not cast shadows.
	 \param onlyShadowCasters denote if only objects that are allowed to cast shadows should be taken into account
	 \return the scene bounding box
	 */
	BoundingBox computeBoundingBox(bool onlyShadowCasters = false);

	Material _backgroundMaterial;  			 ///< Background material, containing the optional textures to use.
	Camera _camera;							 ///< The initial viewpoint on the scene.
	BoundingBox _bbox;						 ///< The scene bounding box.
	glm::mat4 _sceneModel = glm::mat4(1.0f); ///< The scene global transformation.
	std::string _name;						 ///< The scene file name.
	bool _loaded = false;					 ///< Has the scene already been loaded from disk.
	bool _animated = false;					 ///< Is the scene using animations.
	bool _transparent = false;				 ///< Is the scene containing transparent objects.
};
