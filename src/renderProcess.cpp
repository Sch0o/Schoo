#include"renderProcess.hpp"
#include"schoo/Shader.hpp"
#include "schoo/vertexData.hpp"

namespace schoo {
    void RenderProcess::InitPipeline() {
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
                .setFrontFace(vk::FrontFace::eClockwise)
                .setPolygonMode(vk::PolygonMode::eFill)
                .setLineWidth(1);
        createInfo.setPRasterizationState(&rasterization);

        //multiSample
        vk::PipelineMultisampleStateCreateInfo multisample;
        multisample.setSampleShadingEnable(false)
                .setRasterizationSamples(vk::SampleCountFlagBits::e1);
        createInfo.setPMultisampleState(&multisample);

        //test


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

    void RenderProcess::InitLayout() {
        vk::PipelineLayoutCreateInfo createInfo;
        layout = Context::GetInstance().device.createPipelineLayout(createInfo);
    }

    void RenderProcess::InitRenderPass() {
        vk::RenderPassCreateInfo createInfo;

        vk::AttachmentDescription description;
        description.setFormat(Context::GetInstance().swapchain->swapchainInfo.surfaceFormat.format)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setSamples(vk::SampleCountFlagBits::e1);
        createInfo.setAttachments(description);

        vk::AttachmentReference reference;
        reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setAttachment(0);

        vk::SubpassDescription subpassDescription;
        subpassDescription.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setColorAttachments(reference);
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
        device.destroyPipelineLayout(layout);
        device.destroyPipeline(pipeline);
    }
}