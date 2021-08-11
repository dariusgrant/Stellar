#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <QGuiApplication>
#include "DGVulkan.hpp"


uint32_t _windowWidth = 1920.0f, _windowHeight = 1080.0f;
float _windowAspectRatio = static_cast<float>(_windowWidth) / _windowHeight;
glm::vec3 cameraPosition = glm::vec3(0, 0, -5);
glm::vec3 lookAtPosition = glm::vec3(0, 0, 0);
glm::vec3 upDirection = glm::vec3(0, -1, 0);
std::string shaderDirectory = "/home/shinobu/Qt-Projects/StellarEngine/shaders/";

int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);

    DG::DGVulkan b{_windowWidth, _windowHeight};
    b.init_surface_and_swapchain();
    b.init_swapchain_image_views();

    auto depthImage = b.create_image_2D(b.get_surface_width(), b.get_surface_height(), 1, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::Format::eD32Sfloat, vk::MemoryPropertyFlagBits::eDeviceLocal);
    b.init_depth_image_and_view(&depthImage);

    DG::RenderPassAttachments renderPassAttachments;
    renderPassAttachments.add_attachment(b.get_surface_format(), vk::ImageLayout::ePresentSrcKHR, false);
    renderPassAttachments.add_attachment(vk::Format::eD32Sfloat, vk::ImageLayout::eDepthStencilReadOnlyOptimal, true);
    b.init_render_pass(renderPassAttachments);

    b.init_framebuffers();

    DG::DescriptorPools descriptorPools;
    descriptorPools.add_descriptor_size(vk::DescriptorType::eUniformBuffer, 1);
    b.init_descriptor_pool(descriptorPools);
    DG::DescriptorSetLayoutBindings descriptorSetLayoutBindings;
    descriptorSetLayoutBindings.add_binding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
    b.init_descriptor_set_layout(descriptorSetLayoutBindings);
    b.init_descriptor_set();

    b.init_vertex_shader(shaderDirectory + "1-vs.spv");
    b.init_fragment_shader(shaderDirectory + "1-fs.spv");
    b.init_pipeline_layout();
    DG::Pipeline pipeline;
    pipeline.add_vertex_input_binding(0, sizeof(float) * 3);
    pipeline.add_vertex_input_attribute(0, 0, vk::Format::eR32G32B32Sfloat, 0);
    b.init_pipeline(pipeline);
    b.init_sync_objects();
    b.init_viewport(0, 0, b.get_surface_width(), b.get_surface_height());
    b.init_scissor(0, 0, b.get_surface_width(), b.get_surface_height());
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), _windowAspectRatio, 0.1f, 1000.0f);
    glm::mat4 view = glm::lookAt(
        cameraPosition,
        lookAtPosition,
        upDirection
    );
    glm::mat4 model = glm::mat4(1);
    glm::mat4 mvp = projection * view * model;

    auto uniformStagingBuffer = b.create_buffer(sizeof(mvp), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);

    b.copy_to_resource_memory(&uniformStagingBuffer, &mvp);

    auto uniformBuffer = b.create_buffer(sizeof(mvp), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

    b.cmd_start_recording();
    b.cmd_copy_buffer(&uniformStagingBuffer, &uniformBuffer);
    b.cmd_end_recording();
    b.submit_commands();

    b.write_buffer_to_descriptor_set(uniformBuffer, 0, vk::DescriptorType::eUniformBuffer);

    float vertices[9] = {
        -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
    };
    auto vertexBuffer = b.create_buffer(sizeof(vertices), vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	
    b.copy_to_resource_memory(&vertexBuffer, &vertices);

    int indices[3] = { 0, 1, 2 };
    auto indexBuffer = b.create_buffer(sizeof(indices), vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);

    b.copy_to_resource_memory(&indexBuffer, &indices);

    b.set_vertex_buffer(&vertexBuffer);
    b.set_index_buffer(&indexBuffer);
    b.set_index_count(sizeof(indices) / sizeof(float));

    b.render();


    return app.exec();
}
