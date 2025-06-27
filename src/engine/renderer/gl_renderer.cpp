#include "gl_renderer.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include <engine/core/logger.hpp>
#include <glad/gl.h>

namespace engine::renderer {
static void GLAPIENTRY debugCallback([[maybe_unused]] GLenum source, [[maybe_unused]] GLenum type,
                                     [[maybe_unused]] GLuint id, GLenum severity, [[maybe_unused]] GLsizei length,
                                     const GLchar *message, [[maybe_unused]] const void *userParam) {
  if (severity == GL_DEBUG_SEVERITY_HIGH) {
    LOG_ERROR("OpenGL: {}", message);
  } else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
    LOG_WARN("OpenGL: {}", message);
  } else if (severity == GL_DEBUG_SEVERITY_LOW) {
    LOG_INFO("OpenGL: {}", message);
  } else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
    LOG_TRACE("OpenGL: {}", message);
  } else {
    LOG_INFO("OpenGL: {}", message);
  }
}

GlRenderer::GlRenderer(SDL_Window *window) : m_window{window} {
  m_glCtx = SDL_GL_CreateContext(m_window);
  gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
  glEnable(GL_DEPTH_TEST);
#ifndef NDEBUG
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(debugCallback, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplSDL3_InitForOpenGL(m_window, m_glCtx);
  ImGui_ImplOpenGL3_Init();
  ImGui::StyleColorsDark();
}

GlRenderer::~GlRenderer() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  SDL_GL_DestroyContext(m_glCtx);
}

void GlRenderer::beginFrame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // For testing only
  ImGui::ShowDemoWindow();

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GlRenderer::endFrame() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow(m_window);
}

void GlRenderer::onResize(int width, int height) { glViewport(0, 0, width, height); }
} // namespace engine::renderer
