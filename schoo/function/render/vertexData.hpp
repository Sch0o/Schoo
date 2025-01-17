#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <array>
#include"vulkan/vulkan.hpp"

namespace schoo {

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 texCoord;


        bool operator==(const Vertex &other) const {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }

        static vk::VertexInputBindingDescription getBindingDescription() {
            vk::VertexInputBindingDescription bindingDescription;
            bindingDescription.setBinding(0)
                    .setInputRate(vk::VertexInputRate::eVertex)
                    .setStride(sizeof(Vertex));
            return bindingDescription;
        }

        static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
            std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions{};

            attributeDescriptions[0].setFormat(vk::Format::eR32G32B32Sfloat)
                    .setBinding(0)
                    .setLocation(0)
                    .setOffset(offsetof(Vertex, pos));

            attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat)
                    .setBinding(0)
                    .setLocation(1)
                    .setOffset(offsetof(Vertex, color));

            attributeDescriptions[2].setFormat(vk::Format::eR32G32B32Sfloat)
                    .setBinding(0)
                    .setLocation(2)
                    .setOffset(offsetof(Vertex, normal));

            attributeDescriptions[3].setFormat(vk::Format::eR32G32Sfloat)
                    .setBinding(0)
                    .setLocation(3)
                    .setOffset(offsetof(Vertex, texCoord));
            return attributeDescriptions;
        }
    };

    static std::vector<Vertex> quadVertices = {
            {{-1.0f, 1.0f, 0.f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},  // Top-left
            {{1.0f, 1.0f, 0.f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},  // Top-right
            {{1.0f, -1.0f, 0.f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},  // Bottom-right
            {{-1.0f, -1.0f, 0.f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}   // Bottom-left
    };
    static std::vector<uint32_t> quadIndices = {
            0, 1, 2,
            2, 3, 0
    };

}


