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
            vk::Format format;
            vk::ColorSpaceKHR color_space;
            vk::Extent2D extent;
            uint32_t current_image_index;
		};

        struct Subpass {
            std::vector<vk::AttachmentReference> color_references;
            vk::AttachmentReference depth_reference;
        };

        struct FramebufferReference {
            const vk::UniqueRenderPass& render_pass;
            std::vector<uint32_t> image_view_reference;
        };

		template <typename T>
		struct Resource {
			T _object;
			vk::DeviceSize _deviceSize;
			vk::MemoryRequirements _memoryRequirements;
			vk::UniqueDeviceMemory _deviceMemory;

		protected:
<<<<<<< HEAD
			Resource( T&& obj, vk::DeviceSize devSize, vk::MemoryRequirements memReqs, vk::UniqueDeviceMemory devMem ) : _object( obj ), _deviceSize( devSize ), _memoryRequirements( memReqs ), _deviceMemory( std::move( devMem ) ) {
				static_assert(std::is_same_v<T, vk::UniqueBuffer> || std::is_same_v<T, vk::UniqueImage>, "Resource must be a buffer or an image.");
=======
            Resource( T& obj, vk::DeviceSize devSize, vk::MemoryRequirements memReqs, vk::UniqueDeviceMemory& devMem ) : _object( std::move( obj ) ), _deviceSize( devSize ), _memoryRequirements( memReqs ), _deviceMemory( std::move( devMem ) ) {
                static_assert(std::is_same_v<T, vk::UniqueImage> || std::is_same_v<T, vk::UniqueBuffer>, "Resource must be a buffer or an image.");
>>>>>>> 40ac455a26e24b123555ab53b6a781bda433c843
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
            Image( vk::UniqueImage& image, vk::DeviceSize devSize, vk::MemoryRequirements memReqs, vk::UniqueDeviceMemory& devMem, uint32_t width, uint32_t height, uint32_t channels, vk::Format format, vk::ImageLayout layout ) : Resource<vk::UniqueImage>( image, devSize, memReqs, devMem ), _width( width ), _height( height ), _channels( channels ), _format( format ), _imageLayout( layout ) {}
		};

		struct Buffer : Resource<vk::UniqueBuffer> {
			friend RendererCore;

			vk::BufferUsageFlags usage;

		protected:
            Buffer( vk::UniqueBuffer& buffer, vk::DeviceSize devSize, vk::MemoryRequirements memReqs, vk::UniqueDeviceMemory& devMem, vk::BufferUsageFlags usage ) : Resource<vk::UniqueBuffer>( buffer, devSize, memReqs, devMem ), usage( usage ) {}
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
        RendererCore::Image depth_image;
        vk::UniqueImageView depth_image_view;
        vk::UniqueSampler sampler;

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

<<<<<<< HEAD
		const uint32_t get_memory_type_index( vk::MemoryRequirements memReq, vk::MemoryPropertyFlags memProps ) {
=======
        const uint32_t get_memory_type_index( vk::MemoryRequirements memReq, vk::MemoryPropertyFlags memProps ) {
>>>>>>> 40ac455a26e24b123555ab53b6a781bda433c843
			for( uint32_t i = 0; i < selected_device->memory_properties.memoryProperties.memoryTypeCount; ++i ) {
				if( (memReq.memoryTypeBits & (1 << i)) &&
                    ( ( selected_device->memory_properties.memoryProperties.memoryTypes[i].propertyFlags & memProps ) == memProps) )
					return i;
			}

<<<<<<< HEAD
			return UINT32_MAX;
=======
            return UINT32_MAX;
>>>>>>> 40ac455a26e24b123555ab53b6a781bda433c843
		}

        vk::UniqueDescriptorPool create_descriptor_pool( vk::ArrayProxy<vk::DescriptorPoolSize> pool_sizes, uint32_t sets = 1 );
        vk::UniqueDescriptorSetLayout create_descriptor_set_layout( vk::ArrayProxy<vk::DescriptorSetLayoutBinding> set_layout_bindings );
        vk::UniqueDescriptorSet allocate_descriptor_set( vk::UniqueDescriptorPool& pool, vk::UniqueDescriptorSetLayout& set_layout );
        std::vector<vk::UniqueDescriptorSet> allocate_descriptor_sets( vk::UniqueDescriptorPool& pool, vk::ArrayProxyNoTemporaries<vk::UniqueDescriptorSetLayout> set_layouts );
        vk::UniqueRenderPass create_render_pass( vk::ArrayProxy<vk::AttachmentDescription> descriptions, vk::ArrayProxy<RendererCore::Subpass> subpasses );
        vk::UniqueFramebuffer create_framebuffer( vk::UniqueRenderPass& render_pass, vk::ArrayProxy<vk::UniqueImageView*> attachments );
        vk::UniqueShaderModule create_shader_module( std::string spv_file );
        RendererCore::Buffer create_buffer( vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_properties );
        vk::UniquePipelineLayout create_pipeline_layout( vk::ArrayProxy<vk::UniqueDescriptorSetLayout*> layouts, vk::ArrayProxy<vk::PushConstantRange> push_constants = {} );
        RendererCore::Image create_image_2d( uint32_t width, uint32_t height, vk::Format format = vk::Format::eR32G32B32A32Sfloat );
        vk::UniqueImageView create_image_view_2d( RendererCore::Image& image );


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
        vk::UniqueSampler create_sampler();
        std::vector<char> get_shader_data(std::string spv_file);


		void render_loop();
	};
}
