#pragma once

#include <vma_usage.hpp>
#include <QObject>

namespace stlr{
    class resource : public QObject{
        Q_OBJECT

    protected:
        static vma::Allocator allocator;
        static vma::AllocatorCreateInfo allocator_create_info;
        vma::Allocation allocation;
        vma::AllocationCreateInfo allocation_create_info;

    protected:
        resource() = default;
        virtual ~resource() = default;

    public slots:
        static void init_allocator(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device);
        static void destroy_allocator();
    };

    class buffer : resource{
    protected:
        vk::UniqueBuffer unique_buffer;
        vk::BufferCreateInfo buffer_create_info;

        buffer(vk::DeviceSize size, vk::BufferUsageFlags usage);
    };
}
