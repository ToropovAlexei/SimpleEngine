#include "renderer/vulkan/vulkan_utils.hpp"
#include <cstddef>
#include <map>
#include <set>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#define VMA_IMPLEMENTATION
#include "vulkan_device.hpp"
#include <SDL3/SDL_vulkan.h>
#include <core/exception.hpp>
#include <core/logger.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    //"VK_KHR_get_physical_device_properties2",
    "VK_KHR_maintenance1", "VK_KHR_maintenance3", "VK_KHR_draw_indirect_count", "VK_KHR_shader_subgroup_extended_types",
    "VK_EXT_descriptor_indexing", "VK_EXT_sampler_filter_minmax", "VK_EXT_host_query_reset",
    "VK_KHR_shader_float16_int8", "VK_KHR_shader_atomic_int64"};

#ifndef NDEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                    [[maybe_unused]] void *pUserData) {
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    LOG_ERROR("Validation Error: {}", pCallbackData->pMessage);
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    LOG_WARN("Validation Warning: {}", pCallbackData->pMessage);
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    LOG_INFO("Validation Info: {}", pCallbackData->pMessage);
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    LOG_TRACE("Validation Verbose: {}", pCallbackData->pMessage);
  } else {
    LOG_INFO("Validation Debug: {}", pCallbackData->pMessage);
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
  m_device.destroy();
  LOG_INFO("Vulkan device destroyed");

#ifndef NDEBUG
  if (m_debugMessenger) {
    m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, dldi);
  }
#endif

  m_instance.destroySurfaceKHR(m_surface);
  LOG_INFO("Vulkan surface destroyed");
  m_instance.destroy();
  LOG_INFO("Vulkan instance destroyed");
}

void VulkanDevice::initVulkan() {
#ifndef NDEBUG
  checkValidationLayerSupport();
#endif

  vk::ApplicationInfo appInfo = {
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
  vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
  createInfo.ppEnabledLayerNames = validationLayers.data();
  populateDebugMessengerCreateInfo(debugCreateInfo);
  createInfo.pNext = &debugCreateInfo;
#endif

  m_instance = vk::createInstance(createInfo);
  LOG_INFO("Vulkan instance created");
}

void VulkanDevice::setupDebugMessenger() {
#ifndef NDEBUG
  vk::DebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);

  dldi = vk::detail::DispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);

  m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(createInfo, nullptr, dldi);
  LOG_INFO("Vulkan debug messenger created");
#endif
}

void VulkanDevice::createSurface() {
  VkSurfaceKHR surface;
  SDL_Vulkan_CreateSurface(m_window, m_instance, nullptr, &surface);
  m_surface = vk::SurfaceKHR(surface);
  LOG_INFO("Vulkan surface created");
}

void VulkanDevice::pickPhysicalDevice() {
  auto devices = m_instance.enumeratePhysicalDevices();

  // Use an ordered map to automatically sort candidates by increasing score
  std::multimap<int, vk::PhysicalDevice> candidates;

  for (const auto &device : devices) {
    LOG_INFO("Found device: {}", std::string(device.getProperties().deviceName));
    int score = rateDeviceSuitability(device);
    candidates.insert(std::make_pair(score, device));
  }

  // Check if the best candidate is suitable at all
  if (candidates.rbegin()->first > 0) {
    m_physicalDevice = candidates.rbegin()->second;
    LOG_INFO("Selected device: {}", std::string(m_physicalDevice.getProperties().deviceName));
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

  vk::PhysicalDeviceHostQueryResetFeaturesEXT resetFeatures = {
      .hostQueryReset = VK_TRUE,
  };

  vk::PhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR shaderSubgroupFeatures = {
      .pNext = &resetFeatures,
      .shaderSubgroupExtendedTypes = VK_TRUE,
  };

  vk::PhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures = {
      .pNext = &shaderSubgroupFeatures,
      .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
      .runtimeDescriptorArray = VK_TRUE,
  };

  vk::PhysicalDeviceShaderAtomicInt64FeaturesKHR atomicInt64Features = {
      .pNext = &descriptorIndexingFeatures,
      .shaderBufferInt64Atomics = VK_TRUE,
  };

  vk::PhysicalDeviceFloat16Int8FeaturesKHR float16Int8Features = {
      .pNext = &atomicInt64Features,
      .shaderFloat16 = VK_TRUE,
  };

  vk::PhysicalDeviceFeatures2 deviceFeatures = {
      .pNext = &float16Int8Features,
      .features =
          {
              .independentBlend = VK_TRUE,
              .geometryShader = VK_TRUE,
              .multiDrawIndirect = VK_TRUE,
              .drawIndirectFirstInstance = VK_TRUE,
              .depthClamp = VK_TRUE,
              .fillModeNonSolid = VK_TRUE,
              .samplerAnisotropy = VK_TRUE,
              .vertexPipelineStoresAndAtomics = VK_TRUE,
              .fragmentStoresAndAtomics = VK_TRUE,
              .shaderImageGatherExtended = VK_TRUE,
              .shaderStorageImageReadWithoutFormat = VK_TRUE,
              .shaderInt64 = VK_TRUE,
          },
  };
  // TODO Check if device features are supported
  // checkDeviceFeatureSupport(m_physicalDevice, deviceFeatures);

  vk::DeviceCreateInfo createInfo = {
      .pNext = &deviceFeatures,
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
      .pQueueCreateInfos = queueCreateInfos.data(),
      .pEnabledFeatures = NULL,
  };

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

  m_device = m_physicalDevice.createDevice(createInfo);
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
  auto availableLayers = vk::enumerateInstanceLayerProperties();

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
  auto extensions = vk::enumerateInstanceExtensionProperties();

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
  uint32_t glfwExtensionCount = 0;
  const char *const *sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(sdlExtensions, sdlExtensions + glfwExtensionCount);

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
void VulkanDevice::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
  createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
  createInfo.pfnUserCallback = debugCallback;
}
#endif

int VulkanDevice::rateDeviceSuitability(const vk::PhysicalDevice &device) {
  vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
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
  auto availableExtensions = device.enumerateDeviceExtensionProperties();

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

    vk::Bool32 presentSupport = device.getSurfaceSupportKHR(i, m_surface);

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
  return {
      .capabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface),
      .formats = m_physicalDevice.getSurfaceFormatsKHR(m_surface),
      .presentModes = m_physicalDevice.getSurfacePresentModesKHR(m_surface),
  };
}

void VulkanDevice::createImageWithInfo(const vk::ImageCreateInfo &imageInfo, VmaMemoryUsage memoryUsage,
                                       vk::Image &image, VmaAllocation &imageAllocation) {
  vk::ImageCreateInfo imageCreateInfo = imageInfo;

  VmaAllocationCreateInfo allocCreateInfo{};
  allocCreateInfo.usage = memoryUsage;

  VkImage rawImage;
  VK_CHECK_RESULT(vmaCreateImage(m_allocator, reinterpret_cast<const VkImageCreateInfo *>(&imageCreateInfo),
                                 &allocCreateInfo, &rawImage, &imageAllocation, nullptr));

  image = vk::Image(rawImage);
}

vk::Format VulkanDevice::findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
                                             vk::FormatFeatureFlags features) {
  for (vk::Format format : candidates) {

    vk::FormatProperties props = m_physicalDevice.getFormatProperties(format);

    if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
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

void VulkanDevice::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage,
                                VmaAllocationCreateFlags flags, vk::Buffer &buffer, VmaAllocation &allocation,
                                VmaAllocationInfo &allocInfo) {
  vk::BufferCreateInfo bufferInfo = {.size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};

  VmaAllocationCreateInfo allocCreateInfo{};
  allocCreateInfo.usage = memoryUsage;
  allocCreateInfo.flags = flags;

  VkBuffer bufferRaw;
  VK_CHECK_RESULT(vmaCreateBuffer(m_allocator, reinterpret_cast<const VkBufferCreateInfo *>(&bufferInfo),
                                  &allocCreateInfo, &bufferRaw, &allocation, &allocInfo));

  buffer = vk::Buffer(bufferRaw);
}

void VulkanDevice::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
  vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

  vk::BufferCopy copyRegion = {.srcOffset = 0, .dstOffset = 0, .size = size};
  commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

  endSingleTimeCommands(commandBuffer);
}