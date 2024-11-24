#include"schoo/swapchain.hpp"
#include"schoo/context.hpp"
#include"schoo/utils.hpp"

namespace schoo {
    Swapchain::Swapchain(uint32_t w, uint32_t h) {
        width=w;
        height=h;
        queryInfo(w, h);

        vk::SwapchainCreateInfoKHR createInfo;
        createInfo.setClipped(true)
                .setImageArrayLayers(1)
                .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                .setSurface(Context::GetInstance().surface)
                .setImageColorSpace(swapchainInfo.surfaceFormat.colorSpace)
                .setImageFormat(swapchainInfo.surfaceFormat.format)
                .setImageExtent(swapchainInfo.imageExtent)
                .setMinImageCount(swapchainInfo.imageCount)
                .setPresentMode(swapchainInfo.presentMode);

        auto &queueIndices = Context::GetInstance().queueFamilyIndices;
        if (queueIndices.isCompeleted()) {
            createInfo.setQueueFamilyIndices(queueIndices.presentQueue.value())
            .setImageSharingMode(vk::SharingMode::eExclusive);
        }else{
            std::array indices={queueIndices.graphicQueue.value(),queueIndices.graphicQueue.value()};
            createInfo.setQueueFamilyIndices(indices)
            .setImageSharingMode(vk::SharingMode::eConcurrent);
        }

        swapchain=Context::GetInstance().device.createSwapchainKHR(createInfo);

        getImages();
        createImageViews();

        createDepthImage();
        createDepthImageView();
        transitionImageLayoutToDepth();

    }

    Swapchain::~Swapchain() {
        auto&device=Context::GetInstance().device;
        for(uint32_t i=0;i<imageViews.size();i++){
            device.destroyFramebuffer(frameBuffers[i]);
            device.destroyImageView(imageViews[i]);
        }

        device.destroyImageView(depthImageView);
        device.destroyImage(depthImage);
        device.freeMemory(depthImageMemory);

        Context::GetInstance().device.destroySwapchainKHR(swapchain);
    }

    void Swapchain::queryInfo(uint32_t w, uint32_t h) {
        auto &physicalDevice = Context::GetInstance().physicalDevice;
        auto &surface = Context::GetInstance().surface;
        auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);

        swapchainInfo.surfaceFormat = surfaceFormats[0];
        for (const auto &format: surfaceFormats) {
            if (format.format == vk::Format::eR8G8B8A8Srgb &&
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                swapchainInfo.surfaceFormat = format;
                break;
            }
        }

        auto capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
        swapchainInfo.imageCount = std::clamp<uint32_t>(2, capabilities.minImageCount, capabilities.maxImageCount);
        swapchainInfo.imageExtent.width = std::clamp(w, capabilities.minImageExtent.width,
                                                     capabilities.maxImageExtent.width);
        swapchainInfo.imageExtent.height = std::clamp(h, capabilities.minImageExtent.height,
                                                      capabilities.maxImageExtent.height);

        swapchainInfo.transform = capabilities.currentTransform;

        auto presents = physicalDevice.getSurfacePresentModesKHR(surface);
        swapchainInfo.presentMode=vk::PresentModeKHR::eFifo;
        for (const auto &present: presents) {
            if (present == vk::PresentModeKHR::eMailbox)
                swapchainInfo.presentMode = present;
            break;
        }

    }

    void Swapchain::getImages(){
        images=Context::GetInstance().device.getSwapchainImagesKHR(swapchain);
    }

    void Swapchain::createImageViews() {
        imageViews.resize(images.size());
        for(int i=0;i<images.size();i++){
            vk::ImageViewCreateInfo createInfo;
            vk::ComponentMapping mapping;
            vk::ImageSubresourceRange range;
            range.setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setLayerCount(1);

            createInfo.setImage(images[i])
            .setViewType(vk::ImageViewType::e2D)
            .setComponents(mapping)
            .setFormat(swapchainInfo.surfaceFormat.format)
            .setSubresourceRange(range);

            imageViews[i]=Context::GetInstance().device.createImageView(createInfo);

        }
    }

    void Swapchain::CreateFramebuffers() {
        frameBuffers.resize(imageViews.size());
        for(int i=0;i<frameBuffers.size();i++){
            vk::FramebufferCreateInfo createInfo;
            std::array<vk::ImageView,2>attachments={imageViews[i],depthImageView};
            createInfo.setAttachments(attachments)
            .setWidth(width)
            .setHeight(height)
            .setRenderPass(Context::GetInstance().renderProcess->meshRenderPass)
            .setLayers(1);
            frameBuffers[i]=Context::GetInstance().device.createFramebuffer(createInfo);
        }
    }

    void Swapchain::createDepthImage(){
        vk::ImageCreateInfo createInfo;
        createInfo.setImageType(vk::ImageType::e2D)
                .setArrayLayers(1)
                .setMipLevels(1)
                .setExtent({width, height, 1})
                .setFormat(vk::Format::eD24UnormS8Uint)
                .setTiling(vk::ImageTiling::eOptimal)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setSharingMode(vk::SharingMode::eExclusive)
                .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
        depthImage = Context::GetInstance().device.createImage(createInfo);

        auto &device = Context::GetInstance().device;
        vk::MemoryAllocateInfo allocateInfo;
        auto requirements = device.getImageMemoryRequirements(depthImage);
        allocateInfo.setAllocationSize(requirements.size);

        auto index = Buffer::QueryBufferMemTypeIndex(requirements.memoryTypeBits,
                                                     vk::MemoryPropertyFlagBits::eDeviceLocal);

        allocateInfo.setMemoryTypeIndex(index);
        depthImageMemory = device.allocateMemory(allocateInfo);

        //bind
        device.bindImageMemory(depthImage, depthImageMemory, 0);
    }

    void Swapchain::createDepthImageView() {
        depthImageView= CreateImageView(depthImage,vk::Format::eD24UnormS8Uint,vk::ImageAspectFlagBits::eDepth);
    }

    void Swapchain::transitionImageLayoutToDepth() {
        vk::ImageLayout oldLayout=vk::ImageLayout::eUndefined;
        vk::ImageLayout newLayout=vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::CommandBuffer cmdBuffer = Context::GetInstance().commandManager->AllocateCmdBuffer();
        Command::BeginSingleTimeCommands(cmdBuffer);
        vk::ImageMemoryBarrier barrier;
        barrier.setOldLayout(oldLayout)
                .setNewLayout(newLayout)
                .setImage(depthImage);
        vk::ImageSubresourceRange range;
        range.setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1)
                .setAspectMask(vk::ImageAspectFlagBits::eDepth|vk::ImageAspectFlagBits::eStencil);
        barrier.setSubresourceRange(range);
        barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead|
                vk::AccessFlagBits::eDepthStencilAttachmentWrite);

        vk::PipelineStageFlags srcStage=vk::PipelineStageFlagBits::eTopOfPipe;
        vk::PipelineStageFlags dstStage=vk::PipelineStageFlagBits::eEarlyFragmentTests;

        cmdBuffer.pipelineBarrier(srcStage, dstStage, {},
                                  {},
                                  nullptr, barrier);

        Command::EndSingleTimeComamands(cmdBuffer, Context::GetInstance().graphicsQueue);
        Context::GetInstance().commandManager->FreeCommandbuffers(cmdBuffer);
    }
}
