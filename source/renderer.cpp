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
        loadModels();
        mesh_pass.reset(new MeshPass(models));
        ui_pass.reset(new UIPass());
    }

    Renderer::~Renderer() {
        mesh_pass.reset();
        ui_pass.reset();

        auto &commandManager = Context::GetInstance().commandManager;
        auto &device = Context::GetInstance().device;

        for (int i = 0; i < frameNums; i++) {
            device.destroySemaphore(imageAvaliables_[i]);
            device.destroySemaphore(imageDrawFinshs_[i]);
            device.destroySemaphore(uiDrawFinshs_[i]);
            device.destroyFence(cmdAvaliableFences_[i]);
        }

    }

    void Renderer::Render() {
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

        mesh_pass->draw();
        ui_pass->draw();

        vk::SubmitInfo submitInfo;
        std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submitInfo.setCommandBuffers(mesh_pass->cmdBuffers[currentFrame])
                .setWaitSemaphores(imageAvaliables_[currentFrame])
                .setSignalSemaphores(imageDrawFinshs_[currentFrame])
                .setWaitDstStageMask(waitStages);
        Context::GetInstance().graphicsQueue.submit(submitInfo);

        submitInfo.setCommandBuffers(ui_pass->cmdBuffers[currentFrame])
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
        imageAvaliables_.resize(frameNums);
        imageDrawFinshs_.resize(frameNums);
        uiDrawFinshs_.resize(frameNums);

        vk::SemaphoreCreateInfo createInfo;
        for (int i = 0; i < frameNums; i++) {
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

    void Renderer::loadModels() {
        models.resize(2);
        models[0].reset(new Model(R"(..\..\assets\models\marry\Marry.obj)",
                                  R"(..\..\assets\models\marry\MC003_Kozakura_Mari.png)",
                                  glm::vec3(0, 0.5f, 0)));
        models[1].reset(new Model(R"(..\..\assets\models\floor\floor.obj)",
                                  R"(..\..\assets\textures\2x2white.png)",
                                  glm::vec3(0, 0, 0)));
    }

}
