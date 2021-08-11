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

        };

        struct Swapchain {
            vk::UniqueSwapchainKHR swapchain;
            std::vector<vk::Image> images;
        };

#ifndef NDEBUG
        struct DebugInfo {
            static constexpr std::array<const char*, 2> instance_debug_layers {
                "VK_LAYER_KHRONOS_validation",
                "VK_LAYER_LUNARG_api_dump"
            };

            static constexpr std::array<const char*, 0> instance_debug_extensions {

            };

            static constexpr std::array<vk::ValidationFeatureEnableEXT, 4> feature_enables {
                  vk::ValidationFeatureEnableEXT::eGpuAssisted,
                  vk::ValidationFeatureEnableEXT::eBestPractices,
                  vk::ValidationFeatureEnableEXT::eSynchronizationValidation
            };

            static constexpr vk::ValidationFeaturesEXT validation_features {
                    feature_enables.size(),
                    feature_enables.data()
            };

            static inline const ExtensionMap debug_extension_map {
                validation_features,
            };
        };
#endif // NDEBUG

        static constexpr std::array<const char*, 3> required_instance_extensions {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
            STLR_PLATFORM_SURFACE_EXTENSION_NAME,
        };

        static constexpr std::array<const char*, 1> required_device_extension {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

    protected:
        Window& window;
        vk::UniqueInstance instance;
        vk::UniqueSurfaceKHR surface;
        std::vector<Device> devices;
        std::vector<Device>::iterator selected_device;
        vk::UniqueCommandPool present_command_pool;
        vk::UniqueCommandPool transfer_command_pool;
        vk::UniqueCommandBuffer present_command_buffer;
        vk::UniqueCommandBuffer transfer_command_buffer;
        Swapchain swapchain;
        uint32_t current_image_index;

        Timer timer;
        double delta_time;

        using pfn_update = void ( * )();
        pfn_update pre_update;
        pfn_update post_update;

    public:
        RendererCore( Window& window );
        void run();

    protected:
        virtual void update() = 0;
        virtual void render() = 0;


    private:
        vk::UniqueInstance create_instance();
        vk::UniqueSurfaceKHR create_surface();
        std::vector<Device> create_devices();
        std::optional<Device> create_device( const vk::PhysicalDevice& p );
        std::pair<uint32_t, uint32_t> get_graphics_and_transfer_queue_indices( const vk::PhysicalDevice& p, const std::vector<vk::QueueFamilyProperties2>& queue_fam_props );
        std::vector<Device>::iterator select_best_device();
        std::pair<vk::UniqueCommandPool, vk::UniqueCommandPool>&& create_command_pools();
        std::pair<vk::UniqueCommandBuffer, vk::UniqueCommandBuffer>&& allocate_command_buffers();
        Swapchain&& create_swapchain();

        void render_loop();
    };
}
