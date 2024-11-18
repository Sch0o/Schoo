#include "schoo/utils.hpp"
#include"schoo/context.hpp"
#include "filesystem"

namespace schoo{
    std::string ReadWholeFile(const std::string&filename){
        std::ifstream file(filename,std::ios::binary|std::ios::ate);

        if(!file.is_open()){
            std::cout<<"file open failed"<<std::endl;
            return std::string {};
        }

        auto size=file.tellg();
        std::string context;
        context.resize(size);
        file.seekg(0);
        file.read(context.data(),context.size());

        return context;
    }

    bool CheckPath(const std::string path){
        if(std::filesystem::exists(path)){
            return true;
        }else{
            std::cout<<"this path :"<<path<<" is not exist"<<std::endl;
            return false;
        }
    }

    vk::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags){

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

}