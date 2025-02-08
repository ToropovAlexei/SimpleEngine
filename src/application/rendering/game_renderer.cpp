#include "game_renderer.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"

GameRenderer::GameRenderer(engine::core::Window &window) : m_window{window} {
  m_renderer = std::make_unique<engine::renderer::VulkanRenderer>(m_window.getWindow());
  m_testRenderer = std::make_unique<TestRenderer>(m_renderer.get());
}

void GameRenderer::render(float dt) {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  ImGui::ShowDemoWindow();
  ImGui::Render();
  ImDrawData *draw_data = ImGui::GetDrawData();
  if (auto commandBuffer = m_renderer->beginFrame()) {
    m_renderer->beginRendering(commandBuffer);
    m_testRenderer->render(commandBuffer);
    ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
    m_renderer->endRendering(commandBuffer);
    m_renderer->endFrame();
  }
}

void GameRenderer::setRenderSize(int width, int height) { m_renderer->onResize(width, height); }

void GameRenderer::updateRenderers(float dt) { m_testRenderer->update(dt); }
