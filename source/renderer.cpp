#include"schoo/renderer.hpp"
#include"schoo/context.hpp"
#include"schoo/utils.hpp"
#include"schoo/schoo.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include"external/glm/ext/matrix_clip_space.hpp"

namespace schoo {

    Renderer::Renderer(uint32_t frameNums) {

        this->frameNums = frameNums;
        createSemaphores();
        createFences();
        createCmdBuffers();

        initVP();
        loadModels();

        createUniformBuffer();
        loadUniformData();
        createSampler();

        createDescriptorPool();
        allocateSets();
        updateSets();

    }

    Renderer::~Renderer() {
        auto &commandManager = Context::GetInstance().commandManager;
        auto &device = Context::GetInstance().device;

        device.destroySampler(sampler_);


        device.destroyDescriptorPool(descriptorPool_);


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
        //render
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
            vk::RenderPassBeginInfo renderPassBeginInfo;
            vk::Rect2D area({0, 0}, {swapchain->width, swapchain->height});
            std::array<vk::ClearValue, 2> clearValues;
            clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
            clearValues[1].depthStencil = vk::ClearDepthStencilValue(0.99f, 0);
            renderPassBeginInfo.setRenderPass(renderProcess->renderPass)
                    .setRenderArea(area)
                    .setFramebuffer(swapchain->frameBuffers[imageIndex])
                    .setClearValues(clearValues);

            cmdBuffers_[currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                         Context::GetInstance().renderProcess->layout, 0,
                                                         vpSets_[0], {});

            cmdBuffers_[currentFrame].beginRenderPass(renderPassBeginInfo, {});
            for (const auto &model: models) {
                vk::DeviceSize offset = 0;
                cmdBuffers_[currentFrame].bindVertexBuffers(0, model->vertexBuffer->buffer, offset);
                cmdBuffers_[currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                             Context::GetInstance().renderProcess->layout, 1,
                                                             model->sets[0], {});
                cmdBuffers_[currentFrame].bindIndexBuffer(model->indexBuffer->buffer, 0, vk::IndexType::eUint32);
                cmdBuffers_[currentFrame].drawIndexed(model->indices.size(), 1, 0, 0, 0);
            }
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


    void Renderer::loadUniformData() {
        void *data = Context::GetInstance().device.mapMemory(hostUniformBuffer_->memory, 0, hostUniformBuffer_->size);
        memcpy(data, &vp, hostUniformBuffer_->size);
        Context::GetInstance().device.unmapMemory(hostUniformBuffer_->memory);

        copyBuffer(hostUniformBuffer_->buffer, deviceUniformBuffer_->buffer, hostUniformBuffer_->size, 0, 0);
    }

    void Renderer::createDescriptorPool() {
        vk::DescriptorPoolCreateInfo createInfo;
        std::array<vk::DescriptorPoolSize, 2> poolSizes;
        poolSizes[0].setType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(models.size() + 1);
        poolSizes[1].setType(vk::DescriptorType::eCombinedImageSampler)
                .setDescriptorCount(models.size());
        createInfo.setMaxSets(models.size() + 1)
                .setPoolSizes(poolSizes);
        descriptorPool_ = Context::GetInstance().device.createDescriptorPool(createInfo);
    }

    void Renderer::allocateSets() {
        std::vector<vk::DescriptorSetLayout> vpSetLayouts(1, Context::GetInstance().renderProcess->vpSetLayout);
        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.setSetLayouts(vpSetLayouts)
                .setDescriptorPool(descriptorPool_)
                .setDescriptorSetCount(1);
        vpSets_ = Context::GetInstance().device.allocateDescriptorSets(allocateInfo);

        for (auto &model: models) {
            std::vector<vk::DescriptorSetLayout> modelSetLayouts(1,
                                                                 Context::GetInstance().renderProcess->modelSetLayout);
            vk::DescriptorSetAllocateInfo allocateInfoModel;
            allocateInfoModel.setSetLayouts(modelSetLayouts)
                    .setDescriptorPool(descriptorPool_)
                    .setDescriptorSetCount(1);
            model->sets = Context::GetInstance().device.allocateDescriptorSets(allocateInfoModel);
        }
    }

    void Renderer::updateSets() {
        //vp
        vk::DescriptorBufferInfo vpBufferInfo;
        vpBufferInfo.setOffset(0)
                .setBuffer(deviceUniformBuffer_->buffer)
                .setRange(deviceUniformBuffer_->size);
        std::array<vk::WriteDescriptorSet, 1> vpWriter;
        vpWriter[0].setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setBufferInfo(vpBufferInfo)
                .setDstBinding(0)
                .setDstSet(vpSets_[0])
                .setDstArrayElement(0);
        Context::GetInstance().device.updateDescriptorSets(vpWriter, {});
        //model and sampler
        std::array<vk::WriteDescriptorSet, 2> modelWriters;
        for (auto &model: models) {
            auto &set = model->sets[0];
            //model matrix
            vk::DescriptorBufferInfo modelBufferInfo;
            modelBufferInfo.setOffset(0)
                    .setBuffer(model->modelMatBuffer->buffer)
                    .setRange(model->modelMatBuffer->size);
            modelWriters[0].setDescriptorCount(1)
                    .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                    .setBufferInfo(modelBufferInfo)
                    .setDstBinding(0)
                    .setDstSet(set)
                    .setDstArrayElement(0);
            //texcoord
            vk::DescriptorImageInfo imageInfo;
            imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                    .setImageView(model->texture->view)
                    .setSampler(sampler_);

            modelWriters[1].setDescriptorCount(1)
                    .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                    .setImageInfo(imageInfo)
                    .setDstBinding(1)
                    .setDstSet(set)
                    .setDstArrayElement(0);
            Context::GetInstance().device.updateDescriptorSets(modelWriters, {});
        }
    }

    void Renderer::initVP() {
        vp.view = Schoo::GetInstance().camera->GetViewMatrix();

        uint32_t width = Context::GetInstance().swapchain->width;
        uint32_t height = Context::GetInstance().swapchain->height;
        vp.project = glm::perspective(glm::radians(fov_), width / (height * 1.0f), 0.1f, 100.0f);
        vp.project[1][1] *= -1;
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
        vp.view = Schoo::GetInstance().camera->GetViewMatrix();
        loadUniformData();
    }

    void Renderer::loadModels() {
        models.resize(2);
        models[0].reset(new Model(R"(..\..\assets\models\marry\Marry.obj)",
                                  R"(..\..\assets\models\marry\MC003_Kozakura_Mari.png)",
                                  glm::vec3(0, 0.5f, 0)));
        models[1].reset(new Model(R"(..\..\assets\models\floor\floor.obj)",
                                  R"(..\..\assets\textures\2x2white.png)",
                                  glm::vec3(0, 0, 0)));
    }

}
