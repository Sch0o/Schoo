#include"schoo/buffer.hpp"
#include"schoo/context.hpp"

namespace schoo {
    Buffer::Buffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property) {
        this->size=size;
        createBuffer(size, usage);
        auto info = queryMemoryInfo(property);
        //std::cout<<"size::"<<info.size<<std::endl;
        allocateMemory(info);
        bindingMemoryToBuffer();
    }

    Buffer::~Buffer() {
        //Context::GetInstance().device.unmapMemory(memory);
        Context::GetInstance().device.freeMemory(memory);
        Context::GetInstance().device.destroyBuffer(buffer);
    }

    void Buffer::createBuffer(size_t size, vk::BufferUsageFlags usage) {
        vk::BufferCreateInfo createInfo;
        createInfo.setSize(size)
                .setUsage(usage)
                .setSharingMode(vk::SharingMode::eExclusive);
        buffer = Context::GetInstance().device.createBuffer(createInfo);
    }

    void Buffer::allocateMemory(MemoryInfo info) {
        vk::MemoryAllocateInfo allocateInfo;
        allocateInfo.setMemoryTypeIndex(info.index)
                .setAllocationSize(info.size);
        memory = Context::GetInstance().device.allocateMemory(allocateInfo);
    }

    void Buffer::bindingMemoryToBuffer() {
        Context::GetInstance().device.bindBufferMemory(buffer,memory,0);
    }

    Buffer::MemoryInfo Buffer::queryMemoryInfo(vk::MemoryPropertyFlags property) {
        MemoryInfo info{};
        auto requirements = Context::GetInstance().device.getBufferMemoryRequirements(buffer);
        info.size = requirements.size;

        auto properties=Context::GetInstance().physicalDevice.getMemoryProperties();
        for(int i=0;i<properties.memoryTypeCount;i++){
            if((1<<i)&requirements.memoryTypeBits&&
            properties.memoryTypes[i].propertyFlags&property){
                info.index=i;
                break;
            }
        }

        return info;
    }

    std::uint32_t Buffer::QueryBufferMemTypeIndex(std::uint32_t type,vk::MemoryPropertyFlags flags){
        auto property=Context::GetInstance().physicalDevice.getMemoryProperties();
        for(std::uint32_t i=0;i<property.memoryTypeCount;i++){
            if((1<<i)&type&&property.memoryTypes[i].propertyFlags&flags){
                return i;
            }
        }
    }

}