/*********************************
 * author     : mostafa khaled
 * date       : 
 * desc       : 
 ********************************/
#ifndef GAME_HPP
#define GAME_HPP

#include <string>

#include <GLFW/glfw3.h>

#include "camera.hpp"

class game 
{
  public:
    game(int width, int height, std::string title, bool fullscreen, bool debug_mode);
    bool init();
    void update();
    void render();
    void run();

  private:
    GLFWwindow* m_window;

    camera      m_cam;

    int         m_width, m_height;
    bool        m_fullscreen, m_debug_mode;
    std::string m_title;

};

void message_callback(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, char const* message, void const* user_param);

#endif /* !GAME_HPP */
