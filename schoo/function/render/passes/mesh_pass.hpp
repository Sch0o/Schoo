#pragma once

#include "schoo/function/render/vulkan/renderPass.hpp"
#include"schoo/resource/objModel.hpp"
#include"schoo/function/render/light.hpp"

namespace schoo {
    class MeshPass : public RenderPass {
    public:
        vk::DescriptorPool descriptorPool;

        vk::DescriptorSetLayout globalSetLayout;
        vk::DescriptorSetLayout pre_drawcall_setLayout;

        std::vector<vk::DescriptorSet> globalSets;

        RenderPipeline renderPipeline;
        std::vector<vk::Framebuffer> frameBuffers;
        vk::RenderPass renderPass;


        struct UniformConstantsData {
            glm::mat4 view;
            glm::mat4 projection;
            glm::mat4 lightSpace;
            glm::vec4 lightPos;
            glm::vec4 lightColor;
            glm::vec4 viewPos;
        } uniformConstants;

        UniformBuffer constant;

        std::vector<vk::CommandBuffer> cmdBuffers;

        float fov = 75;

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

        void UpdateConstants();

        void initUniform();
    };
}