#pragma once
#include<iostream>
#include"vulkan/vulkan.hpp"
namespace schoo {
    class RenderPass {
    public:
        struct FrameBufferAttachment {
            vk::Image image;
            vk::DeviceMemory memory;
            vk::ImageView imageView;
            vk::Format format;
        };
        struct FrameBuffer {
            uint32_t width;
            uint32_t height;
            vk::RenderPass renderPass;
            vk::Framebuffer framebuffer;
            std::vector<FrameBufferAttachment>attachments;
        };

        struct RenderPipeline {
            vk::Pipeline pipeline;
            vk::PipelineLayout layout;
        };

        virtual void draw() = 0;

    };
}