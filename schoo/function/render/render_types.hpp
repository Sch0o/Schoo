#include "external/vulkanmemoryallocator/vk_mem_alloc.h"
#include"schoo/function/render/vulkan/context.hpp"
namespace schoo{
    struct AllocatedBuffer{
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo info;

        AllocatedBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        void destroyBuffer();
    };



}