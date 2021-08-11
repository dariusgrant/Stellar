#pragma once

#include <vulkan/vulkan.hpp>

namespace stellar{
    class stellar_engine{
        vk::UniqueInstance instance;
        std::vector<vk::PhysicalDevice> physicalDevices;
        std::vector<vk::UniqueDevice> devices;
    };
}
