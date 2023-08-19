#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <SDL.h>

#include "Input.h"

namespace Speck
{

Camera::Camera(float aspect, float size)
  : m_Aspect(aspect), m_OrthographicSize(size)
{
  CalculateMatrices();
}

void Camera::Update()
{
  // Calculate the time in seconds since the last time we updated the camera
  static float lastTime = SDL_GetTicks();
  float curTime = SDL_GetTicks();
  float timestep = (curTime - lastTime) * 0.001f;
  lastTime = curTime;
  
  // Store whether we've moved so we don't wastefully calculate matrices
  bool shouldUpdateMatrices = false;
  
  // Handle moving with WASD
  float moveSpeed = m_OrthographicSize * 1.5f;
  if (Input::KeyDown(SDL_SCANCODE_W))
  {
    m_Position.y += moveSpeed * timestep;
    shouldUpdateMatrices = true;
  }
  if (Input::KeyDown(SDL_SCANCODE_A))
  {
    m_Position.x -= moveSpeed * timestep;
    shouldUpdateMatrices = true;
  }
  if (Input::KeyDown(SDL_SCANCODE_S))
  {
    m_Position.y -= moveSpeed * timestep;
    shouldUpdateMatrices = true;
  }
  if (Input::KeyDown(SDL_SCANCODE_D))
  {
    m_Position.x += moveSpeed * timestep;
    shouldUpdateMatrices = true;
  }
  
  // Handle rotation
  float rotationSpeed = 200.0f;
  if (Input::KeyDown(SDL_SCANCODE_LEFT))
  {
    m_Rotation += rotationSpeed * timestep;
    shouldUpdateMatrices = true;
  }
  if (Input::KeyDown(SDL_SCANCODE_RIGHT))
  {
    m_Rotation -= rotationSpeed * timestep;
    shouldUpdateMatrices = true;
  }
  
  // Recalculate matrices as needed
  if (shouldUpdateMatrices)
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

  m_Projection = glm::ortho(left, right, bottom, top);

  // Cache the view projection matrix in a variable as well (first view then projection-translate then stretch)
  m_ViewProjection = m_Projection * m_View;
}

}
