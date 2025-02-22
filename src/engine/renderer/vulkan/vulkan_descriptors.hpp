#pragma once

#include "vulkan_device.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace engine {
namespace renderer {
class VulkanDescriptorSetLayout {
public:
  class Builder {
  public:
    Builder(VulkanDevice *device) : m_device{device} {}

    Builder &addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags,
                        uint32_t count = 1);
    std::unique_ptr<VulkanDescriptorSetLayout> build() const;

  private:
    VulkanDevice *m_device;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };

  VulkanDescriptorSetLayout(VulkanDevice *device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~VulkanDescriptorSetLayout();
  VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout &) = delete;
  VulkanDescriptorSetLayout &operator=(const VulkanDescriptorSetLayout &) = delete;

  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

private:
  VulkanDevice *m_device;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

  friend class VulkanDescriptorWriter;
};

class VulkanDescriptorPool {
public:
  class Builder {
  public:
    Builder(VulkanDevice *device) : m_device{device} {}

    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<VulkanDescriptorPool> build() const;

  private:
    VulkanDevice *m_device;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags{};
  };

  VulkanDescriptorPool(VulkanDevice *device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
                       const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~VulkanDescriptorPool();
  VulkanDescriptorPool(const VulkanDescriptorPool &) = delete;
  VulkanDescriptorPool &operator=(const VulkanDescriptorPool &) = delete;

  bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

  void resetPool();

private:
  VulkanDevice *m_device;
  VkDescriptorPool m_descriptorPool;

  friend class VulkanDescriptorWriter;
};

class VulkanDescriptorWriter {
public:
  VulkanDescriptorWriter(VulkanDescriptorSetLayout &setLayout, VulkanDescriptorPool &pool);

  VulkanDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  VulkanDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);

private:
  VulkanDescriptorSetLayout &m_setLayout;
  VulkanDescriptorPool &m_pool;
  std::vector<VkWriteDescriptorSet> m_writes;
};
} // namespace renderer
} // namespace engine
