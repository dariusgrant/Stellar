#include "RendererCore.hpp"

namespace stlr {
	RendererCore::RendererCore( Window& window )
		: window( window )
		, instance( create_instance() )
		, surface( create_surface() )
		, devices( create_devices() )
		, selected_device( select_best_device() )
		, present_command_pool( create_graphics_command_pool() )
		, transfer_command_pool( create_transfer_command_pool() )
		, present_command_buffer( allocate_graphics_command_buffer() )
		, transfer_command_buffer( allocate_transfer_command_buffer() )
		, swapchain( create_swapchain() )
		, depth_image( nullptr )
		, current_image_index( 0 )
		, timer()
		, delta_time( 0.0f )
		, pre_update( nullptr )
		, post_update( nullptr ) {}

	RendererCore::Image RendererCore::create_image_2d( uint32_t width, uint32_t height, vk::Format format ) {
		vk::ImageCreateInfo ci {
			{},
			vk::ImageType::e2D,
			format,
			vk::Extent3D{ width, height, 1 },
			1,
			1,
			vk::SampleCountFlagBits::e1,
			vk::ImageTiling::eOptimal,
			{
				vk::ImageUsageFlagBits::eColorAttachment |
				vk::ImageUsageFlagBits::eDepthStencilAttachment |
				vk::ImageUsageFlagBits::eTransferDst
			},
			vk::SharingMode::eExclusive,
			0,
			nullptr,
			vk::ImageLayout::ePreinitialized
		};

		vk::UniqueImage image { selected_device->device->createImageUnique( ci ) };
		vk::MemoryRequirements mem_reqs { selected_device->device->getImageMemoryRequirements( image.get() ) };
		vk::MemoryAllocateInfo mem_ai {mem_reqs.size, get_memory_type_index(mem_reqs, vk::MemoryPropertyFlagBits::eDeviceLocal) };
		vk::UniqueDeviceMemory dev_mem{ selected_device->device->allocateMemoryUnique( mem_ai ) };
		
		selected_device->device->bindImageMemory( image.get(), dev_mem.get(), 0 );
		vk::DeviceSize size { static_cast<vk::DeviceSize>( width ) * height * format_utils::get_format_component_count( format ) };
		return RendererCore::Image( std::move( image ), size, mem_reqs, std::move( dev_mem ), width, height, format_utils::get_format_component_count(format), format, vk::ImageLayout::ePreinitialized );
	}

	vk::UniqueInstance RendererCore::create_instance() {
		std::vector<const char*> layers{
#ifndef NDEBUG
			DebugInfo::instance_debug_layers.begin(),
			DebugInfo::instance_debug_layers.end()
#endif // NDEBUG
		};

		std::vector<const char*> extensions{
			required_instance_extensions.begin(),
			required_instance_extensions.end()
		};
#ifndef NDEBUG
		//extensions.insert( extensions.end(), DebugInfo::instance_debug_extensions.begin(), DebugInfo::instance_debug_extensions.end() );
#endif // NDEBUG

		vk::ApplicationInfo app_info{
			{},
			{},
			{},
			{},
			VK_API_VERSION_1_2
		};

		vk::InstanceCreateInfo ci{
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
		vk::XlibSurfaceCreateInfoKHR ci{
			{},
			window.get_x11_info().display,
			window.get_x11_info().window
		};

		return instance->createXlibSurfaceKHRUnique( ci );

#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
		vk::Win32SurfaceCreateInfoKHR ci{
			{},
			{},
			window.get_hwnd()
		};

		return instance->createWin32SurfaceKHRUnique( ci );
#endif // VK_USE_PLATFORM_WIN32_KHR
	}

	std::vector<RendererCore::Device> RendererCore::create_devices() {
		std::vector<vk::PhysicalDevice> phyDevs{ instance->enumeratePhysicalDevices() };
		std::vector<Device> devs;
		devs.reserve( phyDevs.size() );

		for( const auto& p : phyDevs ) {
			std::optional<Device> d = create_device( p );
			if( d.has_value() ) {
				devs.push_back( std::move( d.value() ) );
			}
		}

		if( devs.empty() ) {
			throw std::runtime_error( "No devices found." );
		}
		return devs;
	}

	std::optional<RendererCore::Device> RendererCore::create_device( const vk::PhysicalDevice& p ) {
		std::vector<vk::QueueFamilyProperties2> queue_fam_props{ p.getQueueFamilyProperties2() };
		uint32_t gfx_queue_index = UINT32_MAX;
		uint32_t trfr_queue_index = UINT32_MAX;
		std::tie( gfx_queue_index, trfr_queue_index ) = get_graphics_and_transfer_queue_indices( p, queue_fam_props );

		// If there's no graphics queue, don't continue.
		if( gfx_queue_index == UINT32_MAX )
			return std::nullopt;

		vk::PhysicalDeviceMemoryProperties2 memProps{ p.getMemoryProperties2() };
		vk::PhysicalDeviceFeatures2 feats{ p.getFeatures2() };
		vk::PhysicalDeviceProperties2 props{ p.getProperties2() };
		vk::SurfaceCapabilities2KHR surf_cap{ p.getSurfaceCapabilities2KHR( surface.get() ) };
		std::vector<vk::SurfaceFormat2KHR> surf_forms{ p.getSurfaceFormats2KHR( surface.get() ) };

		bool same_queue_family{ gfx_queue_index == trfr_queue_index };
		bool more_than_one_gfx_queue{ queue_fam_props[gfx_queue_index].queueFamilyProperties.queueCount > 1 };
		bool same_multiple_queue_family = same_queue_family && more_than_one_gfx_queue;

		std::vector<float> priority{
			same_multiple_queue_family ? std::vector<float>{ 1.0f, 0.0f } : std::vector<float>{ 1.0f }
		};

		std::vector<vk::DeviceQueueCreateInfo> queue_ci;

		// If the transfer queue is exclusive from the graphics family, create a queue
		// for each family. If it shares the same family as graphics and has more than
		// one queue available, create 2 queues of the graphics family - one for
		// graphics and one for transfer. Otherwise, they both share the same queue.
		if( !same_queue_family ) {
			queue_ci = std::vector<vk::DeviceQueueCreateInfo>{
				vk::DeviceQueueCreateInfo( {}, gfx_queue_index, 1, priority.data() ),
				vk::DeviceQueueCreateInfo( {}, trfr_queue_index, 1, priority.data() )
			};
		}
		else if( same_multiple_queue_family ) {
			queue_ci = std::vector<vk::DeviceQueueCreateInfo>{
				vk::DeviceQueueCreateInfo( {}, gfx_queue_index, 2, priority.data() ),
			};
		}
		else {
			queue_ci = std::vector<vk::DeviceQueueCreateInfo>{
				vk::DeviceQueueCreateInfo( {}, gfx_queue_index, 1, priority.data() ),
			};
		}

		vk::DeviceCreateInfo dev_ci{
			{},
			queue_ci,
			nullptr,
			required_device_extension
		};

		dev_ci.setPNext( &feats );
		vk::UniqueDevice dev{ p.createDeviceUnique( dev_ci ) };

		vk::Queue gfx_queue{ dev->getQueue( gfx_queue_index, 0 ) };

		vk::Queue trfr_queue;
		if( same_multiple_queue_family ) {
			trfr_queue = dev->getQueue( gfx_queue_index, 1 );
		}
		else if( same_queue_family && !more_than_one_gfx_queue ) {
			trfr_queue = dev->getQueue( gfx_queue_index, 0 );
		}
		else {
			trfr_queue = dev->getQueue( trfr_queue_index, 0 );
		}

		return stlr::RendererCore::Device {
			p,
			queue_fam_props,
			memProps,
			feats,
			props,
			surf_cap,
			surf_forms,
			std::move( dev ),
			gfx_queue,
			trfr_queue,
			gfx_queue_index,
			trfr_queue_index
		};
	}

	std::pair<uint32_t, uint32_t> RendererCore::get_graphics_and_transfer_queue_indices( const vk::PhysicalDevice& p, const std::vector<vk::QueueFamilyProperties2>& queue_fam_props ) {
		uint32_t gfx_queue_index { UINT32_MAX };
		uint32_t trfr_queue_index { UINT32_MAX };
		uint32_t comp_queue_index { UINT32_MAX };
		bool gfx_queue_found { false };
		bool trfr_queue_found { false };
		bool comp_queue_found { false };

		for( uint32_t i = 0; i < queue_fam_props.size(); i++ ) {
			vk::QueueFamilyProperties q{ queue_fam_props[i].queueFamilyProperties };

			// If the family supports presentation and graphics, use it exclusively.
			if( !gfx_queue_found && p.getSurfaceSupportKHR( i, surface.get() ) && q.queueFlags & vk::QueueFlagBits::eGraphics ) {
				gfx_queue_index = i;
				gfx_queue_found = true;
			}

			const bool is_transfer_only {
				q.queueFlags & vk::QueueFlagBits::eTransfer &&
				!( q.queueFlags & vk::QueueFlagBits::eGraphics ) &&
				!( q.queueFlags & vk::QueueFlagBits::eCompute )
			};

			// If the family supports transfer only (preferably) without graphics, use it exclusively.
			if( !trfr_queue_found && is_transfer_only ) {
				trfr_queue_index = i;
				trfr_queue_found = true;
			}

			// Transfer is implicit for compute and graphics, no need to check transfer.
			const bool is_compute_only {
				q.queueFlags & vk::QueueFlagBits::eCompute &&
				!( q.queueFlags & vk::QueueFlagBits::eGraphics )
			};

			if( !comp_queue_found && is_compute_only ) {
				comp_queue_index = i;
				comp_queue_found = true;
			}

			if( gfx_queue_found && trfr_queue_found )
				break;
		}

		// If there's no transfer or compute family separate from graphics,
		// assign the transfer queue the same as the graphics family.
		if( !trfr_queue_found && !comp_queue_found ) {
			trfr_queue_index = gfx_queue_index;
		}
		else if( !trfr_queue_found && comp_queue_found ) {
			trfr_queue_index = comp_queue_index;
		}

		return std::pair( gfx_queue_index, trfr_queue_index );
	}

	const std::vector<RendererCore::Device>::iterator RendererCore::select_best_device() {
		if( devices.size() == 1 ) {
			return devices.begin();
		}

		auto find_device_of_type = [=]( vk::PhysicalDeviceType type ) {
			return std::find_if( devices.begin(), devices.end(), [&]( const RendererCore::Device& d ) { return d.properties.properties.deviceType == type; } );
		};

		std::vector<RendererCore::Device>::iterator it = find_device_of_type( vk::PhysicalDeviceType::eDiscreteGpu );
		if( it != devices.end() ) {
			return it;
		}

		it = find_device_of_type( vk::PhysicalDeviceType::eIntegratedGpu );
		if( it != devices.end() ) {
			return it;
		}

		it = find_device_of_type( vk::PhysicalDeviceType::eCpu );
		if( it != devices.end() ) {
			return it;
		}

		return devices.begin();
	}

	vk::UniqueCommandPool RendererCore::create_graphics_command_pool() {
		vk::CommandPoolCreateInfo gfx_ci {
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			selected_device->graphics_queue_index
		};

		return selected_device->device->createCommandPoolUnique( gfx_ci );
	}

	vk::UniqueCommandPool RendererCore::create_transfer_command_pool() {
		vk::CommandPoolCreateInfo trfr_ci {
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			selected_device->transfer_queue_index
		};

		return selected_device->device->createCommandPoolUnique( trfr_ci );
	}

	vk::UniqueCommandBuffer RendererCore::allocate_graphics_command_buffer() {
		vk::CommandBufferAllocateInfo ai {
			present_command_pool.get(),
			vk::CommandBufferLevel::ePrimary,
			1
		};

		return std::move( selected_device->device->allocateCommandBuffersUnique( ai ).front() );
	}

	vk::UniqueCommandBuffer RendererCore::allocate_transfer_command_buffer() {
		vk::CommandBufferAllocateInfo ai {
			transfer_command_pool.get(),
			vk::CommandBufferLevel::ePrimary,
			1
		};

		return std::move( selected_device->device->allocateCommandBuffersUnique( ai ).front() );
	}

	RendererCore::Swapchain RendererCore::create_swapchain() {
		const vk::SurfaceCapabilitiesKHR& surface_capabilities { selected_device->capabilities.surfaceCapabilities };
		const vk::SurfaceFormatKHR& surface_format { selected_device->formats.front().surfaceFormat };
		vk::SwapchainCreateInfoKHR ci{
			{},
			surface.get(),
			surface_capabilities.minImageCount,
			surface_format.format,
			surface_format.colorSpace,
			surface_capabilities.currentExtent,
			1,
			surface_capabilities.supportedUsageFlags,
			vk::SharingMode::eExclusive,
			0,
			nullptr,
			surface_capabilities.currentTransform,
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			vk::PresentModeKHR::eImmediate,
			true,
			nullptr
		};

		vk::UniqueSwapchainKHR swapchain { selected_device->device->createSwapchainKHRUnique( ci ) };
		std::vector<vk::Image> swapchain_images { selected_device->device->getSwapchainImagesKHR( swapchain.get() ) };

		std::vector<vk::UniqueImageView> swapchain_image_views {};
		swapchain_image_views.reserve( swapchain_images.size() );

		for( const auto& i : swapchain_images ) {
			vk::ImageViewCreateInfo ivci{
				{},
				i,
				vk::ImageViewType::e2D,
				surface_format.format,
				{},
				vk::ImageSubresourceRange(
					vk::ImageAspectFlagBits::eColor,
					0,
					1,
					0,
					1
				)
			};

			swapchain_image_views.push_back( selected_device->device->createImageViewUnique( ivci ) );
		}


		return Swapchain{ std::move( swapchain ), swapchain_images, std::move( swapchain_image_views ) };
	}
}
