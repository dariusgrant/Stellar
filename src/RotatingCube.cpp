#include "RendererCore.hpp"

class MyRenderer : public stlr::RendererCore {
protected:
    std::array<vk::DescriptorPoolSize, 1> descriptor_pool_sizes =
    {
        vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 )
    };

    std::array<vk::DescriptorSetLayoutBinding, 1> descriptor_set_layout_bindings =
    {
        vk::DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr )
    };

    std::array<vk::AttachmentDescription, 2> attachment_descriptions {
        vk::AttachmentDescription(
            {},
            swapchain.format,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::ePreinitialized,
            vk::ImageLayout::ePresentSrcKHR
        ),
        vk::AttachmentDescription(
            {},
            depth_image._format,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::ePreinitialized,
            vk::ImageLayout::eDepthStencilAttachmentOptimal
        )
    };

    Subpass subpass {
        {
            vk::AttachmentReference( 0, vk::ImageLayout::eColorAttachmentOptimal )
        },
        vk::AttachmentReference( 1, vk::ImageLayout::eDepthStencilAttachmentOptimal )
    };

    vk::UniqueDescriptorPool descriptor_pool;
    vk::UniqueDescriptorSetLayout descriptor_set_layout;
    vk::UniqueDescriptorSet descriptor_set;
    vk::UniqueRenderPass render_pass;
    std::vector<vk::UniqueFramebuffer> framebuffers;
    vk::UniqueShaderModule vertex_shader_module;
    vk::UniqueShaderModule fragment_shader_module;
    RendererCore::Buffer vertex_buffer;
    RendererCore::Buffer index_buffer;
    vk::UniquePipelineLayout pipeline_layout;
    vk::UniquePipeline pipeline;
    vk::UniqueSemaphore image_acquired_semaphore;
    vk::UniqueSemaphore image_ready_semaphore;
    vk::UniqueFence fence;
    vk::Viewport viewport;
    vk::Rect2D scissor;

public:
    MyRenderer( stlr::Window& w )
        : stlr::RendererCore( w )
        , descriptor_pool( create_descriptor_pool( descriptor_pool_sizes ) )
        , descriptor_set_layout( create_descriptor_set_layout( descriptor_set_layout_bindings ) )
        , descriptor_set( allocate_descriptor_set( descriptor_pool, descriptor_set_layout ) )
        , render_pass( create_render_pass( attachment_descriptions, subpass ) )
        , framebuffers( )
        , vertex_shader_module( create_shader_module( "../shaders/2-vs.spv" ) )
        , fragment_shader_module( create_shader_module( "../shaders/2-fs.spv" ) )
        , vertex_buffer( create_buffer( 1024, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent ) )
        , index_buffer( create_buffer( 1024, vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent ) )
        , pipeline_layout( create_pipeline_layout( { &descriptor_set_layout } ) )
    {
        std::array<vk::UniqueImageView*, 2> image_view_attachments;
        image_view_attachments[1] = &depth_image_view;
        for(  auto& i : swapchain.image_views ) {
            image_view_attachments[0] = &i;
            framebuffers.push_back( create_framebuffer( render_pass, image_view_attachments ) );
        }
    }

protected:
    void update() {}
    void render() {}
};

int main(int argc, char** argv) {
    stlr::Window w;
    MyRenderer r(w);
    return 0;
}
