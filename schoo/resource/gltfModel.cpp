#include"gltfModel.hpp"
#include"function/render/vulkan/utils.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>


namespace schoo {

    void GLTFModel::Init(tinygltf::Model &input) {
        std::vector<uint32_t> indices;
        std::vector<Vertex> vertices;

        loadImages(input);
        loadMaterials(input);
        loadTextures(input);
        loadSkins(input);
        loadAnimations(input);
        //updateAnimation();

        const tinygltf::Scene &scene = input.scenes[0];
        for (int i: scene.nodes) {
            const tinygltf::Node node = input.nodes[i];
            loadNode(node, input, nullptr, indices, vertices);
        }
        createBuffers(vertices, indices);

    }

    void GLTFModel::loadTextures(tinygltf::Model &input) {
        textures.resize(input.textures.size());
        for (size_t i = 0; i < input.textures.size(); i++) {
            textures[i].imageIndex = input.textures[i].source;
        }
    }

    void GLTFModel::loadNode(const tinygltf::Node &inputNode, const tinygltf::Model &input, Node *parent,
                             std::vector<uint32_t> &indices, std::vector<Vertex> &vertices) {
        Node *node = new Node{};
        node->matrix = glm::mat4(1.0f);
        node->parent = parent;

        // Get the local node matrix
        // It's either made up from translation, rotation, scale or a 4x4 matrix
        if (inputNode.translation.size() == 3) {
            node->matrix = glm::translate(node->matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
        }
        if (inputNode.rotation.size() == 4) {
            glm::quat q = glm::make_quat(inputNode.rotation.data());
            node->matrix *= glm::mat4(q);
        }
        if (inputNode.scale.size() == 3) {
            node->matrix = glm::scale(node->matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
        }
        if (inputNode.matrix.size() == 16) {
            node->matrix = glm::make_mat4x4(inputNode.matrix.data());
        }
        // Load node's children
        if (!inputNode.children.empty()) {
            for (int i: inputNode.children) {
                loadNode(input.nodes[i], input, node, indices, vertices);
            }
        }

        // If the node contains mesh data, we load vertices and indices from the buffers
        // In glTF this is done via accessors and buffer views
        if (inputNode.mesh > -1) {
            const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
            // Iterate through all primitives of this node's mesh
            for (size_t i = 0; i < mesh.primitives.size(); i++) {
                const tinygltf::Primitive &glTFPrimitive = mesh.primitives[i];
                auto firstIndex = static_cast<uint32_t>(indices.size());
                auto vertexStart = static_cast<uint32_t>(vertices.size());
                uint32_t indexCount = 0;
                // Vertices
                {
                    const float *positionBuffer = nullptr;
                    const float *normalsBuffer = nullptr;
                    const float *texCoordsBuffer = nullptr;
                    const uint16_t *jointIndicesBuffer = nullptr;
                    const float *jointWeightsBuffer = nullptr;
                    size_t vertexCount = 0;

                    // Get buffer data for vertex positions
                    if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
                        const tinygltf::Accessor &accessor = input.accessors[glTFPrimitive.attributes.find(
                                "POSITION")->second];
                        const tinygltf::BufferView &view = input.bufferViews[accessor.bufferView];
                        positionBuffer = reinterpret_cast<const float *>(&(input.buffers[view.buffer].data[
                                accessor.byteOffset + view.byteOffset]));
                        vertexCount = accessor.count;
                    }
                    // Get buffer data for vertex normals
                    if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
                        const tinygltf::Accessor &accessor = input.accessors[glTFPrimitive.attributes.find(
                                "NORMAL")->second];
                        const tinygltf::BufferView &view = input.bufferViews[accessor.bufferView];
                        normalsBuffer = reinterpret_cast<const float *>(&(input.buffers[view.buffer].data[
                                accessor.byteOffset + view.byteOffset]));
                    }
                    // Get buffer data for vertex texture coordinates
                    // glTF supports multiple sets, we only load the first one
                    if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
                        const tinygltf::Accessor &accessor = input.accessors[glTFPrimitive.attributes.find(
                                "TEXCOORD_0")->second];
                        const tinygltf::BufferView &view = input.bufferViews[accessor.bufferView];
                        texCoordsBuffer = reinterpret_cast<const float *>(&(input.buffers[view.buffer].data[
                                accessor.byteOffset + view.byteOffset]));
                    }
                    //Get vertex joint indices
                    if (glTFPrimitive.attributes.find("JOINTS_0") != glTFPrimitive.attributes.end()) {
                        const tinygltf::Accessor &accessor = input.accessors[
                                glTFPrimitive.attributes.find("JOINTS_0")->second];
                        const tinygltf::BufferView &view = input.bufferViews[accessor.bufferView];
                        jointIndicesBuffer = reinterpret_cast<const uint16_t *>(&(input.buffers[view.buffer].data[
                                accessor.byteOffset + view.byteOffset]));

                    }
                    //Get vertex joint weights
                    if (glTFPrimitive.attributes.find("WEIGHTS_0") != glTFPrimitive.attributes.end()) {
                        const tinygltf::Accessor &accessor = input.accessors[
                                glTFPrimitive.attributes.find("WEIGHTS_0")->second];
                        const tinygltf::BufferView &view = input.bufferViews[accessor.bufferView];
                        jointWeightsBuffer = reinterpret_cast<const float *>(&(input.buffers[view.buffer].data[
                                accessor.byteOffset + view.byteOffset]));

                    }

                    bool hasSkin = (jointIndicesBuffer && jointWeightsBuffer);

                    // Append data to model's vertex buffer
                    for (size_t v = 0; v < vertexCount; v++) {
                        Vertex vert{};
                        vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                        vert.normal = glm::normalize(
                                glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
                        vert.texCoord = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
                        vert.color = glm::vec3(1.0f);
                        vert.jointIndices = hasSkin ? glm::vec4(glm::make_vec4(&jointIndicesBuffer[v * 4])) : glm::vec4(
                                0.0f);
                        vert.jointWeights = hasSkin ? glm::vec4(glm::make_vec4(&jointWeightsBuffer[v * 4])) : glm::vec4(
                                0.0f);
                        vertices.push_back(vert);
                    }
                }
                // Indices
                {
                    const tinygltf::Accessor &accessor = input.accessors[glTFPrimitive.indices];
                    const tinygltf::BufferView &bufferView = input.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer &buffer = input.buffers[bufferView.buffer];

                    indexCount += static_cast<uint32_t>(accessor.count);

                    // glTF supports different component types of indices
                    switch (accessor.componentType) {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                            const uint32_t *buf = reinterpret_cast<const uint32_t *>(&buffer.data[accessor.byteOffset +
                                                                                                  bufferView.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++) {
                                indices.push_back(buf[index] + vertexStart);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                            const uint16_t *buf = reinterpret_cast<const uint16_t *>(&buffer.data[accessor.byteOffset +
                                                                                                  bufferView.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++) {
                                indices.push_back(buf[index] + vertexStart);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                            const uint8_t *buf = reinterpret_cast<const uint8_t *>(&buffer.data[accessor.byteOffset +
                                                                                                bufferView.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++) {
                                indices.push_back(buf[index] + vertexStart);
                            }
                            break;
                        }
                        default:
                            std::cerr << "Index component type " << accessor.componentType << " not supported!"
                                      << std::endl;
                            return;
                    }
                }
                Primitive primitive{};
                primitive.firstIndex = firstIndex;
                primitive.indexCount = indexCount;
                primitive.materialIndex = glTFPrimitive.material;
                node->mesh.primitives.push_back(primitive);
            }
        }

        if (parent) {
            parent->children.push_back(node);
        } else {
            nodes.push_back(node);
        }
    }

    void GLTFModel::loadImages(tinygltf::Model &input) {
        // Images can be stored inside the glTF (which is the case for the sample model), so instead of directly
        // loading them from disk, we fetch them from the glTF loader and upload the buffers
        images.resize(input.images.size());
        for (size_t i = 0; i < input.images.size(); i++) {
            tinygltf::Image &glTFImage = input.images[i];
            // Get the image data from the glTF loader
            unsigned char *buffer = nullptr;
            VkDeviceSize bufferSize = 0;
            bool deleteBuffer = false;
            // We convert RGB-only images to RGBA, as most devices don't support RGB-formats in Vulkan
            if (glTFImage.component == 3) {
                bufferSize = glTFImage.width * glTFImage.height * 4;
                buffer = new unsigned char[bufferSize];
                unsigned char *rgba = buffer;
                unsigned char *rgb = &glTFImage.image[0];
                for (size_t i = 0; i < glTFImage.width * glTFImage.height; ++i) {
                    memcpy(rgba, rgb, sizeof(unsigned char) * 3);
                    rgba += 4;
                    rgb += 3;
                }
                deleteBuffer = true;
            } else {
                buffer = &glTFImage.image[0];
                bufferSize = glTFImage.image.size();
            }
            // Load texture from image buffer
            images[i].texture.reset(new Texture(buffer, bufferSize, glTFImage.width, glTFImage.height));

            if (deleteBuffer) {
                delete[] buffer;
            }

        }
    }

    void GLTFModel::loadMaterials(tinygltf::Model &input) {
        materials.resize(input.materials.size());
        for (size_t i = 0; i < input.materials.size(); i++) {
            // We only read the most basic properties required for our sample
            tinygltf::Material glTFMaterial = input.materials[i];
            // Get the base color factor
            if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
                materials[i].baseColorFactor = glm::make_vec4(
                        glTFMaterial.values["baseColorFactor"].ColorFactor().data());
            }
            // Get base color texture index
            if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
                materials[i].baseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
            }
        }
    }

    void
    GLTFModel::drawNode(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, Node *node, int passStage) {
        if (!node->mesh.primitives.empty()) {
            // Pass the node's matrix via push constants
            // Traverse the node hierarchy to the top-most parent to get the final matrix of the current node
            glm::mat4 nodeMatrix = node->matrix;
            Node *currentParent = node->parent;
            while (currentParent) {
                nodeMatrix = currentParent->matrix * nodeMatrix;
                currentParent = currentParent->parent;
            }
            // Pass the final matrix to the vertex shader using push constants
            commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4),
                                        &nodeMatrix);
            for (Primitive &primitive: node->mesh.primitives) {
                if (primitive.indexCount > 0) {
                    // Get the texture index for this primitive
                    Texture2D texture = textures[materials[primitive.materialIndex].baseColorTextureIndex];
                    // Bind the descriptor for the current primitive's texture
                    if (passStage == 1)
                        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1, 1,
                                                         &images[texture.imageIndex].descriptorSet, 0, nullptr);
                    commandBuffer.drawIndexed(primitive.indexCount, 1, primitive.firstIndex, 0, 0);
                }
            }
        }
        for (auto &child: node->children) {
            drawNode(commandBuffer, pipelineLayout, child, passStage);
        }
    }

    void GLTFModel::draw(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, int passStage) {
        vk::DeviceSize offsets[1] = {0};
        commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer->buffer, offsets);
        commandBuffer.bindIndexBuffer(indexBuffer->buffer, 0, vk::IndexType::eUint32);
        for (auto &node: nodes) {
            drawNode(commandBuffer, pipelineLayout, node, passStage);
        }
    }

    GLTFModel::~GLTFModel() {
        for (auto node: nodes) {
            delete node;
        }
        for (auto &image: images) {
            image.texture.reset();
        }
        vertexBuffer.reset();
        indexBuffer.reset();
    }

    void GLTFModel::createBuffers(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices) {
        //vertexBuffer;
        vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
        auto vertexStaging = std::make_shared<Buffer>(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                                                      vk::MemoryPropertyFlagBits::eHostVisible |
                                                      vk::MemoryPropertyFlagBits::eHostCoherent);
        vertexBuffer = std::make_shared<Buffer>(bufferSize, vk::BufferUsageFlagBits::eTransferDst |
                                                            vk::BufferUsageFlagBits::eVertexBuffer,
                                                vk::MemoryPropertyFlagBits::eDeviceLocal);

        loadDataHostToDevice(vertexStaging, vertexBuffer, vertices.data());


        bufferSize = sizeof(indices[0]) * indices.size();
        auto indexStaging = std::make_shared<Buffer>(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                                                     vk::MemoryPropertyFlagBits::eHostVisible |
                                                     vk::MemoryPropertyFlagBits::eHostCoherent);
        indexBuffer = std::make_shared<Buffer>(bufferSize, vk::BufferUsageFlagBits::eTransferDst |
                                                           vk::BufferUsageFlagBits::eIndexBuffer,
                                               vk::MemoryPropertyFlagBits::eDeviceLocal);

        loadDataHostToDevice(indexStaging, indexBuffer, indices.data());
        vertexStaging.reset();
        indexStaging.reset();
    }

    //TODO 优化：不存储ibm,直接存入buffer中
    void GLTFModel::loadSkins(tinygltf::Model &input) {
        skins.resize(input.skins.size());
        //traversal every skin
        for (size_t i{0}; i < input.skins.size(); i++) {
            tinygltf::Skin gltfSkin = input.skins[i];
            if (gltfSkin.inverseBindMatrices > -1) {
                skins[i].name = gltfSkin.name;
                const tinygltf::Accessor &accessor = input.accessors[gltfSkin.inverseBindMatrices];
                const tinygltf::BufferView &bufferView = input.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer = input.buffers[bufferView.buffer];
                skins[i].ibm.resize(accessor.count); //ibm size

                vk::DeviceSize bufferSize = accessor.count * sizeof(glm::mat4);
                memcpy(skins[i].ibm.data(), &buffer.data[bufferView.byteOffset + accessor.byteOffset],
                       bufferSize);
                auto ibmStaging = std::make_shared<Buffer>(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                                                           vk::MemoryPropertyFlagBits::eHostVisible |
                                                           vk::MemoryPropertyFlagBits::eHostCoherent);

                skins[i].jointMatricesBuffer = std::make_shared<Buffer>(bufferSize,
                                                                        vk::BufferUsageFlagBits::eTransferDst
                                                                        | vk::BufferUsageFlagBits::eStorageBuffer,
                                                                        vk::MemoryPropertyFlagBits::eDeviceLocal);
                loadDataHostToDevice(ibmStaging, skins[i].jointMatricesBuffer, skins[i].ibm.data());
                ibmStaging.reset();
            }
        }
    }

    void GLTFModel::loadAnimations(tinygltf::Model &input) {
        animations.resize(input.animations.size());
        for (int i = 0; i < input.animations.size(); i++) {
            auto &gltfAnimation = input.animations[i];
            animations[i].name = gltfAnimation.name;
            animations[i].samplers.resize(gltfAnimation.samplers.size());

            for (size_t j = 0; j < gltfAnimation.samplers.size(); j++) {
                const auto &sampler = gltfAnimation.samplers[j];
                auto &dstSampler = animations[i].samplers[j];
                dstSampler.interpolation = sampler.interpolation;

                //input value --key frame
                {
                    const tinygltf::Accessor &accessor = input.accessors[sampler.input];
                    const tinygltf::BufferView &bufferView = input.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer &buffer = input.buffers[bufferView.buffer];
                    const auto *buf = reinterpret_cast<const float *>(&buffer.data[
                            bufferView.byteOffset + accessor.byteOffset]);

                    for (size_t index = 0; index < accessor.count; index++) {
                        dstSampler.inputs.push_back(buf[index]);
                    }
                    for (auto keyframe: dstSampler.inputs) {
                        if (keyframe > animations[i].end) {
                            animations[i].end = keyframe;
                        }
                        if (keyframe < animations[i].start) {
                            animations[i].start = keyframe;
                        }
                    }
                }

                //Read samplers  keyframe output translate/rotate/scale values
                const tinygltf::Accessor &accessor = input.accessors[sampler.output];
                const tinygltf::BufferView &bufferView = input.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer = input.buffers[bufferView.buffer];
                const void *dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
                switch (accessor.type) {
                    case TINYGLTF_TYPE_VEC3: {
                        const auto *buf = static_cast<const glm::vec3 * >(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++) {
                            dstSampler.outputsVec4.emplace_back(buf[index], 1.0f);
                        }
                        break;
                    }
                    case TINYGLTF_TYPE_VEC4: {
                        const auto *buf = static_cast<const glm::vec4 *>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++) {
                            dstSampler.outputsVec4.push_back(buf[index]);
                        }
                        break;
                    }
                    case TINYGLTF_TYPE_SCALAR:{
                        const auto *buf = static_cast<const float *>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++) {
                            dstSampler.outputsVec4.emplace_back(buf[index]);
                        }
                        break;
                    }
                    default: {
                        std::cout << "unknown type" << std::endl;
                        break;
                    }
                }
            }

            animations[i].channels.resize(gltfAnimation.channels.size());
            for (size_t j = 0; j < gltfAnimation.channels.size(); j++) {
                auto &gltfChannel = gltfAnimation.channels[j];
                auto &dstChannel = animations[i].channels[j];
                dstChannel.targetProperty = gltfChannel.target_path;
                dstChannel.samplerIndex = gltfChannel.sampler;
                dstChannel.node = nodeFromIndex(gltfChannel.target_node);
            }
        }
    }

    GLTFModel::Node *GLTFModel::findNode(schoo::GLTFModel::Node *parent, uint32_t index) {
        Node *nodeFound{nullptr};
        for (auto child: parent->children) {
            if (child->index == index)
                return child;
            nodeFound = findNode(child, index);
            if (nodeFound) break;
        }
        return nodeFound;
    }

    GLTFModel::Node *GLTFModel::nodeFromIndex(uint32_t index) {
        Node *nodeFound = nullptr;
        for (auto node: nodes) {
            if (node->index == index)
                return node;
            nodeFound = findNode(node, index);
            if (nodeFound) break;
        }
        return nodeFound;
    }

    void GLTFModel::updateAnimation() {
        Animation &animation = animations[activeAnimation];
        if (animation.currentTime > animation.end) {
            animation.currentTime -= animation.end;
        }
        for (auto channel: animation.channels) {
            auto &sampler = animation.samplers[channel.samplerIndex];
            for (int i = 0; i < sampler.inputs.size() - 1; i++) {
                if ((animation.currentTime >= sampler.inputs[i]) && (animation.currentTime <= sampler.inputs[i + 1])) {
                    std::cout<<sampler.interpolation<<std::endl;
                    float interpolation = (animation.currentTime - sampler.inputs[i]) /
                                          (sampler.inputs[i + 1] - animation.currentTime);
                    const auto &output1=sampler.outputsVec4[i];
                    const auto &output2 =sampler.outputsVec4[i+1];
                    if(channel.targetProperty=="translation"){
                        channel.node->translation=glm::mix(output1,output2,interpolation);
                    }else if(channel.targetProperty=="rotation"){
                        glm::quat quat1(output1.w,output1.x,output1.y,output1.z);
                        glm::quat quat2(output2.w,output2.x,output2.y,output2.z);
                        channel.node->rotation=glm::normalize(glm::slerp(quat1,quat2,interpolation));
                    }else if(channel.targetProperty=="scale"){
                        channel.node->scale=glm::mix(sampler.outputsVec4[i],sampler.outputsVec4[i+1],interpolation);
                    }
                }
            }

        }
    }

    glm::mat4 GLTFModel::Node::getLocalMatrix() {
        return glm::mat4(1.0f);
    }
}