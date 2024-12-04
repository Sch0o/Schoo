#pragma once

#include"vulkan/vulkan.hpp"
namespace schoo{
    class Buffer final {
    public:
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        size_t size;
//        void *data;

        Buffer(size_t size1,vk::BufferUsageFlags usage,vk::MemoryPropertyFlags property);
        ~Buffer();

        static std::uint32_t QueryBufferMemTypeIndex(std::uint32_t type,vk::MemoryPropertyFlags flags);
        void CopyDataToBuffer(void *src);

    private:

        struct MemoryInfo final {
            size_t size;
            uint32_t index;
        };
        void createBuffer(size_t size,vk::BufferUsageFlags usage);
        void allocateMemory(MemoryInfo info);
        void bindingMemoryToBuffer();
        MemoryInfo queryMemoryInfo(vk::MemoryPropertyFlags property);

    };
}