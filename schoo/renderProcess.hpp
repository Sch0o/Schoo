#pragma once
#include"vulkan/vulkan.hpp"

namespace schoo{
    class RenderProcess final{
    public:
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;
        vk::RenderPass meshRenderPass;
        vk::RenderPass uiRenderPass;
        vk::DescriptorSetLayout vpSetLayout;
        vk::DescriptorSetLayout modelSetLayout;

        RenderProcess();
        ~RenderProcess();

        void CreateRenderPass();
        void CreatePipeline();
    private:
        vk::PipelineLayout createLayout();
        void createSetLayout();
        void createGuiRenderPass();
        void createMeshRenderPass();
    };
}