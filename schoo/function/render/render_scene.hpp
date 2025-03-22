#pragma once
#include<glm/glm.hpp>
#include <vector>
#include <string>

struct RenderEntity{
    uint32_t instance_id{0};
    uint32_t mesh_asset_id{0};
    glm::mat4 model_matrix;

};

class RenderScene{
public:
    void Init();
    RenderScene();
    void AddObject();
    void DeleteObject();
private:
    ~RenderScene();
    struct Node {
        Node* parent;
        std::string name;
        std::vector<Node*> children;
        glm::mat4 modelMatrix;
        ~Node() {
            for (auto& child : children) {
                delete child;
            }
        }
    };
    std::vector<Node*>nodes;
};
