//
// Created by schoo on 2024/10/13.
//

#include"context.hpp"
#include"window.hpp"
#include "schoo/schoo.hpp"
#include "schoo/shader.hpp"
#include "schoo/utils.hpp"



namespace schoo {

    std::unique_ptr<Schoo>Schoo::instance_= nullptr;

    Schoo& Schoo::GetInstance() {
        return *instance_;
    }
    Schoo::Schoo() {
    }
    void Schoo::Init(){
        instance_.reset(new Schoo());
        instance_->InitCamera();
        instance_->InitRender();
    }
    void Schoo::TickMain() {
        auto&window=Window::GetInstance();
        while(!window.ShouldClose()){
            window.PollEvents();
            window.SwapBuffer();

            TickLogic();
            TickRender();
        }
    }
    void Schoo::InitRender(){
        Window::Init(width, height, "schoo");
        Context::Init(schoo::Window::GetInstance());
        auto &context = Context::GetInstance();
        context.InitCommandManager();
        context.InitSwapchain(width, height);
        Shader::Init(ReadWholeFile(R"(..\..\shader\vert.spv)"),
                     ReadWholeFile(R"(..\..\shader\frag.spv)"));
        context.InitRenderProcess();
        context.swapchain->CreateFramebuffers();
        context.renderProcess->CreatePipeline();
        context.InitRenderer();
    }

    void Schoo::Quit() {
        auto &context = Context::GetInstance();
        context.device.waitIdle();
        context.DestroyRenderer();
        context.DestroyCommandManager();
        context.DestroyRenderProcess();
        Shader::Quit();
        context.QuitSwapchain();
        Context::Quit();
        Window::Quit();

        instance_.reset();
    }

    void Schoo::TickLogic(){
        float currentTime=glfwGetTime();
        deltaTime=currentTime-lastTime;
        lastTime=currentTime;

        if(currentTime-lastSec>1){
            std::cout<<"FPS: "<<frameNums<<std::endl;
            lastSec=currentTime;
            frameNums=0;
        }
        else
            frameNums++;
    }
    void Schoo::TickRender() {
        //Context::GetInstance().renderer->setViewMatrix(camera.GetViewMatrix());
        Context::GetInstance().renderer->Render();
    }
    void Schoo::InitCamera() {
        camera.reset(new Camera(glm::vec3(0,0,3),0,0,glm::vec3(0,1,0)));
    }

    Schoo::~Schoo() {

    }
}
