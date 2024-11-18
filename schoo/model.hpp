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
        Model(const std::string &model_path, const std::string &texture_path);
        ~Model();

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::shared_ptr<Texture> texture;

        std::shared_ptr<Buffer>indexBuffer;
        std::shared_ptr<Buffer>vertexBuffer;

    private:



        std::unique_ptr<Buffer> hostVertexBuffer_;
        std::unique_ptr<Buffer>hostIndexBuffer_;


        void loadObj(const std::string &model_path);

        void createVertexBuffer();
        void createIndexBuffer();
        void loadIndexData();
        void loadVertexData();

    };
}