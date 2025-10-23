#pragma once
#include"schoo/function/render/vulkan/renderPass.hpp"
#include"schoo/function/render/vulkan/command.hpp"
namespace schoo {
    class UIPass : public RenderPass {
    public:
        std::vector<FrameBufferAttachment> frameBufferAttachments;
        std::vector<vk::Framebuffer> frameBuffers;
        std::shared_ptr<Command> commandManager;
        std::vector<vk::CommandBuffer> cmdBuffers;
        vk::DescriptorPool descriptorPool;
        vk::RenderPass renderPass;

        UIPass();

        ~UIPass();

        void createCommandBuffers();

        void draw() override;

        void createDescriptorPool();

        static void check_vk_result(VkResult err);

        void createFrameBuffers();

        static void drawWindows();

        static void drawFPS();

        static void drawObjectTree();

        void createRenderPass() ;

        void initImGui();
    };
}