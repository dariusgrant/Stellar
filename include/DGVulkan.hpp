#pragma once
#include <QWindow>

#ifdef Q_OS_WINDOWS
    #define VK_USE_PLATFORM_WIN32_KHR
#elif defined Q_OS_LINUX
    #include <QX11Info>
    #define VK_USE_PLATFORM_XLIB_KHR
    #define GLFW_EXPOSE_NATIVE_X11
#endif

#include <vulkan/vulkan.hpp>
#include <fstream>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
//#include "Timer.hpp"
class QWindow;

namespace DG {
	/// <summary>
	/// A resource that's either a buffer or an image.
	/// </summary>
	/// <typeparam name="T">The type of resource.</typeparam>
	template <typename T>
	struct Resource {
		T _object;
		vk::DeviceSize _deviceSize;
		vk::MemoryRequirements _memoryRequirements;
		vk::DeviceMemory _deviceMemory;

	protected:
		Resource(T obj, vk::DeviceSize devSize, vk::MemoryRequirements memReqs, vk::DeviceMemory devMem) : _object(obj), _deviceSize(devSize), _memoryRequirements(memReqs), _deviceMemory(devMem) {
			static_assert(std::is_same_v<T, vk::Buffer> || std::is_same_v<T, vk::Image>, "Resource must be a buffer or an image.");
		}
	};

	struct Image : Resource<vk::Image> {
		uint32_t _width;
		uint32_t _height;
		uint32_t _channels;
		vk::Format _format;
		vk::ImageLayout _imageLayout;

		Image(vk::Image image, vk::DeviceSize devSize, vk::MemoryRequirements memReqs, vk::DeviceMemory devMem, uint32_t width, uint32_t height, uint32_t channels, vk::Format format, vk::ImageLayout layout) : Resource<vk::Image>(image, devSize, memReqs, devMem), _width(width), _height(height), _channels(channels), _format(format), _imageLayout(layout) {}
	};

	struct Buffer : Resource<vk::Buffer> {

		Buffer(vk::Buffer buffer, vk::DeviceSize devSize, vk::MemoryRequirements memReqs, vk::DeviceMemory devMem) : Resource<vk::Buffer>(buffer, devSize, memReqs, devMem) {}
	};

	template <typename T, typename E>
	struct ResourceView {
		T _resource;
		E _view;
		vk::Format _format;

		ResourceView(T resource, E view, vk::Format format) : _resource(resource), _view(view), _format(format) {}
	};

	struct ImageView : ResourceView<Image, vk::ImageView> {
		vk::ImageAspectFlags _aspects;

		ImageView(Image image, vk::ImageView view, vk::ImageAspectFlags aspects) : ResourceView<Image, vk::ImageView>(image, view, image._format), _aspects(aspects) {}
	};

	struct BufferView : ResourceView<Buffer, vk::BufferView> {
		vk::DeviceSize _offset;
		vk::DeviceSize _range;

		BufferView(Buffer buffer, vk::BufferView view, vk::Format format, vk::DeviceSize offset, vk::DeviceSize range) : ResourceView<Buffer, vk::BufferView>(buffer, view, format), _offset(offset), _range(range) {}
	};

	struct DescriptorPools {
		std::vector<vk::DescriptorPoolSize> _poolSizes;

		DescriptorPools() = default;
		DescriptorPools(std::vector<vk::DescriptorPoolSize> poolSizes) : _poolSizes(poolSizes) {};

		void add_descriptor_size(vk::DescriptorType type, uint32_t count) {
			auto s = vk::DescriptorPoolSize(type, count);
			_poolSizes.push_back(s);
		}
	};

	struct DescriptorSetLayoutBindings {
		std::vector<vk::DescriptorSetLayoutBinding> _bindings;

		DescriptorSetLayoutBindings() = default;
		DescriptorSetLayoutBindings(std::vector<vk::DescriptorSetLayoutBinding> bindings) : _bindings(bindings) {};

		void add_binding(uint32_t binding, vk::DescriptorType type, uint32_t count, vk::ShaderStageFlags stages, const vk::Sampler* immutableSamplers = nullptr) {
			auto b = vk::DescriptorSetLayoutBinding(
				binding,
				type,
				count,
				stages,
				immutableSamplers
			);
			_bindings.push_back(b);
		}
	};

	struct RenderPassAttachments {
		std::vector<vk::AttachmentDescription> _attachmentDescriptions;
		std::vector<vk::AttachmentReference> _colorAttachmentReferences;
		vk::AttachmentReference _depthStencilAttahcmentReference;

		RenderPassAttachments() = default;
		RenderPassAttachments(std::vector<vk::AttachmentDescription> attachments, std::vector<vk::AttachmentReference> colorReferences, vk::AttachmentReference depthReference) : _attachmentDescriptions(attachments), _colorAttachmentReferences(colorReferences), _depthStencilAttahcmentReference(depthReference) {}

		void add_attachment(vk::Format format, vk::ImageLayout finalLayout, bool isDepthAttachment) {
			auto a = vk::AttachmentDescription(
				vk::AttachmentDescriptionFlags(),
				format,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				finalLayout
			);

			_attachmentDescriptions.push_back(a);

			auto r = vk::AttachmentReference(_attachmentDescriptions.size() - 1);
			if (!isDepthAttachment) {
				r.layout = vk::ImageLayout::eColorAttachmentOptimal;
				_colorAttachmentReferences.push_back(r);
			}
			else {
				r.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
				_depthStencilAttahcmentReference = r;
			}
		}
	};

	struct Pipeline {
		std::vector<vk::VertexInputBindingDescription> _vertexInputBindingDescriptions;
		std::vector<vk::VertexInputAttributeDescription> _vertexInputAttributeDescriptions;

		Pipeline() = default;
		Pipeline(std::vector<vk::VertexInputBindingDescription> bindings, std::vector<vk::VertexInputAttributeDescription> attributes) : _vertexInputBindingDescriptions(bindings), _vertexInputAttributeDescriptions(attributes) {}

		void add_vertex_input_binding(uint32_t binding, uint32_t stride, vk::VertexInputRate rate = vk::VertexInputRate::eVertex) {
			_vertexInputBindingDescriptions.push_back(vk::VertexInputBindingDescription(binding, stride, rate));
		}

		void add_vertex_input_attribute(uint32_t binding, uint32_t location, vk::Format format, uint32_t offset) {
			_vertexInputAttributeDescriptions.push_back(vk::VertexInputAttributeDescription(location, binding, format, offset));
		}
	};

	class DGVulkan {
	protected:
        //QWindow _window;
        GLFWwindow* _glfwWindow;
		vk::Instance _instance;
		vk::PhysicalDevice _physicalDevice;
		vk::PhysicalDeviceMemoryProperties _physicalDeviceMemoryProperties;
		vk::Device _device;
		vk::Queue _queue;
		vk::CommandPool _commandPool;
		vk::CommandBuffer _commandBuffer;
		vk::SurfaceKHR _surface;
		vk::SurfaceCapabilitiesKHR _surfaceCapabilites;
		vk::SurfaceFormatKHR _surfaceFormat;
		vk::SwapchainKHR _swapchain;
		std::vector<vk::Image> _swapchainImages;
		std::vector<vk::ImageView> _swapchainImageViews;
		vk::Image _depthImage;
		vk::ImageView _depthImageView;
		vk::Sampler _sampler;
		vk::DescriptorPool _descriptorPool;
		vk::DescriptorSetLayout _descriptorSetLayout;
		vk::DescriptorSet _descriptorSet;
		vk::RenderPass _renderPass;
		std::vector<vk::Framebuffer> _framebuffers;
		vk::ShaderModule _vertexShaderModule;
		vk::ShaderModule _fragmentShaderModule;
		vk::PipelineLayout _pipelineLayout;
		vk::Pipeline _pipeline;
		vk::Semaphore _imageAcquiredSemaphore;
		vk::Semaphore _imageReadySemaphore;
		vk::Fence _fence;
		vk::Viewport _viewport;
		vk::Rect2D _scissor;
		vk::Buffer* _vertexBuffer;
		vk::Buffer* _indexBuffer;
		uint32_t _indexCount;
		uint32_t _imageIndex;

	public:
		/// <summary>
		/// Creates the vulkan's instance and defaults to the first enumerated device and queue
		/// along with 1 command pool and command buffer.
		/// </summary>
		/// <returns></returns>
        DGVulkan(uint32_t width, uint32_t height){
//            _window.setWidth(width);
//            _window.setHeight(height);
//            _window.show();
//            _window.setSurfaceType(QSurface::VulkanSurface);

            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            _glfwWindow = glfwCreateWindow(width, height, "Title", nullptr, nullptr);

			auto layerNames = std::vector<const char*>{
				//"VK_LAYER_LUNARG_api_dump",
				"VK_LAYER_KHRONOS_validation"
			};

			auto extensionNames = std::vector<const char*>{
                VK_KHR_SURFACE_EXTENSION_NAME,
                #ifdef VK_USE_PLATFORM_WIN32_KHR
                VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
                #elif defined VK_USE_PLATFORM_XLIB_KHR
                VK_KHR_XLIB_SURFACE_EXTENSION_NAME
                #endif
			};

			auto instanceCI = vk::InstanceCreateInfo(
				vk::InstanceCreateFlags(),
				nullptr,
				layerNames.size(),
				layerNames.data(),
				extensionNames.size(),
				extensionNames.data()
			);

			_instance = vk::createInstance(instanceCI);

			_physicalDevice = _instance.enumeratePhysicalDevices().front();
			_physicalDeviceMemoryProperties = _physicalDevice.getMemoryProperties();
			
			//auto deviceExtensions = _physicalDevice.enumerateDeviceExtensionProperties();
			std::vector<const char*> deviceExtensionNames = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
			auto physicalDeviceFeatures = _physicalDevice.getFeatures();
			float queuePriority = 1.0f;
			vk::DeviceQueueCreateInfo deviceQueueCI = vk::DeviceQueueCreateInfo(
				vk::DeviceQueueCreateFlags(),
				0,
				1,
				&queuePriority
			);
			auto deviceCI = vk::DeviceCreateInfo(
				vk::DeviceCreateFlags(),
				1,
				&deviceQueueCI,
				0,
				nullptr,
				deviceExtensionNames.size(),
				deviceExtensionNames.data(),
				&physicalDeviceFeatures
			);
			_device = _physicalDevice.createDevice(deviceCI);
			
			_queue = _device.getQueue(0, 0);

			auto commandPoolCI = vk::CommandPoolCreateInfo(
				vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				0
			);
			_commandPool = _device.createCommandPool(commandPoolCI);

			auto commandBufferAI = vk::CommandBufferAllocateInfo(_commandPool, vk::CommandBufferLevel::ePrimary, 1);
			_commandBuffer = _device.allocateCommandBuffers(commandBufferAI).front();
		}

        GLFWwindow* get_window(){
            return _glfwWindow;
        }

        bool is_window_close(){
            return glfwWindowShouldClose(_glfwWindow);
        }
		vk::Format get_surface_format() {
			return _surfaceFormat.format;
		}

        uint32_t get_surface_width(){
            return _surfaceCapabilites.currentExtent.width;
        }

        uint32_t get_surface_height(){
            return _surfaceCapabilites.currentExtent.height;
        }
		/// <summary>
		/// Initiates the surface and swapchain using the default information obtained from the surface.
		/// </summary>
		/// <param name="hwnd">The window's handle.</param>
        void init_surface_and_swapchain() {

#ifdef VK_USE_PLATFORM_WIN32_KHR
			vk::Win32SurfaceCreateInfoKHR win32SurfaceCI = vk::Win32SurfaceCreateInfoKHR(vk::Win32SurfaceCreateFlagsKHR(), nullptr, hwnd);
			_surface = _instance.createWin32SurfaceKHR(win32SurfaceCI);
#elif defined VK_USE_PLATFORM_XLIB_KHR
            auto display = glfwGetX11Display();
            auto win = glfwGetX11Window(_glfwWindow);
            vk::XlibSurfaceCreateInfoKHR xlibSurfaceCI = vk::XlibSurfaceCreateInfoKHR({}, display, win);
            _surface = _instance.createXlibSurfaceKHR(xlibSurfaceCI);
#endif
			auto surfaceSupported = _physicalDevice.getSurfaceSupportKHR(0, _surface);
			_surfaceCapabilites = _physicalDevice.getSurfaceCapabilitiesKHR(_surface);
			_surfaceFormat = _physicalDevice.getSurfaceFormatsKHR(_surface).front();
			auto ci = vk::SwapchainCreateInfoKHR(
				vk::SwapchainCreateFlagsKHR(),
				_surface,
				_surfaceCapabilites.minImageCount,
				_surfaceFormat.format,
				_surfaceFormat.colorSpace,
				_surfaceCapabilites.currentExtent,
				1,
				vk::ImageUsageFlagBits::eColorAttachment,
				vk::SharingMode::eExclusive,
				0,
				nullptr,
				_surfaceCapabilites.currentTransform,
				vk::CompositeAlphaFlagBitsKHR::eOpaque,
                vk::PresentModeKHR::eImmediate,
				VK_TRUE,
				nullptr
			);
			
			_swapchain = _device.createSwapchainKHR(ci);
		}

		void init_swapchain_image_views() {
			_swapchainImages = _device.getSwapchainImagesKHR(_swapchain);

			for (auto& i : _swapchainImages) {
				auto ci = vk::ImageViewCreateInfo(
					vk::ImageViewCreateFlags(),
					i,
					vk::ImageViewType::e2D,
					_surfaceFormat.format,
					vk::ComponentMapping(),
					vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS)
				);

				_swapchainImageViews.push_back(_device.createImageView(ci));
			}
		}

		void init_depth_image_and_view(Image* image) {
			_depthImage = image->_object;
			auto ci = vk::ImageViewCreateInfo(
				vk::ImageViewCreateFlags(),
				_depthImage,
				vk::ImageViewType::e2D,
				image->_format,
				vk::ComponentMapping(),
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS)
			);

			_depthImageView = _device.createImageView(ci);
		}

		void init_sampler() {
			auto ci = vk::SamplerCreateInfo(
				vk::SamplerCreateFlags(),
				vk::Filter::eLinear,
				vk::Filter::eLinear,
				vk::SamplerMipmapMode::eLinear,
				vk::SamplerAddressMode::eRepeat,
				vk::SamplerAddressMode::eRepeat,
				vk::SamplerAddressMode::eRepeat,
				0.0f,
				false,
				0.0f,
				false,
				vk::CompareOp::eNever,
				0.0f,
				0.0f,
				vk::BorderColor::eIntOpaqueBlack,
				false
			);

			_sampler = _device.createSampler(ci);
		}

		/// <summary>
		/// Initiates the descriptor pool that allows a max of 1 set to be allocated from.
		/// </summary>
		/// <param name="pool">The descriptor pool info containing the sizes of descriptors to have in the pool.</param>
		void init_descriptor_pool(DescriptorPools pool) {
			auto ci = vk::DescriptorPoolCreateInfo(
				vk::DescriptorPoolCreateFlags(),
				1,
				pool._poolSizes.size(),
				pool._poolSizes.data()
			);

			_descriptorPool = _device.createDescriptorPool(ci);
		}

		/// <summary>
		/// Initiates the descriptor set layout.
		/// </summary>
		/// <param name="bindings">The layout's bindings description.</param>
		void init_descriptor_set_layout(DescriptorSetLayoutBindings bindings) {
			auto ci = vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),
				bindings._bindings.size(),
				bindings._bindings.data()
			);

			_descriptorSetLayout = _device.createDescriptorSetLayout(ci);
		}

		/// <summary>
		/// Initiates the descriptor set. The descriptor pool and descriptor set layout must be already initialized.
		/// </summary>
		void init_descriptor_set() {
			auto ai = vk::DescriptorSetAllocateInfo(_descriptorPool, 1, &_descriptorSetLayout);
			_descriptorSet = _device.allocateDescriptorSets(ai).front();
		}

		void init_render_pass(RenderPassAttachments attachments) {
			auto s = vk::SubpassDescription(
				vk::SubpassDescriptionFlags(),
				vk::PipelineBindPoint::eGraphics,
				0,
				nullptr,
				attachments._colorAttachmentReferences.size(),
				attachments._colorAttachmentReferences.data(),
				nullptr,
				&attachments._depthStencilAttahcmentReference,
				0,
				nullptr
			);

			auto ci = vk::RenderPassCreateInfo(
				vk::RenderPassCreateFlags(),
				attachments._attachmentDescriptions.size(),
				attachments._attachmentDescriptions.data(),
				1,
				&s,
				0,
				nullptr
			);

			_renderPass = _device.createRenderPass(ci);
		}

		void init_framebuffers() {
			for (auto& i : _swapchainImageViews) {
				auto attachments = std::array<vk::ImageView, 2>{i, _depthImageView};
				auto ci = vk::FramebufferCreateInfo(
					vk::FramebufferCreateFlags(),
					_renderPass,
					attachments.size(),
					attachments.data(),
					_surfaceCapabilites.currentExtent.width,
					_surfaceCapabilites.currentExtent.height,
					1
				);

				_framebuffers.push_back(_device.createFramebuffer(ci));
			}
		}

		void init_vertex_shader(std::string spvFilePath) {
			auto s = get_shader_data(spvFilePath);
			auto ci = vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),
				s.size(),
				reinterpret_cast<uint32_t*>(s.data())
			);

			_vertexShaderModule = _device.createShaderModule(ci);
		}

		void init_fragment_shader(std::string spvFilePath) {
			auto s = get_shader_data(spvFilePath);
			auto ci = vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),
				s.size(),
				reinterpret_cast<uint32_t*>(s.data())
			);

			_fragmentShaderModule = _device.createShaderModule(ci);
		}


		void init_pipeline_layout() {
			auto ci = vk::PipelineLayoutCreateInfo(
				vk::PipelineLayoutCreateFlags(),
				1,
				&_descriptorSetLayout,
				0,
				nullptr
			);

			_pipelineLayout = _device.createPipelineLayout(ci);
		}

		void init_pipeline(Pipeline pipeline) {
			auto vertexShaderStage = vk::PipelineShaderStageCreateInfo(
				vk::PipelineShaderStageCreateFlags(),
				vk::ShaderStageFlagBits::eVertex,
				_vertexShaderModule,
				"main",
				nullptr
			);

			auto fragmentShaderStage = vertexShaderStage;
			fragmentShaderStage.setStage(vk::ShaderStageFlagBits::eFragment);
			fragmentShaderStage.setModule(_fragmentShaderModule);

			auto shaderStagesCI = std::array<vk::PipelineShaderStageCreateInfo, 2>{vertexShaderStage, fragmentShaderStage};

			auto vertexInputCI = vk::PipelineVertexInputStateCreateInfo(
				vk::PipelineVertexInputStateCreateFlags(),
				pipeline._vertexInputBindingDescriptions.size(),
				pipeline._vertexInputBindingDescriptions.data(),
				pipeline._vertexInputAttributeDescriptions.size(),
				pipeline._vertexInputAttributeDescriptions.data()
			);

			auto inputAssemblyCI = vk::PipelineInputAssemblyStateCreateInfo(
				vk::PipelineInputAssemblyStateCreateFlags(),
				vk::PrimitiveTopology::eTriangleList,
				false
			);

			auto viewportCI = vk::PipelineViewportStateCreateInfo(
				vk::PipelineViewportStateCreateFlags(),
				1,
				nullptr,
				1,
				nullptr
			);

			auto rasterizationCI = vk::PipelineRasterizationStateCreateInfo(
				vk::PipelineRasterizationStateCreateFlags(),
				false,
				false,
				vk::PolygonMode::eFill,
				vk::CullModeFlagBits::eNone,
				vk::FrontFace::eCounterClockwise,
				false,
				0.0f,
				0.0f,
				0.0f,
				1.0f
			);

			auto multisampleCI = vk::PipelineMultisampleStateCreateInfo(
				vk::PipelineMultisampleStateCreateFlags(),
				vk::SampleCountFlagBits::e1,
				false,
				0.0f,
				nullptr,
				false,
				false
			);

			auto depthStencilCI = vk::PipelineDepthStencilStateCreateInfo(
				vk::PipelineDepthStencilStateCreateFlags(),
				true,
				true,
				vk::CompareOp::eLess,
				false,
				false,
				vk::StencilOpState(),
				vk::StencilOpState(),
				0.0f,
				0.0f
			);

			auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState(
				false,
				vk::BlendFactor::eZero,
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd,
				vk::BlendFactor::eZero,
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd,
				static_cast<vk::ColorComponentFlags>(0xF)
			);
			auto colorBlendCI = vk::PipelineColorBlendStateCreateInfo(
				vk::PipelineColorBlendStateCreateFlags(),
				false,
				vk::LogicOp::eNoOp,
				1,
				&colorBlendAttachment,
				{}
			);

			auto dynamicStates = std::array<vk::DynamicState, 2>{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
			auto dynamicCI = vk::PipelineDynamicStateCreateInfo(
				vk::PipelineDynamicStateCreateFlags(),
				dynamicStates.size(),
				dynamicStates.data()
			);

			auto ci = vk::GraphicsPipelineCreateInfo(
				vk::PipelineCreateFlags(),
				shaderStagesCI.size(),
				shaderStagesCI.data(),
				&vertexInputCI,
				&inputAssemblyCI,
				nullptr,
				&viewportCI,
				&rasterizationCI,
				&multisampleCI,
				&depthStencilCI,
				&colorBlendCI,
				&dynamicCI,
				_pipelineLayout,
				_renderPass,
				0,
				vk::Pipeline(),
				0
			);

			_pipeline = _device.createGraphicsPipeline(vk::PipelineCache(), ci).value;
		}

		void init_sync_objects() {
			_imageAcquiredSemaphore = _device.createSemaphore(vk::SemaphoreCreateInfo());
			_imageReadySemaphore = _device.createSemaphore(vk::SemaphoreCreateInfo());
			_fence = _device.createFence(vk::FenceCreateInfo());
		}

		void init_viewport(float x, float y, float width, float height) {
			_viewport = vk::Viewport(x, y, width, height, 0.0f, 1.0f);
		}

		void init_scissor(int32_t offsetX, int32_t offsetY, int32_t width, int32_t height) {
			_scissor = vk::Rect2D(
				vk::Offset2D(offsetX, offsetY),
				vk::Extent2D(width, height)
			);
		}

		void set_vertex_buffer(Buffer* buffer) {
			_vertexBuffer = &buffer->_object;
		}

		void set_index_buffer(Buffer* buffer) {
			_indexBuffer = &buffer->_object;
		}
		void set_index_count(uint32_t count) {
			_indexCount = count;
		}
		/// <summary>
		/// Writes the buffer to the descriptor set.
		/// </summary>
		/// <param name="buffer">The buffer to write from.</param>
		/// <param name="binding">The binding of the descriptor.</param>
		/// <param name="type">The type of descriptor to write to.</param>
		/// <param name="index">The starting index of the descriptor if it is an array.</param>
		/// <param name="count">The number of descriptors after the starting index to write to.</param>
		void write_buffer_to_descriptor_set(Buffer buffer, uint32_t binding, vk::DescriptorType type, uint32_t index = 0, uint32_t count = 1) {
			auto bi = vk::DescriptorBufferInfo(buffer._object, 0, VK_WHOLE_SIZE);
			auto write = vk::WriteDescriptorSet(
				_descriptorSet,
				binding,
				index,
				count,
				type,
				nullptr,
				&bi,
				nullptr
			);

			_device.updateDescriptorSets(write, nullptr);
		}

		void write_image_view_to_descriptor_set(ImageView view, vk::ImageLayout layout, uint32_t binding, vk::DescriptorType type, uint32_t index = 0, uint32_t count = 1) {
			auto ii = vk::DescriptorImageInfo(
				_sampler,
				view._view,
				layout
			);
			auto write = vk::WriteDescriptorSet(
				_descriptorSet,
				binding,
				index,
				count,
				type,
				&ii,
				nullptr,
				nullptr
			);

			_device.updateDescriptorSets(write, nullptr);
		}

		/// <summary>
		/// Creates a buffer with no flags and exclusive sharing mode.
		/// Additionally, it binds the buffer to it's allocated memory.
		/// </summary>
		/// <param name="size">The size of the buffer.</param>
		/// <param name="usage">The usage type of the buffer.</param>
		/// <param name="memProps">The memory properties to use for selecting the memory type to allocate from.</param>
		/// <returns>A buffer resource.</returns>
		Buffer create_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memProps) {
			auto ci = vk::BufferCreateInfo(
				vk::BufferCreateFlags(),
				size,
				usage,
				vk::SharingMode::eExclusive,
				0,
				nullptr
			);
			auto buffer = _device.createBuffer(ci);
			auto bufferMR = _device.getBufferMemoryRequirements(buffer);
			auto bufferMemoryAI = vk::MemoryAllocateInfo(bufferMR.size, get_memory_type_index(bufferMR, memProps));
			auto bufferDM = _device.allocateMemory(bufferMemoryAI);

			_device.bindBufferMemory(buffer, bufferDM, 0);

			return Buffer(buffer, size, bufferMR, bufferDM);
		}

		/// <summary>
		/// Creates a 2D image with no flags, optimal image tiling, exlusive sharing mode, 1 depth, mip level, array layer, and sample count.
		/// Additionally, it binds the image to it's allocated memory.
		/// </summary>
		/// <param name="usage">The usage type of the image.</param>
		/// <param name="format">The format of the image.</param>
		/// <param name="memProps">The memory properties to use for selecting the memory type to allocate from.</param>
		/// <returns>An image resouce.</returns>
		Image create_image_2D(uint32_t width, uint32_t height, uint32_t channels, vk::ImageUsageFlags usage, vk::Format format, vk::MemoryPropertyFlags memProps) {
			auto ci = vk::ImageCreateInfo(
				vk::ImageCreateFlags(),
				vk::ImageType::e2D,
				format,
				vk::Extent3D(width, height, 1),
				1,
				1,
				vk::SampleCountFlagBits::e1,
				vk::ImageTiling::eOptimal,
				usage,
				vk::SharingMode::eExclusive,
				0,
				nullptr,
				vk::ImageLayout::ePreinitialized
			);

			auto image = _device.createImage(ci);
			auto imageMR = _device.getImageMemoryRequirements(image);
			auto imageMemoryAI = vk::MemoryAllocateInfo(imageMR.size, get_memory_type_index(imageMR, memProps));
			auto imageDM = _device.allocateMemory(imageMemoryAI);

			_device.bindImageMemory(image, imageDM, 0);

			vk::DeviceSize size = static_cast<vk::DeviceSize>(width) * height * channels;
			return Image(image, size, imageMR, imageDM, width, height, channels, format, vk::ImageLayout::eUndefined);
		}

		Image create_image_2D_cube(uint32_t length, uint32_t channels, vk::ImageUsageFlags usage, vk::Format format, vk::MemoryPropertyFlags memProps) {
			auto ci = vk::ImageCreateInfo(
				vk::ImageCreateFlagBits::eCubeCompatible,
				vk::ImageType::e2D,
				format,
				vk::Extent3D(length, length, 1),
				1,
				6,
				vk::SampleCountFlagBits::e1,
				vk::ImageTiling::eOptimal,
				usage,
				vk::SharingMode::eExclusive,
				0,
				nullptr,
				vk::ImageLayout::ePreinitialized
			);

			auto image = _device.createImage(ci);
			auto imageMR = _device.getImageMemoryRequirements(image);
			auto imageMemoryAI = vk::MemoryAllocateInfo(imageMR.size, get_memory_type_index(imageMR, memProps));
			auto imageDM = _device.allocateMemory(imageMemoryAI);

			_device.bindImageMemory(image, imageDM, 0);

			vk::DeviceSize size = static_cast<vk::DeviceSize>(length) * length * channels;
			return Image(image, size, imageMR, imageDM, length, length, channels, format, vk::ImageLayout::eUndefined);
		}

		ImageView create_image_view_2D(Image* image, vk::ImageAspectFlags aspects) {
			auto ci = vk::ImageViewCreateInfo(
				vk::ImageViewCreateFlags(),
				image->_object,
				vk::ImageViewType::e2D,
				image->_format,
				vk::ComponentMapping(),
				vk::ImageSubresourceRange(aspects, 0, 1, 0, 1)
			);

			auto view = _device.createImageView(ci);

			return ImageView(*image, view, aspects);
		}

		ImageView create_image_view_2D_cube(Image* image, vk::ImageAspectFlags aspects) {
			auto ci = vk::ImageViewCreateInfo(
				vk::ImageViewCreateFlags(),
				image->_object,
				vk::ImageViewType::eCube,
				image->_format,
				vk::ComponentMapping(),
				vk::ImageSubresourceRange(aspects, 0, 1, 0, 6)
			);
			auto view = _device.createImageView(ci);

			return ImageView(*image, view, aspects);
		}

		/// <summary>
		/// Destroys a resource.
		/// </summary>
		/// <typeparam name="T">The type of resource.</typeparam>
		/// <param name="resource">The resource to destroy.</param>
		template<typename T>
		void destroy_resource(Resource<T>* resource) {
			_device.freeMemory(resource->_deviceMemory);
			if (std::is_same<T, vk::Buffer>::value)
				_device.destroyBuffer(resource->_object);
			else
				_device.destroyImage(resource->_object);

			resource->_memoryRequirements = vk::MemoryRequirements();
		}

		/// <summary>
		/// Copies data to the resources device memory. Assumes that the whole resource size will be used.
		/// </summary>
		/// <typeparam name="T">The resource type.</typeparam>
		/// <param name="resource">The resource to copy data to.</param>
		/// <param name="data">The data to copy from to the resource.</param>
		template<typename T>
		void copy_to_resource_memory(Resource<T>* resource, void* data) {
			void* pMap = nullptr;
            auto res = _device.mapMemory(resource->_deviceMemory, 0, VK_WHOLE_SIZE, vk::MemoryMapFlags(), &pMap);
			memcpy(pMap, data, resource->_deviceSize);
			_device.unmapMemory(resource->_deviceMemory);
		}

		/// <summary>
		/// Starts recording commands.
		/// </summary>
		void cmd_start_recording() {
			_commandBuffer.begin(vk::CommandBufferBeginInfo());
		}

		/// <summary>
		/// Stops recording commands.
		/// </summary>
		void cmd_end_recording() {
			_commandBuffer.end();
		}


		/// <summary>
		/// Copies a buffer to another.
		/// </summary>
		/// <param name="src">The buffer to copy from.</param>
		/// <param name="dst">The buffer to copy to.</param>
		void cmd_copy_buffer(Buffer* src, Buffer* dst, vk::DeviceSize dataSize = 0) {
			auto bufferCopy = vk::BufferCopy(0, 0, dataSize == 0 ? src->_deviceSize : dataSize);
			_commandBuffer.copyBuffer(src->_object, dst->_object, bufferCopy);
		}

		/// <summary>
		/// Copys a buffer to an image.
		/// </summary>
		/// <param name="src">The buffer to copy from.</param>
		/// <param name="dst">The image to copy to.</param>
		/// <param name="extent">The dimensions of the image.</param>
		/// <param name="aspect">The aspect of the image.</param>
		void cmd_copy_buffer_to_image(Buffer* src, Image* dst, vk::ImageAspectFlags aspect) {
			auto bufferImageCopy = vk::BufferImageCopy(
				0,
				0,
				0,
				vk::ImageSubresourceLayers(aspect, 0, 0, 1),
				vk::Offset3D(0, 0, 0),
				vk::Extent3D(dst->_width, dst->_height, 1)
			);
			_commandBuffer.copyBufferToImage(src->_object, dst->_object, vk::ImageLayout::eTransferDstOptimal, bufferImageCopy);
		}

		void cmd_copy_buffer_to_image_cube(Buffer* src, Image* dst, vk::ImageAspectFlags aspect) {
			auto copies = std::vector<vk::BufferImageCopy>();
			copies.reserve(6);

			for (uint32_t i = 0; i < 6; i++) {
				auto bufferImageCopy = vk::BufferImageCopy(
					0,
					0,
					0,
					vk::ImageSubresourceLayers(aspect, 0, i, 1),
					vk::Offset3D(0, 0, 0),
					vk::Extent3D(dst->_width, dst->_height, 1)
				);
				copies.push_back(bufferImageCopy);
			}
			_commandBuffer.copyBufferToImage(src->_object, dst->_object, vk::ImageLayout::eTransferDstOptimal, copies);
		}

		void cmd_change_image_layout(Image* image, vk::AccessFlags srcAccess, vk::AccessFlags dstAccess, vk::ImageLayout layout, vk::ImageAspectFlags aspects, vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage) {
			auto b = vk::ImageMemoryBarrier(
				srcAccess,
				dstAccess,
				image->_imageLayout,
				layout,
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				image->_object,
				vk::ImageSubresourceRange(aspects, 0, 1, 0, 1)
			);

			_commandBuffer.pipelineBarrier(
				srcStage,
				dstStage,
				vk::DependencyFlags(),
				nullptr,
				nullptr,
				b
			);

			image->_imageLayout = layout;
		}

		void cmd_change_image_cube_layout(Image* image, vk::AccessFlags srcAccess, vk::AccessFlags dstAccess, vk::ImageLayout layout, vk::ImageAspectFlags aspects, vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage) {
			for (uint32_t i = 0; i < 6; i++) {
				auto b = vk::ImageMemoryBarrier(
					srcAccess,
					dstAccess,
					image->_imageLayout,
					layout,
					VK_QUEUE_FAMILY_IGNORED,
					VK_QUEUE_FAMILY_IGNORED,
					image->_object,
					vk::ImageSubresourceRange(aspects, 0, 1, i, 1)
				);

				_commandBuffer.pipelineBarrier(
					srcStage,
					dstStage,
					vk::DependencyFlags(),
					nullptr,
					nullptr,
					b
				);
			}
			
			image->_imageLayout = layout;
		}

		/// <summary>
		/// Submits commands to the queue.
		/// </summary>
		/// <param name="waitSems">The semaphores to wait for.</param>
		/// <param name="semsWaitStages">The pipeline stages to wait semaphores for the semaphores.</param>
		/// <param name="signalSems">The semaphores to signal.</param>
		/// <param name="fence">The fence to sync to.</param>
		void submit_commands(std::vector<vk::Semaphore> waitSems = {}, std::vector<vk::PipelineStageFlags> semsWaitStages = {}, std::vector<vk::Semaphore> signalSems = {}, vk::Fence fence = {}) {
			auto submitInfo = vk::SubmitInfo(
				waitSems.size(),
				waitSems.data(),
				semsWaitStages.data(),
				1,
				&_commandBuffer,
				signalSems.size(),
				signalSems.data()
			);

			_queue.submit(submitInfo, fence);
			_queue.waitIdle();
		}

		void render() {
			if (_imageIndex >= _swapchainImages.size()) {
				_imageIndex = 0;
			}

            vk::Result res = _device.acquireNextImageKHR(_swapchain, UINT64_MAX, _imageAcquiredSemaphore, nullptr, &_imageIndex);

			auto clearValues = std::array<vk::ClearValue, 2>{
				vk::ClearValue(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 255.0f}),
					vk::ClearValue({ 1, 0 })
			};
			auto piplineStageFlags = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
			auto submitInfo = vk::SubmitInfo(
				1,
				&_imageAcquiredSemaphore,
				&piplineStageFlags,
				1,
				&_commandBuffer,
				1,
				&_imageReadySemaphore
			);
			auto presentInfo = vk::PresentInfoKHR(
				1,
				&_imageReadySemaphore,
				1,
				&_swapchain,
				&_imageIndex,
				nullptr
			);


			auto renderPassBI = vk::RenderPassBeginInfo(
				_renderPass,
				_framebuffers[_imageIndex],
				_scissor,
				clearValues.size(),
				clearValues.data()
			);

			_commandBuffer.begin(vk::CommandBufferBeginInfo());
			_commandBuffer.beginRenderPass(renderPassBI, vk::SubpassContents::eInline);
			_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
			_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, _descriptorSet, nullptr);
			_commandBuffer.bindVertexBuffers(0, *_vertexBuffer, static_cast<vk::DeviceSize>(0));
			_commandBuffer.bindIndexBuffer(*_indexBuffer, 0, vk::IndexType::eUint32);
			_commandBuffer.setViewport(0, _viewport);
			_commandBuffer.setScissor(0, _scissor);
			_commandBuffer.drawIndexed(_indexCount, 1, 0, 0, 0);
			_commandBuffer.endRenderPass();
			_commandBuffer.end();

			_queue.submit(submitInfo, _fence);

            res = _queue.presentKHR(presentInfo);

            res = _device.waitForFences(_fence, false, UINT32_MAX);
            _device.resetFences(_fence);

			_imageIndex++;

		}

	protected:
		/// <summary>
		/// Gets the index of the memory type desired.
		/// </summary>
		/// <param name="memReq">The memory requirements of the resource.</param>
		/// <param name="memProps">The memory properties desired for selecting the memory type.</param>
		/// <returns>A positive interger if the memory type exist or -1 if it doesn't.</returns>
		int get_memory_type_index(vk::MemoryRequirements memReq, vk::MemoryPropertyFlags memProps) {
			for (uint32_t i = 0; i < _physicalDeviceMemoryProperties.memoryTypeCount; ++i) {
				if ((memReq.memoryTypeBits & (1 << i)) &&
					((_physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memProps) == memProps))
					return i;
			}

			return -1;
		}

		std::vector<char> get_shader_data(std::string spvFilePath) {
			auto f = std::ifstream(spvFilePath, std::ios::ate | std::ios::binary);
            if(!f.is_open())
                printf("Could not open file");
			auto size = static_cast<size_t>(f.tellg());
			auto data = std::vector<char>(size);
			f.seekg(0);
			f.read(data.data(), size);
			f.close();
			return data;
		}
	};
}
