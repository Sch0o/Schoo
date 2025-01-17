#pragma once
#include"function/render/vulkan/renderPass.hpp"

namespace schoo {
    class FxaaPass : public RenderPass {
    public:
        std::vector<vk::Framebuffer> frameBuffers;

        vk::DescriptorSetLayout textureSetLayout;

        std::vector<vk::DescriptorSet> sets;

        void init();

        void draw() override;

        ~FxaaPass();

    private:

        std::shared_ptr<Buffer> vertexBuffer;

        std::shared_ptr<Buffer> indexBuffer;

        void setupBuffers();

        void setupFrameBuffers();

        void setupPipeline();

        void setupRenderPass();

        void setupDescriptors();
    };
}