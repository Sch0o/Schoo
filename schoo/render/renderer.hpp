#pragma once

#include"vulkan/vulkan.hpp"
#include"schoo/render/buffer.hpp"
#include"glm/glm.hpp"
#include "schoo/render/vertexData.hpp"
#include"schoo/render/texture.hpp"
#include"schoo/render/model.hpp"
#include"schoo/render/passes/mesh_pass.hpp"
#include"schoo/render/passes/ui_pass.hpp"

namespace schoo {
    class Renderer final {
    public:
        Renderer(uint32_t frameNums = 2);

        ~Renderer();

        void Render();


        uint32_t currentFrame = 0;
        uint32_t imageIndex=0;

    private:
        std::shared_ptr<MeshPass>mesh_pass;
        std::shared_ptr<UIPass>ui_pass;
        std::vector<std::shared_ptr<Model>>models;

        uint32_t frameNums;

        std::vector<vk::Semaphore> imageAvaliables_;
        std::vector<vk::Semaphore> imageDrawFinshs_;
        std::vector<vk::Semaphore> uiDrawFinshs_;
        std::vector<vk::Fence> cmdAvaliableFences_;

        std::vector<vk::CommandBuffer> cmdBuffers_;

        std::unique_ptr<Buffer> hostUniformBuffer_;
        std::unique_ptr<Buffer> deviceUniformBuffer_;


        std::vector<vk::DescriptorSet> vpSets_;

        struct VP {
            glm::mat4 view = glm::mat4(1.0f);
            glm::mat4 project = glm::mat4(1.0f);
        } vp;
        float fov_ = 90;

        void createSemaphores();

        void createFences();

        void loadModels();

    };
}