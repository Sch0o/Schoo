#include"schoo/function/render/render_system.hpp"
#include"schoo/function/render/vulkan/context.hpp"
#include"schoo/function/render/vulkan/utils.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include"external/glm/ext/matrix_clip_space.hpp"

#include"schoo/engine.hpp"
#include "resource/asset_manager.hpp"

namespace schoo {

    Renderer::Renderer(uint32_t frameNums) {
        this->frameNums = frameNums;
        createSemaphores();
        createFences();
        loadScene();
    }

    Renderer::~Renderer() {
        passes.destoryPasses();
        shaders.render_shader.reset();
        shaders.shadow_map_shader.reset();

        auto &commandManager = Context::GetInstance().commandManager;
        auto &device = Context::GetInstance().device;

        for (int i = 0; i < frameNums; i++) {
            device.destroySemaphore(shadowMapAvaliables_[i]);
            device.destroySemaphore(imageAvaliables_[i]);
            device.destroySemaphore(imageDrawFinshs_[i]);
            device.destroySemaphore(uiDrawFinshs_[i]);
            device.destroySemaphore(fxaaAvaliables_[i]);
            device.destroyFence(cmdAvaliableFences_[i]);
        }

    }

    void Renderer::Render() {
        glm::vec3 lightPos(0.0f);
        lightPos.x = cos(glfwGetTime()) * 3;
        lightPos.y = 4;
        lightPos.z = sin(glfwGetTime()) * 3;
        lights.plight.position=lightPos;
        //render
        auto &device = Context::GetInstance().device;
        auto &swapchain = Context::GetInstance().swapchain;

        if (Context::GetInstance().device.waitForFences(cmdAvaliableFences_[currentFrame], true,
                                                        std::numeric_limits<uint64_t>::max()) !=
            vk::Result::eSuccess) {
            std::cout << "wait for fence failed" << std::endl;
        }
        device.resetFences(cmdAvaliableFences_[currentFrame]);

        auto result = device.acquireNextImageKHR(swapchain->swapchain,
                                                 std::numeric_limits<uint64_t>::max(),
                                                 imageAvaliables_[currentFrame]);
        if (result.result != vk::Result::eSuccess) {
            throw std::runtime_error("acquire next image failed");
        }
        imageIndex = result.value;

        passes.shadow_map_pass->draw();
        passes.mesh_pass->draw();
        passes.fxaa_pass->draw();
        passes.ui_pass->draw();


        vk::SubmitInfo submitInfo;


        std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eLateFragmentTests};
        submitInfo.setCommandBuffers(passes.shadow_map_pass->cmdBuffers[currentFrame])
                .setWaitSemaphores(imageAvaliables_[currentFrame])
                .setSignalSemaphores(shadowMapAvaliables_[currentFrame])
                .setWaitDstStageMask(waitStages);
        Context::GetInstance().graphicsQueue.submit(submitInfo);

        waitStages={vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submitInfo.setCommandBuffers(passes.mesh_pass->cmdBuffers[currentFrame])
                .setWaitSemaphores(shadowMapAvaliables_[currentFrame])
                .setSignalSemaphores(imageDrawFinshs_[currentFrame])
                .setWaitDstStageMask(waitStages);
        Context::GetInstance().graphicsQueue.submit(submitInfo);

        waitStages={vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submitInfo.setCommandBuffers(passes.fxaa_pass->cmdBuffers[currentFrame])
                .setWaitSemaphores(imageDrawFinshs_[currentFrame])
                .setSignalSemaphores(fxaaAvaliables_[currentFrame])
                .setWaitDstStageMask(waitStages);
        Context::GetInstance().graphicsQueue.submit(submitInfo);

        submitInfo.setCommandBuffers(passes.ui_pass->cmdBuffers[currentFrame])
                .setWaitSemaphores(fxaaAvaliables_[currentFrame])
                .setSignalSemaphores(uiDrawFinshs_[currentFrame])
                .setWaitDstStageMask(waitStages);
        Context::GetInstance().graphicsQueue.submit(submitInfo, cmdAvaliableFences_[currentFrame]);

        vk::PresentInfoKHR presentInfoKhr;
        presentInfoKhr.setImageIndices(imageIndex)
                .setSwapchains(swapchain->swapchain)
                .setWaitSemaphores(uiDrawFinshs_[currentFrame]);
        if (Context::GetInstance().presentQueue.presentKHR(presentInfoKhr) != vk::Result::eSuccess) {
            std::cout << "image present failed" << std::endl;
        }

        currentFrame = (currentFrame + 1) % frameNums;
    }

    void Renderer::createSemaphores() {
        shadowMapAvaliables_.resize(frameNums);
        imageAvaliables_.resize(frameNums);
        imageDrawFinshs_.resize(frameNums);
        uiDrawFinshs_.resize(frameNums);
        fxaaAvaliables_.resize(frameNums);

        vk::SemaphoreCreateInfo createInfo;
        for (int i = 0; i < frameNums; i++) {
            shadowMapAvaliables_[i]=Context::GetInstance().device.createSemaphore(createInfo);
            imageAvaliables_[i] = Context::GetInstance().device.createSemaphore(createInfo);
            imageDrawFinshs_[i] = Context::GetInstance().device.createSemaphore(createInfo);
            uiDrawFinshs_[i] = Context::GetInstance().device.createSemaphore(createInfo);
            fxaaAvaliables_[i] = Context::GetInstance().device.createSemaphore(createInfo);
        }
    }

    void Renderer::createFences() {
        cmdAvaliableFences_.resize(frameNums, nullptr);
        vk::FenceCreateInfo createInfo;
        createInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
        for (int i = 0; i < frameNums; i++) {
            cmdAvaliableFences_[i] = Context::GetInstance().device.createFence(createInfo);
        }
    }

    void Renderer::loadScene() {
        AssetManager::GetInstance().loadGLTFFile(R"(..\..\assets\models\nagant2\nagant2.gltf)");

        lights.plight={glm::vec3(0,4,2),glm::vec3(1.0,1.0,1.0)};
    }

    void Renderer::InitPasses() {
        shaders.shadow_map_shader.reset(new Shader(ReadWholeFile(R"(..\..\shader\spv\shadowmap_vert.spv)"),
                                                          ReadWholeFile(R"(..\..\shader\spv\shadowmap_frag.spv)")));
        shaders.render_shader.reset(new Shader(ReadWholeFile(R"(..\..\shader\spv\mesh_vert.spv)"),
                                     ReadWholeFile(R"(..\..\shader\spv\mesh_frag.spv)")));
        shaders.fxaa_shader.reset(new Shader(ReadWholeFile(R"(..\..\shader\spv\fxaa_vert.spv)"),
                                               ReadWholeFile(R"(..\..\shader\spv\fxaa_frag.spv)")));

        passes.shadow_map_pass.reset(new ShadowMapPass());
        passes.shadow_map_pass->init();

        passes.mesh_pass.reset(new MeshPass());
        passes.mesh_pass->init();

        passes.fxaa_pass.reset(new FxaaPass());
        passes.fxaa_pass->init();

        passes.ui_pass.reset(new UIPass());
    }

    void Renderer::Passes::destoryPasses() {
        shadow_map_pass.reset();
        mesh_pass.reset();
        fxaa_pass.reset();
        ui_pass.reset();
    }

}
