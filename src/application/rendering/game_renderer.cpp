#include "game_renderer.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include <vulkan/vulkan_core.h>

GameRenderer::GameRenderer(engine::core::Window &window) : m_window{window} {
  m_renderer = std::make_unique<engine::renderer::VulkanRenderer>(m_window.getWindow());
  m_testRenderer = std::make_unique<TestRenderer>(m_renderer.get());

  m_globalPool =
      engine::renderer::VulkanDescriptorPool::Builder(m_renderer->m_device.get())
          .setMaxSets(engine::renderer::VulkanSwapchain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, engine::renderer::VulkanSwapchain::MAX_FRAMES_IN_FLIGHT)
          .build();

  m_globalBufferIds.resize(engine::renderer::VulkanSwapchain::MAX_FRAMES_IN_FLIGHT);

  engine::renderer::BufferDesc uniformBufferDesc = {
      .usage = engine::renderer::BufferUsage::UNIFORM_BUFFER,
      .cpuAccess = engine::renderer::BufferCPUAccess::WriteOnly,
      .size = sizeof(GlobalUBO),
  };

  for (size_t i = 0; i < engine::renderer::VulkanSwapchain::MAX_FRAMES_IN_FLIGHT; i++) {
    m_globalBufferIds[i] = m_renderer->createBuffer(uniformBufferDesc);
  }

  auto globalSetLayout =
      engine::renderer::VulkanDescriptorSetLayout::Builder(m_renderer->m_device.get())
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  m_globalDescriptorSets.resize(engine::renderer::VulkanSwapchain::MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < m_globalDescriptorSets.size(); i++) {
    VkDescriptorBufferInfo bufferInfo = {
        .buffer = m_renderer->m_bufferManager->getBuffer(m_globalBufferIds[i]),
        .offset = 0,
        .range = sizeof(GlobalUBO),
    };
    engine::renderer::VulkanDescriptorWriter(*globalSetLayout, *m_globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(m_globalDescriptorSets[i]);
  }
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
