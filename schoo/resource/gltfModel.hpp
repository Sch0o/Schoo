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
            int32_t skin = -1;     //store the index of th skin
            glm::mat4 matrix;

            glm::mat4 getLocalMatrix();

            ~Node() {
                for (auto &child: children) {
                    delete child;
                }
            }
        };

        struct Skin {
            std::string name;
            Node *skeletonRoot{nullptr};
            std::vector<glm::mat4> ibm; //inverseBindMatrices;
            std::vector<Node *> joints;
            std::shared_ptr<Buffer> jointMatricesBuffer;
            vk::DescriptorSet descriptorSet;
        };

        struct AnimationSampler {
            std::string interpolation;
            std::vector<float> inputs;    //key frame
            std::vector<glm::vec4> outputsVec4;
        };

        struct AnimationChannel {
            std::string targetProperty;//translation,rotation,scale,wights
            Node *node;
            uint32_t samplerIndex; //refer to one animationSampler
        };
        struct Animation {
            std::string name;
            std::vector<AnimationSampler> samplers;
            std::vector<AnimationChannel> channels;
            float start = std::numeric_limits<float>::max();
            float end = std::numeric_limits<float>::min();
            float currentTime = 0.0f;
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
        std::vector<Skin> skins;
        std::vector<Animation>animations;
        uint32_t activeAnimation;

        ~GLTFModel();

        GLTFModel() = default;

        void loadSkins(tinygltf::Model &input);

        void loadImages(tinygltf::Model &input);

        void loadTextures(tinygltf::Model &input);

        void loadMaterials(tinygltf::Model &input);

        void loadAnimations(tinygltf::Model &input);

        void loadNode(const tinygltf::Node &inputNode, const tinygltf::Model &input, Node *parent,
                      std::vector<uint32_t> &indices, std::vector<Vertex> &vertices);

        void draw(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, int passStage);

        void drawNode(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, Node *node, int passStage);

        void createBuffers(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

        void updateAnimation();

        Node*nodeFromIndex(uint32_t index);
        Node*findNode(Node*parent,uint32_t index);

        void Init(tinygltf::Model &input);
    };
}