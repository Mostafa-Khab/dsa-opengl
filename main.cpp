#include "glm/ext/matrix_transform.hpp"
#include <algorithm>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

extern "C" {
  #include <image.h>
}

#define error(X) fprintf(stderr, "ERROR: %s\n", X)

int window_width  = 800;
int window_height = 800;

bool loadfile(std::string filepath, std::string& src);

void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param);

char elog[2048];
bool check_shader_program_linkage(unsigned int id);
void check_shader_compilation(unsigned int id);

float lerp(float a, float b, float t);
float smoothstep(float x);

unsigned int create_shader_program(std::string vshader_file, std::string fshader_file);
GLFWwindow* create_opengl_context(int width, int height, bool fullscreen, bool enable_debug);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

struct vertex
{
  float x, y, z;
  float r, g, b, a;
  float u, v;
};

vertex points[] =
{
 -0.5, 0.5, 0.0,   1.0, 0.0, 0.0, 1.0,   0.0, 0.0,
  0.5, 0.5, 0.0,   1.0, 0.0, 0.0, 1.0,   0.5, 0.0,
 -0.5,-0.5, 0.0,   0.0, 1.0, 0.0, 1.0,   0.0, 1.0,
  0.5,-0.5, 0.0,   0.0, 0.0, 1.0, 1.0,   0.5, 1.0,
};

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

camera cam;

int main(int argc, const char* argv[])
{
  GLFWwindow* window = create_opengl_context(window_width, window_height, (argc > 1), true);

  if(!window)
  {
    error("failed to create glfw window, terminating glfw");
    glfwTerminate();
    return -1;
  }

  glfwSetScrollCallback(window, scroll_callback);

  unsigned int prg = create_shader_program("./shaders/shader.vert", "./shaders/shader.frag");

  int mvp_loc = glGetUniformLocation(prg, "u_mvp");
  int res_loc = glGetUniformLocation(prg, "u_resolution");
  int u_time  = glGetUniformLocation(prg, "u_time");
  int u_tex0  = glGetUniformLocation(prg, "u_tex0");
  int u_tex1  = glGetUniformLocation(prg, "u_tex1");

  unsigned int buffer;
  glCreateBuffers(1, &buffer);
  glNamedBufferData(buffer, sizeof(vertex) * 4, points, GL_STATIC_DRAW);
  //glNamedBufferStorage(vbo, sizeof(vertex)*vertex_count, vertices, GL_DYNAMIC_STORAGE_BIT);

  unsigned int vao;
  glCreateVertexArrays(1, &vao);

  glVertexArrayVertexBuffer(vao, 0, buffer, 0, sizeof(vertex));

  glEnableVertexArrayAttrib(vao, 0);
  glEnableVertexArrayAttrib(vao, 1);
  glEnableVertexArrayAttrib(vao, 2);

  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribFormat(vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(vertex, r));
  glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(vertex, u));

  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribBinding(vao, 1, 0);
  glVertexArrayAttribBinding(vao, 2, 0);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  Image img;
  Image_load(&img, "./bird64.png");

  Image bg_img;
  Image_load(&bg_img, "./background.png");

  if(!img.data || !bg_img.data)
  {
    error("failed to load bird64.png or bg_img");
  }

  unsigned int texture;
  glCreateTextures(GL_TEXTURE_2D, 1, &texture);
  glBindTextureUnit(0, texture);

  glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTextureStorage2D(texture, 1, GL_RGBA8, img.w, img.h);
  glTextureSubImage2D(texture, 0, 0, 0, img.w, img.h, GL_RGBA, GL_UNSIGNED_BYTE, img.data);

  unsigned int background;
  glCreateTextures(GL_TEXTURE_2D, 1, &background);
  glBindTextureUnit(1, background);

  glTextureParameteri(background, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTextureParameteri(background, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTextureParameteri(background, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(background, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTextureStorage2D(background, 1, GL_RGBA8, bg_img.w, bg_img.h);
  glTextureSubImage2D(background, 0, 0, 0, bg_img.w, bg_img.h, GL_RGBA, GL_UNSIGNED_BYTE, bg_img.data);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  float currentTime = glfwGetTime();
  float time = 0;

  float fps_time = 0;
  int fps = 0;
  bool recording = false;
  float atime = 0;

  while(!glfwWindowShouldClose(window))
  {
    float dt    = glfwGetTime() - currentTime;
    currentTime = glfwGetTime();

    time += dt;
    fps_time += dt;

    if(time >= 0.6)
    {
      for(int i = 0; i < 4; ++i)
        points[i].u += 0.5;

      time = 0;
    }

    glNamedBufferData(buffer, sizeof(vertex) * 4, points, GL_STATIC_DRAW);

    ++fps;
    if(fps_time >= 1.f)
    {
      //std::cout << "fps: " << fps << "\n";
      fps_time -= 1;
      fps=0;
    }
    
    if(recording)
    {
      Image img;
      Image_alloc(&img, window_width, window_height, 4);

      glReadPixels(0, 0, img.w, img.h, GL_RGBA, GL_UNSIGNED_BYTE, img.data); //nice trick

      Image_flip_y(img);
      std::cout.write((const char*)img.data, img.w * img.h * img.c);
      //Image_save(img, "screenshot.png");
      //Image_free(&img);
    }

    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
      glfwSetWindowShouldClose(window, 1);
    }

    if(glfwGetKey(window, GLFW_KEY_W)) {
      atime += dt;
      cam.z -= lerp(0, 1.5, smoothstep(atime)) * dt;//1.5 is max velocity of the camera
    } else if(glfwGetKey(window, GLFW_KEY_S)) {
      atime += dt;
      cam.z += lerp(0, 1.5, smoothstep(atime)) * dt;
    } else {
      atime = 0;
    }

    glBindTextureUnit(0, texture);
    glBindTextureUnit(1, background);

    glUseProgram(prg);

    glUniform1i(u_tex0, 0);
    glUniform1i(u_tex1, 1);


    cam.reset();
    cam.update_view_vectors();
    cam.model      = glm::rotate(cam.model, cam.yradians , glm::vec3(1.0f, 0.0f, 0.0f));
    cam.view       = glm::lookAt(cam.cameraPos, cam.targetPos, cam.upVector);
    cam.projection = glm::perspective(glm::radians(60.f), 1.f, 0.1f, 100.0f);
    //projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -0.1f, 100.0f);

    glm::mat4 mvp = cam.mvp();
    glUniformMatrix4fv(mvp_loc,1, GL_FALSE, glm::value_ptr(mvp));

    glUniform2f(res_loc, window_width, window_height);
    glUniform1f(u_time, glfwGetTime());

    glClearColor(0.2, 0.2, 0.2, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      //bind texture 
      glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glfwPollEvents();
    glfwSwapInterval(1); //vsync on
    glfwSwapBuffers(window);
  }

  Image_free(&img);
  glfwTerminate();
    
  return 0;
}

bool loadfile(std::string filepath, std::string& src)
{
  std::ifstream file(filepath, std::fstream::binary);
  std::string line;

  src = "";

  if(!file.is_open())
  {
    error("failed to load file from specified path");
    return false;
  }

  while(std::getline(file, line))
  {
    src += line;
  }

  return true;
}

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

void check_shader_compilation(unsigned int id)
{
  glCompileShader(id);

  int status = -1;
  glGetShaderiv(id, GL_COMPILE_STATUS, &status);

  if(status == GL_FALSE)
  {
     int logsize = 0;
     glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logsize);
     glGetShaderInfoLog(id, logsize, &logsize, elog);
     fprintf(stderr, "SHADER ERROR: %s\n", elog);
  }
}

bool check_shader_program_linkage(unsigned int id) {

  int lparams = -1;
  glGetProgramiv(id, GL_LINK_STATUS, &lparams);

  if(GL_TRUE == lparams)
    return true;

  fprintf(stderr, "failed to link shader program with GL index %u\n", id);

  const int max_length = 2048;
  int actual_length    = 0;

  glGetProgramInfoLog(id, max_length, &actual_length, elog);
  fprintf(stderr, "program info log for program with GL index %u\n\t%s", id, elog);

  glDeleteProgram(id);
  return false;
}

float lerp(float a, float b, float t)
{
  return (b-a)*t + a;
}

float smoothstep(float x)
{
  return (x <= 0)? 0 : (x >= 1)? 1 : (6*pow(x, 2)-5*pow(x, 3));
}

unsigned int create_shader_program(std::string vshader_file, std::string fshader_file)
{
  //load shaders && create program
  std::string vs_srcfile, fs_srcfile;
  loadfile(vshader_file, vs_srcfile);
  loadfile(fshader_file, fs_srcfile);

  const char* vs_src = vs_srcfile.data();
  const char* fs_src = fs_srcfile.data();

  unsigned int vs, fs, prg;
  vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vs_src, NULL);
  glCompileShader(vs);
  check_shader_compilation(vs);

  fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fs_src, NULL);
  glCompileShader(fs);
  check_shader_compilation(fs);

  prg = glCreateProgram();
  glAttachShader(prg, vs);
  glAttachShader(prg, fs);
  glLinkProgram(prg);
  check_shader_program_linkage(prg);

  glDeleteShader(vs);
  glDeleteShader(fs);
  return prg;
}

GLFWwindow* create_opengl_context(int width, int height, bool fullscreen, bool enable_debug)
{
  if(!glfwInit())
  {
    error("failed to init glfw");
    return nullptr;
  }

  const GLFWvidmode* mode  = glfwGetVideoMode(glfwGetPrimaryMonitor());
  static GLFWwindow* window = nullptr;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  if(fullscreen)
  {
    window_width  = mode->width;
    window_height = mode->height;
    window = glfwCreateWindow(window_width, window_height, "nice-gfx", glfwGetPrimaryMonitor(), NULL);
  } else {
    window = glfwCreateWindow(width, height, "nice-gfx", NULL, NULL);
  }

  if(!window)
  {
    error("failed to create glfw window");
    return nullptr;
  }

  glfwMakeContextCurrent(window);

  int glad_version = gladLoadGL(glfwGetProcAddress);
  if(glad_version == 0)
  {
     fprintf(stderr, "ERROR: failed to load opengl context (glad)\n");
     return nullptr;
  }

  if(enable_debug) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(message_callback, nullptr);
  }

  return window;
}

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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  cam.yradians += yoffset * 18.f / 60.f;
  cam.xradians -= xoffset * 18.f / 60.f;
}
