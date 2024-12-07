#include"schoo/render/model.hpp"
#include"schoo/render/context.hpp"
#include"schoo/render/utils.hpp"
#define  TINYOBJLOADER_IMPLEMENTATION
#include"tinyobjloader/tiny_obj_loader.h"
namespace schoo {
    Model::Model(const std::string &model_path, const std::string &texture_path,glm::vec3 position) {
        CheckPath(model_path);
        CheckPath(texture_path);

        texture.reset(new Texture(texture_path));

        loadObj(model_path);

        createVertexBuffer();
        //loadVertexData();

        createIndexBuffer();
        //loadIndexData();
        createModelMatBuffer();

        this->position=position;
    }
    Model::~Model(){
        auto&device=Context::GetInstance().device;

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

                vertex.normal={
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                };

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
        std::shared_ptr<Buffer>hostVertexBuffer;
        hostVertexBuffer.reset(new Buffer(bufferSize,
                                           vk::BufferUsageFlagBits::eTransferSrc,
                                           vk::MemoryPropertyFlagBits::eHostCoherent |
                                           vk::MemoryPropertyFlagBits::eHostVisible));

        vertexBuffer.reset(new Buffer(bufferSize,
                                      vk::BufferUsageFlagBits::eVertexBuffer |
                                      vk::BufferUsageFlagBits::eTransferDst,
                                      vk::MemoryPropertyFlagBits::eDeviceLocal));
        loadDataHostToDevice(hostVertexBuffer,vertexBuffer,vertices.data());
    }

    void Model::createIndexBuffer() {
        size_t bufferSize = sizeof(indices[0]) * indices.size();
        std::shared_ptr<Buffer>hostIndexBuffer;
        hostIndexBuffer.reset(new Buffer(bufferSize,
                                          vk::BufferUsageFlagBits::eTransferSrc,
                                          vk::MemoryPropertyFlagBits::eHostCoherent |
                                          vk::MemoryPropertyFlagBits::eHostVisible));

        indexBuffer.reset(new Buffer(bufferSize,
                                     vk::BufferUsageFlagBits::eIndexBuffer |
                                     vk::BufferUsageFlagBits::eTransferDst,
                                     vk::MemoryPropertyFlagBits::eDeviceLocal));

        loadDataHostToDevice(hostIndexBuffer,indexBuffer,indices.data());
    }

    void Model::
    createModelMatBuffer() {
        glm::mat4 modelMat=glm::mat4(1.0f);
        modelMat=glm::translate(modelMat,position);

        vk::DeviceSize bufferSize=sizeof(modelMat);
        std::shared_ptr<Buffer>stagingBuffer;
        stagingBuffer.reset(new Buffer(bufferSize,
                                          vk::BufferUsageFlagBits::eTransferSrc,
                                          vk::MemoryPropertyFlagBits::eHostCoherent |
                                          vk::MemoryPropertyFlagBits::eHostVisible));

        modelMatBuffer.reset(new Buffer(bufferSize,
                                      vk::BufferUsageFlagBits::eUniformBuffer |
                                      vk::BufferUsageFlagBits::eTransferDst,
                                      vk::MemoryPropertyFlagBits::eDeviceLocal));
        loadDataHostToDevice(stagingBuffer,modelMatBuffer,&modelMat);
    }


    void Model::loadDataHostToDevice(const std::shared_ptr<Buffer> &hostBuffer, const std::shared_ptr<Buffer> &DeviceBuffer,
                                const void *src) {
        void *data = Context::GetInstance().device.mapMemory(hostBuffer->memory, 0, hostBuffer->size);
        memcpy(data, src, hostBuffer->size);
        Context::GetInstance().device.unmapMemory(hostBuffer->memory);

        copyBuffer(hostBuffer->buffer, DeviceBuffer->buffer, hostBuffer->size, 0, 0);

    }


}