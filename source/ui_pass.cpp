#include"schoo/render/context.hpp"
#include"render/passes/ui_pass.hpp"
#include"imgui/imgui.h"
#include"imgui/imgui_impl_glfw.h"
#include"imgui/imgui_impl_vulkan.h"

namespace schoo {
    UIPass::UIPass() {
        commandManager.reset(new Command());
        createDescriptorPool();
        createRenderPass();
        createFramebuffers();
        createCommandBuffers();
        initImGui();
    }

    UIPass::~UIPass() {
        auto &device = Context::GetInstance().device;

        for (int i = 0; i < cmdBuffers.size(); i++) {
            device.destroyFramebuffer(frameBuffers[i]);
            commandManager->FreeCommandbuffers(cmdBuffers[i]);
        }
        commandManager.reset();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        device.destroyRenderPass(renderPass);
        device.destroyDescriptorPool(descriptorPool);
    }

    void UIPass::createDescriptorPool() {
        std::array<vk::DescriptorPoolSize, 1> poolSizes;
        poolSizes[0].setType(vk::DescriptorType::eCombinedImageSampler)
                .setDescriptorCount(1);
        vk::DescriptorPoolCreateInfo pool_info;
        pool_info.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
        pool_info.setMaxSets(1);
        pool_info.setPoolSizes(poolSizes);
        descriptorPool = Context::GetInstance().device.createDescriptorPool(pool_info);
    }

    void UIPass::createFramebuffers() {
        Context &context = Context::GetInstance();
        frameBuffers.resize(context.swapchain->imageViews.size());
        for (int i = 0; i < frameBuffers.size(); i++) {
            vk::FramebufferCreateInfo createInfo;
            std::array<vk::ImageView, 1> attachments = {context.swapchain->imageViews[i]};
            createInfo.setAttachments(attachments)
                    .setWidth(context.swapchain->width)
                    .setHeight(context.swapchain->height)
                    .setRenderPass(renderPass)
                    .setLayers(1);
            frameBuffers[i] = Context::GetInstance().device.createFramebuffer(createInfo);
        }
    }

    void UIPass::createCommandBuffers() {
        cmdBuffers = commandManager->AllocateCmdBuffers(2);
    }

    void UIPass::check_vk_result(VkResult err) {
        if (err == 0)
            return;
        std::cout << "[vulkan] Error: VkResult = " << err << std::endl;
        if (err < 0)
            abort();
    }

    void UIPass::createRenderPass() {
        vk::RenderPassCreateInfo createInfo;

        std::array<vk::AttachmentDescription, 1> descriptions;
        descriptions[0].setFormat(Context::GetInstance().swapchain->swapchainInfo.surfaceFormat.format)
                .setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
                .setLoadOp(vk::AttachmentLoadOp::eLoad)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setSamples(vk::SampleCountFlagBits::e1);
        createInfo.setAttachments(descriptions);

        vk::AttachmentReference colorReference;
        colorReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setAttachment(0);
        vk::SubpassDescription subpassDescription;
        subpassDescription.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setColorAttachments(colorReference);
        createInfo.setSubpasses(subpassDescription);


        vk::SubpassDependency dependency;
        dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
                .setDstSubpass(0)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        createInfo.setDependencies(dependency);

        renderPass = Context::GetInstance().device.createRenderPass(createInfo);
    }

    void UIPass::draw() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawWindows();

        auto &swapchain = Context::GetInstance().swapchain;
        uint32_t currentFrame = Context::GetInstance().renderer->currentFrame;
        uint32_t imageIndex = Context::GetInstance().renderer->imageIndex;

        cmdBuffers[currentFrame].reset();

        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdBuffers[currentFrame].begin(beginInfo);
        {
            vk::RenderPassBeginInfo renderPassBeginInfo;
            vk::Rect2D area({0, 0}, {swapchain->width, swapchain->height});
            std::array<vk::ClearValue, 1> clearValues;
            clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
            renderPassBeginInfo.setRenderPass(renderPass)
                    .setRenderArea(area)
                    .setFramebuffer(frameBuffers[imageIndex])
                    .setClearValues(clearValues);
            cmdBuffers[currentFrame].beginRenderPass(renderPassBeginInfo, {});
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffers[currentFrame]);
            cmdBuffers[currentFrame].endRenderPass();
        }
        cmdBuffers[currentFrame].end();

        ImGui::UpdatePlatformWindows();
    }

    void UIPass::drawWindows() {
        bool showMainWindow= true;
        glm::vec3 &light_pos=Context::GetInstance().renderer->lights.plight.position;
        static float light_pos_f[3]={light_pos.x,light_pos.y,light_pos.z};


        ImGuiIO &io = ImGui::GetIO();

//        ImGui::SetNextWindowPos(ImVec2(301, 150));
        ImGui::SetNextWindowSize(ImVec2(300, 200));
        ImGui::Begin("Control menu ", &showMainWindow);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::BeginChild("light position");
        ImGui::Text("light position");
        ImGui::InputFloat3("",light_pos_f);
        ImGui::EndChild();
        ImGui::End();
        ImGui::Render();

        if(light_pos.y!=light_pos_f[1]){
            std::cout<<light_pos.y<<std::endl;
        }
        light_pos={light_pos_f[0],light_pos_f[1],light_pos_f[2]};
    }

    void UIPass::initImGui() {
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
        init_info.DescriptorPool = descriptorPool;
        init_info.RenderPass = renderPass;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = 2;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = VK_NULL_HANDLE;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info);
    }
}