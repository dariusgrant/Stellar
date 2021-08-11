#pragma once

#include <vulkan/vulkan.hpp>
#include <QObject>

namespace stlr{
    class base : public QObject{
        Q_OBJECT

        vk::UniqueInstance instance;
        std::vector<vk::PhysicalDevice> physicalDevices;
        std::vector<vk::UniqueDevice> devices;
        uint32_t selected_device_index;

    public:
        base(vk::ArrayProxyNoTemporaries<const char* const> instance_layer_names,
             vk::ArrayProxyNoTemporaries<const char* const> instance_extension_names,
             vk::ArrayProxyNoTemporaries<const char* const> device_layer_names,
             vk::ArrayProxyNoTemporaries<const char* const> device_extension_names);
    };
}
