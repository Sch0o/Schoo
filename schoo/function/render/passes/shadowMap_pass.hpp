#pragma once
#include"schoo/function/render/vulkan/renderPass.hpp"
#include"schoo/resource/objModel.hpp"

namespace schoo {
    class ShadowMapPass : public RenderPass {
    public:

        struct UniformDataoffscreen {
            glm::mat4 depthMVP;
        }uboData;

        const uint32_t shadowMapSize{2048};
        const vk::Format depthFormat=vk::Format::eD16Unorm;

        std::vector<FrameBuffer>framebuffers;


        UniformBuffer mvp;

        vk::DescriptorSetLayout setLayout;
        vk::DescriptorSet writerDescriptorSet;

        void draw() override;
        void createRenderPass();
        void createFrameBuffer();
        void setupPipeline();
        void setupDescriptors();
        void updateUniform();
        void updateUniformBuffer();
        void init();

        ~ShadowMapPass();

    };
}