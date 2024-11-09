#pragma once
#include"vulkan/vulkan.hpp"
#include"buffer.hpp"
#include"glm/glm.hpp"


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

        std::unique_ptr<Buffer>hostVertexBuffer_;
        std::unique_ptr<Buffer>deviceVertexBuffer_;

        std::unique_ptr<Buffer>hostIndexBuffer_;
        std::unique_ptr<Buffer>deviceIndexBuffer_;

        std::unique_ptr<Buffer>hostUniformBuffer_;
        std::unique_ptr<Buffer>deviceUniformBuffer_;

        vk::DescriptorPool descriptorPool_;
        std::vector<vk::DescriptorSet>sets_;

        struct VP{
            glm::mat4 view=glm::mat4(1.0f);
            glm::mat4 project=glm::mat4(1.0f);
        }vp;
        float fov_=90;

        void initVP();
        void createCmdBuffers();
        void createSemaphores();
        void createFences();
        void createVertexBuffer();
        void createIndexBuffer();
        void createUniformBuffer();

        void loadIndexData();
        void loadVertexData();
        void loadUniformData();

        void createDescriptorPool();
        void allocateSets();
        void updateSets();

        void copyBuffer(vk::Buffer&src,vk::Buffer&dst,size_t size,size_t srcOffset,size_t dstOffset);
    };
}