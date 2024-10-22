#pragma once
#include<iostream>
#include"vulkan/vulkan.hpp"
#include"schoo/context.hpp"

namespace schoo {
    class Shader final {
    public:
        static void init(const std::string& vertexSource,const std::string& fragSource);
        static void quit();

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