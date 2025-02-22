#include "vulkan_descriptors.hpp"
#include <cassert>
#include <vulkan/vulkan_core.h>

namespace engine {
namespace renderer {
// *************** Descriptor Set Layout Builder *********************

VulkanDescriptorSetLayout::Builder &VulkanDescriptorSetLayout::Builder::addBinding(uint32_t binding,
                                                                                   VkDescriptorType descriptorType,
                                                                                   VkShaderStageFlags stageFlags,
                                                                                   uint32_t count) {
  assert(bindings.count(binding) == 0 && "Binding already in use");
  VkDescriptorSetLayoutBinding layoutBinding = {
      .binding = binding,
      .descriptorType = descriptorType,
      .descriptorCount = count,
      .stageFlags = stageFlags,
  };

  bindings[binding] = layoutBinding;
  return *this;
}

std::unique_ptr<VulkanDescriptorSetLayout> VulkanDescriptorSetLayout::Builder::build() const {
  return std::make_unique<VulkanDescriptorSetLayout>(m_device, bindings);
}

// *************** Descriptor Set Layout *********************

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
    VulkanDevice *device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : m_device{device}, bindings{bindings} {
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
  for (auto kv : bindings) {
    setLayoutBindings.push_back(kv.second);
  }

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
      .pBindings = setLayoutBindings.data(),
  };

  vkCreateDescriptorSetLayout(m_device->getDevice(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout);
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
  vkDestroyDescriptorSetLayout(m_device->getDevice(), descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool Builder *********************

VulkanDescriptorPool::Builder &VulkanDescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType,
                                                                          uint32_t count) {
  poolSizes.push_back({descriptorType, count});
  return *this;
}

VulkanDescriptorPool::Builder &VulkanDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) {
  poolFlags = flags;
  return *this;
}
VulkanDescriptorPool::Builder &VulkanDescriptorPool::Builder::setMaxSets(uint32_t count) {
  maxSets = count;
  return *this;
}

std::unique_ptr<VulkanDescriptorPool> VulkanDescriptorPool::Builder::build() const {
  return std::make_unique<VulkanDescriptorPool>(m_device, maxSets, poolFlags, poolSizes);
}

// *************** Descriptor Pool *********************

VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice *device, uint32_t maxSets,
                                           VkDescriptorPoolCreateFlags poolFlags,
                                           const std::vector<VkDescriptorPoolSize> &poolSizes)
    : m_device{device} {
  VkDescriptorPoolCreateInfo descriptorPoolInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = poolFlags,
      .maxSets = maxSets,
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes = poolSizes.data(),
  };

  vkCreateDescriptorPool(m_device->getDevice(), &descriptorPoolInfo, nullptr, &m_descriptorPool);
}

VulkanDescriptorPool::~VulkanDescriptorPool() {
  vkDestroyDescriptorPool(m_device->getDevice(), m_descriptorPool, nullptr);
}

bool VulkanDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout,
                                              VkDescriptorSet &descriptor) const {
  VkDescriptorSetAllocateInfo allocInfo = {
      .descriptorPool = m_descriptorPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptorSetLayout,
  };

  // Might want to create a "DescriptorPoolManager" class that handles this
  // case, and builds a new pool whenever an old pool fills up. But this is
  // beyond our current scope
  auto res = vkAllocateDescriptorSets(m_device->getDevice(), &allocInfo, &descriptor);
  if (res != VK_SUCCESS) {
    return false;
  }
  return true;
}

void VulkanDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
  vkFreeDescriptorSets(m_device->getDevice(), m_descriptorPool, static_cast<uint32_t>(descriptors.size()),
                       descriptors.data());
}

void VulkanDescriptorPool::resetPool() { vkResetDescriptorPool(m_device->getDevice(), m_descriptorPool, 0); }

// *************** Descriptor Writer *********************

VulkanDescriptorWriter::VulkanDescriptorWriter(VulkanDescriptorSetLayout &setLayout, VulkanDescriptorPool &pool)
    : m_setLayout{setLayout}, m_pool{pool} {}

VulkanDescriptorWriter &VulkanDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
  assert(m_setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

  auto &bindingDescription = m_setLayout.bindings[binding];

  assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet = VK_NULL_HANDLE,
      .dstBinding = binding,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = bindingDescription.descriptorType,
      .pImageInfo = nullptr,
      .pBufferInfo = bufferInfo,
      .pTexelBufferView = nullptr,
  };

  m_writes.push_back(write);
  return *this;
}

VulkanDescriptorWriter &VulkanDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo) {
  assert(m_setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

  auto &bindingDescription = m_setLayout.bindings[binding];

  assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet = VK_NULL_HANDLE,
      .dstBinding = binding,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = bindingDescription.descriptorType,
      .pImageInfo = imageInfo,
      .pBufferInfo = nullptr,
      .pTexelBufferView = nullptr,
  };

  m_writes.push_back(write);
  return *this;
}

bool VulkanDescriptorWriter::build(VkDescriptorSet &set) {
  bool success = m_pool.allocateDescriptor(m_setLayout.getDescriptorSetLayout(), set);
  if (!success) {
    return false;
  }
  overwrite(set);
  return true;
}

void VulkanDescriptorWriter::overwrite(VkDescriptorSet &set) {
  for (auto &write : m_writes) {
    write.dstSet = set;
  }
  vkUpdateDescriptorSets(m_pool.m_device->getDevice(), static_cast<uint32_t>(m_writes.size()), m_writes.data(), 0,
                         nullptr);
}

} // namespace renderer
} // namespace engine
