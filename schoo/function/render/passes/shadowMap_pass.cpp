#include"schoo/function/render/passes/shadowMap_pass.hpp"
#include "schoo/function/render/vulkan/context.hpp"
#include "schoo/function/render/vulkan/utils.hpp"
#include"resource/asset_manager.hpp"

namespace schoo {
    ShadowMapPass::~ShadowMapPass(){
        auto &commandManager = Context::GetInstance().commandManager;
        auto &device = Context::GetInstance().device;

        device.destroyDescriptorPool(descriptorPool);
        mvp.destoryUniformBuffer();
        for (int i = 0; i < 2; i++) {
            commandManager->FreeCommandbuffers(cmdBuffers[i]);
            device.destroyFramebuffer(framebuffers[i].framebuffer);
            for(auto&attachment:framebuffers[i].attachments){
                device.destroyImageView(attachment.imageView);
                device.destroyImage(attachment.image);
                device.freeMemory(attachment.memory);
            }
        }
        device.destroyPipeline(renderPipeline.pipeline);
        device.destroyPipelineLayout(renderPipeline.layout);
        device.destroyRenderPass(renderPass);
        device.destroyDescriptorSetLayout(setLayout);
    }

    void ShadowMapPass::draw() {
        updateUniform();

        uint32_t currentFrame = Context::GetInstance().renderer->currentFrame;
        uint32_t imageIndex = Context::GetInstance().renderer->imageIndex;
        cmdBuffers[currentFrame].reset();
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdBuffers[currentFrame].begin(beginInfo);
        {
            cmdBuffers[currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, renderPipeline.pipeline);
            vk::RenderPassBeginInfo renderPassBeginInfo;
            vk::Rect2D area({0, 0}, {shadowMapSize, shadowMapSize});
            std::array<vk::ClearValue, 1> clearValues;
            clearValues[0].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
            renderPassBeginInfo.setRenderPass(renderPass)
                    .setRenderArea(area)
                    .setFramebuffer(framebuffers[imageIndex].framebuffer)
                    .setClearValues(clearValues);

            cmdBuffers[currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                        renderPipeline.layout, 0,
                                                        writerDescriptorSet, {});

            cmdBuffers[currentFrame].beginRenderPass(renderPassBeginInfo, {});
            AssetManager::GetInstance().glTFModel.draw(cmdBuffers[currentFrame],renderPipeline.layout,0);
            cmdBuffers[currentFrame].endRenderPass();

        }
        cmdBuffers[currentFrame].end();
    }

    void ShadowMapPass::init() {
        //command buffer
        cmdBuffers = Context::GetInstance().commandManager->AllocateCmdBuffers(2);
        //renderPass
        createRenderPass();
        //framebuffer
        createFrameBuffer();
        //uniform buffer
        size_t bufferSize = sizeof(uboData);
        mvp.stagingBuffer.reset(new Buffer(bufferSize,
                                           vk::BufferUsageFlagBits::eTransferSrc,
                                           vk::MemoryPropertyFlagBits::eHostCoherent |
                                           vk::MemoryPropertyFlagBits::eHostVisible));

        mvp.deviceBuffer.reset(new Buffer(bufferSize,
                                          vk::BufferUsageFlagBits::eUniformBuffer |
                                          vk::BufferUsageFlagBits::eTransferDst,
                                          vk::MemoryPropertyFlagBits::eDeviceLocal));
        updateUniform();
        //descriptor
        setupDescriptors();
        //pipeline
        setupPipeline();

    }

    void ShadowMapPass::createFrameBuffer() {
        framebuffers.resize(2);
        for (auto &framebuffer: framebuffers) {
            framebuffer.attachments.resize(1);
            framebuffer.width = shadowMapSize;
            framebuffer.height = shadowMapSize;
        }

        //createImage
        vk::ImageCreateInfo imageCreateInfo;
        imageCreateInfo.setImageType(vk::ImageType::e2D)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFormat(depthFormat)
                .setExtent({shadowMapSize, shadowMapSize, 1})
                .setMipLevels(1)
                .setArrayLayers(1)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setTiling(vk::ImageTiling::eOptimal)
                .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);

        //create frame attachment
        auto &device = Context::GetInstance().device;
        for (auto &framebuffer: framebuffers) {
            FrameBufferAttachment &attachment = framebuffer.attachments[0];
            attachment.image = Context::GetInstance().device.createImage(imageCreateInfo);
            vk::MemoryAllocateInfo allocateInfo;
            auto requirements = device.getImageMemoryRequirements(attachment.image);
            allocateInfo.setAllocationSize(requirements.size);
            auto index = Buffer::QueryBufferMemTypeIndex(requirements.memoryTypeBits,
                                                         vk::MemoryPropertyFlagBits::eDeviceLocal);

            allocateInfo.setMemoryTypeIndex(index);
            attachment.memory = device.allocateMemory(allocateInfo);
            //bind
            device.bindImageMemory(attachment.image, attachment.memory, 0);

            attachment.format = depthFormat;
            attachment.imageView = CreateImageView(attachment.image, attachment.format,
                                                   vk::ImageAspectFlagBits::eDepth);


        }
        // Create frame buffer
        vk::FramebufferCreateInfo createInfo;
        for (auto &framebuffer: framebuffers) {
            std::array<vk::ImageView, 1> attachments = {framebuffer.attachments[0].imageView};
            createInfo.setAttachments(attachments)
                    .setWidth(framebuffer.width)
                    .setHeight(framebuffer.height)
                    .setRenderPass(renderPass)
                    .setLayers(1);
            framebuffer.framebuffer = Context::GetInstance().device.createFramebuffer(createInfo);
        }
    }

    void ShadowMapPass::createRenderPass() {
        vk::RenderPassCreateInfo createInfo;

        std::array<vk::AttachmentDescription, 1> des;
        des[0].setFormat(depthFormat)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal);
        createInfo.setAttachments(des);
        vk::AttachmentReference depthReference;
        depthReference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setAttachment(0);

        vk::SubpassDescription subpassDescription;
        subpassDescription.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setPDepthStencilAttachment(&depthReference);
        createInfo.setSubpasses(subpassDescription);

        std::array<vk::SubpassDependency, 2> dependencies;
        dependencies[0].setSrcSubpass(VK_SUBPASS_EXTERNAL)
                .setDstSubpass(0)
                .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                .setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
                .setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader)
                .setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests);
        dependencies[1].setSrcSubpass(0)
                .setDstSubpass(VK_SUBPASS_EXTERNAL)
                .setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                .setSrcStageMask(vk::PipelineStageFlagBits::eLateFragmentTests)
                .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader);
        createInfo.setDependencies(dependencies);

        renderPass = Context::GetInstance().device.createRenderPass(createInfo);

    }

    void ShadowMapPass::setupDescriptors() {
        //pool
        vk::DescriptorPoolCreateInfo createInfo;
        std::array<vk::DescriptorPoolSize, 1> poolSizes;
        poolSizes[0].setType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(3);
        createInfo.setMaxSets(3)
                .setPoolSizes(poolSizes);
        descriptorPool = Context::GetInstance().device.createDescriptorPool(createInfo);

        //setLayout
        vk::DescriptorSetLayoutCreateInfo createInfoModel;
        std::array<vk::DescriptorSetLayoutBinding, 1> bindings;
        bindings[0].setBinding(0)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setStageFlags(vk::ShaderStageFlagBits::eVertex)
                .setDescriptorCount(1);
        createInfoModel.setBindings(bindings);
        setLayout = Context::GetInstance().device.createDescriptorSetLayout(createInfoModel);



        //allocateSets
        std::vector<vk::DescriptorSetLayout> setLayouts(1, setLayout);
        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.setSetLayouts(setLayouts)
                .setDescriptorPool(descriptorPool)
                .setDescriptorSetCount(1);
        writerDescriptorSet = Context::GetInstance().device.allocateDescriptorSets(allocateInfo)[0];

        //updateSets
        vk::DescriptorBufferInfo bufferInfo;
        bufferInfo.setOffset(0)
                .setBuffer(mvp.deviceBuffer->buffer)
                .setRange(mvp.deviceBuffer->size);

        std::array<vk::WriteDescriptorSet, 1> writers;
        writers[0].setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setBufferInfo(bufferInfo)
                .setDstBinding(0)
                .setDstSet(writerDescriptorSet)
                .setDstArrayElement(0);
        Context::GetInstance().device.updateDescriptorSets(writers, {});
    }

    void ShadowMapPass::updateUniform() {
        //data
        glm::vec3 lightPos = Context::GetInstance().renderer->lights.plight.position;
        glm::mat4 depthProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.f);
        depthProjection[1][1] *= -1;
        glm::mat4 depthView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
        uboData.depthMVP = depthProjection * depthView ;

        updateUniformBuffer();
    }

    void ShadowMapPass::updateUniformBuffer() {
        void *data = Context::GetInstance().device.mapMemory(mvp.stagingBuffer->memory, 0,
                                                             mvp.stagingBuffer->size);
        memcpy(data, &uboData, mvp.stagingBuffer->size);
        Context::GetInstance().device.unmapMemory(mvp.stagingBuffer->memory);
        copyBuffer(mvp.stagingBuffer->buffer, mvp.deviceBuffer->buffer, mvp.stagingBuffer->size, 0, 0);
    }

    void ShadowMapPass::setupPipeline() {
        //layout
        std::array<vk::DescriptorSetLayout, 1> setLayouts = {setLayout};

        vk::PushConstantRange pushConstantRange;
        pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex)
                .setOffset(0).setSize(sizeof(glm::mat4));
        vk::PipelineLayoutCreateInfo layoutCreateInfo;
        layoutCreateInfo.setSetLayouts(setLayouts).setPushConstantRanges(pushConstantRange);
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
        auto stages= Context::GetInstance().renderer->shaders.shadow_map_shader->getStages();
        createInfo.setStages(stages[0]);

        //viewport
        vk::PipelineViewportStateCreateInfo viewportCreateInfo;
        vk::Viewport viewports(0, 0, shadowMapSize,
                               shadowMapSize, 0, 1);
        viewportCreateInfo.setViewports(viewports);
        vk::Rect2D rect({0, 0},
                        {shadowMapSize, shadowMapSize});
        viewportCreateInfo.setScissors(rect);
        createInfo.setPViewportState(&viewportCreateInfo);

        //Rasterization
        vk::PipelineRasterizationStateCreateInfo rasterization;
        rasterization.setRasterizerDiscardEnable(false)
                .setCullMode(vk::CullModeFlagBits::eNone)
                .setFrontFace(vk::FrontFace::eCounterClockwise)
                .setPolygonMode(vk::PolygonMode::eFill)
                .setLineWidth(1);
//                .setDepthBiasEnable(true);
        createInfo.setPRasterizationState(&rasterization);

//        //dynamic
//        vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo;
//        std::array<vk::DynamicState,1>dynamicStates={vk::DynamicState::eDepthBias};
//        dynamicStateCreateInfo.setDynamicStates(dynamicStates);
//        createInfo.setPDynamicState(&dynamicStateCreateInfo);

        //multiSample
        vk::PipelineMultisampleStateCreateInfo multisample;
        multisample.setSampleShadingEnable(false)
                .setRasterizationSamples(vk::SampleCountFlagBits::e1);
        createInfo.setPMultisampleState(&multisample);
        //test
        vk::PipelineDepthStencilStateCreateInfo depthStencilState;
        depthStencilState.setDepthTestEnable(vk::True)
                .setDepthWriteEnable(vk::True)
                .setDepthCompareOp(vk::CompareOp::eLessOrEqual);
        createInfo.setPDepthStencilState(&depthStencilState);
        //color blending
        vk::PipelineColorBlendStateCreateInfo colorBlend;
        colorBlend.setLogicOpEnable(false)
                .setAttachmentCount(0);
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

}

