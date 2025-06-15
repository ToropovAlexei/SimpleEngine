#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
  Camera();

  void setPosition(const glm::vec3 &position);
  void setDirection(const glm::vec3 &direction);
  void setUp(const glm::vec3 &up);

  const glm::vec3 &getPosition() const;
  const glm::vec3 &getDirection() const;
  const glm::vec3 &getUp() const;
  const glm::vec3 &getRight() const;

  void setPerspective(float fov, float aspect, float near, float far);

  const glm::mat4 &getViewMatrix();
  const glm::mat4 &getProjectionMatrix();
  const glm::mat4 getProjectionViewMatrix() const;

private:
  void updateViewMatrix();

private:
  glm::vec3 m_position = glm::vec3(0.0f);
  glm::vec3 m_direction = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 m_right = glm::vec3(1.0f, 0.0f, 0.0f);

  glm::mat4 m_viewMatrix = glm::mat4(1.0f);
  glm::mat4 m_projectionMatrix = glm::mat4(1.0f);
};