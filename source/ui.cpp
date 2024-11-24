#include"ui.hpp"
#include"imgui/imgui.h"
#include"imgui/imgui_impl_glfw.h"
#include"imgui/imgui_impl_vulkan.h"
#include"context.hpp"

namespace schoo {
    WindowGui::WindowGui() {
        commandManager.reset(new Command());
        createDescriptorPool();
        createFramebuffers();
        createCommandBuffers();
        ImGuiInit();
    }

    void WindowGui::ImGuiInit() {
        //setup context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

        //setup Gui style
        ImGui::StyleColorsDark();
        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer backends
        Context &context = Context::GetInstance();
        ImGui_ImplGlfw_InitForVulkan(Window::GetInstance().glfwWindow, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = context.instance;
        init_info.PhysicalDevice = context.physicalDevice;
        init_info.Device = context.device;
        init_info.QueueFamily = context.queueFamilyIndices.graphicQueue.value();
        init_info.Queue = context.graphicsQueue;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = descriptorPool_;
        init_info.RenderPass = context.renderProcess->uiRenderPass;
        init_info.Subpass = 0;
        init_info.MinImageCount = minImageCount_;
        init_info.ImageCount = minImageCount_;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = VK_NULL_HANDLE;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info);
    }

    void WindowGui::check_vk_result(VkResult err) {
        if (err == 0)
            return;
        std::cout << "[vulkan] Error: VkResult = " << err << std::endl;
        if (err < 0)
            abort();
    }

    WindowGui::~WindowGui() {
        auto&device=Context::GetInstance().device;
        for (int i = 0; i < cmdBuffers_.size(); i++) {
            device.destroyFramebuffer(frameBuffers_[i]);
            commandManager->FreeCommandbuffers(cmdBuffers_[i]);
        }
        commandManager.reset();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        device.destroyDescriptorPool(descriptorPool_);
    }

    void WindowGui::Render() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawWindows();

        auto &swapchain = Context::GetInstance().swapchain;
        auto &renderProcess = Context::GetInstance().renderProcess;
        uint32_t currentFrame = Context::GetInstance().renderer->currentFrame;
        uint32_t imageIndex=Context::GetInstance().renderer->imageIndex;

        cmdBuffers_[currentFrame].reset();

        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdBuffers_[currentFrame].begin(beginInfo);
        {
            vk::RenderPassBeginInfo renderPassBeginInfo;
            vk::Rect2D area({0, 0}, {swapchain->width, swapchain->height});
            std::array<vk::ClearValue, 1> clearValues;
            clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
            renderPassBeginInfo.setRenderPass(renderProcess->uiRenderPass)
                    .setRenderArea(area)
                    .setFramebuffer(frameBuffers_[imageIndex])
                    .setClearValues(clearValues);
            cmdBuffers_[currentFrame].beginRenderPass(renderPassBeginInfo, {});
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffers_[currentFrame]);
            cmdBuffers_[currentFrame].endRenderPass();
        }
        cmdBuffers_[currentFrame].end();

        ImGui::UpdatePlatformWindows();
    }

    void WindowGui::drawWindows() {
        ImGuiIO &io = ImGui::GetIO();

        ImGui::SetNextWindowSize(ImVec2(350,60));
        ImGui::Begin(
                "detail ");                          // Create a window called "Hello, world!" and append into it.
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
        ImGui::Render();
    }

    void WindowGui::createDescriptorPool() {
        std::array<vk::DescriptorPoolSize, 1> poolSizes;
        poolSizes[0].setType(vk::DescriptorType::eCombinedImageSampler)
                .setDescriptorCount(1);
        vk::DescriptorPoolCreateInfo pool_info;
        pool_info.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
        pool_info.setMaxSets(1);
        pool_info.setPoolSizes(poolSizes);
        descriptorPool_ = Context::GetInstance().device.createDescriptorPool(pool_info);
    }

    void WindowGui::createFramebuffers() {
        Context &context = Context::GetInstance();
        frameBuffers_.resize(context.swapchain->imageViews.size());
        for (int i = 0; i < frameBuffers_.size(); i++) {
            vk::FramebufferCreateInfo createInfo;
            std::array<vk::ImageView, 1> attachments = {context.swapchain->imageViews[i]};
            createInfo.setAttachments(attachments)
                    .setWidth(context.swapchain->width)
                    .setHeight(context.swapchain->height)
                    .setRenderPass(Context::GetInstance().renderProcess->uiRenderPass)
                    .setLayers(1);
            frameBuffers_[i] = Context::GetInstance().device.createFramebuffer(createInfo);
        }
    }

    void WindowGui::createCommandBuffers() {
        cmdBuffers_ = commandManager->AllocateCmdBuffers(2);
    }


}