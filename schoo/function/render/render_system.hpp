#pragma once

#include"vulkan/vulkan.hpp"
#include"schoo/function/render/vulkan/buffer.hpp"
#include"glm/glm.hpp"
#include "vertexData.hpp"
#include"texture.hpp"
#include"schoo/resource/objModel.hpp"
#include"schoo/function/render/passes/mesh_pass.hpp"
#include"schoo/function/render/passes/ui_pass.hpp"
#include"schoo/function/render/passes/shadowMap_pass.hpp"
#include"schoo/function/render/passes/fxaa_pass.hpp"
#include"light.hpp"
#include"schoo/function/render/vulkan/shader.hpp"


namespace schoo {
    class Renderer final {
    public:
        Renderer(uint32_t frameNums = 2);

        ~Renderer();

        void Render();
        void InitPasses();

        uint32_t currentFrame = 0;
        uint32_t imageIndex=0;


        struct Lightlist {
            PointLight plight;
        }lights;

        struct Shaders{
            std::shared_ptr<Shader> shadow_map_shader;
            std::shared_ptr<Shader> render_shader;
            std::shared_ptr<Shader> fxaa_shader;
        }shaders;

        struct Passes{
            std::shared_ptr<ShadowMapPass>shadow_map_pass;
            std::shared_ptr<MeshPass>mesh_pass;
            std::shared_ptr<FxaaPass>fxaa_pass;
            std::shared_ptr<UIPass>ui_pass;
            void destoryPasses();
        }passes;


    private:

        uint32_t frameNums;

        std::vector<vk::Semaphore> fxaaAvaliables_;
        std::vector<vk::Semaphore> shadowMapAvaliables_;
        std::vector<vk::Semaphore> imageAvaliables_;
        std::vector<vk::Semaphore> imageDrawFinshs_;
        std::vector<vk::Semaphore> uiDrawFinshs_;
        std::vector<vk::Fence> cmdAvaliableFences_;

        void createSemaphores();

        void createFences();

        void loadScene();


    };
}