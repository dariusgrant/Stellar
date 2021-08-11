#include "renderer.hpp"

namespace stellar {

    renderer::renderer(const QWindow& window) : window(window) {
        create_instance();
    }

    ///
    /// \brief renderer::create_instance
    /// Creates a vulkan instance.
    ///
    /// Note:
    void renderer::create_instance()
    {
        auto layers = vk::enumerateInstanceLayerProperties();
        auto extensions = vk::enumerateInstanceExtensionProperties();


        auto ci = vk::InstanceCreateInfo()
                  .setPEnabledLayerNames(instance_layers)
                  .setPEnabledExtensionNames(instance_extensions);

        instance = vk::createInstanceUnique(ci);
    }

}
