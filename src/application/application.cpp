#include "application.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "imgui_impl_sdl3.h"
#include <SDL3/SDL_events.h>
#include <engine/core/assert.hpp>
#include <engine/renderer/renderer_types.hpp>
#include <glm/gtx/quaternion.hpp>

Application::Application(int width, int height, std::string_view name)
    : m_width{width}, m_height{height}, m_window{width, height, name}, m_gameRenderer{m_window} {
  m_camera.setPerspective(60.0f, static_cast<float>(m_width) / static_cast<float>(m_height), 0.1f, 1000.0f);
  SDL_GL_SetSwapInterval(0);
  using Key = engine::core::Keyboard::Key;
  m_keyboard.onKeyDown(Key::F, [this]() {
    m_mouseCaptured = !m_mouseCaptured;
    SDL_SetWindowRelativeMouseMode(m_window.getWindow(), m_mouseCaptured);
  });
}

Application::~Application() {
  // SDL_Quit();
}

int Application::run() {
  while (m_running) {
    float deltaTime = m_timer.getDeltaTime();
    m_timer.tick();
    handleEvents();
    update(deltaTime);
    render(deltaTime);
  }

  return 0;
}

void Application::handleEvents() {
  m_mouse.clearDeltas();
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    if (event.type == SDL_EVENT_QUIT) {
      m_running = false;
    }
    if (event.type == SDL_EVENT_WINDOW_RESIZED) {
      m_width = event.window.data1;
      m_height = event.window.data2;
      m_gameRenderer.setRenderSize(m_width, m_height);
      m_camera.setPerspective(60.0f, static_cast<float>(m_width) / static_cast<float>(m_height), 0.1f, 1000.0f);
    }
    m_keyboard.handleEvent(event);
    if (!ImGui::GetIO().WantCaptureMouse) {
      m_mouse.handleEvent(event);
    }
  }
}

void Application::update(float dt) {
  updateCamera(dt);
  m_gameRenderer.updateRenderers(dt);
  m_gameRenderer.setView(m_camera.getViewMatrix());
  m_gameRenderer.setProjection(m_camera.getProjectionMatrix());
  m_gameRenderer.setCameraPos(m_camera.getPosition());
}

void Application::render(float dt) { m_gameRenderer.render(dt); }

void Application::updateCamera(float dt) {
  using Key = engine::core::Keyboard::Key;

  if (!m_mouseCaptured) {
    return;
  }

  const float moveSpeed = 15.0f; // Скорость движения
  const float mouseSensitivity = 0.003f;

  glm::vec3 moveDirection(0.0f);

  if (m_keyboard.isKeyDown(Key::W))
    moveDirection += m_camera.getDirection();
  if (m_keyboard.isKeyDown(Key::S))
    moveDirection -= m_camera.getDirection();
  if (m_keyboard.isKeyDown(Key::A))
    moveDirection -= m_camera.getRight();
  if (m_keyboard.isKeyDown(Key::D))
    moveDirection += m_camera.getRight();
  if (m_keyboard.isKeyDown(Key::SPACE))
    moveDirection.y += 1.0f;
  if (m_keyboard.isKeyDown(Key::X))
    moveDirection.y -= 1.0f;

  if (glm::length(moveDirection) > 0.0f) {
    moveDirection = glm::normalize(moveDirection);
    m_camera.setPosition(m_camera.getPosition() + moveDirection * moveSpeed * dt);
  }

  float xoffset = m_mouse.getMouseDeltaX();
  float yoffset = m_mouse.getMouseDeltaY();

  xoffset *= mouseSensitivity;
  yoffset *= mouseSensitivity;

  auto direction = m_camera.getDirection();
  auto right = m_camera.getRight();

  glm::quat yawRotation = glm::angleAxis(-xoffset, glm::vec3(0.0f, 1.0f, 0.0f)); // yaw
  glm::vec3 newDirection = glm::normalize(glm::rotate(yawRotation, direction));

  glm::quat pitchRotation = glm::angleAxis(-yoffset, right); // pitch
  newDirection = glm::normalize(glm::rotate(pitchRotation, newDirection));

  m_camera.setDirection(newDirection);
}