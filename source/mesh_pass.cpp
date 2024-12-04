#include"render/passes/mesh_pass.hpp"
#include "render/shader.hpp"
#include "render/utils.hpp"
#include "engine.hpp"

namespace schoo {

    MeshPass::MeshPass(std::vector<std::shared_ptr<Model>> &models) :
            renderResource(models) {
        createDescriptorPool();
        createRenderPass();
        createFrameBuffers();
        createSetLayout();
        createUniformBuffer();
        createRenderPipeline();
        createSampler();
        allocateSets();
        updateSets();
        createCmdBuffer();
        initVP();
    }

    MeshPass::~MeshPass() {
        auto &commandManager = Context::GetInstance().commandManager;
        auto &device = Context::GetInstance().device;

        device.destroySampler(sampler);
        device.destroyDescriptorPool(descriptorPool);
        hostUniformBuffer.reset();
        deviceUniformBuffer.reset();
        for (int i=0;i<2;i++) {

            commandManager->FreeCommandbuffers(cmdBuffers[i]);
            device.destroyFramebuffer(frameBuffers[i]);
        }
        device.destroyPipeline(renderPipeline.pipeline);
        device.destroyPipelineLayout(renderPipeline.layout);
        device.destroyRenderPass(renderPass);
        device.destroyDescriptorSetLayout(vpSetLayout);
        device.destroyDescriptorSetLayout(modelSetLayout);
    }

    void MeshPass::createRenderPass() {
        vk::RenderPassCreateInfo createInfo;

        std::array<vk::AttachmentDescription, 2> descriptions;
        descriptions[0].setFormat(Context::GetInstance().swapchain->swapchainInfo.surfaceFormat.format)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setSamples(vk::SampleCountFlagBits::e1);
        descriptions[1].setFormat(vk::Format::eD24UnormS8Uint)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                .setStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
        createInfo.setAttachments(descriptions);

        vk::AttachmentReference colorReference;
        colorReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setAttachment(0);
        vk::AttachmentReference depthReference;
        depthReference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setAttachment(1);
        vk::SubpassDescription subpassDescription;
        subpassDescription.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setColorAttachments(colorReference)
                .setPDepthStencilAttachment(&depthReference);
        createInfo.setSubpasses(subpassDescription);


        vk::SubpassDependency dependency;
        dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
                .setDstSubpass(0)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        createInfo.setDependencies(dependency);

        renderPass = Context::GetInstance().device.createRenderPass(createInfo);
    }

    void MeshPass::draw() {
        UpdateViewMatrix();

        auto &swapchain = Context::GetInstance().swapchain;
        uint32_t currentFrame = Context::GetInstance().renderer->currentFrame;
        uint32_t imageIndex = Context::GetInstance().renderer->imageIndex;
        cmdBuffers[currentFrame].reset();
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdBuffers[currentFrame].begin(beginInfo);
        {
            cmdBuffers[currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, renderPipeline.pipeline);
            vk::RenderPassBeginInfo renderPassBeginInfo;
            vk::Rect2D area({0, 0}, {swapchain->width, swapchain->height});
            std::array<vk::ClearValue, 2> clearValues;
            clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
            clearValues[1].depthStencil = vk::ClearDepthStencilValue(0.99f, 0);
            renderPassBeginInfo.setRenderPass(renderPass)
                    .setRenderArea(area)
                    .setFramebuffer(frameBuffers[imageIndex])
                    .setClearValues(clearValues);

            cmdBuffers[currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                        renderPipeline.layout, 0,
                                                        vpSets[0], {});

            cmdBuffers[currentFrame].beginRenderPass(renderPassBeginInfo, {});
            for (const auto &model: renderResource) {
                vk::DeviceSize offset = 0;
                cmdBuffers[currentFrame].bindVertexBuffers(0, model->vertexBuffer->buffer, offset);
                cmdBuffers[currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                            renderPipeline.layout, 1,
                                                            model->sets[0], {});
                cmdBuffers[currentFrame].bindIndexBuffer(model->indexBuffer->buffer, 0, vk::IndexType::eUint32);
                cmdBuffers[currentFrame].drawIndexed(model->indices.size(), 1, 0, 0, 0);
            }
            cmdBuffers[currentFrame].endRenderPass();

        }
        cmdBuffers[currentFrame].end();

    }

    void MeshPass::UpdateViewMatrix() {
        vp.view = SchooEngine::GetInstance().camera->GetViewMatrix();
        loadUniformData();
    }

    void MeshPass::createRenderPipeline() {

        //layout
        std::array<vk::DescriptorSetLayout, 2> setLayouts = {vpSetLayout, modelSetLayout};

        vk::PipelineLayoutCreateInfo layoutCreateInfo;
        layoutCreateInfo.setSetLayouts(setLayouts);
        renderPipeline.layout = Context::GetInstance().device.createPipelineLayout(layoutCreateInfo);

        //pipeline
        vk::GraphicsPipelineCreateInfo createInfo;

        //vertex input
        vk::PipelineVertexInputStateCreateInfo inputState;
        auto attribute = Vertex::getAttributeDescriptions();
        auto binding = Vertex::getBindingDescription();
        inputState.setVertexAttributeDescriptions(attribute)
                .setVertexBindingDescriptions(binding);

        createInfo.setPVertexInputState(&inputState);

        //Vertex Assembly
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
        inputAssemblyState.setPrimitiveRestartEnable(false)
                .setTopology(vk::PrimitiveTopology::eTriangleList);
        createInfo.setPInputAssemblyState(&inputAssemblyState);

        //Shader
        auto stages = Shader::GetInstance().getStages();
        createInfo.setStages(stages);

        //viewport
        vk::PipelineViewportStateCreateInfo viewportCreateInfo;
        vk::Viewport viewports(0, 0, Window::GetInstance().width,
                               Window::GetInstance().height, 0, 1);
        viewportCreateInfo.setViewports(viewports);
        vk::Rect2D rect({0, 0},
                        {Window::GetInstance().width, Window::GetInstance().height});
        viewportCreateInfo.setScissors(rect);
        createInfo.setPViewportState(&viewportCreateInfo);

        //Rasterization
        vk::PipelineRasterizationStateCreateInfo rasterization;
        rasterization.setRasterizerDiscardEnable(false)
                .setCullMode(vk::CullModeFlagBits::eBack)
                .setFrontFace(vk::FrontFace::eCounterClockwise)
                .setPolygonMode(vk::PolygonMode::eFill)
                .setLineWidth(1);
        createInfo.setPRasterizationState(&rasterization);

        //multiSample
        vk::PipelineMultisampleStateCreateInfo multisample;
        multisample.setSampleShadingEnable(false)
                .setRasterizationSamples(vk::SampleCountFlagBits::e1);
        createInfo.setPMultisampleState(&multisample);
        //test
        vk::PipelineDepthStencilStateCreateInfo depthStencilState;
        depthStencilState.setDepthTestEnable(vk::True)
                .setDepthWriteEnable(vk::True)
                .setDepthCompareOp(vk::CompareOp::eLess);
        createInfo.setPDepthStencilState(&depthStencilState);


        //color blending
        vk::PipelineColorBlendStateCreateInfo colorBlend;
        vk::PipelineColorBlendAttachmentState attachmentState;
        attachmentState.setBlendEnable(false)
                .setColorWriteMask(vk::ColorComponentFlagBits::eA |
                                   vk::ColorComponentFlagBits::eB |
                                   vk::ColorComponentFlagBits::eR |
                                   vk::ColorComponentFlagBits::eG);

        colorBlend.setLogicOpEnable(false)
                .setAttachments(attachmentState);
        createInfo.setPColorBlendState(&colorBlend);

        //renderPass and layout
        createInfo.setLayout(renderPipeline.layout).setRenderPass(renderPass);

        //create pipeline
        auto resultValue = Context::GetInstance().device.createGraphicsPipeline(nullptr, createInfo);
        if (resultValue.result != vk::Result::eSuccess) {
            throw std::runtime_error("create pipeline failed");
        }
        renderPipeline.pipeline = resultValue.value;
    }

    void MeshPass::createSetLayout() {
        //model matrix and sampler
        vk::DescriptorSetLayoutCreateInfo createInfoModel;
        std::array<vk::DescriptorSetLayoutBinding, 2> bindingsModel;
        bindingsModel[0].setBinding(0)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setStageFlags(vk::ShaderStageFlagBits::eVertex)
                .setDescriptorCount(1);
        bindingsModel[1].setBinding(1)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setStageFlags(vk::ShaderStageFlagBits::eFragment)
                .setDescriptorCount(1);
        createInfoModel.setBindings(bindingsModel);
        modelSetLayout = Context::GetInstance().device.createDescriptorSetLayout(createInfoModel);

        //vp matrix
        vk::DescriptorSetLayoutCreateInfo createInfoVP;
        std::array<vk::DescriptorSetLayoutBinding, 1> bindingsVP;
        bindingsVP[0].setBinding(0)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setStageFlags(vk::ShaderStageFlagBits::eVertex)
                .setDescriptorCount(1);
        createInfoVP.setBindings(bindingsVP);
        vpSetLayout = Context::GetInstance().device.createDescriptorSetLayout(createInfoVP);
    }

    void MeshPass::createDescriptorPool() {
        vk::DescriptorPoolCreateInfo createInfo;
        std::array<vk::DescriptorPoolSize, 2> poolSizes;
        poolSizes[0].setType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(3);
        poolSizes[1].setType(vk::DescriptorType::eCombinedImageSampler)
                .setDescriptorCount(3);
        createInfo.setMaxSets(3)
                .setPoolSizes(poolSizes);
        descriptorPool = Context::GetInstance().device.createDescriptorPool(createInfo);
    }

    void MeshPass::allocateSets() {
        std::vector<vk::DescriptorSetLayout> vpSetLayouts(1, vpSetLayout);
        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.setSetLayouts(vpSetLayouts)
                .setDescriptorPool(descriptorPool)
                .setDescriptorSetCount(1);
        vpSets = Context::GetInstance().device.allocateDescriptorSets(allocateInfo);

        for (auto &model: renderResource) {
            std::vector<vk::DescriptorSetLayout> modelSetLayouts(1,
                                                                 modelSetLayout);
            vk::DescriptorSetAllocateInfo allocateInfoModel;
            allocateInfoModel.setSetLayouts(modelSetLayouts)
                    .setDescriptorPool(descriptorPool)
                    .setDescriptorSetCount(1);
            model->sets = Context::GetInstance().device.allocateDescriptorSets(allocateInfoModel);
        }

    }

    void MeshPass::updateSets() {
        //vp
        vk::DescriptorBufferInfo vpBufferInfo;
        vpBufferInfo.setOffset(0)
                .setBuffer(deviceUniformBuffer->buffer)
                .setRange(deviceUniformBuffer->size);
        std::array<vk::WriteDescriptorSet, 1> vpWriter;
        vpWriter[0].setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setBufferInfo(vpBufferInfo)
                .setDstBinding(0)
                .setDstSet(vpSets[0])
                .setDstArrayElement(0);
        Context::GetInstance().device.updateDescriptorSets(vpWriter, {});
        //model and sampler
        std::array<vk::WriteDescriptorSet, 2> modelWriters;
        for (auto &model: renderResource) {
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
                    .setSampler(sampler);

            modelWriters[1].setDescriptorCount(1)
                    .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                    .setImageInfo(imageInfo)
                    .setDstBinding(1)
                    .setDstSet(set)
                    .setDstArrayElement(0);
            Context::GetInstance().device.updateDescriptorSets(modelWriters, {});
        }
    }

    void MeshPass::createSampler() {
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

        sampler = device.createSampler(createInfo);
    }

    void MeshPass::createCmdBuffer() {
        cmdBuffers = Context::GetInstance().commandManager->AllocateCmdBuffers(2);
    }

    void MeshPass::createUniformBuffer() {
        size_t bufferSize = sizeof(vp);
        //std::cout << "UniformSize:"<<bufferSize<<std::endl;
        hostUniformBuffer.reset(new Buffer(bufferSize,
                                           vk::BufferUsageFlagBits::eTransferSrc,
                                           vk::MemoryPropertyFlagBits::eHostCoherent |
                                           vk::MemoryPropertyFlagBits::eHostVisible));

        deviceUniformBuffer.reset(new Buffer(bufferSize,
                                             vk::BufferUsageFlagBits::eUniformBuffer |
                                             vk::BufferUsageFlagBits::eTransferDst,
                                             vk::MemoryPropertyFlagBits::eDeviceLocal));
    }

    void MeshPass::loadUniformData() {
        void *data = Context::GetInstance().device.mapMemory(hostUniformBuffer->memory, 0, hostUniformBuffer->size);
        memcpy(data, &vp, hostUniformBuffer->size);
        Context::GetInstance().device.unmapMemory(hostUniformBuffer->memory);

        copyBuffer(hostUniformBuffer->buffer, deviceUniformBuffer->buffer, hostUniformBuffer->size, 0, 0);
    }

    void MeshPass::initVP() {
        vp.view = SchooEngine::GetInstance().camera->GetViewMatrix();

        uint32_t width = Context::GetInstance().swapchain->width;
        uint32_t height = Context::GetInstance().swapchain->height;
        vp.project = glm::perspective(glm::radians(fov), width / (height * 1.0f), 0.1f, 100.0f);
        vp.project[1][1] *= -1;
    }

    void MeshPass::createFrameBuffers() {
        Context &context = Context::GetInstance();
        frameBuffers.resize(context.swapchain->imageViews.size());
        for (int i = 0; i < frameBuffers.size(); i++) {
            vk::FramebufferCreateInfo createInfo;
            std::array<vk::ImageView, 2> attachments = {context.swapchain->imageViews[i],
                                                        context.swapchain->depthImageView};
            createInfo.setAttachments(attachments)
                    .setWidth(context.swapchain->width)
                    .setHeight(context.swapchain->height)
                    .setRenderPass(renderPass)
                    .setLayers(1);
            frameBuffers[i] = Context::GetInstance().device.createFramebuffer(createInfo);
        }
    }

}