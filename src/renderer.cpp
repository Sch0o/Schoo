#include"schoo/renderer.hpp"
#include"schoo/Context.hpp"
#include"schoo/vertexData.hpp"

namespace schoo {


    Renderer::Renderer(uint32_t frameNums) {
        this->frameNums = frameNums;
        createSemaphores();
        createFences();
        createCmdBuffers();
        createVertexBuffer();
        loadVertexDate();
    }

    Renderer::~Renderer() {
        vertexBuffer_.reset();

        auto &commandManager = Context::GetInstance().commandManager;
        auto &device = Context::GetInstance().device;

        for (int i = 0; i < frameNums; i++) {
            commandManager->FreeCommandbuffers(cmdBuffers_[i]);
            device.destroySemaphore(imageAvaliables_[i]);
            device.destroySemaphore(imageDrawFinshs_[i]);
            device.destroyFence(cmdAvaliableFences_[i]);
        }

    }

    void Renderer::Render() {
        auto &device = Context::GetInstance().device;
        auto &swapchain = Context::GetInstance().swapchain;
        auto &renderProcess = Context::GetInstance().renderProcess;


        if (Context::GetInstance().device.waitForFences(cmdAvaliableFences_[currentFrame], true,
                                                        std::numeric_limits<uint64_t>::max()) !=
            vk::Result::eSuccess) {
            std::cout << "wait for fence failed" << std::endl;
        }
        device.resetFences(cmdAvaliableFences_[currentFrame]);

        auto result = device.acquireNextImageKHR(swapchain->swapchain,
                                                 std::numeric_limits<uint64_t>::max(),
                                                 imageAvaliables_[currentFrame]);
        if (result.result != vk::Result::eSuccess) {
            throw std::runtime_error("acquire next image failed");
        }
        auto imageIndex = result.value;

        cmdBuffers_[currentFrame].reset();
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdBuffers_[currentFrame].begin(beginInfo);
        {
            cmdBuffers_[currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, renderProcess->pipeline);

            vk::DeviceSize offset=0;
            cmdBuffers_[currentFrame].bindVertexBuffers(0,vertexBuffer_->buffer,offset);

            vk::RenderPassBeginInfo renderPassBeginInfo;
            vk::Rect2D area({0, 0}, {swapchain->width, swapchain->height});
            vk::ClearValue clearValue;
            clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
            renderPassBeginInfo.setRenderPass(renderProcess->renderPass)
                    .setRenderArea(area)
                    .setFramebuffer(swapchain->frameBuffers[imageIndex])
                    .setClearValues(clearValue);

            cmdBuffers_[currentFrame].beginRenderPass(renderPassBeginInfo, {});
            cmdBuffers_[currentFrame].draw(vertices.size(), 1, 0, 0);
            cmdBuffers_[currentFrame].endRenderPass();
        }
        cmdBuffers_[currentFrame].end();

        vk::SubmitInfo submitInfo;
        std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submitInfo.setCommandBuffers(cmdBuffers_[currentFrame])
                .setWaitSemaphores(imageAvaliables_[currentFrame])
                .setSignalSemaphores(imageDrawFinshs_[currentFrame])
                .setWaitDstStageMask(waitStages);
        Context::GetInstance().graphicsQueue.submit(submitInfo, cmdAvaliableFences_[currentFrame]);

        vk::PresentInfoKHR presentInfoKhr;
        presentInfoKhr.setImageIndices(imageIndex)
                .setSwapchains(swapchain->swapchain)
                .setWaitSemaphores(imageDrawFinshs_[currentFrame]);
        if (Context::GetInstance().presentQueue.presentKHR(presentInfoKhr) != vk::Result::eSuccess) {
            std::cout << "image present failed" << std::endl;
        }


        currentFrame = (currentFrame + 1) % frameNums;
    }

    void Renderer::createSemaphores() {
        imageAvaliables_.resize(frameNums);
        imageDrawFinshs_.resize(frameNums);

        vk::SemaphoreCreateInfo createInfo;
        for (int i = 0; i < frameNums; i++) {
            imageAvaliables_[i] = Context::GetInstance().device.createSemaphore(createInfo);
            imageDrawFinshs_[i] = Context::GetInstance().device.createSemaphore(createInfo);
        }
    }

    void Renderer::createFences() {
        cmdAvaliableFences_.resize(frameNums, nullptr);
        vk::FenceCreateInfo createInfo;
        createInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
        for (int i = 0; i < frameNums; i++) {
            cmdAvaliableFences_[i] = Context::GetInstance().device.createFence(createInfo);
        }
    }

    void Renderer::createCmdBuffers() {
        cmdBuffers_ = Context::GetInstance().commandManager->AllocateCmdBuffers(frameNums);
    }

    void Renderer::createVertexBuffer() {
        vk::DeviceSize bufferSize=sizeof(vertices[0])*vertices.size();
        vertexBuffer_.reset(new Buffer(bufferSize,
                                       vk::BufferUsageFlagBits::eVertexBuffer,
                                       vk::MemoryPropertyFlagBits::eHostCoherent|vk::MemoryPropertyFlagBits::eHostVisible));
    }

    void Renderer::loadVertexDate() {
        auto &device=Context::GetInstance().device;
        void*data=device.mapMemory(vertexBuffer_->memory,0,vertexBuffer_->size);
        memcpy(data,vertices.data(),vertexBuffer_->size);
        device.unmapMemory(vertexBuffer_->memory);

    }

}
