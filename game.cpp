#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <cstdio>

#include "game.hpp"

#define error(X) fprintf(stderr, "ERROR: %s\n", X);

game::game(int width, int height, std::string title, bool fullscreen, bool debug_mode)
  :m_width(width), m_height(height), m_title(title), m_fullscreen(fullscreen), m_debug_mode(debug_mode)
{
}

bool game::init()
{
  if(!glfwInit())
  {
    error("failed to init glfw");
    return false;
  }

  const GLFWvidmode* mode  = glfwGetVideoMode(glfwGetPrimaryMonitor());

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  if(m_fullscreen)
  {
    m_width  = mode->width;
    m_height = mode->height;
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), glfwGetPrimaryMonitor(), NULL);
  } else {
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);
  }

  if(!m_window)
  {
    error("failed to create glfw window");
    return false;
  }

  glfwMakeContextCurrent(m_window);

  int glad_version = gladLoadGL(glfwGetProcAddress);
  if(glad_version == 0)
  {
     fprintf(stderr, "ERROR: failed to load opengl context (glad)\n");
     return false;
  }

  if(m_debug_mode) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(message_callback, nullptr);
  }

  return m_window;
}

void game::update() {

}

void game::render() {

}

void game::run() {

}



//not in game scope
void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
{
	auto const src_str = [source]() {
		switch (source)
		{
		case GL_DEBUG_SOURCE_API: return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
		case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
		case GL_DEBUG_SOURCE_OTHER: return "OTHER";
		}
	}();

	auto const type_str = [type]() {
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR: return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER: return "MARKER";
		case GL_DEBUG_TYPE_OTHER: return "OTHER";
		}
	}();

	auto const severity_str = [severity]() {
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
		case GL_DEBUG_SEVERITY_LOW: return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
		}
	}();
	std::cout << src_str << ", " << type_str << ", " << severity_str << ", " << id << ": " << message << '\n';
}
