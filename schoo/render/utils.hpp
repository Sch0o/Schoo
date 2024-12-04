#pragma once

#include<functional>
#include<vulkan/vulkan.hpp>
#include <fstream>
#include <iostream>
#include<iomanip>

namespace schoo {
    std::string ReadWholeFile(const std::string &filename);

    bool CheckPath(const std::string path);

    vk::ImageView CreateImageView(vk::Image, vk::Format format, vk::ImageAspectFlags aspectFlags);

    vk::Image CreateImage();

    void transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newlayout);

    void copyBuffer(vk::Buffer &src, vk::Buffer &dst, size_t size, size_t srcOffset, size_t dstOffset);
}
