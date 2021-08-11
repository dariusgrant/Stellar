#pragma once

#include <vulkan/vulkan.hpp>


class QWindow;

namespace stellar {
    class renderer {
    protected:

        const std::vector<const char*> instance_layers {
            "VK_LAYER_KHRONOS_validation",
    #ifdef QT_DEBUG
            "VK_LAYER_KRONOS_api_dump",
    #endif
        };

        const std::vector<const char*> instance_extensions {
            VK_KHR_SURFACE_EXTENSION_NAME,
    #ifdef VK_USE_PLATFORM_XLIB_KHR
            VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
    #elif VK_USE_PLATFORM_WIN32_KHR
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    #endif
        };

        const QWindow& window;
        vk::UniqueInstance instance;
        vk::UniqueSurfaceKHR surface;
        vk::PhysicalDevice physical_device;
        vk::UniqueDevice device;
        vk::Queue queue;
        vk::UniqueCommandPool command_pool;
        vk::UniqueCommandBuffer command_buffer;
        vk::UniqueSwapchainKHR swapchain;
        vk::UniquePipeline pipeline;

    public:
        renderer(const QWindow& window);

        void update();

        //~renderer() = default;

    private:
        void create_instance();
    };
}
