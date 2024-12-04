#pragma once
#include<iostream>
#include"vulkan/vulkan.hpp"
#include"schoo/render/context.hpp"

namespace schoo {
    class Shader final {
    public:
        static void Init(const std::string& vertexSource,const std::string& fragSource);
        static void Quit();

        static Shader& GetInstance();

        vk::ShaderModule vertexModule;
        vk::ShaderModule fragmentModule;

        std::vector<vk::PipelineShaderStageCreateInfo> getStages();
        ~Shader();
    private:
        static std::unique_ptr<Shader>instance_;

        std::vector<vk::PipelineShaderStageCreateInfo>stages_;

        Shader(const std::string& vertexSource,const std::string& fragSource);

        void InitShaderStage();

    };
}