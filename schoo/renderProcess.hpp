#pragma once
#include"vulkan/vulkan.hpp"

namespace schoo{
    class RenderProcess final{
    public:
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;
        vk::RenderPass renderPass;

        ~RenderProcess();

        void InitLayout();
        void InitRenderPass();
        void InitPipeline();
    };
}