#pragma once

#include <optional>
#include "ExtensionMap.hpp"
#include "Window.hpp"
#include "Timer.hpp"

#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#endif // __linux__
#ifdef _WIN64
#define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN64

#include <vulkan/vulkan.hpp>

#ifdef VK_USE_PLATFORM_XLIB_KHR
#define STLR_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
#define STLR_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif // VK_USE_PLATFORM_WIN32_KHR

#include "Utils.hpp"

#ifndef NDEBUG
#include <iostream>
#endif // NDEBUG

namespace stlr {
	class RendererCore {
	public:

		struct Device {
			vk::PhysicalDevice physical_device;
			std::vector<vk::QueueFamilyProperties2> queue_family_properties;
			vk::PhysicalDeviceMemoryProperties2 memory_properties;
			vk::PhysicalDeviceFeatures2 features;
			vk::PhysicalDeviceProperties2 properties;
			vk::SurfaceCapabilities2KHR capabilities;
			std::vector<vk::SurfaceFormat2KHR> formats;
			vk::UniqueDevice device;
			vk::Queue graphics_queue;
			vk::Queue transfer_queue;
			uint32_t graphics_queue_index;
			uint32_t transfer_queue_index;
		};

		struct Swapchain {
			vk::UniqueSwapchainKHR swapchain;
			std::vector<vk::Image> images;
			std::vector<vk::UniqueImageView> image_views;
		};

		template <typename T>
		struct Resource {
			T _object;
			vk::DeviceSize _deviceSize;
			vk::MemoryRequirements _memoryRequirements;
			vk::UniqueDeviceMemory _deviceMemory;

		protected:
			Resource( T obj, vk::DeviceSize devSize, vk::MemoryRequirements memReqs, vk::UniqueDeviceMemory devMem ) : _object( obj ), _deviceSize( devSize ), _memoryRequirements( memReqs ), _deviceMemory( devMem ) {
				static_assert(std::is_same_v<T, vk::Buffer> || std::is_same_v<T, vk::Image>, "Resource must be a buffer or an image.");
			}
		};

		struct Image : Resource<vk::UniqueImage> {
			friend RendererCore;

			uint32_t _width;
			uint32_t _height;
			uint32_t _channels;
			vk::Format _format;
			vk::ImageLayout _imageLayout;

		protected:
			Image( vk::UniqueImage image, vk::DeviceSize devSize, vk::MemoryRequirements memReqs, vk::UniqueDeviceMemory devMem, uint32_t width, uint32_t height, uint32_t channels, vk::Format format, vk::ImageLayout layout ) : Resource<vk::UniqueImage>( std::move( image ), devSize, memReqs, std::move( devMem ) ), _width( width ), _height( height ), _channels( channels ), _format( format ), _imageLayout( layout ) {}
		};

		struct Buffer : Resource<vk::UniqueBuffer> {
			friend RendererCore;

			vk::BufferUsageFlags usage;

		protected:
			Buffer( vk::UniqueBuffer buffer, vk::DeviceSize devSize, vk::MemoryRequirements memReqs, vk::UniqueDeviceMemory devMem ) : Resource<vk::UniqueBuffer>( std::move( buffer ), devSize, memReqs, std::move( devMem ) ) {}
		};

#ifndef NDEBUG
		struct DebugInfo {
			static constexpr std::array<const char*, 2> instance_debug_layers{
				"VK_LAYER_KHRONOS_validation",
				"VK_LAYER_LUNARG_api_dump"
			};

			static constexpr std::array<const char*, 0> instance_debug_extensions{

			};

			static constexpr std::array<vk::ValidationFeatureEnableEXT, 4> feature_enables{
				  vk::ValidationFeatureEnableEXT::eGpuAssisted,
				  vk::ValidationFeatureEnableEXT::eBestPractices,
				  vk::ValidationFeatureEnableEXT::eSynchronizationValidation
			};

			static constexpr vk::ValidationFeaturesEXT validation_features{
					feature_enables.size(),
					feature_enables.data()
			};

			static inline const ExtensionMap debug_extension_map{
				validation_features,
			};
		};
#endif // NDEBUG

		static constexpr std::array<const char*, 3> required_instance_extensions{
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
			STLR_PLATFORM_SURFACE_EXTENSION_NAME,
		};

		static constexpr std::array<const char*, 1> required_device_extension{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

	protected:
		Window& window;
		vk::UniqueInstance instance;
		vk::UniqueSurfaceKHR surface;
		std::vector<RendererCore::Device> devices;
		std::vector<RendererCore::Device>::iterator selected_device;
		vk::UniqueCommandPool present_command_pool;
		vk::UniqueCommandPool transfer_command_pool;
		vk::UniqueCommandBuffer present_command_buffer;
		vk::UniqueCommandBuffer transfer_command_buffer;
		RendererCore::Swapchain swapchain;
		vk::UniqueImage depth_image;
		uint32_t current_image_index;

		Timer timer;
		double delta_time;

		using pfn_update = void (*)();
		pfn_update pre_update;
		pfn_update post_update;

	public:
		RendererCore( Window& window );
		void run();

	protected:
		virtual void update() = 0;
		virtual void render() = 0;

		constexpr int get_memory_type_index( vk::MemoryRequirements memReq, vk::MemoryPropertyFlags memProps ) {
			for( uint32_t i = 0; i < selected_device->memory_properties.memoryProperties.memoryTypeCount; ++i ) {
				if( (memReq.memoryTypeBits & (1 << i)) &&
					((selected_device->memory_properties.memoryProperties.memoryTypes[i].propertyFlags & memProps) == memProps) )
					return i;
			}

			return -1;
		}

		RendererCore::Image create_image_2d( uint32_t width, uint32_t height, vk::Format format = vk::Format::eR32G32B32A32Sfloat );

	private:
		vk::UniqueInstance create_instance();
		vk::UniqueSurfaceKHR create_surface();
		std::vector<RendererCore::Device> create_devices();
		std::optional<RendererCore::Device> create_device( const vk::PhysicalDevice& p );
		std::pair<uint32_t, uint32_t> get_graphics_and_transfer_queue_indices( const vk::PhysicalDevice& p, const std::vector<vk::QueueFamilyProperties2>& queue_fam_props );
		const std::vector<RendererCore::Device>::iterator select_best_device();
		vk::UniqueCommandPool create_graphics_command_pool();
		vk::UniqueCommandPool create_transfer_command_pool();
		vk::UniqueCommandBuffer allocate_graphics_command_buffer();
		vk::UniqueCommandBuffer allocate_transfer_command_buffer();
		RendererCore::Swapchain create_swapchain();

		void render_loop();
	};
}
