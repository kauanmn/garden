#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

void Camera::computeProjectionMatrix(int width, int height) {
	m_projMatrix = glm::mat4(1.0f);
	const auto aspect{static_cast<float>(width) / static_cast<float>(height)};
	m_projMatrix = glm::perspective(glm::radians(70.0f), aspect, 0.1f, 5.0f);
}

void Camera::computeViewMatrix() {
	m_viewMatrix = glm::lookAt(m_eye, m_at, m_up);
}

void Camera::dolly(float speed) {
	// Compute forward vector (view direction)
	const glm::vec3 forward{glm::normalize(m_at - m_eye)};;

	// Move eye and center forward (speed > 0) or backward (speed < 0)
	m_eye.x += forward.x * speed;
	m_eye.z += forward.z * speed;
	m_at.x += forward.x * speed;
	m_at.z += forward.z * speed;

	computeViewMatrix();
}

void Camera::truck(float speed) {
	// compute forward vector (view direction)
	const glm::vec3 forward{glm::normalize(m_at - m_eye)};

	// compute vector to the left
	const glm::vec3 left{glm::cross(m_up, forward)};

	// move eye and center to the left (speed < 0) or to the right (speed > 0)
	m_at.x  -= left.x * speed;
	m_at.z  -= left.z * speed;
	m_eye.x -= left.x * speed;
	m_eye.z -= left.z * speed;

	computeViewMatrix();
}

void Camera::pan(float speed) {
	glm::mat4 transform{glm::mat4(1.0f)};

	// rotate camera around its local y axis
	transform = glm::translate(transform, m_eye);
	transform = glm::rotate(transform, -speed, glm::vec3(0.0f, 1.0f, 0.0f));
	transform = glm::translate(transform, -m_eye);

	m_at = transform * glm::vec4(m_at, 1.0f);

	computeViewMatrix();
}


// nova movimentação de câmera: tilt

void Camera::tilt(float speed) {
	glm::mat4 transform{glm::mat4(1.0f)};

	// rotate camera around its local y axis
	transform = glm::translate(transform, m_eye);
	transform = glm::rotate(transform, -speed, glm::vec3(1.0f, 0.0f, 0.0f));
	transform = glm::translate(transform, -m_eye);

	m_at = transform * glm::vec4(m_at, 1.0f);

	computeViewMatrix();
}