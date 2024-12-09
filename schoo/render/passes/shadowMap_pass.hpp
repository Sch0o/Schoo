#pragma once
#include"render/renderPass.hpp"
#include"render/model.hpp"

namespace schoo {
    class ShadowMapPass : public RenderPass {
    public:

        struct UniformDataoffscreen {
            glm::mat4 depthMVP;
        }uboData;

        const uint32_t shadowMapSize{2048};
        const vk::Format depthFormat=vk::Format::eD16Unorm;


        std::vector<std::shared_ptr<Model>> renderResource;
        std::vector<vk::CommandBuffer> cmdBuffers;

        std::vector<FrameBuffer>framebuffers;
        vk::RenderPass renderPass;
        RenderPipeline renderPipeline;

        UniformBuffer mvp;

        vk::DescriptorPool descriptorPool;
        vk::DescriptorSetLayout setLayout;
        vk::DescriptorSet writerDescriptorSet;

        void draw() override;
        void createRenderPass();
        void createFrameBuffer();
        void setupPipeline();
        void setupDescriptors();
        void initUniform();
        void updateUniformBuffer();
        void init();

        ~ShadowMapPass();

    };
}