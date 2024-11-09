//
// Created by schoo on 2024/10/13.
//
#include "schoo/schoo.hpp"
#include "schoo/shader.hpp"
#include "schoo/utils.hpp"

const uint32_t width = 1600;
const uint32_t height = 1200;

namespace schoo {
    void Init() {
        Window::Init(width, height, "schoo");
        Context::Init(schoo::Window::GetInstance());
        auto &context = Context::GetInstance();
        context.InitSwapchain(width, height);
        Shader::Init(ReadWholeFile(R"(..\..\shader\vert.spv)"),
                     ReadWholeFile(R"(..\..\shader\frag.spv)"));
        //context.renderProcess->CreateLayout();
        context.InitRenderProcess();
        context.swapchain->CreateFramebuffers();
        context.renderProcess->CreatePipeline();
        context.InitCommandManager();
        context.InitRenderer();
    }

    void Quit() {
        auto &context = Context::GetInstance();
        context.device.waitIdle();
        context.DestroyRenderer();
        context.DestroyCommandManager();
        context.DestroyRenderProcess();
        Shader::Quit();
        context.QuitSwapchain();
        Context::Quit();
        Window::Quit();
    }

    void Render() {
        Context::GetInstance().renderer->Render();
    }
}
