#pragma once
#include<iostream>
#include"texture.hpp"
#include"schoo/vertexData.hpp"
#include"schoo/buffer.hpp"


#include<unordered_map>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/hash.hpp>

namespace std{
    template<>
    struct std::hash<schoo::Vertex> {
        size_t operator()(schoo::Vertex const &vertex) const {
            return ((std::hash<glm::vec3>()(vertex.pos) ^
                     (std::hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (std::hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
namespace schoo {

    class Model {
    public:
        Model(const std::string &model_path, const std::string &texture_path,glm::vec3 Position);
        ~Model();

        glm::vec3 position={0,0,0};

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::shared_ptr<Texture> texture;

        std::shared_ptr<Buffer>indexBuffer;
        std::shared_ptr<Buffer>vertexBuffer;
        std::shared_ptr<Buffer>modelMatBuffer;

        std::vector<vk::DescriptorSet>sets;

    private:
        std::unique_ptr<Buffer>hostModelBuffer_;


        void loadObj(const std::string &model_path);

        void createVertexBuffer();
        void createModelMatBuffer();
        void createIndexBuffer();
//        void loadIndexData();
//        void loadVertexData();
//        void loadModelMatData();
        void loadDataHostToDevice(const std::shared_ptr<Buffer>&hostBuffer,
                                  const std::shared_ptr<Buffer>&DeviceBuffer,
                                  const void*src);

    };
}