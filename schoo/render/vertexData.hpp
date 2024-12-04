#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <array>
#include"vulkan/vulkan.hpp"

namespace schoo {

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        bool operator==(const Vertex&other)const{
            return pos==other.pos&&color==other.color&&texCoord==other.texCoord;
        }

        static vk::VertexInputBindingDescription getBindingDescription() {
            vk::VertexInputBindingDescription bindingDescription;
            bindingDescription.setBinding(0)
                    .setInputRate(vk::VertexInputRate::eVertex)
                    .setStride(sizeof(Vertex));
            return bindingDescription;
        }

        static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
            std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};

            attributeDescriptions[0].setFormat(vk::Format::eR32G32B32Sfloat)
                    .setBinding(0)
                    .setLocation(0)
                    .setOffset(offsetof(Vertex, pos));

            attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat)
                    .setBinding(0)
                    .setLocation(1)
                    .setOffset(offsetof(Vertex, color));

            attributeDescriptions[2].setFormat(vk::Format::eR32G32Sfloat)
            .setBinding(0)
            .setLocation(2)
            .setOffset(offsetof(Vertex,texCoord));
            return attributeDescriptions;
        }
    };
}


