#include "base.hpp"

namespace stlr{
    base::base(vk::ArrayProxyNoTemporaries<const char* const> instance_layer_names,
               vk::ArrayProxyNoTemporaries<const char* const> instance_extension_names,
               vk::ArrayProxyNoTemporaries<const char* const> device_layer_names,
               vk::ArrayProxyNoTemporaries<const char* const> device_extension_names)
        : selected_device_index(0){
        vk::InstanceCreateInfo ici;
        ici.setEnabledLayerCount(instance_layer_names.size())
                .setPEnabledLayerNames(instance_layer_names)
                .setEnabledExtensionCount(instance_extension_names.size())
                .setPEnabledExtensionNames(instance_extension_names);
        instance = vk::createInstanceUnique(ici);

        physicalDevices = instance->enumeratePhysicalDevices();
        devices.resize(physicalDevices.size());

        vk::PhysicalDeviceFeatures2 features = physicalDevices[selected_device_index].getFeatures2();
    }
}

#include "moc_base.cpp"
