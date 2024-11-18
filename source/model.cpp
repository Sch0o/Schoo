#include"model.hpp"
#include"context.hpp"
#include"utils.hpp"
#define  TINYOBJLOADER_IMPLEMENTATION
#include"tinyobjloader/tiny_obj_loader.h"
namespace schoo {
    Model::Model(const std::string &model_path, const std::string &texture_path) {
        CheckPath(model_path);
        CheckPath(texture_path);

        texture.reset(new Texture(texture_path));

        loadObj(model_path);

        createVertexBuffer();
        loadVertexData();

        createIndexBuffer();
        loadIndexData();
    }
    Model::~Model(){
        auto&device=Context::GetInstance().device;

        hostVertexBuffer_.reset();
        hostIndexBuffer_.reset();
        indexBuffer.reset();
        vertexBuffer.reset();
        
        texture.reset();
    }

    void Model::loadObj(const std::string &model_path) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err, warn;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path.c_str())) {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices;

        for (const auto &shape: shapes) {
            for (const auto &index: shape.mesh.indices) {
                Vertex vertex = {};
                vertex.pos = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                };
                vertex.texCoord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1-attrib.texcoords[2 * index.texcoord_index + 1],
                };
                vertex.color = {1.0f, 1.0f, 1.0f};

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }

        }
    }

    void Model::createVertexBuffer() {
        vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
        hostVertexBuffer_.reset(new Buffer(bufferSize,
                                           vk::BufferUsageFlagBits::eTransferSrc,
                                           vk::MemoryPropertyFlagBits::eHostCoherent |
                                           vk::MemoryPropertyFlagBits::eHostVisible));

        vertexBuffer.reset(new Buffer(bufferSize,
                                      vk::BufferUsageFlagBits::eVertexBuffer |
                                      vk::BufferUsageFlagBits::eTransferDst,
                                      vk::MemoryPropertyFlagBits::eDeviceLocal));

    }

    void Model::createIndexBuffer() {
        size_t bufferSize = sizeof(indices[0]) * indices.size();
        hostIndexBuffer_.reset(new Buffer(bufferSize,
                                          vk::BufferUsageFlagBits::eTransferSrc,
                                          vk::MemoryPropertyFlagBits::eHostCoherent |
                                          vk::MemoryPropertyFlagBits::eHostVisible));

        indexBuffer.reset(new Buffer(bufferSize,
                                     vk::BufferUsageFlagBits::eIndexBuffer |
                                     vk::BufferUsageFlagBits::eTransferDst,
                                     vk::MemoryPropertyFlagBits::eDeviceLocal));
    }

    void Model::loadVertexData() {
        void *data = Context::GetInstance().device.mapMemory(hostVertexBuffer_->memory, 0, hostVertexBuffer_->size);
        memcpy(data, vertices.data(), hostVertexBuffer_->size);
        Context::GetInstance().device.unmapMemory(hostVertexBuffer_->memory);

        copyBuffer(hostVertexBuffer_->buffer, vertexBuffer->buffer, hostVertexBuffer_->size, 0, 0);
    }

    void Model::loadIndexData() {
        void *data = Context::GetInstance().device.mapMemory(hostIndexBuffer_->memory, 0, hostIndexBuffer_->size);
        memcpy(data, indices.data(), hostIndexBuffer_->size);
        Context::GetInstance().device.unmapMemory(hostIndexBuffer_->memory);

        copyBuffer(hostIndexBuffer_->buffer, indexBuffer->buffer, hostIndexBuffer_->size, 0, 0);
    }
}