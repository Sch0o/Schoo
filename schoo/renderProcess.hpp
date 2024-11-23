#pragma once
#include"vulkan/vulkan.hpp"

namespace schoo{
    class RenderProcess final{
    public:
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;
        vk::RenderPass renderPass;
        vk::DescriptorSetLayout vpSetLayout;
        vk::DescriptorSetLayout modelSetLayout;

        RenderProcess();
        ~RenderProcess();

        void CreateRenderPass();
        void CreatePipeline();
    private:
        vk::PipelineLayout createLayout();
        void createSetLayout();
    };
}