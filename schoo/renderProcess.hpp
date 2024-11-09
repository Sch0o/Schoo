#pragma once
#include"vulkan/vulkan.hpp"

namespace schoo{
    class RenderProcess final{
    public:
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;
        vk::RenderPass renderPass;
        vk::DescriptorSetLayout setLayout;

        RenderProcess();
        ~RenderProcess();

        void InitLayout();
        void CreateRenderPass();
        void CreatePipeline();
    private:
        vk::PipelineLayout createLayout();
        vk::DescriptorSetLayout createSetLayout();
    };
}