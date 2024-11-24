#pragma once

#include"vulkan/vulkan.hpp"
#include "command.hpp"

namespace schoo {
    class WindowGui {
    public:
        static std::unique_ptr<WindowGui> instance_;
        uint32_t minImageCount_ = 2;

        vk::DescriptorPool descriptorPool_;
        std::vector<vk::CommandBuffer> cmdBuffers_;
        std::vector<vk::Framebuffer> frameBuffers_;
        std::shared_ptr<Command> commandManager;


        WindowGui();

        ~WindowGui();

        void ImGuiInit();

        void createDescriptorPool();

        void createCommandBuffers();

        void Render();


        static void check_vk_result(VkResult err);

        void createFramebuffers();

        void drawWindows();
    };
}