#pragma once
#include"schoo/function/render/vulkan/renderPass.hpp"
#include"schoo/resource/objModel.hpp"

namespace schoo{
    class OutLinePass:public RenderPass{
    public:
        vk::DescriptorSetLayout globalSetLayout;
        std::vector<vk::DescriptorSet>globalSets;

        std::vector<FrameBuffer> frameBuffers;

        struct UniformData{
            glm::mat4 view;
            glm::mat4 projection;
        }uniformData;

        void init();

        void draw() override;

        ~OutLinePass();

    private:
        void setupFrameBuffers();

        void setupPipeline();

        void setupRnderPass();

        void setupDescriptor();
    };
}