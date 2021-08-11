#include "resource.hpp"

namespace stlr{
    void resource::init_allocator(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device){
        allocator_create_info.setInstance(instance).setPhysicalDevice(physicalDevice).setDevice(device);
        allocator = vma::createAllocator(allocator_create_info);
    }

    void resource::destroy_allocator(){
        allocator.destroy();
    }

    buffer::buffer(vk::DeviceSize size, vk::BufferUsageFlags usage) {
        buffer_create_info.setSize(size).setUsage(usage);
        allocation_create_info.setUsage(vma::MemoryUsage::eCpuToGpu);
        auto [unique_buffer, allocation] = allocator.createBuffer(buffer_create_info, allocation_create_info);
    }
}

#include "moc_resource.cpp"
