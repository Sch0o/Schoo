#pragma once
#include<iostream>
#include"vulkan/vulkan.hpp"
#include"buffer.hpp"
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
            vk::Framebuffer framebuffer;
            std::vector<FrameBufferAttachment>attachments;
            vk::ImageView getImageView(int index);
        };

        struct RenderPipeline {
            vk::Pipeline pipeline;
            vk::PipelineLayout layout;
        };

        struct UniformBuffer{
            std::shared_ptr<Buffer>stagingBuffer;
            std::shared_ptr<Buffer>deviceBuffer;

            void destoryUniformBuffer(){
                stagingBuffer.reset();
                deviceBuffer.reset();
            }
        };

        std::vector<vk::CommandBuffer> cmdBuffers;
        vk::DescriptorPool descriptorPool;
        vk::RenderPass renderPass;
        RenderPipeline renderPipeline;
        vk::Sampler sampler;

        virtual void draw() = 0;

    };
}