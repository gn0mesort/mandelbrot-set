/**
 * @file main.cpp
 * @brief Client-Side Application Program
 * @author Alexander Rothman <[gnomesort@megate.ch](mailto:gnomesort@megate.ch)>
 * @copyright AGPL-3.0-or-later
 * @date 2024
 */
#include <cstdlib>
#include <cmath>

#include <array>
#include <string>
#include <iostream>
#include <algorithm>
#include <chrono>

#include <SDL2/SDL.h>

#include "glad/glad.h"

#include "config.hpp"
#include "shaders.hpp"

#define SDL2_CHECK(exp) \
  do \
  { \
    if ((exp)) \
    { \
      sdlfail("\"" #exp "\" failed."); \
    } \
  } \
  while (0)

#define GL_CHECK(exp) \
  do \
  { \
    if (!(exp)) \
    { \
      std::cerr << "\"" #exp "\" failed." << std::endl; \
      std::exit(1); \
    } \
  } \
  while (0)

#define GL_ERR_CHECK(exp) \
  do \
  { \
    if (((exp), glGetError()) != GL_NO_ERROR) \
    { \
      std::cerr << "\"" #exp "\" failed." << std::endl; \
      std::exit(1); \
    } \
  } \
  while (0)

[[noreturn]]
void sdlfail(const char *const msg) {
  std::cerr << msg << std::endl;
  std::cerr << "SDL2 Error: " << SDL_GetError() << std::endl;
  std::exit(1);
}

void check_shader(const GLuint shader) {
  auto status = GLint{ };
  GL_ERR_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &status));
  if (status != GL_TRUE)
  {
    GL_ERR_CHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &status));
    auto log = std::string(status, '\0');
    GL_ERR_CHECK(glGetShaderInfoLog(shader, log.size(), nullptr, log.data()));
    std::cerr << "Shader compilation failed." << std::endl;
    std::cerr << "Reason:" << std::endl;
    std::cerr << log << std::endl;
    std::exit(1);
  }
}

void check_program(const GLuint program) {
  auto status = GLint{ };
  GL_ERR_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &status));
  if (status != GL_TRUE)
  {
    GL_ERR_CHECK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &status));
    auto log = std::string(status, '\0');
    GL_ERR_CHECK(glGetProgramInfoLog(program, log.size(), nullptr, log.data()));
    std::cerr << "Program linking failed." << std::endl;
    std::cerr << "Reason:" << std::endl;
    std::cerr << log << std::endl;
    std::exit(1);
  }
}

const std::array<GLfloat, 8> VERTICES{ -1.0f, -1.0f,
                                       -1.0f, 1.0f,
                                        1.0f, 1.0f,
                                        1.0f, -1.0f };
const std::array<GLuint, 6> INDICES{ 0, 1, 2, 0, 2, 3 };

int main() {
  auto width = int{ CONFIG_WINDOW_WIDTH };
  auto height = int{ CONFIG_WINDOW_HEIGHT };
  if (SDL_Init(SDL_INIT_VIDEO))
  {
    sdlfail("SDL2 initialization failed.");
  }
  SDL2_CHECK(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8));
  SDL2_CHECK(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8));
  SDL2_CHECK(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8));
  SDL2_CHECK(SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8));
  SDL2_CHECK(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0));
  SDL2_CHECK(SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0));
  SDL2_CHECK(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1));
  SDL2_CHECK(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4));
  SDL2_CHECK(SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1));
  SDL2_CHECK(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4));
  SDL2_CHECK(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5));
  SDL2_CHECK(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
  auto win = SDL_CreateWindow("Mandelbrot", 0, 0, width, height,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
  if (!win)
  {
    sdlfail("SDL2 window creation failed.");
  }
  auto gl = SDL_GL_CreateContext(win);
  if (!gl)
  {
    sdlfail("SDL2 OpenGL context creation failed.");
  }
  SDL2_CHECK(SDL_GL_MakeCurrent(win, gl));
  // This enables FIFO/Vsync/synchronized output rather than immediate output of OpenGL frames.
  // It's possible for this to fail, but it's not a critical error for the application. If this fails just leave it
  // alone.
  // Since Vulkan requires VK_PRESENT_MODE_FIFO_KHR if VK_KHR_surface is supported, can this even realistically be
  // unsupported on real graphics hardware?
  SDL_GL_SetSwapInterval(1);
  if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
  {
    std::cerr << "Glad failed to load OpenGL." << std::endl;
    return 1;
  }
  auto vert = GLuint{ };
  GL_CHECK((vert = glCreateShader(GL_VERTEX_SHADER)));
  GL_ERR_CHECK(glShaderSource(vert, 1, &vertex_shader_src, nullptr));
  GL_ERR_CHECK(glCompileShader(vert));
  check_shader(vert);
  auto frag = GLuint{ };
  GL_CHECK((frag = glCreateShader(GL_FRAGMENT_SHADER)));
  GL_ERR_CHECK(glShaderSource(frag, 1, &fragment_shader_src, nullptr));
  GL_ERR_CHECK(glCompileShader(frag));
  check_shader(frag);
  auto prog = GLuint{ };
  GL_CHECK((prog = glCreateProgram()));
  GL_ERR_CHECK(glAttachShader(prog, vert));
  GL_ERR_CHECK(glAttachShader(prog, frag));
  GL_ERR_CHECK(glLinkProgram(prog));
  check_program(prog);
  GL_ERR_CHECK(glDeleteShader(frag));
  GL_ERR_CHECK(glDeleteShader(vert));
  auto vao = GLuint{ };
  GL_ERR_CHECK(glCreateVertexArrays(1, &vao));
  GL_ERR_CHECK(glVertexArrayAttribBinding(vao, 0, 0));
  GL_ERR_CHECK(glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0));
  GL_ERR_CHECK(glEnableVertexArrayAttrib(vao, 0));
  auto buffers = std::array<GLuint, 2>{ };
  GL_ERR_CHECK(glCreateBuffers(buffers.size(), buffers.data()));
  const auto& [ vertices, indices ] = buffers;
  GL_ERR_CHECK(glNamedBufferStorage(vertices, 8 * sizeof(GLfloat), VERTICES.data(), 0));
  GL_ERR_CHECK(glNamedBufferStorage(indices, 6 * sizeof(GLuint), INDICES.data(), 0));
  GL_ERR_CHECK(glVertexArrayVertexBuffer(vao, 0, vertices, 0, 2 * sizeof(GLfloat)));
  GL_ERR_CHECK(glVertexArrayElementBuffer(vao, indices));
  SDL_ShowWindow(win);
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  GL_ERR_CHECK(glEnable(GL_MULTISAMPLE));
  GL_ERR_CHECK(glEnable(GL_FRAMEBUFFER_SRGB));
  auto zoom = 1.0f;
  auto desired_zoom = 1.0f;
  auto offset_x = 0.0f;
  auto offset_y = 0.0f;
  auto controls = std::array<bool, 5>{ false, false, false, false, false };
  auto& [ up, down, left, right, escape ] = controls;
  auto ev = SDL_Event{ };
  auto quit = false;
  // Assume the viewport is dirty on the first frame.
  auto dirty = true;
  auto start = std::chrono::steady_clock::now();
  auto dt = std::chrono::duration<float>{ };
  auto acc = std::chrono::duration<float>{ };
  const auto ft = std::chrono::duration<float>{ 1.0f / 60.0f };
  std::clog << "WASD or the arrow keys to pan." << std::endl;
  std::clog << "Scroll the mouse wheel to zoom in/out." << std::endl;
  std::clog << "Click the mouse wheel to reset the scene." << std::endl;
  std::clog << "Press Escape to exit..." << std::endl;
  while (!quit)
  {
    acc += (dt = std::chrono::steady_clock::now() - start);
    start = std::chrono::steady_clock::now();
    GL_ERR_CHECK(glClear(GL_COLOR_BUFFER_BIT));
    GL_ERR_CHECK(glBindVertexArray(vao));
    GL_ERR_CHECK(glUseProgram(prog));
    if (dirty)
    {
      SDL_GL_GetDrawableSize(win, &width, &height);
      GL_ERR_CHECK(glViewport(0, 0, width, height));
    }
    GL_ERR_CHECK(glUniform1ui(0, width));
    GL_ERR_CHECK(glUniform1ui(1, height));
    GL_ERR_CHECK(glUniform1ui(2, 1000));
    GL_ERR_CHECK(glUniform1f(3, zoom));
    GL_ERR_CHECK(glUniform2f(4, offset_x, offset_y));
    GL_ERR_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
    SDL_GL_SwapWindow(win);
    while (SDL_PollEvent(&ev))
    {
      switch (ev.type)
      {
      // Quit when the application receives a quit signal from the window system.
      case SDL_QUIT:
        quit = true;
        break;
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        {
          switch (ev.key.keysym.scancode)
          {
          case SDL_SCANCODE_ESCAPE:
            escape = ev.key.state == SDL_PRESSED;
            break;
          case SDL_SCANCODE_W:
          case SDL_SCANCODE_UP:
            up = ev.key.state == SDL_PRESSED;
            break;
          case SDL_SCANCODE_S:
          case SDL_SCANCODE_DOWN:
            down = ev.key.state == SDL_PRESSED;
            break;
          case SDL_SCANCODE_A:
          case SDL_SCANCODE_LEFT:
            left = ev.key.state == SDL_PRESSED;
            break;
          case SDL_SCANCODE_D:
          case SDL_SCANCODE_RIGHT:
            right = ev.key.state == SDL_PRESSED;
            break;
          default:
            break;
          }
        }
        break;
      case SDL_MOUSEBUTTONUP:
        if (ev.button.button == 2)
        {
          offset_x = 0.0f;
          offset_y = 0.0f;
          zoom = 1.0f;
          desired_zoom =  1.0f;
        }
        break;
      // Mark the viewport as dirty when the window is resized.
      case SDL_WINDOWEVENT:
        if (ev.window.event == SDL_WINDOWEVENT_RESIZED)
        {
          dirty = true;
        }
        break;
      // When the user scrolls the mouse wheel, zoom the scene in or out.
      case SDL_MOUSEWHEEL:
        desired_zoom = std::clamp(((ev.wheel.y > 0) * (zoom / 1.025f)) + ((ev.wheel.y < 0) * (zoom * 1.025f)), 0.00001f,
                                  100.0f);
        break;
      default:
        break;
      }
    }
    if (escape)
    {
      quit = true;
    }
    if (up)
    {
      offset_y = std::clamp(offset_y + 0.01f * zoom * 30.0f * dt.count(), -16.0f, 16.0f);
    }
    if (down)
    {
      offset_y = std::clamp(offset_y - 0.01f * zoom * 30.0f * dt.count(), -16.0f, 16.0f);
    }
    if (left)
    {
      offset_x = std::clamp(offset_x - 0.01f * zoom * 30.0f * dt.count(), -16.0f, 16.0f);
    }
    if (right)
    {
      offset_x = std::clamp(offset_x + 0.01f * zoom * 30.0f * dt.count(), -16.0f, 16.0f);
    }
    while (acc >= ft)
    {
      if (std::abs(zoom - desired_zoom) > 1e-5f)
      {
        zoom = std::lerp(zoom, desired_zoom, 10.0f * ft.count());
      }
      acc -= ft;
    }
  }
  SDL_HideWindow(win);
  GL_ERR_CHECK(glDeleteBuffers(buffers.size(), buffers.data()));
  GL_ERR_CHECK(glDeleteVertexArrays(1, &vao));
  GL_ERR_CHECK(glDeleteProgram(prog));
  SDL_GL_DeleteContext(gl);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
