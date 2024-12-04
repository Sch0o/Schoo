#pragma once

#include"vulkan/vulkan.hpp"

namespace schoo{
    class Swapchain final{
    public :
        vk::SwapchainKHR swapchain;
        Swapchain(uint32_t w,uint32_t h);
        ~Swapchain();

        uint32_t width,height;

        struct SwapchainInfo{
            vk::Extent2D imageExtent;
            vk::SurfaceFormatKHR surfaceFormat;
            uint32_t imageCount;
            vk::SurfaceTransformFlagsKHR transform;
            vk::PresentModeKHR presentMode;
        };

        std::vector<vk::Image>images;
        std::vector<vk::ImageView>imageViews;

        vk::Image depthImage;
        vk::DeviceMemory depthImageMemory;
        vk::ImageView depthImageView;

        SwapchainInfo swapchainInfo;
        void queryInfo(uint32_t w,uint32_t h);


    private:
        void getImages();
        void createDepthImage();
        void createDepthImageView();
        void createImageViews();
        void transitionImageLayoutToDepth();
    };
}