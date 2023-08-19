#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Speck
{

Camera::Camera(float aspect, float size)
  : m_Aspect(aspect), m_OrthographicSize(size)
{
  CalculateMatrices();
}

void Camera::SetPosition(const glm::vec3& position)
{
  m_Position = position;
  CalculateMatrices();
}

void Camera::SetRotation(float rotation)
{
  m_Rotation = rotation;
  CalculateMatrices();
}

void Camera::SetOrthographicSize(float size)
{
  m_OrthographicSize = size;
  CalculateMatrices();
}

void Camera::SetAspect(float aspect)
{
  m_Aspect = aspect;
  CalculateMatrices();
}

void Camera::CalculateMatrices()
{
  // Calculate the view matrix (translates from world space to camera space-inverse of camera's transform)
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position)
                      * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f)); // rotate and then translate
  
  m_View = glm::inverse(transform);

  // Calculate the projection matrix (use glm::ortho)
  float left, right, top, bottom;
  top = m_OrthographicSize;
  bottom = -m_OrthographicSize;
  left = -m_OrthographicSize * m_Aspect;
  right = m_OrthographicSize * m_Aspect;

  m_Projection = glm::ortho(left, right, top, bottom);

  // Cache the view projection matrix in a variable as well (first view then projection-translate then stretch)
  m_ViewProjection = m_Projection * m_View;
}

}