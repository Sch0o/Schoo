#include"schoo/render/shader.hpp"
#include"schoo/render/context.hpp"

namespace schoo {
    Shader::Shader(const std::string &vertexSource, const std::string &fragSource) {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.setCodeSize(vertexSource.size())
                .setPCode((uint32_t *) vertexSource.data());

        vertexModule = Context::GetInstance().device.createShaderModule(createInfo);

        createInfo.setCodeSize(fragSource.size())
                .setPCode((uint32_t *) fragSource.data());

        fragmentModule = Context::GetInstance().device.createShaderModule(createInfo);

        InitShaderStage();
    }

    Shader::~Shader(){
        auto& device=Context::GetInstance().device;
        device.destroyShaderModule(vertexModule);
        device.destroyShaderModule(fragmentModule);
    }

    std::vector<vk::PipelineShaderStageCreateInfo> Shader::getStages() {
        return stages_;
    }

    void Shader::InitShaderStage() {
        stages_.resize(2);

        stages_[0].setStage(vk::ShaderStageFlagBits::eVertex)
        .setModule(vertexModule).setPName("main");

        stages_[1].setStage(vk::ShaderStageFlagBits::eFragment)
        .setModule(fragmentModule).setPName("main");

    }
}
