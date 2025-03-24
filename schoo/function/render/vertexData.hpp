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
        glm::vec4 jointIndices;
        glm::vec4 jointWeights;


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

        static vk::VertexInputAttributeDescription createAttribute(uint32_t location, vk::Format format, size_t offset){
            return vk::VertexInputAttributeDescription{}
            .setBinding(0)
            .setLocation(location)
            .setFormat(format)
            .setOffset(static_cast<uint32_t>(offset));
        }
        //attention the array size is static,so when add a new property,need change the array size
        static auto getAttributeDescriptions() {
            return std::array{
                createAttribute(0,vk::Format::eR32G32B32Sfloat, offsetof(Vertex,pos)),
                createAttribute(1,vk::Format::eR32G32B32Sfloat, offsetof(Vertex,color)),
                createAttribute(2,vk::Format::eR32G32B32Sfloat, offsetof(Vertex,normal)),
                createAttribute(3,vk::Format::eR32G32Sfloat, offsetof(Vertex,texCoord)),
                createAttribute(4,vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex,jointIndices)),
                createAttribute(5,vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex,jointWeights))
                };
        }
    };

    static std::vector<Vertex> quadVertices = {
            {{-1.0f, 1.0f,  0.f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},  // Top-left
            {{1.0f,  1.0f,  0.f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},  // Top-right
            {{1.0f,  -1.0f, 0.f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},  // Bottom-right
            {{-1.0f, -1.0f, 0.f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}   // Bottom-left
    };
    static std::vector<uint32_t> quadIndices = {
            0, 1, 2,
            2, 3, 0
    };

}


