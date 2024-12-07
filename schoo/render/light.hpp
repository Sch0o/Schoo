#pragma once
#include"external/glm/glm.hpp"
namespace schoo{

    struct AmbientLight{
        glm::vec3 irradiance;
    };
    struct DirectionalLight{
        glm::vec3 direction;
        glm::vec3 color;
    };
    struct LightResource {
        AmbientLight ambientLight;
        DirectionalLight directionalLight;
    };
}