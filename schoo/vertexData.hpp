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

    const std::vector<Vertex> rect_vertices = {
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f},{1.0f,0.0f}}, // 0
            {{0.5f,  -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f},{0.0f,0.0f}},  // 1
            {{0.5f,  0.5f,  0.0f}, {1.0f, 1.0f, 1.0f},{0.0f,1.0f}},  // 2
            {{-0.5f, 0.5f,  0.0f}, {1.0f, 1.0f, 1.0f},{1.0f,1.0f}}, // 3
    };

    const std::vector<Vertex> cube_vertices = {
            {{-0.5f, -0.5f, 0.5f},  {1.0f, 1.0f, 1.0f},{1.0f,0.0f}}, // 0
            {{0.5f,  -0.5f, 0.5f},  {1.0f, 1.0f, 1.0f},{0.0f,0.0f}},  // 1
            {{0.5f,  0.5f,  0.5f},  {1.0f, 1.0f, 1.0f},{0.0f,1.0f}},  // 2
            {{-0.5f, 0.5f,  0.5f},  {1.0f, 1.0f, 1.0f},{1.0f,1.0f}}, // 3
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f},{1.0f,1.0f}}, // 4
            {{0.5f,  -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f},{0.0f,1.0f}},  // 5
            {{0.5f,  0.5f,  -0.5f}, {0.0f, 0.0f, 0.0f},{0.0f,1.0f}},  // 6
            {{-0.5f, 0.5f,  -0.5f}, {1.0f, 1.0f, 1.0f},{1.0f,1.0f}}  // 7
    };

    const std::vector<uint32_t> rect_indices = {
            0, 1, 2, 0, 2, 3
    };
    const std::vector<uint32_t> cube_indices = {
            // 正面
            0, 1, 2, 2, 3, 0,
            // 背面
            6, 5, 4, 4, 7, 6,
            // 左面
            4, 0, 3, 4, 3, 7,
            // 右面
            1, 5, 6, 1, 6, 2,
            // 顶面
            4, 5, 1, 1, 0, 4,
            // 底面
            3, 2, 7, 2, 6, 7
    };


}


