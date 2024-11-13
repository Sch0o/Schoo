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
            std::cout<<"this path :"<<path<<"is not exist"<<std::endl;
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

//    void transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newlayout) {
//        vk::CommandBuffer cmdBuffer = Context::GetInstance().commandManager->AllocateCmdBuffer();
//        Command::BeginSingleTimeCommands(cmdBuffer);
//        vk::ImageMemoryBarrier barrier;
//        barrier.setOldLayout(oldLayout)
//                .setNewLayout(newlayout)
//                .setImage(image);
//        vk::ImageSubresourceRange range;
//        range.setBaseMipLevel(0)
//                .setLevelCount(1)
//                .setBaseArrayLayer(0)
//                .setLayerCount(1)
//                .setAspectMask(vk::ImageAspectFlagBits::eColor);
//        barrier.setSubresourceRange(range);
//
//        vk::PipelineStageFlags srcStage;
//        vk::PipelineStageFlags dstStage;
//        if (oldLayout == vk::ImageLayout::eUndefined && newlayout == vk::ImageLayout::eTransferDstOptimal) {
//            barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
//                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
//            srcStage=vk::PipelineStageFlagBits::eTopOfPipe;
//            dstStage=vk::PipelineStageFlagBits::eTransfer;
//        }
//        else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newlayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
//            barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
//                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
//            srcStage=vk::PipelineStageFlagBits::eTransfer;
//            dstStage=vk::PipelineStageFlagBits::eFragmentShader;
//        }else{
//            throw std::runtime_error("unsupported layout transition");
//        }
//
//        cmdBuffer.pipelineBarrier(srcStage, dstStage, {},
//                                  {},
//                                  nullptr, barrier);
//
//        Command::EndSingleTimeComamands(cmdBuffer, Context::GetInstance().graphicsQueue);
//        Context::GetInstance().commandManager->FreeCommandbuffers(cmdBuffer);
//    }
}