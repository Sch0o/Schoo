#include"renderProcess.hpp"
#include"schoo/shader.hpp"
#include "schoo/vertexData.hpp"


namespace schoo {
    RenderProcess::RenderProcess() {
        createSetLayout();
        layout = createLayout();
        CreateRenderPass();
        pipeline = nullptr;
    }

    void RenderProcess::CreatePipeline() {
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
        createInfo.setLayout(layout).setRenderPass(renderPass);

        //create pipeline
        auto resultValue = Context::GetInstance().device.createGraphicsPipeline(nullptr, createInfo);
        if (resultValue.result != vk::Result::eSuccess) {
            throw std::runtime_error("create pipeline failed");
        }
        pipeline = resultValue.value;

    }

    vk::PipelineLayout RenderProcess::createLayout() {
        std::array<vk::DescriptorSetLayout, 2> setLayouts = {vpSetLayout, modelSetLayout};

        vk::PipelineLayoutCreateInfo createInfo;
        createInfo.setSetLayouts(setLayouts);
        return Context::GetInstance().device.createPipelineLayout(createInfo);

    }

    void RenderProcess::CreateRenderPass() {
        vk::RenderPassCreateInfo createInfo;

        std::array<vk::AttachmentDescription,2> descriptions;
        descriptions[0].setFormat(Context::GetInstance().swapchain->swapchainInfo.surfaceFormat.format)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
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

    RenderProcess::~RenderProcess() {
        auto &device = Context::GetInstance().device;
        device.destroyRenderPass(renderPass);
        device.destroyDescriptorSetLayout(vpSetLayout);
        device.destroyDescriptorSetLayout(modelSetLayout);
        device.destroyPipelineLayout(layout);
        device.destroyPipeline(pipeline);
    }

    void RenderProcess::createSetLayout() {
        //model matrix and sampler
        vk::DescriptorSetLayoutCreateInfo createInfoModel;
        std::array<vk::DescriptorSetLayoutBinding,2> bindingsModel;
        bindingsModel[0].setBinding(0)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setStageFlags(vk::ShaderStageFlagBits::eVertex)
                .setDescriptorCount(1);
        bindingsModel[1].setBinding(1)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setStageFlags(vk::ShaderStageFlagBits::eFragment)
                .setDescriptorCount(1);
        createInfoModel.setBindings(bindingsModel);
        modelSetLayout=Context::GetInstance().device.createDescriptorSetLayout(createInfoModel);

        //vp matrix
        vk::DescriptorSetLayoutCreateInfo createInfoVP;
        std::array<vk::DescriptorSetLayoutBinding,1> bindingsVP;
        bindingsVP[0].setBinding(0)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setStageFlags(vk::ShaderStageFlagBits::eVertex)
                .setDescriptorCount(1);
        createInfoVP.setBindings(bindingsVP);
        vpSetLayout=Context::GetInstance().device.createDescriptorSetLayout(createInfoVP);
    }

}