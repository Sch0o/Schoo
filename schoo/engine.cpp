//
// Created by schoo on 2024/10/13.
//
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION

#include "schoo/engine.hpp"
#include"schoo/function/render/vulkan/context.hpp"
#include "schoo/function/render/vulkan/shader.hpp"
#include "resource/asset_manager.hpp"




namespace schoo {

    std::unique_ptr<SchooEngine>SchooEngine::instance_= nullptr;

    SchooEngine& SchooEngine::GetInstance() {
        return *instance_;
    }
    void SchooEngine::Init(){
        instance_.reset(new SchooEngine());

        instance_->InitCamera();
        instance_->InitRender();

    }
    void SchooEngine::TickMain() {
        auto&window=Window::GetInstance();
        while(!window.ShouldClose()){
            window.PollEvents();
            window.SwapBuffer();
            TickLogic();
            TickRender();
        }
    }
    void SchooEngine::InitRender(){
        Window::Init(width, height, "schoo");
        Context::Init(schoo::Window::GetInstance());
        auto &context = Context::GetInstance();
        AssetManager::Init();
        context.InitCommandManager();
        context.InitSwapchain(width, height);
        context.InitRenderer();
    }

    void SchooEngine::Quit() {
        auto &context = Context::GetInstance();
        context.device.waitIdle();
        context.DestroyRenderer();
        context.DestroyCommandManager();
        context.QuitSwapchain();
        AssetManager::Quit();
        Context::Quit();
        Window::Quit();

        instance_.reset();
    }

    void SchooEngine::TickLogic(){
        float currentTime=glfwGetTime();
        deltaTime=currentTime-lastTime;
        lastTime=currentTime;
    }
    void SchooEngine::TickRender() {
        Context::GetInstance().renderer->Render();
    }
    void SchooEngine::InitCamera() {
        camera.reset(new Camera(glm::vec3(1,1,2),0,-90,glm::vec3(0,1,0)));
    }

    SchooEngine::~SchooEngine() {

    }
}
