#pragma once

#include "system/Codable.hpp"
#include "Common.hpp"

/** \brief An animation is a transformation evaluated at each frame and applied to an object.
 	\ingroup Scene
 */
class Animation {
public:
	/** \brief Frame in which the transformation shoud be applied. */
	enum class Frame {
		MODEL, ///< Model space (right multiplication)
		WORLD  ///< World space (left multiplication)
	};

	/** Apply the animation transformation to the input matrix.
	 \param m the matrix to transform
	 \param fullTime the time elapsed since the beginning of the rendering loop
	 \param frameTime the time elapsed since last frame
	 \return the transformed matrix.
	 */
	virtual glm::mat4 apply(const glm::mat4 & m, double fullTime, double frameTime) = 0;

	/** Apply the animation transformation to the input vector.
	 \param v the vector to transform
	 \param fullTime the time elapsed since the beginning of the rendering loop
	 \param frameTime the time elapsed since last frame
	 \return the transformed matrix.
	 */
	virtual glm::vec4 apply(const glm::vec4 & v, double fullTime, double frameTime) = 0;

	/** Generate a key-values representation of the animation. See decode for the keywords and layout.
	 \return a tuple representing the animation.
	 */
	virtual KeyValues encode() const;
	
	/** Virtual destructor. */
	virtual ~Animation() = default;

	/** Copy constructor.*/
	Animation(const Animation &) = delete;

	/** Copy assignment.
	 \return a reference to the object assigned to
	 */
	Animation & operator=(const Animation &) = delete;

	/** Move constructor.*/
	Animation(Animation &&) = delete;

	/** Move assignment.
	 \return a reference to the object assigned to
	 */
	Animation & operator=(Animation &&) = delete;

	/** Helper that can instantiate a list of animations of any type from the passed keywords and parameters.
	 \param params a list of key-value tuple containing animations parameters
	 \return a vector of animations
	 */
	static std::vector<std::shared_ptr<Animation>> decode(const std::vector<KeyValues> & params);
	
	/** Helper that can instantiate a list of key-values tuples that are Codable-compatible from the passed animations.
	\param anims a vector of animations
	\return a list of key-value tuple containing animations parameters
	*/
	static std::vector<KeyValues> encode(const std::vector<std::shared_ptr<Animation>> & anims);
	
protected:
	
	/** Constructor. */
	Animation() = default;
	
	/** Constructor.
	 \param frame the frame in which the transformation is expressed
	 \param speed the speed of the animation
	 */
	Animation(Frame frame, float speed);

	/** Setup shared animation parameters from a key-value tuple. The expected format is as follows:
	 \verbatim
	 animationtype: speed frame ...
	 \endverbatim
	 (where frame is one of 'world' or 'model').
	 \param params the parameters tuple
	 \return decoding status
	 */
	bool decodeBase(const KeyValues & params);
	
	Frame _frame = Frame::WORLD; ///< The frame of transformation.
	float _speed = 0.0f;		 ///< Speed of the animation.
};

/** \brief Rotate an object around an axis.
 	\ingroup Scene
 */
class Rotation final : public Animation {
public:
	/** Default constructor. */
	Rotation() = default;

	/** Setup a rotation animation.
	 \param axis the rotation axis
	 \param speed the animation speed
	 \param frame the animation frame
	 */
	Rotation(const glm::vec3 & axis, float speed, Frame frame);

	/** Apply the animation transformation to the input matrix.
	 \param m the matrix to transform
	 \param fullTime the time elapsed since the beginning of the rendering loop
	 \param frameTime the time elapsed since last frame
	 \return the transformed matrix.
	 */
	glm::mat4 apply(const glm::mat4 & m, double fullTime, double frameTime) override;

	/** Apply the animation transformation to the input vector.
	 \param v the vector to transform
	 \param fullTime the time elapsed since the beginning of the rendering loop
	 \param frameTime the time elapsed since last frame
	 \return the transformed matrix.
	 */
	glm::vec4 apply(const glm::vec4 & v, double fullTime, double frameTime) override;

	/** Setup rotation animation parameters from a key-value tuple. The expected format is as follows:
	 \verbatim
	 rotation: speed frame axisX,axisY,axisZ
	 \endverbatim
	 (where frame is one of 'world' or 'model').
	 \param params the parameters tuple
	 \return decoding status
	 */
	bool decode(const KeyValues & params);

	/** Generate a key-values representation of the animation. See decode for the keywords and layout.
	\return a tuple representing the animation.
	*/
	KeyValues encode() const override;
	
private:
	
	glm::vec3 _axis = glm::vec3(1.0f, 0.0f, 0.0f); ///< Rotation axis.
};

/** \brief Translate an object back and forth along a direction.
 	\ingroup Scene
 */
class BackAndForth final : public Animation {
public:
	/** Default constructor. */
	BackAndForth() = default;

	/** Setup a back and forth animation.
	 \param axis the translation direction
	 \param speed the animation speed
	 \param amplitude the amplitude of the movement
	 \param frame the animation frame
	 */
	BackAndForth(const glm::vec3 & axis, float speed, float amplitude, Frame frame);

	/** Apply the animation transformation to the input matrix.
	 \param m the matrix to transform
	 \param fullTime the time elapsed since the beginning of the rendering loop
	 \param frameTime the time elapsed since last frame
	 \return the transformed matrix.
	 */
	glm::mat4 apply(const glm::mat4 & m, double fullTime, double frameTime) override;

	/** Apply the animation transformation to the input vector.
	 \param v the vector to transform
	 \param fullTime the time elapsed since the beginning of the rendering loop
	 \param frameTime the time elapsed since last frame
	 \return the transformed matrix.
	 */
	glm::vec4 apply(const glm::vec4 & v, double fullTime, double frameTime) override;

	/** Setup back-and-forth translation animation parameters from a key-value tuple. The expected format is as follows:
	 \verbatim
	 backandforth: speed frame axisX,axisY,axisZ amplitude
	 \endverbatim
	 (where frame is one of 'world' or 'model').
	 \param params the parameters tuple
	 \return decoding status	 
	 */
	bool decode(const KeyValues & params);

	/** Generate a key-values representation of the animation. See decode for the keywords and layout.
	\return a tuple representing the animation.
	*/
	KeyValues encode() const override;
	
private:
		
	glm::vec3 _axis			 = glm::vec3(1.0f, 0.0f, 0.0f); ///< Translation direction.
	float _amplitude		 = 0.0f;						///< Amplitude of the translation (maximum distance).
	double _previousAbscisse = 0.0f;						///< Position on the path at the previous frame.
};
