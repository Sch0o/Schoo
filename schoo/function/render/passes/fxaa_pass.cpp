#include"fxaa_pass.hpp"
#include"function/render/vulkan/context.hpp"
#include"function/render/vulkan/utils.hpp"
#include"function/render/vulkan/buffer.hpp"
#include"function/render/vertexData.hpp"

namespace schoo {

    void FxaaPass::init() {
        cmdBuffers = Context::GetInstance().commandManager->AllocateCmdBuffers(2);
        setupRenderPass();
        setupFrameBuffers();
        setupBuffers();
        sampler = createSampler();
        setupDescriptors();
        setupPipeline();

    }

    FxaaPass::~FxaaPass(){
        auto &commandManager = Context::GetInstance().commandManager;
        auto &device = Context::GetInstance().device;

        device.destroySampler(sampler);
        device.destroyDescriptorPool(descriptorPool);
        indexBuffer.reset();
        vertexBuffer.reset();

        for(int i=0;i<frameBuffers.size();i++){
            commandManager->FreeCommandbuffers(cmdBuffers[i]);
            device.destroyFramebuffer(frameBuffers[i]);
        }
        device.destroyPipeline(renderPipeline.pipeline);
        device.destroyPipelineLayout(renderPipeline.layout);
        device.destroyRenderPass(renderPass);
        device.destroyDescriptorSetLayout(textureSetLayout);
    }
    void FxaaPass::draw() {
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
            std::array<vk::ClearValue, 1> clearValues;
            clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
            renderPassBeginInfo.setRenderPass(renderPass)
                    .setRenderArea(area)
                    .setFramebuffer(frameBuffers[imageIndex])
                    .setClearValues(clearValues);

            cmdBuffers[currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                        renderPipeline.layout, 0,
                                                        sets[imageIndex], {});

            cmdBuffers[currentFrame].beginRenderPass(renderPassBeginInfo, {});
            vk::DeviceSize offsets[1] = {0};
            cmdBuffers[currentFrame].bindVertexBuffers(0, 1, &vertexBuffer->buffer, offsets);
            cmdBuffers[currentFrame].bindIndexBuffer(indexBuffer->buffer, 0, vk::IndexType::eUint32);
            cmdBuffers[currentFrame].drawIndexed(quadIndices.size(),1,0,0,0);
            cmdBuffers[currentFrame].endRenderPass();

        }
        cmdBuffers[currentFrame].end();
    }

    void FxaaPass::setupBuffers() {


        vk::DeviceSize bufferSize = sizeof(quadVertices[0]) * quadVertices.size();
        std::shared_ptr<Buffer> stagingBuffer;
        stagingBuffer.reset(new Buffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                                       vk::MemoryPropertyFlagBits::eHostVisible |
                                       vk::MemoryPropertyFlagBits::eHostCoherent));
        vertexBuffer.reset(new Buffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst
                                                  | vk::BufferUsageFlagBits::eVertexBuffer,
                                      vk::MemoryPropertyFlagBits::eDeviceLocal));
        loadDataHostToDevice(stagingBuffer, vertexBuffer, quadVertices.data());

        bufferSize = sizeof(quadIndices[0]) * quadIndices.size();
        stagingBuffer.reset(new Buffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                                       vk::MemoryPropertyFlagBits::eHostVisible |
                                       vk::MemoryPropertyFlagBits::eHostCoherent));
        indexBuffer.reset(new Buffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst
                                                 | vk::BufferUsageFlagBits::eIndexBuffer,
                                     vk::MemoryPropertyFlagBits::eDeviceLocal));
        loadDataHostToDevice(stagingBuffer, indexBuffer, quadIndices.data());

        stagingBuffer.reset();
    }

    void FxaaPass::setupFrameBuffers() {
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

    void FxaaPass::setupPipeline() {
        //layout
        //pushConstant
        vk::PushConstantRange pushConstantRange;
        pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex)
                .setOffset(0).setSize(sizeof(glm::mat4));

        std::array<vk::DescriptorSetLayout, 1> setLayouts = {textureSetLayout};
        vk::PipelineLayoutCreateInfo layoutCreateInfo;
        layoutCreateInfo.setSetLayouts(setLayouts)
                .setPushConstantRanges(pushConstantRange);
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
        auto stages = Context::GetInstance().renderer->shaders.fxaa_shader->getStages();
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
                .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                .setColorBlendOp(vk::BlendOp::eAdd)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
                .setAlphaBlendOp(vk::BlendOp::eAdd)
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

    void FxaaPass::setupRenderPass() {
        vk::RenderPassCreateInfo createInfo;

        std::array<vk::AttachmentDescription, 1> descriptions;
        descriptions[0].setFormat(Context::GetInstance().swapchain->swapchainInfo.surfaceFormat.format)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
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
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        createInfo.setDependencies(dependency);

        renderPass = Context::GetInstance().device.createRenderPass(createInfo);
    }

    void FxaaPass::setupDescriptors() {

        vk::DescriptorPoolCreateInfo createInfo;
        std::array<vk::DescriptorPoolSize, 1> poolSizes;
        poolSizes[0].setType(vk::DescriptorType::eCombinedImageSampler)
                .setDescriptorCount(2);
        createInfo.setMaxSets(2)
                .setPoolSizes(poolSizes);
        descriptorPool = Context::GetInstance().device.createDescriptorPool(createInfo);

        //setLayout
        //texture for draw call
        vk::DescriptorSetLayoutCreateInfo createInfo_preDrawCall;
        std::array<vk::DescriptorSetLayoutBinding, 1> bindings_preDrawCall;
        bindings_preDrawCall[0].setBinding(0)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setStageFlags(vk::ShaderStageFlagBits::eFragment)
                .setDescriptorCount(1);
        createInfo_preDrawCall.setBindings(bindings_preDrawCall);
        textureSetLayout = Context::GetInstance().device.createDescriptorSetLayout(createInfo_preDrawCall);

        //allocate Sets
        sets.resize(frameBuffers.size());
        for (int i = 0; i < frameBuffers.size(); i++) {
            sets[i] = allocateDescriptor(textureSetLayout, descriptorPool, 1)[0];
        }

        //update sets
        std::array<vk::WriteDescriptorSet, 1> Writers1;
        for (int i = 0; i < sets.size(); i++) {
            vk::DescriptorImageInfo imageInfo;
            imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                    .setImageView(Context::GetInstance().renderer->passes.mesh_pass->frameBuffers[i].getImageView(0))
                    .setSampler(sampler);
            Writers1[0].setDescriptorCount(1)
                    .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                    .setImageInfo(imageInfo)
                    .setDstBinding(0)
                    .setDstSet(sets[i])
                    .setDstArrayElement(0);
            Context::GetInstance().device.updateDescriptorSets(Writers1, {});
        }

    }


}

