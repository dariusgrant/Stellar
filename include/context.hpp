#pragma once

#include <vulkan/vulkan.hpp>

namespace se{
    class context{
    public:
        struct physical_device_info{
            vk::PhysicalDeviceFeatures2 features;
            vk::PhysicalDeviceProperties2 properties;
            vk::PhysicalDeviceMemoryProperties2 memory_properties;

            physical_device_info(vk::PhysicalDeviceFeatures2 features,
                                 vk::PhysicalDeviceProperties2 properties,
                                 vk::PhysicalDeviceMemoryProperties2 memory_properties);
        };

        using device_tuple = std::tuple<vk::PhysicalDevice, physical_device_info, vk::UniqueDevice>;

    private:
        constexpr static uint32_t physical_device_index = 0, physical_device_info_index = 1, device_index = 2;
        vk::UniqueInstance instance;
        std::vector<device_tuple> devices;
        std::vector<device_tuple>::const_iterator current_device_iterator;

    public:
        context(vk::ArrayProxyNoTemporaries<const char* const> instance_layers,
                vk::ArrayProxyNoTemporaries<const char* const> instance_extensions,
                vk::ArrayProxyNoTemporaries<const char* const> device_extensions);

        constexpr uint32_t get_device_index() const noexcept;

        const vk::PhysicalDevice& get_physical_device() const noexcept;

        const physical_device_info& get_physical_device_info() const noexcept;

        const vk::UniqueDevice& get_device() const noexcept;

        constexpr std::vector<device_tuple>::size_type get_num_devices() const noexcept;

        const std::vector<device_tuple>::const_iterator get_next_device() noexcept;

    private:
        vk::UniqueInstance create_instance(vk::ArrayProxyNoTemporaries<const char* const> instance_layers,
                                           vk::ArrayProxyNoTemporaries<const char* const> instance_extensions);

        physical_device_info retrieve_physical_device_info(const vk::PhysicalDevice& physical_device);

        vk::UniqueDevice create_device(vk::ArrayProxyNoTemporaries<const char* const> device_extensions);
    };
}
