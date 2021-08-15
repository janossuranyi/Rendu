#pragma once

#include "scene/Animated.hpp"
#include "scene/Animation.hpp"
#include "resources/ResourcesManager.hpp"
#include "system/Codable.hpp"
#include "Common.hpp"

/**
 \brief Represent a 3D textured object.
 \ingroup Scene
 */
class Object {

public:
	/// \brief Type of shading/effects.
	enum Type : int {
		None = 0,  ///< Any type of shading.
		Regular,  ///< PBR shading. \see GPU::Vert::Object_gbuffer, GPU::Frag::Object_gbuffer
		Parallax, ///< PBR with parallax mapping. \see GPU::Vert::Object_parallax_gbuffer, GPU::Frag::Object_parallax_gbuffer
		Emissive,  	 ///< Emissive objects (no shading, pure emitter)
		Transparent, ///< Transparent object
	};

	/** Constructor */
	Object() = default;

	/** Construct a new object.
	 \param type the type of shading and effects to use when rendering this object
	 \param mesh the geometric mesh infos
	 \param castShadows denote if the object should cast shadows
	 */
	Object(Type type, const Mesh * mesh, bool castShadows);

	/** Register a texture.
	 \param infos the texture infos to add
	 */
	void addTexture(const Texture * infos);

	/** Add an animation to apply at each frame.
	 \param anim the animation to add
	 */
	void addAnimation(const std::shared_ptr<Animation> & anim);

	/** Update the object transformation matrix.
	 \param model the new model matrix
	 */
	void set(const glm::mat4 & model);

	/** Apply the animations for a frame duration.
	 \param fullTime the time since the launch of the application
	 \param frameTime the time elapsed since the last frame
	 */
	virtual void update(double fullTime, double frameTime);

	/** Query the bounding box of the object in world space.
	 \return the bounding box
	 \note For mesh space bounding box, call boundingBox on mesh().
	 */
	const BoundingBox & boundingBox() const;

	/** Mesh getter.
	 \return the mesh infos
	 */
	const Mesh * mesh() const { return _mesh; }

	/** Textures array getter.
	 \return a vector containing the infos of the textures associated to the object
	 */
	const std::vector<const Texture *> & textures() const { return _textures; }

	/** Object pose getter.
	 \return the model matrix
	 */
	const glm::mat4 & model() const { return _model; }

	/** Type getter.
	 \return the type of the object
	 \note This can be used in different way by different applications.
	 */
	const Type & type() const { return _material; }

	/** Is the object casting a shadow.
	 \return a boolean denoting if the object is a caster
	 */
	bool castsShadow() const { return _castShadow; }

	/** Are the object faces visible from both sides.
	 \return a boolean denoting if the faces have two sides
	 */
	bool twoSided() const { return _twoSided; }

	/** Should an alpha clip mask be applied when rendering the object (for leaves or chains for instance).
	 \return a boolean denoting if masking should be applied
	 */
	bool masked() const { return _masked; }

	/** Should the object use its texture coordinates (if they exist)
	 \return a boolean denoting if the UV should be used
	 */
	bool useTexCoords() const { return !_skipUVs; }

	/** Check if the object is moving over time.
	 \return a boolean denoting if animations are applied to the object
	 */
	bool animated() const { return !_animations.empty(); }

	/** Setup an object parameters from a list of key-value tuples. The following keywords will be searched for:
	 \verbatim
	 type: objecttype
	 mesh: meshname
	 translation: X,Y,Z
	 scaling: scale
	 orientation: axisX,axisY,axisZ angle
	 shadows: bool
	 twosided: bool
	 skipuvs: bool
	 masked: bool
	 textures:
	 	- texturetype: ...
	 	- ...
	 animations:
	 	- animationtype: ...
	 	- ...
	 \endverbatim
	 \param params the parameters tuple
	 \param options data loading and storage options
	 */
	virtual void decode(const KeyValues & params, Storage options);

	/** Generate a key-values representation of the object. See decode for the keywords and layout.
	\return a tuple representing the object.
	*/
	virtual KeyValues encode() const;
	
	/** Destructor.*/
	virtual ~Object() = default;

	/** Copy constructor.*/
	Object(const Object &) = delete;

	/** Copy assignment.
	 \return a reference to the object assigned to
	 */
	Object & operator=(const Object &) = delete;

	/** Move constructor.*/
	Object(Object &&) = default;

	/** Move assignment.
	 \return a reference to the object assigned to
	 */
	Object & operator=(Object &&) = default;

protected:
	const Mesh * _mesh = nullptr;						 ///< Geometry of the object.
	std::vector<const Texture *> _textures;				 ///< Textures used by the object.
	std::vector<std::shared_ptr<Animation>> _animations; ///< Animations list (applied in order).
	Animated<glm::mat4> _model { glm::mat4(1.0f) };		///< The transformation matrix of the 3D model, updated by the animations.
	mutable BoundingBox _bbox;							///< The world space object bounding box.
	Type _material   = Type::None;		 ///< The material type.
	bool _castShadow = true;			 ///< Can the object casts shadows.
	bool _twoSided   = false;			 ///< Should faces of the object be visible from the two sides.
	bool _masked	 = false;			 ///< The object RGB texture has a non-empty alpha channel.
	bool _skipUVs	 = false;			 ///< The object doesn't use UV coordinates.

	mutable bool _dirtyBbox  = true;	 ///< Has the bounding box been updated following an animation update.
};

STD_HASH(Object::Type);
