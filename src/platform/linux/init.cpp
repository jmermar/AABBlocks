#include "init.hpp"
#include "VkBootstrap.h"
#include <iostream>

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			  VkDebugUtilsMessageTypeFlagsEXT messageType,
			  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			  void* pUserData)
{

	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

	// Si es un error, detenemos el programa
	if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		std::abort();
	}

	return VK_FALSE;
}

namespace vblck
{
System initSystemLinux(const char* win_name, int W, int H)
{
	System system;
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

	system.window = SDL_CreateWindow(win_name, W, H, window_flags);

	vkb::InstanceBuilder builder;

	auto inst_ret = builder.set_app_name("VBlck")
						.request_validation_layers(true)
						.set_debug_callback(debugCallback)
						.require_api_version(1, 3, 0)
						.build();

	vkb::Instance vkb_inst = inst_ret.value();

	system.instance = vkb_inst.instance;
	system.debugMessenger = vkb_inst.debug_messenger;

	SDL_Vulkan_CreateSurface(system.window, system.instance, nullptr, &system.surface);

	// vulkan 1.3 features
	VkPhysicalDeviceVulkan13Features features13{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
	features13.dynamicRendering = true;
	features13.synchronization2 = true;

	VkPhysicalDeviceFeatures features;
	features.multiDrawIndirect = true;
	features.shaderInt64 = true;

	// vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features features12{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
	features12.bufferDeviceAddress = true;
	features12.descriptorIndexing = true;
	features12.drawIndirectCount = true;

	vkb::PhysicalDeviceSelector selector{vkb_inst};
	vkb::PhysicalDevice physicalDevice = selector.set_minimum_version(1, 3)
											 .set_required_features_13(features13)
											 .set_required_features_12(features12)
											 .set_required_features(features)
											 .set_surface(system.surface)
											 .select()
											 .value();

	vkb::DeviceBuilder deviceBuilder{physicalDevice};

	vkb::Device vkbDevice = deviceBuilder.build().value();

	system.device = vkbDevice.device;
	system.chosenGPU = physicalDevice.physical_device;

	system.graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	system.graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	return system;
}

void finishSystemLinux(const System& system)
{
	vkDestroySurfaceKHR(system.instance, system.surface, nullptr);
	vkDestroyDevice(system.device, nullptr);

	vkb::destroy_debug_utils_messenger(system.instance, system.debugMessenger);
	vkDestroyInstance(system.instance, nullptr);
	SDL_DestroyWindow(system.window);

	SDL_Quit();
}
} // namespace vblck