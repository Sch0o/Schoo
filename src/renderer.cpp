#include"schoo/renderer.hpp"
#include"schoo/Context.hpp"
#include"schoo/utils.hpp"
#include"schoo/schoo.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include"glm/ext/matrix_clip_space.hpp"
#include"glm/gtc/matrix_transform.hpp"

namespace schoo {

    Renderer::Renderer(uint32_t frameNums) {

        this->frameNums = frameNums;
        createSemaphores();
        createFences();
        createCmdBuffers();

        createVertexBuffer();
        loadVertexData();

        initVP();

        createUniformBuffer();
        loadUniformData();

        createIndexBuffer();
        loadIndexData();

        createTextures();
        createSampler();

        createDescriptorPool();
        allocateSets();
        writeSets();

    }

    Renderer::~Renderer() {
        auto &commandManager = Context::GetInstance().commandManager;
        auto &device = Context::GetInstance().device;

        device.destroySampler(sampler_);
        texture_.reset();

        device.destroyDescriptorPool(descriptorPool_);

        hostVertexBuffer_.reset();
        deviceVertexBuffer_.reset();
        hostUniformBuffer_.reset();
        deviceUniformBuffer_.reset();


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

            vk::DeviceSize offset = 0;
            cmdBuffers_[currentFrame].bindVertexBuffers(0, deviceVertexBuffer_->buffer, offset);
            cmdBuffers_[currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                         Context::GetInstance().renderProcess->layout, 0,
                                                         sets_[currentFrame], {});
            cmdBuffers_[currentFrame].bindIndexBuffer(deviceIndexBuffer_->buffer, 0, vk::IndexType::eUint32);
            vk::RenderPassBeginInfo renderPassBeginInfo;
            vk::Rect2D area({0, 0}, {swapchain->width, swapchain->height});
            std::array<vk::ClearValue,2> clearValues;
            clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
            clearValues[1].depthStencil=vk::ClearDepthStencilValue(0.99f,0);
            renderPassBeginInfo.setRenderPass(renderProcess->renderPass)
                    .setRenderArea(area)
                    .setFramebuffer(swapchain->frameBuffers[imageIndex])
                    .setClearValues(clearValues);

            cmdBuffers_[currentFrame].beginRenderPass(renderPassBeginInfo, {});
            cmdBuffers_[currentFrame].drawIndexed(indices.size(), 1, 0, 0, 0);
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
        vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
        //std::cout<<"VertexSize:"<<bufferSize<<std::endl;
        hostVertexBuffer_.reset(new Buffer(bufferSize,
                                           vk::BufferUsageFlagBits::eTransferSrc,
                                           vk::MemoryPropertyFlagBits::eHostCoherent |
                                           vk::MemoryPropertyFlagBits::eHostVisible));

        deviceVertexBuffer_.reset(new Buffer(bufferSize,
                                             vk::BufferUsageFlagBits::eVertexBuffer |
                                             vk::BufferUsageFlagBits::eTransferDst,
                                             vk::MemoryPropertyFlagBits::eDeviceLocal));

    }

    void Renderer::createIndexBuffer() {
        size_t bufferSize = sizeof(indices[0]) * indices.size();
        //std::cout << "IndexSize:"<<bufferSize<<std::endl;
        hostIndexBuffer_.reset(new Buffer(bufferSize,
                                          vk::BufferUsageFlagBits::eTransferSrc,
                                          vk::MemoryPropertyFlagBits::eHostCoherent |
                                          vk::MemoryPropertyFlagBits::eHostVisible));

        deviceIndexBuffer_.reset(new Buffer(bufferSize,
                                            vk::BufferUsageFlagBits::eIndexBuffer |
                                            vk::BufferUsageFlagBits::eTransferDst,
                                            vk::MemoryPropertyFlagBits::eDeviceLocal));
    }

    void Renderer::createUniformBuffer() {
        size_t bufferSize = sizeof(vp);
        //std::cout << "UniformSize:"<<bufferSize<<std::endl;
        hostUniformBuffer_.reset(new Buffer(bufferSize,
                                            vk::BufferUsageFlagBits::eTransferSrc,
                                            vk::MemoryPropertyFlagBits::eHostCoherent |
                                            vk::MemoryPropertyFlagBits::eHostVisible));

        deviceUniformBuffer_.reset(new Buffer(bufferSize,
                                              vk::BufferUsageFlagBits::eUniformBuffer |
                                              vk::BufferUsageFlagBits::eTransferDst,
                                              vk::MemoryPropertyFlagBits::eDeviceLocal));

    }

    void Renderer::copyBuffer(vk::Buffer &src, vk::Buffer &dst, size_t size, size_t srcOffset, size_t dstOffset) {
        auto cmdBuffer = Context::GetInstance().commandManager->AllocateCmdBuffer();

        vk::CommandBufferBeginInfo begin;
        begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdBuffer.begin(begin);
        vk::BufferCopy region;
        region.setSize(size)
                .setSrcOffset(srcOffset)
                .setDstOffset(dstOffset);
        cmdBuffer.copyBuffer(src, dst, region);
        cmdBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(cmdBuffer);
        Context::GetInstance().graphicsQueue.submit(submitInfo);
        Context::GetInstance().device.waitIdle();
        Context::GetInstance().commandManager->FreeCommandbuffers(cmdBuffer);
    }

    void Renderer::loadVertexData() {
        void *data = Context::GetInstance().device.mapMemory(hostVertexBuffer_->memory, 0, hostVertexBuffer_->size);
        memcpy(data, vertices.data(), hostVertexBuffer_->size);
        Context::GetInstance().device.unmapMemory(hostVertexBuffer_->memory);

        copyBuffer(hostVertexBuffer_->buffer, deviceVertexBuffer_->buffer, hostVertexBuffer_->size, 0, 0);
    }

    void Renderer::loadIndexData() {
        void *data = Context::GetInstance().device.mapMemory(hostIndexBuffer_->memory, 0, hostIndexBuffer_->size);
        memcpy(data, indices.data(), hostIndexBuffer_->size);
        Context::GetInstance().device.unmapMemory(hostIndexBuffer_->memory);

        copyBuffer(hostIndexBuffer_->buffer, deviceIndexBuffer_->buffer, hostIndexBuffer_->size, 0, 0);
    }

    void Renderer::loadUniformData() {
        void *data = Context::GetInstance().device.mapMemory(hostUniformBuffer_->memory, 0, hostUniformBuffer_->size);
        memcpy(data, &vp, hostUniformBuffer_->size);
        Context::GetInstance().device.unmapMemory(hostUniformBuffer_->memory);

        copyBuffer(hostUniformBuffer_->buffer, deviceUniformBuffer_->buffer, hostUniformBuffer_->size, 0, 0);
    }

    void Renderer::createDescriptorPool() {
        vk::DescriptorPoolCreateInfo createInfo;
        std::array<vk::DescriptorPoolSize,2> poolSizes;
        poolSizes[0].setType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(frameNums);
        poolSizes[1].setType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(frameNums);
        createInfo.setMaxSets(frameNums)
                .setPoolSizes(poolSizes);
        descriptorPool_ = Context::GetInstance().device.createDescriptorPool(createInfo);
    }

    void Renderer::allocateSets() {
        std::vector<vk::DescriptorSetLayout> layouts(frameNums, Context::GetInstance().renderProcess->setLayout);
        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.setSetLayouts(layouts)
                .setDescriptorPool(descriptorPool_)
                .setDescriptorSetCount(frameNums);

        sets_ = Context::GetInstance().device.allocateDescriptorSets(allocateInfo);
    }

    void Renderer::writeSets() {
        for (int i = 0; i < frameNums; i++) {
            auto &set = sets_[i];
            vk::DescriptorBufferInfo bufferInfo;
            vk::DescriptorImageInfo imageInfo;
            std::array<vk::WriteDescriptorSet,2> writer;

            //mvp
            bufferInfo.setOffset(0)
                    .setBuffer(deviceUniformBuffer_->buffer)
                    .setRange(deviceUniformBuffer_->size);

            writer[0].setDescriptorCount(1)
                    .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                    .setBufferInfo(bufferInfo)
                    .setDstBinding(0)
                    .setDstSet(set)
                    .setDstArrayElement(0);

            //texcoord
            imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImageView(texture_->view)
            .setSampler(sampler_);

            writer[1].setDescriptorCount(1)
                    .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                    .setImageInfo(imageInfo)
                    .setDstBinding(1)
                    .setDstSet(set)
                    .setDstArrayElement(0);


            Context::GetInstance().device.updateDescriptorSets(writer, {});
        }
    }

    void Renderer::initVP() {
        vp.view = Schoo::GetInstance().camera->GetViewMatrix();

        uint32_t width = Context::GetInstance().swapchain->width;
        uint32_t height = Context::GetInstance().swapchain->height;
        vp.project = glm::perspective(glm::radians(fov_), width / (height * 1.0f), 0.1f, 100.0f);
    }

    void Renderer::createTextures() {
        const std::string path = "../../textures/img.png";
        if (CheckPath(path)) {
            texture_.reset(new Texture(path));
        }
    }

    void Renderer::createSampler() {
        auto &device = Context::GetInstance().device;

        vk::SamplerCreateInfo createInfo;
        createInfo.setMagFilter(vk::Filter::eLinear)
                .setMinFilter(vk::Filter::eLinear)
                .setAddressModeU(vk::SamplerAddressMode::eRepeat)
                .setAddressModeV(vk::SamplerAddressMode::eRepeat)
                .setAddressModeW(vk::SamplerAddressMode::eRepeat)
                .setAnisotropyEnable(vk::False)
                .setMaxAnisotropy(1)
                .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
                .setUnnormalizedCoordinates(vk::False)
                .setCompareEnable(vk::False)
                .setCompareOp(vk::CompareOp::eAlways)
                .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                .setMinLod(0.0f)
                .setMaxLod(0.0f)
                .setMipLodBias(0.0f);

        sampler_ = device.createSampler(createInfo);
    }

    void Renderer::UpdateViewMatrix() {
        vp.view=Schoo::GetInstance().camera->GetViewMatrix();
        loadUniformData();
    }


}
