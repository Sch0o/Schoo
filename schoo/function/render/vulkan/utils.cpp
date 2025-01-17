#include "utils.hpp"
#include"context.hpp"
#include "filesystem"

namespace schoo {
    std::string ReadWholeFile(const std::string &filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);

        if (!file.is_open()) {
            std::cout << "file open failed" << std::endl;
            return std::string{};
        }

        auto size = file.tellg();
        std::string context;
        context.resize(size);
        file.seekg(0);
        file.read(context.data(), context.size());

        return context;
    }

    bool CheckPath(const std::string path) {
        if (std::filesystem::exists(path)) {
            return true;
        } else {
            std::cout << "this path :" << path << " is not exist" << std::endl;
            return false;
        }
    }

    vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) {

        vk::ImageViewCreateInfo createInfo;
        vk::ComponentMapping mapping;
        vk::ImageSubresourceRange range;

        range.setAspectMask(aspectFlags);
        range.setBaseArrayLayer(0)
                .setLayerCount(1)
                .setLevelCount(1)
                .setBaseMipLevel(0);
        createInfo.setImage(image)
                .setViewType(vk::ImageViewType::e2D)
                .setComponents(mapping)
                .setFormat(format)
                .setSubresourceRange(range);
        vk::ImageView imageView = Context::GetInstance().device.createImageView(createInfo);
        return imageView;
    }

    void copyBuffer(vk::Buffer &src, vk::Buffer &dst, size_t size, size_t srcOffset, size_t dstOffset) {
        auto cmdBuffer = Context::GetInstance().commandManager->AllocateCmdBuffer();

        vk::CommandBufferBeginInfo begin;
        begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdBuffer.begin(begin);
        vk::BufferCopy region;
        region.setSize(size)
                .setSrcOffset(srcOffset)
                .setDstOffset(dstOffset);
        cmdBuffer.copyBuffer(src, dst, region);
        cmdBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(cmdBuffer);
        Context::GetInstance().graphicsQueue.submit(submitInfo);
        Context::GetInstance().device.waitIdle();
        Context::GetInstance().commandManager->FreeCommandbuffers(cmdBuffer);
    }

    void loadDataHostToDevice(const std::shared_ptr<Buffer> &hostBuffer, const std::shared_ptr<Buffer> &DeviceBuffer,
                              const void *src) {
        void *data = Context::GetInstance().device.mapMemory(hostBuffer->memory, 0, hostBuffer->size);
        memcpy(data, src, hostBuffer->size);
        Context::GetInstance().device.unmapMemory(hostBuffer->memory);

        copyBuffer(hostBuffer->buffer, DeviceBuffer->buffer, hostBuffer->size, 0, 0);

    }

    std::vector<vk::DescriptorSet>
    allocateDescriptor(vk::DescriptorSetLayout layout, vk::DescriptorPool pool, uint32_t count) {
        std::vector<vk::DescriptorSetLayout> setLayouts(count, layout);
        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.setSetLayouts(setLayouts)
                .setDescriptorPool(pool)
                .setDescriptorSetCount(count);
        return Context::GetInstance().device.allocateDescriptorSets(allocateInfo);
    }

    vk::Sampler createSampler() {
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

        return device.createSampler(createInfo);
    }

    vk::Image createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags usageFlags) {
        vk::ImageCreateInfo imageCreateInfo;
        imageCreateInfo.setImageType(vk::ImageType::e2D)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFormat(format)
                .setExtent({width, height, 1})
                .setMipLevels(1)
                .setArrayLayers(1)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setTiling(vk::ImageTiling::eOptimal)
                .setUsage(usageFlags);

        return Context::GetInstance().device.createImage(imageCreateInfo);
    }

    vk::DeviceMemory createImageMemory(vk::Image image) {
        auto &device = Context::GetInstance().device;
        vk::MemoryAllocateInfo allocateInfo;
        auto requirements = device.getImageMemoryRequirements(image);
        allocateInfo.setAllocationSize(requirements.size);
        auto index = Buffer::QueryBufferMemTypeIndex(requirements.memoryTypeBits,
                                                     vk::MemoryPropertyFlagBits::eDeviceLocal);

        allocateInfo.setMemoryTypeIndex(index);
        vk::DeviceMemory memory = device.allocateMemory(allocateInfo);
        //bind
        device.bindImageMemory(image, memory, 0);
        return memory;
    }
}