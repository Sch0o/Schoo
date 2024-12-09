#pragma once

#include"vulkan/vulkan.hpp"
#include"schoo/render/buffer.hpp"
#include"glm/glm.hpp"
#include "schoo/render/vertexData.hpp"
#include"schoo/render/texture.hpp"
#include"schoo/render/model.hpp"
#include"schoo/render/passes/mesh_pass.hpp"
#include"schoo/render/passes/ui_pass.hpp"
#include"schoo/render/passes/shadowMap_pass.hpp"
#include"schoo/render/light.hpp"
#include"schoo/render/shader.hpp"


namespace schoo {
    class Renderer final {
    public:
        Renderer(uint32_t frameNums = 2);

        ~Renderer();

        void Render();
        void InitPasses();

        uint32_t currentFrame = 0;
        uint32_t imageIndex=0;

        struct RenderResource {
            std::vector<std::shared_ptr<Model>>models;
        }res;

        struct Lightlist {
            PointLight plight;
        }lights;

        struct Shaders{
            std::shared_ptr<Shader> shadow_map_shader;
            std::shared_ptr<Shader> render_shader;
        }shaders;
        struct Passes{
            std::shared_ptr<ShadowMapPass>shadow_map_pass;
            std::shared_ptr<MeshPass>mesh_pass;
            std::shared_ptr<UIPass>ui_pass;
            void destoryPasses();
        }passes;


    private:

        uint32_t frameNums;

        std::vector<vk::Semaphore> shadowMapAvaliables_;
        std::vector<vk::Semaphore> imageAvaliables_;
        std::vector<vk::Semaphore> imageDrawFinshs_;
        std::vector<vk::Semaphore> uiDrawFinshs_;
        std::vector<vk::Fence> cmdAvaliableFences_;

        std::vector<vk::CommandBuffer> cmdBuffers_;

        std::unique_ptr<Buffer> hostUniformBuffer_;
        std::unique_ptr<Buffer> deviceUniformBuffer_;


        std::vector<vk::DescriptorSet> vpSets_;

        void createSemaphores();

        void createFences();

        void loadScene();

        void submitCmd();


    };
}