#include <cstdint>
#include <vulkan/vulkan_core.h>
#define VMA_IMPLEMENTATION
#include "vulkan_device.hpp"
#include <SDL3/SDL_vulkan.h>
#include <engine/core/exception.hpp>
#include <engine/core/logger.hpp>
#include <engine/renderer/vulkan/vulkan_swapchain.hpp>
#include <engine/renderer/vulkan/vulkan_utils.hpp>
#include <map>
#include <set>

namespace engine {
namespace renderer {
const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    //"VK_KHR_get_physical_device_properties2",
    "VK_KHR_maintenance1", "VK_KHR_maintenance3", "VK_KHR_draw_indirect_count", "VK_KHR_shader_subgroup_extended_types",
    "VK_EXT_descriptor_indexing", "VK_EXT_sampler_filter_minmax", "VK_EXT_host_query_reset",
    "VK_KHR_shader_float16_int8", "VK_KHR_shader_atomic_int64", VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    VK_EXT_SHADER_OBJECT_EXTENSION_NAME};

#ifndef NDEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                    [[maybe_unused]] void *pUserData) {
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    LOG_ERROR("Vulkan: {}", pCallbackData->pMessage);
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    LOG_WARN("Vulkan: {}", pCallbackData->pMessage);
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    LOG_INFO("Vulkan: {}", pCallbackData->pMessage);
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    LOG_TRACE("Vulkan: {}", pCallbackData->pMessage);
  } else {
    LOG_INFO("Vulkan: {}", pCallbackData->pMessage);
  }

  return VK_FALSE;
}
#endif

VulkanDevice::VulkanDevice(SDL_Window *window) : m_window{window} {
  initVulkan();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createAllocator();
  createCommandPool();
}

VulkanDevice::~VulkanDevice() {
  vmaDestroyAllocator(m_allocator);
  LOG_INFO("VMA allocator destroyed");
  vkDestroyCommandPool(m_device, m_graphicsCommandPool, nullptr);
  vkDestroyCommandPool(m_device, m_transferCommandPool, nullptr);
  LOG_INFO("Vulkan command pools destroyed");
  vkDestroyDevice(m_device, nullptr);
  LOG_INFO("Vulkan device destroyed");

#ifndef NDEBUG
  if (m_debugMessenger) {
    auto func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
    func(m_instance, m_debugMessenger, nullptr);
  }
#endif

  vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
  LOG_INFO("Vulkan surface destroyed");
  vkDestroyInstance(m_instance, nullptr);
  LOG_INFO("Vulkan instance destroyed");
}

void VulkanDevice::initVulkan() {
#ifndef NDEBUG
  checkValidationLayerSupport();
#endif

  vk::ApplicationInfo appInfo = {
      .pNext = nullptr,
      .pApplicationName = "SimpleEngine",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "SimpleEngine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VulkanDevice::VK_API_VERSION,
  };

  vk::InstanceCreateInfo createInfo = {
      .pApplicationInfo = &appInfo,
  };

  auto requiredExtensions = getRequiredExtensions();
  auto remainingRequiredExtensions = requiredExtensions;
  if (!checkInstanceExtensionSupport(remainingRequiredExtensions)) {
    for (auto &extension : remainingRequiredExtensions) {
      LOG_FATAL("Missing required extension: {}", extension);
    }

    SE_THROW_ERROR("Required extensions are missing!");
  }

  createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
  createInfo.ppEnabledExtensionNames = requiredExtensions.data();

#ifndef NDEBUG
  createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
  createInfo.ppEnabledLayerNames = validationLayers.data();
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  populateDebugMessengerCreateInfo(debugCreateInfo);
  createInfo.pNext = &debugCreateInfo;
#endif

  m_instance = vk::createInstance(createInfo).value;
  LOG_INFO("Vulkan instance created");
}

void VulkanDevice::setupDebugMessenger() {
#ifndef NDEBUG
  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);

  auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
  fn(m_instance, &createInfo, nullptr, &m_debugMessenger);
  LOG_INFO("Vulkan debug messenger created");
#endif
}

void VulkanDevice::createSurface() {
  VkSurfaceKHR surface = nullptr;
  SDL_Vulkan_CreateSurface(m_window, static_cast<VkInstance>(m_instance), nullptr, &surface);
  m_surface = surface;
  LOG_INFO("Vulkan surface created");
}

void VulkanDevice::pickPhysicalDevice() {
  auto devices = m_instance.enumeratePhysicalDevices().value;

  // Use an ordered map to automatically sort candidates by increasing score
  std::multimap<int, VkPhysicalDevice> candidates;

  for (const auto &device : devices) {
    auto props = device.getProperties();
    LOG_INFO("Found device: {}", std::string(props.deviceName));
    int score = rateDeviceSuitability(device);
    candidates.insert(std::make_pair(score, device));
  }

  // Check if the best candidate is suitable at all
  if (candidates.rbegin()->first > 0) {
    m_physicalDevice = candidates.rbegin()->second;
    auto props = m_physicalDevice.getProperties();
    LOG_INFO("Selected device: {}", std::string(props.deviceName));
  } else {
    SE_THROW_ERROR("Failed to find a suitable GPU!");
  }
}

void VulkanDevice::createLogicalDevice() {
  QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.transferFamily.value(),
                                            indices.presentFamily.value(), indices.computeFamily.value()};

  LOG_INFO("Unique queue families: {}", uniqueQueueFamilies.size());
  if (uniqueQueueFamilies.size() < 2) {
    LOG_WARN("One queue family is used for all operations"); // TODO not sure if this is correct
  }
  LOG_INFO("Graphics queue family: {}", indices.graphicsFamily.value());
  LOG_INFO("Transfer queue family: {}", indices.transferFamily.value());
  LOG_INFO("Present queue family: {}", indices.presentFamily.value());
  LOG_INFO("Compute queue family: {}", indices.computeFamily.value());

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    queueCreateInfos.push_back({.queueFamilyIndex = queueFamily, .queueCount = 1, .pQueuePriorities = &queuePriority});
  }

  vk::PhysicalDeviceShaderObjectFeaturesEXT enabledShaderObjectFeaturesEXT = {
      .shaderObject = vk::True,
  };

  vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures = {
      .pNext = &enabledShaderObjectFeaturesEXT,
      .dynamicRendering = vk::True,
  };

  vk::PhysicalDeviceHostQueryResetFeaturesEXT resetFeatures = {
      .pNext = &dynamicRenderingFeatures,
      .hostQueryReset = vk::True,
  };

  vk::PhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR shaderSubgroupFeatures = {
      .pNext = &resetFeatures,
      .shaderSubgroupExtendedTypes = vk::True,
  };

  vk::PhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures = {};
  descriptorIndexingFeatures.pNext = &shaderSubgroupFeatures;
  descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = vk::True;
  descriptorIndexingFeatures.runtimeDescriptorArray = vk::True;

  vk::PhysicalDeviceShaderAtomicInt64FeaturesKHR atomicInt64Features = {};
  atomicInt64Features.pNext = &descriptorIndexingFeatures;
  atomicInt64Features.shaderBufferInt64Atomics = vk::True;

  vk::PhysicalDeviceFloat16Int8FeaturesKHR float16Int8Features = {};
  float16Int8Features.pNext = &atomicInt64Features;
  float16Int8Features.shaderFloat16 = vk::True;

  vk::PhysicalDeviceFeatures2 deviceFeatures = {};
  deviceFeatures.pNext = &float16Int8Features;
  deviceFeatures.features.independentBlend = vk::True;
  deviceFeatures.features.geometryShader = vk::True;
  deviceFeatures.features.multiDrawIndirect = vk::True;
  deviceFeatures.features.drawIndirectFirstInstance = vk::True;
  deviceFeatures.features.depthClamp = vk::True;
  deviceFeatures.features.fillModeNonSolid = vk::True;
  deviceFeatures.features.samplerAnisotropy = vk::True;
  deviceFeatures.features.vertexPipelineStoresAndAtomics = vk::True;
  deviceFeatures.features.fragmentStoresAndAtomics = vk::True;
  deviceFeatures.features.shaderImageGatherExtended = vk::True;
  deviceFeatures.features.shaderStorageImageReadWithoutFormat = vk::True;
  deviceFeatures.features.shaderInt64 = vk::True;
  // TODO Check if device features are supported
  // checkDeviceFeatureSupport(m_physicalDevice, deviceFeatures);

  vk::DeviceCreateInfo createInfo = {};
  createInfo.pNext = &deviceFeatures;
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  std::vector<const char *> enabledExtensions;
  for (const char *extension : deviceExtensions) {
    enabledExtensions.push_back(extension);
  }

  createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
  createInfo.ppEnabledExtensionNames = enabledExtensions.data();

#ifndef NDEBUG
  std::vector<const char *> enabledLayers;
  for (const char *layer : validationLayers) {
    enabledLayers.push_back(layer);
  }

  createInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
  createInfo.ppEnabledLayerNames = enabledLayers.data();
#else
  createInfo.enabledLayerCount = 0;
#endif

  m_device = m_physicalDevice.createDevice(createInfo).value;
  m_graphicsQueue = m_device.getQueue(indices.graphicsFamily.value(), 0);
  m_transferQueue = m_device.getQueue(indices.transferFamily.value(), 0);
  m_presentQueue = m_device.getQueue(indices.presentFamily.value(), 0);

  LOG_INFO("Vulkan logical device created");
}

void VulkanDevice::createAllocator() {
  VmaAllocatorCreateInfo allocatorInfo = {
      .flags = 0,
      .physicalDevice = m_physicalDevice,
      .device = m_device,
      .preferredLargeHeapBlockSize = 0,
      .pAllocationCallbacks = NULL,
      .pDeviceMemoryCallbacks = NULL,
      .pHeapSizeLimit = NULL,
      .pVulkanFunctions = NULL,
      .instance = m_instance,
      .vulkanApiVersion = VulkanDevice::VK_API_VERSION,
      .pTypeExternalMemoryHandleTypes = NULL,
  };

  VK_CHECK_RESULT(vmaCreateAllocator(&allocatorInfo, &m_allocator));

  LOG_INFO("Vulkan allocator created");
}

void VulkanDevice::createCommandPool() {
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

  // Create graphics command pool
  {
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // Optional
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
    };

    VK_CHECK_RESULT(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_graphicsCommandPool));
  }

  // Create transfer command pool
  {
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // Optional
        .queueFamilyIndex = queueFamilyIndices.transferFamily.value(),
    };

    VK_CHECK_RESULT(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_transferCommandPool));
  }
  LOG_INFO("Vulkan command pools created");
}

void VulkanDevice::checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      LOG_FATAL("Missing validation layer: {}", layerName);
      throw std::runtime_error("validation layers requested, but not available!");
    }
  }
}

bool VulkanDevice::checkInstanceExtensionSupport(std::vector<const char *> &requiredExtensions) {
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

  for (auto extension : extensions) {
    requiredExtensions.erase(std::remove_if(requiredExtensions.begin(), requiredExtensions.end(),
                                            [&extension](const char *requiredExtension) {
                                              return std::strcmp(requiredExtension, extension.extensionName) == 0;
                                            }),
                             requiredExtensions.end());
  }

  return requiredExtensions.empty();
}

std::vector<const char *> VulkanDevice::getRequiredExtensions() {
  uint32_t sdlExtensionCount = 0;
  const char *const *sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

  std::vector<const char *> extensions(sdlExtensions, sdlExtensions + sdlExtensionCount);

#ifndef NDEBUG
  for (auto extension : extensions) {
    LOG_DEBUG("SDL required extension: {}", extension);
  }
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  extensions.push_back("VK_KHR_get_physical_device_properties2");
  extensions.push_back("VK_EXT_debug_report");
#endif

  return extensions;
};

#ifndef NDEBUG
void VulkanDevice::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
}
#endif

int VulkanDevice::rateDeviceSuitability(const vk::PhysicalDevice &device) {
  auto deviceProperties = device.getProperties();
  // vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

  int score = 0;

  // Discrete GPUs have a significant performance advantage
  if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
    score += 1000;
  }

  // Maximum possible size of textures affects graphics quality
  score += deviceProperties.limits.maxImageDimension2D;

  // Make sure it supports the extensions we need
  bool extensionsSupported = checkDeviceExtensionSupport(device);
  if (!extensionsSupported) {
    return 0;
  }

  return score;
}

bool VulkanDevice::checkDeviceExtensionSupport(const vk::PhysicalDevice &device) {
  auto availableExtensions = device.enumerateDeviceExtensionProperties().value;

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

QueueFamilyIndices VulkanDevice::findQueueFamilies(const vk::PhysicalDevice device) {
  QueueFamilyIndices indices;

  auto queueFamilies = device.getQueueFamilyProperties();

  vk::QueueFlags transferQueueFlags = vk::QueueFlagBits::eTransfer;

  uint32_t i = 0;
  for (const auto &queueFamily : queueFamilies) {
    // TODO Check this loop for correct queue families
    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
      indices.graphicsFamily = i;
      indices.graphicsFamilySupportsTimeStamps = queueFamily.timestampValidBits > 0;
    }

    if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & transferQueueFlags) == transferQueueFlags) {
      indices.transferFamily = i;
      indices.transferFamilySupportsTimeStamps = queueFamily.timestampValidBits > 0;
    }

    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
      indices.computeFamily = i;
      indices.computeFamilySupportsTimeStamps = queueFamily.timestampValidBits > 0;
    }

    auto presentSupport = device.getSurfaceSupportKHR(i, m_surface).value;

    if (queueFamily.queueCount > 0 && presentSupport) {
      indices.presentFamily = i;
      indices.presentFamilySupportsTimeStamps = queueFamily.timestampValidBits > 0;
    }

    if (indices.IsComplete()) {
      break;
    }

    i++;
  }

  return indices;
}

SwapChainSupportDetails VulkanDevice::getSwapChainSupport() {
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);

  std::vector<VkSurfaceFormatKHR> formats;
  uint32_t formatsSize;
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatsSize, nullptr);
  formats.resize(formatsSize);
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatsSize, formats.data());

  std::vector<VkPresentModeKHR> presentModes;
  uint32_t presentModesSize;
  vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModesSize, nullptr);
  presentModes.resize(presentModesSize);
  vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModesSize, presentModes.data());

  return {
      .capabilities = capabilities,
      .formats = formats,
      .presentModes = presentModes,
  };
}

void VulkanDevice::createImageWithInfo(const VkImageCreateInfo &imageInfo, VmaMemoryUsage memoryUsage, VkImage &image,
                                       VmaAllocation &imageAllocation) {

  VmaAllocationCreateInfo allocCreateInfo{};
  allocCreateInfo.usage = memoryUsage;

  VK_CHECK_RESULT(vmaCreateImage(m_allocator, &imageInfo, &allocCreateInfo, &image, &imageAllocation, nullptr));
}

VkFormat VulkanDevice::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                           VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  SE_THROW_ERROR("Failed to find supported format!");
}

VkCommandBuffer VulkanDevice::beginSingleTimeCommands() {
  VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = m_graphicsCommandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };
  VkCommandBuffer commandBuffer;
  VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer));

  VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr,
  };

  VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

  return commandBuffer;
}

void VulkanDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(m_graphicsQueue);

  vkFreeCommandBuffers(m_device, m_graphicsCommandPool, 1, &commandBuffer);
}

void VulkanDevice::copyBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, VkBuffer srcBuffer, VkDeviceSize srcOffset,
                              VkDeviceSize size) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferCopy copyRegion = {.srcOffset = srcOffset, .dstOffset = dstOffset, .size = size};
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(commandBuffer);
}

void VulkanDevice::transitionImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask,
                                         VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout,
                                         VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask,
                                         VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange) {
  VkImageMemoryBarrier imageBarrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = srcAccessMask,
      .dstAccessMask = dstAccessMask,
      .oldLayout = oldImageLayout,
      .newLayout = newImageLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = subresourceRange,
  };

  vkCmdPipelineBarrier(cmdbuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
}
} // namespace renderer
} // namespace engine
