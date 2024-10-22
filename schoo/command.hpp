#pragma once

#include"vulkan/vulkan.hpp"


namespace schoo {
    class Command final {
    public:
        Command();

        ~Command();

        void FreeCommandbuffers(vk::CommandBuffer cmdBuffer);

        std::vector<vk::CommandBuffer> AllocateCmdBuffers(uint32_t count);

        vk::CommandBuffer AllocateCmdBuffer();


    private:
        vk::CommandPool cmdPool_;

        void initCmdPool();

    };
}