#include "camera.hpp"

void camera::reset() {
  model      = glm::mat4(1.0);
  view       = glm::mat4(1.0);
  projection = glm::mat4(1.0);
}

camera::camera() {
  model      = glm::mat4(1.0);
  view       = glm::mat4(1.0);
  projection = glm::mat4(1.0);
  cameraPos  = glm::vec3(0.0f, 0.0f, z);
  targetPos  = glm::vec3(0.0f, 0.0f, 0.0f);
  upVector   = glm::vec3(0.0f, 1.0f, 0.0f);
}

void camera::update_view_vectors()
{
  cameraPos  = glm::vec3(0.0f, 0.0f, z);
  targetPos  = glm::vec3(xradians, yradians, 0.0f);
  upVector   = glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 camera::mvp()
{
  return model * projection * view ;
}
