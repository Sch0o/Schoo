#include "outline_pass.hpp"
#include "schoo/function/render/vulkan/context.hpp"
#include "schoo/function/render/vulkan/utils.hpp"

namespace schoo {
    void OutLinePass::init() {
        setupRnderPass();
        setupFrameBuffers();
        setupDescriptor();
        setupPipeline();
    }

    void OutLinePass::draw() {

    }

    void OutLinePass::setupRnderPass() {
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

    void OutLinePass::setupFrameBuffers() {
        Context &context = Context::GetInstance();
        frameBuffers.resize(context.swapchain->imageViews.size());

        for (int i = 0; i < frameBuffers.size(); i++) {
            frameBuffers[i].attachments.resize(2);
            frameBuffers[i].width = context.swapchain->width;
            frameBuffers[i].height = context.swapchain->height;

            FrameBufferAttachment colorAttachment;
            colorAttachment.format = vk::Format::eR8G8B8A8Srgb;
            colorAttachment.image = context.renderer->passes.mesh_pass->frameBuffers[i].attachments[0].image;
            colorAttachment.memory = context.renderer->passes.mesh_pass->frameBuffers[i].attachments[0].memory;
            colorAttachment.imageView = context.renderer->passes.mesh_pass->frameBuffers[i].attachments[0].imageView;
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

    void OutLinePass::setupDescriptor() {
        auto &context = Context::GetInstance();

        vk::DescriptorPoolCreateInfo createInfo;
        std::array<vk::DescriptorPoolSize, 1> poolSizes;
        poolSizes[0].setType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(2);
        createInfo.setMaxSets(2)
                .setPoolSizes(poolSizes);
        descriptorPool = Context::GetInstance().device.createDescriptorPool(createInfo);

        //setLayout
        //uniform
        vk::DescriptorSetLayoutCreateInfo createInfoVP;
        std::array<vk::DescriptorSetLayoutBinding, 1> bindings;
        bindings[0].setBinding(0)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setStageFlags(vk::ShaderStageFlagBits::eVertex)
                .setDescriptorCount(1);
        createInfoVP.setBindings(bindings);
        globalSetLayout = Context::GetInstance().device.createDescriptorSetLayout(createInfoVP);


        //allocate Sets
        //vp matrix and some constant
        globalSets = allocateDescriptor(globalSetLayout, descriptorPool, 2);

        //update sets
        vk::DescriptorBufferInfo vpBufferInfo;
        vpBufferInfo.setOffset(0)
                .setBuffer(context.renderer->passes.mesh_pass->constant.deviceBuffer->buffer)
                .setRange(sizeof(glm::mat4) * 2);

        std::cout << "size: " << sizeof(glm::mat4) * 2 << std::endl;

        std::array<vk::WriteDescriptorSet, 1> Writers;
        for (auto globalSet: globalSets) {
            Writers[0].setDescriptorCount(1)
                    .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                    .setBufferInfo(vpBufferInfo)
                    .setDstBinding(0)
                    .setDstSet(globalSet)
                    .setDstArrayElement(0);
            Context::GetInstance().device.updateDescriptorSets(Writers, {});
        }
    }
}
