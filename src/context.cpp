#include "context.hpp"

namespace se{
    context::context(vk::ArrayProxyNoTemporaries<const char* const> instance_layers,
                     vk::ArrayProxyNoTemporaries<const char* const> instance_extensions,
                     vk::ArrayProxyNoTemporaries<const char* const> device_extensions)
        : instance(create_instance(instance_layers, instance_extensions))
        , devices()
        , current_device_iterator({}) {


    }

    constexpr uint32_t context::get_device_index() const noexcept
    {
        return std::distance(devices.begin(), current_device_iterator);
    }

    const vk::PhysicalDevice& context::get_physical_device() const noexcept
    {
        return std::get<vk::PhysicalDevice>(*current_device_iterator);
    }

    const context::physical_device_info& context::get_physical_device_info() const noexcept
    {
        return std::get<context::physical_device_info>(*current_device_iterator);
    }

    const vk::UniqueDevice& context::get_device() const noexcept
    {
        return std::get<vk::UniqueDevice>(*current_device_iterator);
    }

    constexpr std::vector<context::device_tuple>::size_type context::get_num_devices() const noexcept
    {
        return devices.size();
    }

    const std::vector<context::device_tuple>::const_iterator context::get_next_device() noexcept
    {
        std::advance(current_device_iterator, 1);
        if(current_device_iterator == devices.end())
            current_device_iterator = devices.begin();

        return current_device_iterator;
    }

    vk::UniqueInstance context::create_instance(vk::ArrayProxyNoTemporaries<const char* const> instance_layers,
                                                vk::ArrayProxyNoTemporaries<const char* const> instance_extensions) {
        auto ci = vk::InstanceCreateInfo(vk::InstanceCreateFlags(),
                                         nullptr,
                                         instance_layers,
                                         instance_extensions);

        return vk::createInstanceUnique(ci);
    }

    context::physical_device_info::physical_device_info(vk::PhysicalDeviceFeatures2 features,
                                  vk::PhysicalDeviceProperties2 properties,
                                  vk::PhysicalDeviceMemoryProperties2 memory_properties)
        : features(features)
        , properties(properties)
        , memory_properties(memory_properties){}

    context::physical_device_info context::retrieve_physical_device_info(const vk::PhysicalDevice& physical_device){
        return context::physical_device_info(physical_device.getFeatures2(),
                                             physical_device.getProperties2(),
                                             physical_device.getMemoryProperties2());
        }

    vk::UniqueDevice context::create_device(vk::ArrayProxyNoTemporaries<const char* const> device_extensions){
        auto ci = vk::DeviceCreateInfo(vk::DeviceCreateFlags(),
                                       {},
                                       {},
                                       {});
        ci.setPNext(&get_physical_device_info().features);

        return get_physical_device().createDeviceUnique(ci);
    }

}
