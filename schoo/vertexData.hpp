#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <array>
#include"vulkan/vulkan.hpp"

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescription() {
        vk::VertexInputBindingDescription bindingDescription;
        bindingDescription.setBinding(0)
                .setInputRate(vk::VertexInputRate::eVertex)
                .setStride(sizeof(Vertex));
        return bindingDescription;
    }

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].setFormat(vk::Format::eR32G32Sfloat)
                .setBinding(0)
                .setLocation(0)
                .setOffset(offsetof(Vertex, pos));

        attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat)
                .setBinding(0)
                .setLocation(1)
                .setOffset(offsetof(Vertex, color));
        return attributeDescriptions;
    }
};

const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
        {{0.5f,  -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f,  0.5f},  {0.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
        {{0.5f,  0.5f},  {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f},  {0.0f, 0.0f, 0.0f}}
};

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};