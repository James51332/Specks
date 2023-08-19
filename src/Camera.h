#pragma once

#include <glm/glm.hpp>

namespace Speck
{

/// <summary>
/// Orthographic camera class to be used with particle shader.
/// 
/// TODO: Add a controller system that allows the user to move the camera with WASD, as well as the mouse.
/// </summary>
class Camera
{
public:
  Camera(float aspect, float size = 5.0f);
  // ~Camera();// No need to destroy any resources

  // Updates the camera based on user input
  void Update();
  
  void SetPosition(const glm::vec3& position);
  void SetRotation(float rotation);
  void SetOrthographicSize(float size);
  void SetAspect(float aspect);

  const glm::vec3& GetPosition() const { return m_Position; }
  float GetRotation() const { return m_Rotation; }
  float GetOrthographicSize() const { return m_OrthographicSize; }
  float GetAspect() const { return m_Aspect; }

  const glm::mat4& GetViewMatrix() const { return m_View; }
  const glm::mat4& GetProjectionMatrix() const { return m_Projection; }
  const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjection; }

private:
  void CalculateMatrices();
private:
  glm::vec3 m_Position = glm::vec3(0.0f);
  float m_Rotation = 0.0f;
  float m_OrthographicSize;
  float m_Aspect;

  glm::mat4 m_Projection;
  glm::mat4 m_View;
  glm::mat4 m_ViewProjection;
};

}
