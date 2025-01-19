#define VMA_IMPLEMENTATION
#include "vulkan_device.hpp"
#include <SDL3/SDL_vulkan.h>
#include <engine/core/exception.hpp>
#include <engine/core/logger.hpp>
#include <engine/renderer/vulkan/vulkan_utils.hpp>
#include <map>
#include <set>

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

  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = "SimpleEngine",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "SimpleEngine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VulkanDevice::VK_API_VERSION,
  };

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

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

  VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &m_instance));
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
  SDL_Vulkan_CreateSurface(m_window, m_instance, nullptr, &m_surface);
  LOG_INFO("Vulkan surface created");
}

void VulkanDevice::pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  std::vector<VkPhysicalDevice> devices;
  vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
  devices.resize(deviceCount);
  vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

  // Use an ordered map to automatically sort candidates by increasing score
  std::multimap<int, VkPhysicalDevice> candidates;

  for (const auto &device : devices) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);
    LOG_INFO("Found device: {}", std::string(props.deviceName));
    int score = rateDeviceSuitability(device);
    candidates.insert(std::make_pair(score, device));
  }

  // Check if the best candidate is suitable at all
  if (candidates.rbegin()->first > 0) {
    m_physicalDevice = candidates.rbegin()->second;
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &props);
    LOG_INFO("Selected device: {}", std::string(props.deviceName));
  } else {
    SE_THROW_ERROR("Failed to find a suitable GPU!");
  }
}

void VulkanDevice::createLogicalDevice() {
  QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
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
    queueCreateInfos.push_back({.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                .pNext = nullptr,
                                .flags = 0,
                                .queueFamilyIndex = queueFamily,
                                .queueCount = 1,
                                .pQueuePriorities = &queuePriority});
  }

  VkPhysicalDeviceHostQueryResetFeaturesEXT resetFeatures = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES,
      .pNext = nullptr,
      .hostQueryReset = VK_TRUE,
  };

  VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR shaderSubgroupFeatures = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES_KHR,
      .pNext = &resetFeatures,
      .shaderSubgroupExtendedTypes = VK_TRUE,
  };

  VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures = {};
  descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
  descriptorIndexingFeatures.pNext = &shaderSubgroupFeatures;
  descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
  descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;

  VkPhysicalDeviceShaderAtomicInt64FeaturesKHR atomicInt64Features = {};
  atomicInt64Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES_KHR;
  atomicInt64Features.pNext = &descriptorIndexingFeatures;
  atomicInt64Features.shaderBufferInt64Atomics = VK_TRUE;

  VkPhysicalDeviceFloat16Int8FeaturesKHR float16Int8Features = {};
  float16Int8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR;
  float16Int8Features.pNext = &atomicInt64Features;
  float16Int8Features.shaderFloat16 = VK_TRUE;

  VkPhysicalDeviceFeatures2 deviceFeatures = {};
  deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
  deviceFeatures.pNext = &float16Int8Features;
  deviceFeatures.features.independentBlend = VK_TRUE;
  deviceFeatures.features.geometryShader = VK_TRUE;
  deviceFeatures.features.multiDrawIndirect = VK_TRUE;
  deviceFeatures.features.drawIndirectFirstInstance = VK_TRUE;
  deviceFeatures.features.depthClamp = VK_TRUE;
  deviceFeatures.features.fillModeNonSolid = VK_TRUE;
  deviceFeatures.features.samplerAnisotropy = VK_TRUE;
  deviceFeatures.features.vertexPipelineStoresAndAtomics = VK_TRUE;
  deviceFeatures.features.fragmentStoresAndAtomics = VK_TRUE;
  deviceFeatures.features.shaderImageGatherExtended = VK_TRUE;
  deviceFeatures.features.shaderStorageImageReadWithoutFormat = VK_TRUE;
  deviceFeatures.features.shaderInt64 = VK_TRUE;
  // TODO Check if device features are supported
  // checkDeviceFeatureSupport(m_physicalDevice, deviceFeatures);

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
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

  VK_CHECK_RESULT(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));
  vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
  vkGetDeviceQueue(m_device, indices.transferFamily.value(), 0, &m_transferQueue);
  vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
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

int VulkanDevice::rateDeviceSuitability(const VkPhysicalDevice &device) {
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  // vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

  int score = 0;

  // Discrete GPUs have a significant performance advantage
  if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
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

bool VulkanDevice::checkDeviceExtensionSupport(const VkPhysicalDevice &device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

QueueFamilyIndices VulkanDevice::findQueueFamilies(const VkPhysicalDevice device) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  VkQueueFlags transferQueueFlags = VK_QUEUE_TRANSFER_BIT;

  uint32_t i = 0;
  for (const auto &queueFamily : queueFamilies) {
    // TODO Check this loop for correct queue families
    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
      indices.graphicsFamilySupportsTimeStamps = queueFamily.timestampValidBits > 0;
    }

    if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & transferQueueFlags) == transferQueueFlags) {
      indices.transferFamily = i;
      indices.transferFamilySupportsTimeStamps = queueFamily.timestampValidBits > 0;
    }

    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
      indices.computeFamily = i;
      indices.computeFamilySupportsTimeStamps = queueFamily.timestampValidBits > 0;
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

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