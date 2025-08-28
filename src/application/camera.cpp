#include "camera.hpp"

Camera::Camera() { updateViewMatrix(); }

void Camera::setPosition(const glm::vec3 &position) {
  m_position = position;
  updateViewMatrix();
}

void Camera::setDirection(const glm::vec3 &direction) {
  m_direction = glm::normalize(direction);
  m_right = glm::normalize(glm::cross(m_direction, glm::vec3(0.0F, 1.0F, 0.0F)));
  m_up = glm::normalize(glm::cross(m_right, m_direction));
  updateViewMatrix();
}

void Camera::setUp(const glm::vec3 &up) {
  m_up = up;
  updateViewMatrix();
}

const glm::vec3 &Camera::getPosition() const { return m_position; }
const glm::vec3 &Camera::getDirection() const { return m_direction; }
const glm::vec3 &Camera::getUp() const { return m_up; }
const glm::vec3 &Camera::getRight() const { return m_right; }

void Camera::setPerspective(float fov, float aspect, float near, float far) {
  m_projectionMatrix = glm::perspective(glm::radians(fov), aspect, near, far);
}

const glm::mat4 &Camera::getViewMatrix() { return m_viewMatrix; }

const glm::mat4 &Camera::getProjectionMatrix() { return m_projectionMatrix; }

const glm::mat4 Camera::getProjectionViewMatrix() const { return m_projectionMatrix * m_viewMatrix; }

void Camera::updateViewMatrix() { m_viewMatrix = glm::lookAt(m_position, m_position + m_direction, m_up); }
