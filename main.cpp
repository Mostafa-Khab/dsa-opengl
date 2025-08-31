#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <iostream>
#include <fstream>
#include <string>

extern "C" {
  #include <image.h>
}

#define error(X) fprintf(stderr, "ERROR: %s\n", X)

const int width  = 800;
const int height = 800;

bool loadfile(std::string filepath, std::string& src);

void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param);

char elog[2048];
bool check_shader_program_linkage(unsigned int id);
void check_shader_compilation(unsigned int id);

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


int main()
{
  if(!glfwInit())
  {
    error("failed to init glfw");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(width, height, "nice", NULL, NULL);
  if(!window)
  {
    error("failed to create glfw window");
    return -1;
  }

  glfwMakeContextCurrent(window);

  int glad_version = gladLoadGL(glfwGetProcAddress);
  if(glad_version == 0)
  {
     fprintf(stderr, "ERROR: failed to load opengl context (glad)\n");
     return -1;
  }

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(message_callback, nullptr);

  //load shaders && create program

  std::string vs_srcfile, fs_srcfile;
  loadfile("./shaders/shader.vert", vs_srcfile);
  loadfile("./shaders/shader.frag", fs_srcfile);

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

  if(!img.data)
  {
    error("failed to load bird64.png image");
  }

  unsigned int texture;
  glCreateTextures(GL_TEXTURE_2D, 1, &texture);

  glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTextureStorage2D(texture, 1, GL_RGBA8, img.w, img.h);
  glTextureSubImage2D(texture, 0, 0, 0, img.w, img.h, GL_RGBA, GL_UNSIGNED_BYTE, img.data);

  glBindTextureUnit(0, texture);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while(!glfwWindowShouldClose(window))
  {
    glClearColor(0.2, 0.2, 0.2, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(prg);

      //bind texture 
      glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glfwPollEvents();
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
