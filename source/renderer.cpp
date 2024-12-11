#include"schoo/render/renderer.hpp"
#include"schoo/render/context.hpp"
#include"schoo/render/utils.hpp"
#include"schoo/engine.hpp"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include"external/glm/ext/matrix_clip_space.hpp"

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

        submitInfo.setCommandBuffers(passes.ui_pass->cmdBuffers[currentFrame])
                .setWaitSemaphores(imageDrawFinshs_[currentFrame])
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

        vk::SemaphoreCreateInfo createInfo;
        for (int i = 0; i < frameNums; i++) {
            shadowMapAvaliables_[i]=Context::GetInstance().device.createSemaphore(createInfo);
            imageAvaliables_[i] = Context::GetInstance().device.createSemaphore(createInfo);
            imageDrawFinshs_[i] = Context::GetInstance().device.createSemaphore(createInfo);
            uiDrawFinshs_[i] = Context::GetInstance().device.createSemaphore(createInfo);
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
        res.models.resize(2);
        res.models[0].reset(new Model(R"(..\..\assets\models\marry\Marry.obj)",
                                  R"(..\..\assets\models\marry\MC003_Kozakura_Mari.png)",
                                  glm::vec3(0, 0.0f, 0)));
        res.models[1].reset(new Model(R"(..\..\assets\models\floor\floor.obj)",
                                  R"(..\..\assets\textures\2x2white.png)",
                                  glm::vec3(0, 0, 0)));

        lights.plight={glm::vec3(0,5,3),glm::vec3(1.0,1.0,1.0)};
    }

    void Renderer::InitPasses() {
        shaders.shadow_map_shader.reset(new Shader(ReadWholeFile(R"(..\..\shader\shadow_map_vert.spv)"),
                                                          ReadWholeFile(R"(..\..\shader\shadow_map_frag.spv)")));
        shaders.render_shader.reset(new Shader(ReadWholeFile(R"(..\..\shader\render_vert.spv)"),
                                     ReadWholeFile(R"(..\..\shader\render_frag.spv)")));

        passes.shadow_map_pass.reset(new ShadowMapPass());
        passes.shadow_map_pass->init();

        passes.mesh_pass.reset(new MeshPass());
        passes.mesh_pass->init();
        passes.ui_pass.reset(new UIPass());
    }

    void Renderer::Passes::destoryPasses() {
        shadow_map_pass.reset();
        mesh_pass.reset();
        ui_pass.reset();
    }

}
