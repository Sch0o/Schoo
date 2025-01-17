#pragma once

#include "schoo/function/render/vulkan/renderPass.hpp"
#include"schoo/resource/objModel.hpp"
#include"schoo/function/render/light.hpp"

namespace schoo {
    class MeshPass : public RenderPass {
    public:

        vk::DescriptorSetLayout globalSetLayout;
        vk::DescriptorSetLayout pre_drawcall_setLayout;

        std::vector<vk::DescriptorSet> globalSets;

        std::vector<FrameBuffer> frameBuffers;


        struct UniformConstantsData {
            glm::mat4 view;
            glm::mat4 projection;
            glm::mat4 lightSpace;
            glm::vec4 lightPos;
            glm::vec4 lightColor;
            glm::vec4 viewPos;
        } uniformConstants;

        UniformBuffer constant;

        float fov = 75;


        MeshPass();

        ~MeshPass();

        void init();

        void createRenderPass();

        void draw() override;

        void createRenderPipeline();

        void setupDescriptors();

        void createUniformBuffers();

        void createFrameBuffers();

        void createSampler();

        void UpdateConstants();

        void initUniform();
    };
}