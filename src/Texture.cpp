#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm.hpp>
#include <gtx/transform.hpp>
#include <DGVulkan.hpp>


float _windowWidth = 1920.0f, _windowHeight = 1080.0f;
float _windowAspectRatio = static_cast<float>(_windowWidth) / _windowHeight;
glm::vec3 cameraPosition = glm::vec3(0, 3, -5);
glm::vec3 lookAtPosition = glm::vec3(0, 0, 0);
glm::vec3 upDirection = glm::vec3(0, -1, 0);
std::string shaderDirectory = "../SharedItems/assets/shaders/";
std::string textureDirectory = "../SharedItems/assets/textures/";

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	auto window = glfwCreateWindow(_windowWidth, _windowHeight, "Title", nullptr, nullptr);

	auto b = new DG::DGVulkan();
	b->init_surface_and_swapchain(glfwGetWin32Window(window));
	b->init_swapchain_image_views();

	auto depthImage = b->create_image_2D(_windowWidth, _windowHeight, 1, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::Format::eD32Sfloat, vk::MemoryPropertyFlagBits::eDeviceLocal);
	b->init_depth_image_and_view(&depthImage);

	b->init_sampler();

	DG::RenderPassAttachments renderPassAttachments;
	renderPassAttachments.add_attachment(b->get_surface_format(), vk::ImageLayout::ePresentSrcKHR, false);
	renderPassAttachments.add_attachment(vk::Format::eD32Sfloat, vk::ImageLayout::eDepthStencilReadOnlyOptimal, true);
	b->init_render_pass(renderPassAttachments);

	b->init_framebuffers();

	DG::DescriptorPools descriptorPools;
	descriptorPools.add_descriptor_size(vk::DescriptorType::eUniformBuffer, 1);
	descriptorPools.add_descriptor_size(vk::DescriptorType::eCombinedImageSampler, 1);
	b->init_descriptor_pool(descriptorPools);
	DG::DescriptorSetLayoutBindings descriptorSetLayoutBindings;
	descriptorSetLayoutBindings.add_binding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
	descriptorSetLayoutBindings.add_binding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	b->init_descriptor_set_layout(descriptorSetLayoutBindings);
	b->init_descriptor_set();

	b->init_vertex_shader(shaderDirectory + "3-vs.spv");
	b->init_fragment_shader(shaderDirectory + "3-fs.spv");
	b->init_pipeline_layout();
	DG::Pipeline pipeline;
	pipeline.add_vertex_input_binding(0, sizeof(float) * 5);	// Input is 5 floats - xyz & uv
	pipeline.add_vertex_input_attribute(0, 0, vk::Format::eR32G32B32Sfloat, 0); // No offset for position
	pipeline.add_vertex_input_attribute(0, 1, vk::Format::eR32G32Sfloat, sizeof(float) * 3); // The offset from position
	b->init_pipeline(pipeline);
	b->init_sync_objects();
	b->init_viewport(0, 0, _windowWidth, _windowHeight);
	b->init_scissor(0, 0, _windowWidth, _windowHeight);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), _windowAspectRatio, 0.1f, 1000.0f);
	glm::mat4 view = glm::lookAt(
		cameraPosition,
		lookAtPosition,
		upDirection
	);
	glm::mat4 model = glm::mat4(1);
	glm::mat4 mvp = projection * view * model;

	auto uniformBuffer = b->create_buffer(sizeof(mvp), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);

	b->copy_to_resource_memory(&uniformBuffer, &mvp);

	b->write_buffer_to_descriptor_set(uniformBuffer, 0, vk::DescriptorType::eUniformBuffer);

	float vertices[] = {
		-1.0f, -1.0f, -1.0f, // left-bottom-front
		0.0f, 0.0f,
		-1.0f, 1.0f, -1.0f,  // left-top-front
		0.0f, 1.0f,

		1.0f, -1.0f, -1.0f, // right-bottom-front
		1.0f, 0.0f,
		1.0f, 1.0f, -1.0f,  // right-top-front
		1.0f, 1.0f,
	};
	auto vertexBuffer = b->create_buffer(sizeof(vertices), vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);

	b->copy_to_resource_memory(&vertexBuffer, &vertices);

	int indices[] = {
		0, 1, 2, 
		2, 1, 3,
	};
	auto indexBuffer = b->create_buffer(sizeof(indices), vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);

	b->copy_to_resource_memory(&indexBuffer, &indices);

	int textureWidth = 0, textureHeight = 0;
	auto textureData = stbi_load((textureDirectory + "Red Stare.jpg").data(), &textureWidth, &textureHeight, nullptr, STBI_rgb_alpha);
	auto textureBufferSize = textureWidth * textureHeight * STBI_rgb_alpha;

	auto textureStagingBuffer = b->create_buffer(static_cast<vk::DeviceSize>(textureBufferSize), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	b->copy_to_resource_memory(&textureStagingBuffer, textureData);

	auto textureImage = b->create_image_2D(textureWidth, textureHeight, 4, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::Format::eR8G8B8A8Srgb, vk::MemoryPropertyFlagBits::eDeviceLocal);

	b->cmd_start_recording();
	b->cmd_change_image_layout(
		&textureImage, 
		vk::AccessFlags(),
		vk::AccessFlagBits::eTransferWrite, 
		vk::ImageLayout::eTransferDstOptimal, 
		vk::ImageAspectFlagBits::eColor, 
		vk::PipelineStageFlagBits::eTopOfPipe, 
		vk::PipelineStageFlagBits::eTransfer
	);
	b->cmd_copy_buffer_to_image(&textureStagingBuffer, &textureImage, vk::ImageAspectFlagBits::eColor);
	b->cmd_change_image_layout(
		&textureImage, 
		vk::AccessFlagBits::eTransferWrite,
		vk::AccessFlagBits::eShaderRead, 
		vk::ImageLayout::eShaderReadOnlyOptimal, 
		vk::ImageAspectFlagBits::eColor, 
		vk::PipelineStageFlagBits::eTransfer, 
		vk::PipelineStageFlagBits::eFragmentShader
	);
	b->cmd_end_recording();
	b->submit_commands();

	auto textureImageView = b->create_image_view_2D(&textureImage, vk::ImageAspectFlagBits::eColor);
	b->write_image_view_to_descriptor_set(textureImageView, vk::ImageLayout::eShaderReadOnlyOptimal, 1, vk::DescriptorType::eCombinedImageSampler);

	b->set_vertex_buffer(&vertexBuffer);
	b->set_index_buffer(&indexBuffer);
	b->set_index_count(sizeof(indices) / sizeof(indices[0]));

	double lastTime = 0.0f;
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		auto currentTime = glfwGetTime();
		auto deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		model = glm::rotate(model, glm::radians(45.0f) * static_cast<float>(deltaTime), { 0, 1, 0 });
		mvp = projection * view * model;

		b->copy_to_resource_memory(&uniformBuffer, &mvp);
		b->write_buffer_to_descriptor_set(uniformBuffer, 0, vk::DescriptorType::eUniformBuffer);

		b->render();
	}

	delete(b);
	return 0;
}
