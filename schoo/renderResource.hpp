#include<iostream>
#include"buffer.hpp"
#include"texture.hpp"

namespace schoo{
    struct renderResource {
        std::shared_ptr<Buffer>vertexBuffer;
        std::shared_ptr<uint32_t >indexBuffer;
        std::shared_ptr<Texture>texture;
    };
}