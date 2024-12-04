#pragma once

#include "render/renderPass.hpp"
#include"render/model.hpp"
#include"render/buffer.hpp"

namespace schoo {
    class MeshPass : public RenderPass {
    public:
        vk::DescriptorPool descriptorPool;

        vk::DescriptorSetLayout vpSetLayout;
        vk::DescriptorSetLayout modelSetLayout;

        std::vector<vk::DescriptorSet> vpSets;

        RenderPipeline renderPipeline;
        std::vector<vk::Framebuffer> frameBuffers;
        vk::RenderPass renderPass;

        std::vector<std::shared_ptr<Model>> renderResource;

        std::shared_ptr<Buffer> hostUniformBuffer;
        std::shared_ptr<Buffer> deviceUniformBuffer;

        std::vector<vk::CommandBuffer> cmdBuffers;


        struct VP {
            glm::mat4 view = glm::mat4(1.0f);
            glm::mat4 project = glm::mat4(1.0f);
        } vp;

        float fov = 90;

        vk::Sampler sampler;


        MeshPass(std::vector<std::shared_ptr<Model>> &);

        ~MeshPass();

        void createRenderPass();

        void draw() override;

        void createRenderPipeline();

        void createSetLayout();

        void createDescriptorPool();

        void allocateSets();

        void updateSets();

        void createUniformBuffer();

        void createFrameBuffers();

        void loadUniformData();

        void createSampler();

        void createCmdBuffer();

        void UpdateViewMatrix();

        void initVP();
    };
}