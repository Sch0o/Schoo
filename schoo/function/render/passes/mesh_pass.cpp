#include"schoo/function/render/passes/mesh_pass.hpp"
#include "schoo/function/render/vulkan/shader.hpp"
#include "schoo/function/render/vulkan/utils.hpp"
#include "engine.hpp"
#include "schoo/function/render/vulkan/context.hpp"
#include "resource/asset_manager.hpp"

namespace schoo {

    MeshPass::MeshPass() {

    }

    void MeshPass::init() {

        cmdBuffers = Context::GetInstance().commandManager->AllocateCmdBuffers(2);
        initUniform();
        createSampler();
        createUniformBuffers();
        createRenderPass();
        createFrameBuffers();
        setupDescriptors();
        createRenderPipeline();
    }

    MeshPass::~MeshPass() {
        auto &commandManager = Context::GetInstance().commandManager;
        auto &device = Context::GetInstance().device;

        device.destroySampler(sampler);
        device.destroyDescriptorPool(descriptorPool);
        constant.stagingBuffer.reset();
        constant.deviceBuffer.reset();
        for (int i = 0; i < 2; i++) {
            commandManager->FreeCommandbuffers(cmdBuffers[i]);
            device.destroyImageView(frameBuffers[i].attachments[0].imageView);
            device.freeMemory(frameBuffers[i].attachments[0].memory);
            device.destroyImage(frameBuffers[i].attachments[0].image);
            device.destroyFramebuffer(frameBuffers[i].framebuffer);
        }
        device.destroyPipeline(renderPipeline.pipeline);
        device.destroyPipelineLayout(renderPipeline.layout);
        device.destroyRenderPass(renderPass);
        device.destroyDescriptorSetLayout(globalSetLayout);
        device.destroyDescriptorSetLayout(pre_drawcall_setLayout);
    }

    void MeshPass::createRenderPass() {
        vk::RenderPassCreateInfo createInfo;

        std::array<vk::AttachmentDescription, 2> descriptions;
        descriptions[0].setFormat(Context::GetInstance().swapchain->swapchainInfo.surfaceFormat.format)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
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


        std::array<vk::SubpassDependency, 2> dependencies;
        dependencies[0].setSrcSubpass(VK_SUBPASS_EXTERNAL)
                .setDstSubpass(0)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        dependencies[1].setSrcSubpass(0)
        .setDstSubpass(VK_SUBPASS_EXTERNAL)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader);
        createInfo.setDependencies(dependencies);

        renderPass = Context::GetInstance().device.createRenderPass(createInfo);
    }

    void MeshPass::draw() {
        UpdateConstants();

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
            clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
            renderPassBeginInfo.setRenderPass(renderPass)
                    .setRenderArea(area)
                    .setFramebuffer(frameBuffers[imageIndex].framebuffer)
                    .setClearValues(clearValues);

            cmdBuffers[currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                        renderPipeline.layout, 0,
                                                        globalSets[imageIndex], {});

            cmdBuffers[currentFrame].beginRenderPass(renderPassBeginInfo, {});
            AssetManager::GetInstance().glTFModel.draw(cmdBuffers[currentFrame], renderPipeline.layout, 1);
            cmdBuffers[currentFrame].endRenderPass();

        }
        cmdBuffers[currentFrame].end();

    }

    void MeshPass::UpdateConstants() {
        uniformConstants.view = SchooEngine::GetInstance().camera->GetViewMatrix();
        uniformConstants.viewPos = glm::vec4(SchooEngine::GetInstance().camera->GetPosition(), 1.0f);
        uniformConstants.lightPos = glm::vec4(Context::GetInstance().renderer->lights.plight.position, 1.0f);
        uniformConstants.lightSpace = Context::GetInstance().renderer->passes.shadow_map_pass->uboData.depthMVP;

        void *data = Context::GetInstance().device.mapMemory(constant.stagingBuffer->memory, 0,
                                                             constant.stagingBuffer->size);
        memcpy(data, &uniformConstants, constant.stagingBuffer->size);
        Context::GetInstance().device.unmapMemory(constant.stagingBuffer->memory);
        copyBuffer(constant.stagingBuffer->buffer, constant.deviceBuffer->buffer, constant.stagingBuffer->size, 0, 0);
    }

    void MeshPass::createRenderPipeline() {

        //layout
        //pushConstant
        vk::PushConstantRange pushConstantRange;
        pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex)
                .setOffset(0).setSize(sizeof(glm::mat4));

        std::array<vk::DescriptorSetLayout, 2> setLayouts = {globalSetLayout, pre_drawcall_setLayout};
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
        auto stages = Context::GetInstance().renderer->shaders.render_shader->getStages();
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
        attachmentState.setBlendEnable(true)
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

    void MeshPass::createSampler() {
        auto &device = Context::GetInstance().device;

        vk::SamplerCreateInfo createInfo;
        createInfo.setMagFilter(vk::Filter::eLinear)
                .setMinFilter(vk::Filter::eLinear)
                .setAddressModeU(vk::SamplerAddressMode::eClampToBorder)
                .setAddressModeV(vk::SamplerAddressMode::eClampToBorder)
                .setAddressModeW(vk::SamplerAddressMode::eClampToBorder)
                .setAnisotropyEnable(vk::False)
                .setMaxAnisotropy(1)
                .setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
                .setUnnormalizedCoordinates(vk::False)
                .setCompareEnable(vk::False)
                .setCompareOp(vk::CompareOp::eAlways)
                .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                .setMinLod(0.0f)
                .setMaxLod(0.0f)
                .setMipLodBias(0.0f);

        sampler = device.createSampler(createInfo);
    }

    void MeshPass::createUniformBuffers() {
        size_t bufferSize = sizeof(uniformConstants);
        //std::cout << "UniformSize:"<<bufferSize<<std::endl;
        constant.stagingBuffer.reset(new Buffer(bufferSize,
                                                vk::BufferUsageFlagBits::eTransferSrc,
                                                vk::MemoryPropertyFlagBits::eHostCoherent |
                                                vk::MemoryPropertyFlagBits::eHostVisible));

        constant.deviceBuffer.reset(new Buffer(bufferSize,
                                               vk::BufferUsageFlagBits::eUniformBuffer |
                                               vk::BufferUsageFlagBits::eTransferDst,
                                               vk::MemoryPropertyFlagBits::eDeviceLocal));

    }

    void MeshPass::initUniform() {

        uniformConstants.view = SchooEngine::GetInstance().camera->GetViewMatrix();
        uint32_t width = Context::GetInstance().swapchain->width;
        uint32_t height = Context::GetInstance().swapchain->height;
        uniformConstants.projection = glm::perspective(glm::radians(fov), width / (height * 1.0f), 0.1f, 100.0f);
        uniformConstants.projection[1][1] *= -1;

        auto &renderer = Context::GetInstance().renderer;
        uniformConstants.lightSpace = renderer->passes.shadow_map_pass->uboData.depthMVP;

        glm::vec3 lightPos = Context::GetInstance().renderer->lights.plight.position;
        glm::vec3 lightColor = Context::GetInstance().renderer->lights.plight.color;

        uniformConstants.lightPos = glm::vec4(lightPos, 1.0f);
        uniformConstants.viewPos = glm::vec4(SchooEngine::GetInstance().camera->GetPosition(), 1.0f);
        uniformConstants.lightColor = glm::vec4(lightColor, 0.0f);
    }

    void MeshPass::createFrameBuffers() {
        Context &context = Context::GetInstance();
        frameBuffers.resize(context.swapchain->imageViews.size());

        for (int i = 0; i < frameBuffers.size(); i++) {
            frameBuffers[i].attachments.resize(2);
            frameBuffers[i].width = context.swapchain->width;
            frameBuffers[i].height = context.swapchain->height;

            FrameBufferAttachment colorAttachment;
            colorAttachment.format = vk::Format::eR8G8B8A8Srgb;
            colorAttachment.image = createImage(frameBuffers[i].width, frameBuffers[i].height, colorAttachment.format,
                                                vk::ImageUsageFlagBits::eColorAttachment |
                                                vk::ImageUsageFlagBits::eSampled);
            colorAttachment.memory = createImageMemory(colorAttachment.image);
            colorAttachment.imageView = createImageView(colorAttachment.image, colorAttachment.format,
                                                        vk::ImageAspectFlagBits::eColor);
            frameBuffers[i].attachments[0] = colorAttachment;

            FrameBufferAttachment depthAttachment;
            depthAttachment.format = context.swapchain->depthFormat;
            depthAttachment.image = context.swapchain->depthImage;
            depthAttachment.imageView = context.swapchain->depthImageView;
            depthAttachment.memory = context.swapchain->depthImageMemory;
            frameBuffers[i].attachments[1] = depthAttachment;

            vk::FramebufferCreateInfo createInfo;
            std::array<vk::ImageView, 2> attachments = {frameBuffers[i].attachments[0].imageView,
                                                        frameBuffers[i].attachments[1].imageView};
            createInfo.setAttachments(attachments)
                    .setWidth(frameBuffers[i].width)
                    .setHeight(frameBuffers[i].height)
                    .setRenderPass(renderPass)
                    .setLayers(1);
            frameBuffers[i].framebuffer = Context::GetInstance().device.createFramebuffer(createInfo);
        }
    }

    void MeshPass::setupDescriptors() {

        auto &images = AssetManager::GetInstance().glTFModel.images;

        int image_size = images.size();

        vk::DescriptorPoolCreateInfo createInfo;
        std::array<vk::DescriptorPoolSize, 2> poolSizes;
        poolSizes[0].setType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(3);
        poolSizes[1].setType(vk::DescriptorType::eCombinedImageSampler)
                .setDescriptorCount(2 + image_size);
        createInfo.setMaxSets(2 + image_size)
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
        pre_drawcall_setLayout = Context::GetInstance().device.createDescriptorSetLayout(createInfo_preDrawCall);

        //vp matrix and some constant
        vk::DescriptorSetLayoutCreateInfo createInfoVP;
        std::array<vk::DescriptorSetLayoutBinding, 2> bindings2;
        bindings2[0].setBinding(0)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setStageFlags(vk::ShaderStageFlagBits::eVertex)
                .setDescriptorCount(1);
        bindings2[1].setBinding(1)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setStageFlags(vk::ShaderStageFlagBits::eFragment)
                .setDescriptorCount(1);

        createInfoVP.setBindings(bindings2);
        globalSetLayout = Context::GetInstance().device.createDescriptorSetLayout(createInfoVP);


        //allocate Sets
        //sampler
        for (auto &image: images) {
            image.descriptorSet = allocateDescriptor(pre_drawcall_setLayout, descriptorPool, 1)[0];
        }
        //vp matrix and some constant
        globalSets = allocateDescriptor(globalSetLayout, descriptorPool, 2);

        //update sets
        vk::DescriptorBufferInfo vpBufferInfo;
        vpBufferInfo.setOffset(0)
                .setBuffer(constant.deviceBuffer->buffer)
                .setRange(constant.deviceBuffer->size);
        std::array<vk::WriteDescriptorSet, 2> Writers1;
        for (int i = 0; i < globalSets.size(); i++) {
            Writers1[0].setDescriptorCount(1)
                    .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                    .setBufferInfo(vpBufferInfo)
                    .setDstBinding(0)
                    .setDstSet(globalSets[i])
                    .setDstArrayElement(0);

            vk::DescriptorImageInfo imageInfo;
            imageInfo.setImageLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal)
                    .setImageView(
                            Context::GetInstance().renderer->passes.shadow_map_pass->framebuffers[i].getImageView(0))
                    .setSampler(sampler);
            Writers1[1].setDescriptorCount(1)
                    .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                    .setImageInfo(imageInfo)
                    .setDstBinding(1)
                    .setDstSet(globalSets[i])
                    .setDstArrayElement(0);
            Context::GetInstance().device.updateDescriptorSets(Writers1, {});
        }

        // sampler
        std::array<vk::WriteDescriptorSet, 1> modelWriters;
        for (auto &image: images) {
            //texcoord
            vk::DescriptorImageInfo imageInfo;
            imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                    .setImageView(image.texture->view)
                    .setSampler(sampler);

            modelWriters[0].setDescriptorCount(1)
                    .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                    .setImageInfo(imageInfo)
                    .setDstBinding(0)
                    .setDstSet(image.descriptorSet)
                    .setDstArrayElement(0);
            Context::GetInstance().device.updateDescriptorSets(modelWriters, {});
        }
    }


}