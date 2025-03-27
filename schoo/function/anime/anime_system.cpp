#include <iostream>
#include"anime_system.hpp"
#include"../../resource/asset_manager.hpp"

namespace schoo {
    void AnimationSystem::updateAnimation(float deltaTime) {
        auto &model = AssetManager::Instance().glTFModel;
        auto &animations = model.animations;
        if (activeAnimation > animations.size() - 1) {
            std::cout << "No animation with index " << activeAnimation << std::endl;
            return;
        }
        auto &animation = animations[activeAnimation];
        animation.currentTime += deltaTime;
        if (animation.currentTime > animation.end) {
            animation.currentTime -= animation.end;
        }

        for (const auto& channel: animation.channels) {
            auto &sampler = animation.samplers[channel.samplerIndex];
            if (sampler.interpolation != "LINEAR") {
                std::cout << "only support linear";
            }

            for (int i = 0; i < sampler.inputs.size() - 1; i++) {
                if ((animation.currentTime >= sampler.inputs[i]) && (animation.currentTime <= sampler.inputs[i + 1])) {
                    std::cout << sampler.interpolation << std::endl;
                    float interpolation = (animation.currentTime - sampler.inputs[i]) /
                                          (sampler.inputs[i + 1] - animation.currentTime);
                    const auto &output1 = sampler.outputsVec4[i];
                    const auto &output2 = sampler.outputsVec4[i + 1];
                    if (channel.targetProperty == "translation") {
                        channel.node->translation = glm::mix(output1, output2, interpolation);
                    } else if (channel.targetProperty == "rotation") {
                        glm::quat quat1(output1.w, output1.x, output1.y, output1.z);
                        glm::quat quat2(output2.w, output2.x, output2.y, output2.z);
                        channel.node->rotation = glm::normalize(glm::slerp(quat1, quat2, interpolation));
                    } else if (channel.targetProperty == "scale") {
                        channel.node->scale = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1],
                                                       interpolation);
                    }
                }
            }

        }
        for(auto&node:model.nodes){
            model.updateJoints(node);
        }
    }

    void AnimationSystem::Init() {

    }

    void AnimationSystem::Quit() {

    }
}