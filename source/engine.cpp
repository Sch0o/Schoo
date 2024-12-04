//
// Created by schoo on 2024/10/13.
//
#include"schoo/render/context.hpp"
#include"schoo/render/window.hpp"
#include "schoo/engine.hpp"
#include "schoo/render/shader.hpp"
#include "schoo/render/utils.hpp"



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
        context.InitCommandManager();
        context.InitSwapchain(width, height);
        Shader::Init(ReadWholeFile(R"(..\..\shader\vert.spv)"),
                     ReadWholeFile(R"(..\..\shader\frag.spv)"));
        context.InitRenderer();
    }

    void SchooEngine::Quit() {
        auto &context = Context::GetInstance();
        context.device.waitIdle();
        context.DestroyRenderer();
        context.DestroyCommandManager();
        Shader::Quit();
        context.QuitSwapchain();
        Context::Quit();
        Window::Quit();

        instance_.reset();
    }

    void SchooEngine::TickLogic(){
        float currentTime=glfwGetTime();
        deltaTime=currentTime-lastTime;
        lastTime=currentTime;

        //Context::GetInstance().renderer->UpdateViewMatrix();
    }
    void SchooEngine::TickRender() {
        Context::GetInstance().renderer->Render();
    }
    void SchooEngine::InitCamera() {
        camera.reset(new Camera(glm::vec3(0,3,5),0,-90,glm::vec3(0,1,0)));
    }

    SchooEngine::~SchooEngine() {

    }
}
