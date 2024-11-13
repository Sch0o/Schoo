#include"schoo/texture.hpp"
#include"schoo/buffer.hpp"
#include"schoo/context.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include"stb_image/stb_image.h"

namespace schoo {
    Texture::Texture(std::string_view filename) {
        auto &device = Context::GetInstance().device;
        int w, h, channels;
        //get pixels
        stbi_uc *pixels = stbi_load(filename.data(), &w, &h, &channels, STBI_rgb_alpha);
        size_t size = w * h * 4;


        if (!pixels) {
            throw std::runtime_error("Image load failed");
            return;
        }
        //store pixels to buffer;
        std::unique_ptr<Buffer> stagingBuffer(new Buffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                                                         vk::MemoryPropertyFlagBits::eHostCoherent |
                                                         vk::MemoryPropertyFlagBits::eHostVisible));
        void *data = device.mapMemory(stagingBuffer->memory, 0, size);
        memcpy(data, pixels, size);
        device.unmapMemory(stagingBuffer->memory);
        stbi_image_free(pixels);

        createImage(w, h);
        allocMemoryToImage();
        createImageView();

        transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        copyBufferToImage(stagingBuffer->buffer, w, h);
        transitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        stagingBuffer.reset();
    }

    Texture::~Texture() {
        auto &device=Context::GetInstance().device;
        device.destroyImageView(view);
        device.freeMemory(memory);
        device.destroyImage(image);
    }

    void Texture::createImage(uint32_t w, uint32_t h) {
        vk::ImageCreateInfo createInfo;
        createInfo.setImageType(vk::ImageType::e2D)
                .setArrayLayers(1)
                .setMipLevels(1)
                .setExtent({w, h, 1})
                .setFormat(vk::Format::eR8G8B8A8Srgb)
                .setTiling(vk::ImageTiling::eOptimal)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setSharingMode(vk::SharingMode::eExclusive)
                .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
        image = Context::GetInstance().device.createImage(createInfo);
    }

    void Texture::createImageView() {
        vk::ImageViewCreateInfo createInfo;
        vk::ComponentMapping mapping;
        vk::ImageSubresourceRange range;

        range.setAspectMask(vk::ImageAspectFlagBits::eColor);
        range.setBaseArrayLayer(0)
                .setLayerCount(1)
                .setLevelCount(1)
                .setBaseMipLevel(0);
        createInfo.setImage(image)
                .setViewType(vk::ImageViewType::e2D)
                .setComponents(mapping)
                .setFormat(vk::Format::eR8G8B8A8Srgb)
                .setSubresourceRange(range);
        view = Context::GetInstance().device.createImageView(createInfo);

    }

    void Texture::allocMemoryToImage() {
        auto &device = Context::GetInstance().device;
        vk::MemoryAllocateInfo allocateInfo;
        auto requirements = device.getImageMemoryRequirements(image);
        allocateInfo.setAllocationSize(requirements.size);

        auto index = Buffer::QueryBufferMemTypeIndex(requirements.memoryTypeBits,
                                                     vk::MemoryPropertyFlagBits::eDeviceLocal);

        allocateInfo.setMemoryTypeIndex(index);
        memory = device.allocateMemory(allocateInfo);

        //bind
        device.bindImageMemory(image, memory, 0);
    }

    void Texture::copyBufferToImage(vk::Buffer &srcBuffer, uint32_t w, uint32_t h) {
        vk::CommandBuffer cmdBuffer = Context::GetInstance().commandManager->AllocateCmdBuffer();
        Command::BeginSingleTimeCommands(cmdBuffer);

        //指定将数据复制到图像的哪一部分
        vk::BufferImageCopy region = {};
        vk::ImageSubresourceLayers subresource;
        subresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setMipLevel(0)
                .setBaseArrayLayer(0)
                .setLayerCount(1);
        region.setImageExtent({w, h, 1})
                .setBufferOffset(0)
                .setBufferRowLength(0)
                .setBufferImageHeight(0)
                .setImageSubresource(subresource)
                .setImageOffset({0, 0, 0});
        cmdBuffer.copyBufferToImage(srcBuffer, image, vk::ImageLayout::eTransferDstOptimal, region);

        Command::EndSingleTimeComamands(cmdBuffer, Context::GetInstance().graphicsQueue);
        Context::GetInstance().commandManager->FreeCommandbuffers(cmdBuffer);
    }

    void Texture::transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newlayout) {
        vk::CommandBuffer cmdBuffer = Context::GetInstance().commandManager->AllocateCmdBuffer();
        Command::BeginSingleTimeCommands(cmdBuffer);
        vk::ImageMemoryBarrier barrier;
        barrier.setOldLayout(oldLayout)
                .setNewLayout(newlayout)
                .setImage(image);
        vk::ImageSubresourceRange range;
        range.setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1)
                .setAspectMask(vk::ImageAspectFlagBits::eColor);
        barrier.setSubresourceRange(range);

        vk::PipelineStageFlags srcStage;
        vk::PipelineStageFlags dstStage;
        if (oldLayout == vk::ImageLayout::eUndefined && newlayout == vk::ImageLayout::eTransferDstOptimal) {
            barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
            srcStage=vk::PipelineStageFlagBits::eTopOfPipe;
            dstStage=vk::PipelineStageFlagBits::eTransfer;
        }
        else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newlayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
            srcStage=vk::PipelineStageFlagBits::eTransfer;
            dstStage=vk::PipelineStageFlagBits::eFragmentShader;
        }else{
            throw std::runtime_error("unsupported layout transition");
        }

        cmdBuffer.pipelineBarrier(srcStage, dstStage, {},
                                  {},
                                  nullptr, barrier);

        Command::EndSingleTimeComamands(cmdBuffer, Context::GetInstance().graphicsQueue);
        Context::GetInstance().commandManager->FreeCommandbuffers(cmdBuffer);
    }
}