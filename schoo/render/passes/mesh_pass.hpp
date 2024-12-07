#pragma once

#include "render/renderPass.hpp"
#include"render/model.hpp"
#include"render/buffer.hpp"
#include"render/light.hpp"

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

        struct UniformBuffer{
            std::shared_ptr<Buffer>stagingBuffer;
            std::shared_ptr<Buffer>deviceBuffer;
        };
        struct UniformDataoffscreen{
            glm::mat4 depthMVP;
        };
        struct UniformConstants {
            glm::mat4 view;
            glm::mat4 projection;
            glm::vec4 lightPos;
            glm::vec4 lightColor;
            glm::vec4 viewPos;
        }uniformConstants;

        UniformBuffer constant;

        std::shared_ptr<Buffer> hostUniformBuffer;
        std::shared_ptr<Buffer> deviceUniformBuffer;


        std::vector<vk::CommandBuffer> cmdBuffers;

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

        void createUniformBuffers();

        void createFrameBuffers();

        void loadUniformData();

        void createSampler();

        void createCmdBuffer();

        void UpdateViewMatrix();

        void initUniform();
    };
}