#pragma once

#include "render/renderPass.hpp"
#include"render/model.hpp"
#include"render/light.hpp"

namespace schoo {
    class MeshPass : public RenderPass {
    public:
        vk::DescriptorPool descriptorPool;

        vk::DescriptorSetLayout globalSetLayout;
        vk::DescriptorSetLayout modelSetLayout;

        std::vector<vk::DescriptorSet> globalSets;

        RenderPipeline renderPipeline;
        std::vector<vk::Framebuffer> frameBuffers;
        vk::RenderPass renderPass;

        std::vector<std::shared_ptr<Model>> renderResource;



        struct UniformConstantsData {
            glm::mat4 view;
            glm::mat4 projection;
            glm::vec4 lightPos;
            glm::vec4 lightColor;
            glm::vec4 viewPos;

        }uniformConstants;

        UniformBuffer constant;

        std::vector<vk::CommandBuffer> cmdBuffers;

        float fov = 90;

        vk::Sampler sampler;

        MeshPass();

        ~MeshPass();

        void init();
        void createRenderPass();

        void draw() override;

        void createRenderPipeline();

        void setupDescriptors();

        void createUniformBuffers();

        void createFrameBuffers();

        void loadUniformData();

        void createSampler();

        void UpdateViewMatrix();

        void initUniform();
    };
}