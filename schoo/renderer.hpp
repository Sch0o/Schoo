#pragma once
#include"vulkan/vulkan.hpp"
#include"buffer.hpp"

namespace schoo {
    class Renderer final {
    public:
        Renderer(uint32_t frameNums=2);
        ~Renderer();

        void Render();
        void InitRenderer();
    private:
        uint32_t frameNums;
        uint32_t currentFrame=0;
        std::vector<vk::Semaphore> imageAvaliables_;
        std::vector<vk::Semaphore> imageDrawFinshs_;
        std::vector<vk::Fence> cmdAvaliableFences_;

        std::vector<vk::CommandBuffer> cmdBuffers_;

        std::unique_ptr<Buffer>vertexBuffer_;


        void createCmdBuffers();
        void createSemaphores();
        void createFences();
        void createVertexBuffer();
        void loadVertexDate();
    };
}