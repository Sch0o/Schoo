#include"schoo/render/command.hpp"
#include"schoo/render/context.hpp"

namespace schoo {
    Command::Command() {
        initCmdPool();
    }

    Command::~Command() {
        auto &device = Context::GetInstance().device;
        device.destroyCommandPool(cmdPool_);
    }

    void Command::initCmdPool() {
        vk::CommandPoolCreateInfo createInfo;
        createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

        cmdPool_ = Context::GetInstance().device.createCommandPool(createInfo);
    }

    vk::CommandBuffer Command::AllocateCmdBuffer() {
        return AllocateCmdBuffers(1)[0];
    }

    std::vector<vk::CommandBuffer> Command::AllocateCmdBuffers(uint32_t count) {
        vk::CommandBufferAllocateInfo allocateInfo;
        allocateInfo.setCommandPool(cmdPool_)
                .setCommandBufferCount(count)
                .setLevel(vk::CommandBufferLevel::ePrimary);

        return Context::GetInstance().device.allocateCommandBuffers(allocateInfo);
    }

    void Command::FreeCommandbuffers(vk::CommandBuffer cmdBuffer) {
        Context::GetInstance().device.freeCommandBuffers(cmdPool_, cmdBuffer);
    }

    void Command::BeginSingleTimeCommands(vk::CommandBuffer &cmdBuffer) {
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdBuffer.begin(beginInfo);
    }

    void Command::EndSingleTimeComamands(vk::CommandBuffer &cmdBuffer,vk::Queue&queue) {
         cmdBuffer.end();
         vk::SubmitInfo submitInfo;
         submitInfo.setCommandBuffers(cmdBuffer);
         queue.submit(submitInfo);
         queue.waitIdle();
    }


}
