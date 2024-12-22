#pragma once
#include<iostream>
#include"vulkan/vulkan.hpp"


namespace schoo {
    class Shader final {
    public:
        Shader()=default;
        Shader(const std::string& vertexSource,const std::string& fragSource);
        vk::ShaderModule vertexModule;
        vk::ShaderModule fragmentModule;

        std::vector<vk::PipelineShaderStageCreateInfo> getStages();
        ~Shader();
    private:

        std::vector<vk::PipelineShaderStageCreateInfo>stages_;
        void InitShaderStage();

    };
}