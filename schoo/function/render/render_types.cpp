#include"render_types.hpp"
namespace schoo{
    AllocatedBuffer::AllocatedBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
        VkBufferCreateInfo createInfo;
        createInfo.usage=usage;
        createInfo.size=allocSize;
        VmaAllocationCreateInfo vmaAllocationCreateInfo;
        vmaAllocationCreateInfo.usage=memoryUsage;
        vmaAllocationCreateInfo.flags= VMA_ALLOCATION_CREATE_MAPPED_BIT;

        //allocate buffer
        vmaCreateBuffer(Context::GetInstance().vmaAllocator,&createInfo,
                        &vmaAllocationCreateInfo,&buffer,&allocation,
                        &info);
    }

    void AllocatedBuffer::destroyBuffer() {
        vmaDestroyBuffer(Context::GetInstance().vmaAllocator,buffer,allocation);
    }
}