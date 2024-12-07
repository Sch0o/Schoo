#pragma once
#include"vulkan/vulkan.hpp"

namespace schoo {
    class Texture final{
    public:
        Texture(std::string_view filename);
        ~Texture();
        vk::Image image;
        vk::DeviceMemory memory;
        vk::ImageView view;

    private:

        void createImage(uint32_t w,uint32_t h);
        void allocMemoryToImage();
        void createImageView();
        uint32_t queryImageViewIndex();
        void copyBufferToImage(vk::Buffer &srcBuffer,uint32_t w,uint32_t h);
        void transitionImageLayout(vk::ImageLayout oldLayout,vk::ImageLayout newLayout);

    };
}