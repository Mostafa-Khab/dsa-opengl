/*********************************
 * author     : mostafa khaled
 * date       : 
 * desc       : 
 ********************************/
#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct camera
{
  camera();
  void reset();
  glm::mat4 mvp();
  void update_view_vectors();

  glm::mat4 model      ;
  glm::mat4 view       ;
  glm::mat4 projection ;
  glm::vec3 cameraPos  ;
  glm::vec3 targetPos  ;
  glm::vec3 upVector   ;
  float     z = 4;
  float     yradians = 0;
  float     xradians = 0;
};

#endif /* !CAMERA_HPP */
