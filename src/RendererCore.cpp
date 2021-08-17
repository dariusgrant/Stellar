#include "RendererCore.hpp"

namespace stlr {
    RendererCore::RendererCore( Window& window )
    : window( window )
    , instance( create_instance() )
    , surface( create_surface() )
    , devices( create_devices() )
    , selected_device()
    , present_command_pool(nullptr)
    , transfer_command_pool(nullptr)
    , present_command_buffer(nullptr)
    , transfer_command_buffer(nullptr)
    , swapchain()
    , current_image_index(0)
    , timer()
    , delta_time(0.0f)
    , pre_update(nullptr)
    , post_update(nullptr) { }

    vk::UniqueInstance RendererCore::create_instance() {
        std::vector<const char*> layers {
#ifndef NDEBUG
            DebugInfo::instance_debug_layers.begin(),
            DebugInfo::instance_debug_layers.end()
#endif // NDEBUG
        };

        std::vector<const char*> extensions {
            required_instance_extensions.begin(),
            required_instance_extensions.end()
        };
#ifndef NDEBUG
        //extensions.insert( extensions.end(), DebugInfo::instance_debug_extensions.begin(), DebugInfo::instance_debug_extensions.end() );
#endif // NDEBUG

        vk::ApplicationInfo app_info {
            {},
            {},
            {},
            {},
            VK_API_VERSION_1_2
        };

        vk::InstanceCreateInfo ci {
            {},
            &app_info,
            layers,
            extensions
        };

#ifndef NDEBUG
        ci.pNext = DebugInfo::debug_extension_map.get_chain();
#endif // NDEBUG

        return vk::createInstanceUnique( ci );
    }

    vk::UniqueSurfaceKHR RendererCore::create_surface() {
#ifdef VK_USE_PLATFORM_XLIB_KHR
        vk::XlibSurfaceCreateInfoKHR ci {
            {},
            window.get_x11_info().display,
            window.get_x11_info().window
        };

        return instance->createXlibSurfaceKHRUnique( ci );

#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
        vk::Win32SurfaceCreateInfoKHR ci {
            {},
            {},
            window.get_hwnd()
        };

        return instance->createWin32SurfaceKHRUnique( ci );
#endif // VK_USE_PLATFORM_WIN32_KHR
    }

    std::vector<RendererCore::Device> RendererCore::create_devices() {
        std::vector<vk::PhysicalDevice> phyDevs { instance->enumeratePhysicalDevices() };
        std::vector<Device> devs;
        devs.reserve( phyDevs.size() );

        for( const auto& p : phyDevs ) {
            std::optional<Device> d = create_device( p );
            if( d.has_value() ) {
                devs.push_back( std::move( d.value() ) );
            }
        }

        return devs;
    }

    std::optional<RendererCore::Device> RendererCore::create_device( const vk::PhysicalDevice& p ) {
        std::vector<vk::QueueFamilyProperties2> queue_fam_props { p.getQueueFamilyProperties2() };
        uint32_t gfx_queue_index = UINT32_MAX;
        uint32_t trfr_queue_index = UINT32_MAX;
        std::tie( gfx_queue_index, trfr_queue_index ) = get_graphics_and_transfer_queue_indices( p, queue_fam_props );

        // If there's no graphics queue, don't continue.
        if( gfx_queue_index == UINT32_MAX )
            return std::nullopt;

        vk::PhysicalDeviceMemoryProperties2 memProps { p.getMemoryProperties2() };
        vk::PhysicalDeviceFeatures2 feats { p.getFeatures2() };
        vk::PhysicalDeviceProperties2 props { p.getProperties2() };
        vk::SurfaceCapabilities2KHR surf_cap { p.getSurfaceCapabilities2KHR( surface.get() ) };
        std::vector<vk::SurfaceFormat2KHR> surf_forms { p.getSurfaceFormats2KHR( surface.get() ) };

        bool same_queue_family { gfx_queue_index == trfr_queue_index };
        bool more_than_one_gfx_queue { queue_fam_props[gfx_queue_index].queueFamilyProperties.queueCount > 1 };
        bool same_multiple_queue_family = same_queue_family && more_than_one_gfx_queue;

        std::vector<float> priority {
            same_multiple_queue_family ? std::vector<float>{ 1.0f, 0.0f } : std::vector<float>{ 1.0f }
        };

        std::vector<vk::DeviceQueueCreateInfo> queue_ci;

        // If the transfer queue is exclusive from the graphics family, create a queue
        // for each family. If it shares the same family as graphics and has more than
        // one queue available, create 2 queues of the graphics family - one for
        // graphics and one for transfer. Otherwise, they both share the same queue.
        if( !same_queue_family ) {
            queue_ci = std::vector<vk::DeviceQueueCreateInfo> {
                vk::DeviceQueueCreateInfo( {}, gfx_queue_index, 1, priority.data() ),
                vk::DeviceQueueCreateInfo( {}, trfr_queue_index, 1, priority.data() )
            };
        } else if( same_multiple_queue_family ) {
            queue_ci = std::vector<vk::DeviceQueueCreateInfo> {
                vk::DeviceQueueCreateInfo( {}, gfx_queue_index, 2, priority.data() ),
            };
        } else {
            queue_ci = std::vector<vk::DeviceQueueCreateInfo> {
                vk::DeviceQueueCreateInfo( {}, gfx_queue_index, 1, priority.data() ),
            };
        }

        vk::DeviceCreateInfo dev_ci {
            {},
            queue_ci,
            nullptr,
            required_device_extension
        };

        dev_ci.setPNext( &feats );
        vk::UniqueDevice dev { p.createDeviceUnique( dev_ci ) };

        vk::Queue gfx_queue { dev->getQueue( gfx_queue_index, 0 ) };

        vk::Queue trfr_queue;
        if( same_multiple_queue_family ) {
            trfr_queue = dev->getQueue( gfx_queue_index, 1 );
        } else if ( same_queue_family && !more_than_one_gfx_queue ) {
            trfr_queue = dev->getQueue( gfx_queue_index, 0 );
        } else {
            trfr_queue = dev->getQueue( trfr_queue_index, 0 );
        }

        return Device {
            p,
            queue_fam_props,
            memProps,
            feats,
            props,
            surf_cap,
            surf_forms,
            std::move(dev),
            gfx_queue,
            trfr_queue
        };
    }

    std::pair<uint32_t, uint32_t> RendererCore::get_graphics_and_transfer_queue_indices( const vk::PhysicalDevice& p, const std::vector<vk::QueueFamilyProperties2>& queue_fam_props ) {
        uint32_t gfx_queue_index = UINT32_MAX;
        uint32_t trfr_queue_index = UINT32_MAX;
        bool gfx_queue_found = false;
        bool trfr_queue_found = false;

        for( uint32_t i = 0; i < queue_fam_props.size(); i++ ) {
            vk::QueueFamilyProperties q { queue_fam_props[i].queueFamilyProperties };

            // If the family supports presentation and graphics, use it exclusively.
            if( p.getSurfaceSupportKHR( i, surface.get() ) && q.queueFlags & vk::QueueFlagBits::eGraphics ) {
                gfx_queue_index = i;
                gfx_queue_found = true;
            }

            // If the family supports transfer only (preferably) or compute (transfer inherent) without graphics, use it exclusively.
            if( q.queueFlags & ( vk::QueueFlagBits::eTransfer | ~vk::QueueFlagBits::eGraphics | ~vk::QueueFlagBits::eCompute ) ||
                q.queueFlags & ( vk::QueueFlagBits::eCompute | ~vk::QueueFlagBits::eGraphics ) ) {
                trfr_queue_index = i;
                trfr_queue_found = true;
            }

            if( gfx_queue_found && trfr_queue_found )
                break;
        }

        // If there's no transfer or compute family separate from graphics,
        // assign the transfer queue the same as the graphics family.
        if( gfx_queue_found && !trfr_queue_found )
            trfr_queue_index = gfx_queue_index;

        return std::pair(gfx_queue_index, trfr_queue_index);
    }
}
