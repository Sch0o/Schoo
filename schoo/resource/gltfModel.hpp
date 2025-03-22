#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "function/render/texture.hpp"
#include "function/render/vertexData.hpp"
#include"function/render/vulkan/buffer.hpp"

#include "external/tinygltf/tiny_gltf.h"

namespace schoo {
    class GLTFModel {
    public:

        struct Primitive {
            uint32_t firstIndex;
            uint32_t indexCount;
            int32_t materialIndex;
        };
        struct Mesh {
            std::vector<Primitive> primitives;
        };

        struct Node {
            Node *parent;
            uint32_t index;
            std::vector<Node *> children;
            Mesh mesh;
            glm::vec3 translation{};
            glm::vec3 scale{1.0f};
            glm::quat rotation{};
            int32_t skin=-1;     //store the index of th skin
            glm::mat4 matrix;

            glm::mat4 getLocalMatrix();
            ~Node() {
                for (auto &child: children) {
                    delete child;
                }
            }
        };

        struct Material {
            glm::vec4 baseColorFactor = glm::vec4(1.0f);
            uint32_t baseColorTextureIndex;
        };
        struct Image {
            std::shared_ptr<Texture> texture;
            vk::DescriptorSet descriptorSet;
        };
        struct Texture2D {
            int32_t imageIndex;
        };

        std::shared_ptr<Buffer> vertexBuffer;
        std::shared_ptr<Buffer> indexBuffer;
        std::vector<Image> images;
        std::vector<Texture2D> textures;
        std::vector<Material> materials;
        std::vector<Node *> nodes;

        ~GLTFModel();

        GLTFModel() = default;

        void loadImages(tinygltf::Model &input);

        void loadTextures(tinygltf::Model &input);

        void loadMaterials(tinygltf::Model &input);

        void loadNode(const tinygltf::Node &inputNode, const tinygltf::Model &input, Node *parent,
                      std::vector<uint32_t> &indices, std::vector<Vertex> &vertices);

        void draw(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, int passStage);

        void drawNode(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, Node *node, int passStage);

        void createBuffers(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

        void Init(tinygltf::Model &input);
    };
}