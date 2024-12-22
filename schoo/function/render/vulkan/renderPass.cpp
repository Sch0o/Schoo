#include"renderPass.hpp"
namespace schoo{
    void RenderPass::draw() {

    }

    vk::ImageView RenderPass::FrameBuffer::getImageView(int index) {
        return attachments[index].imageView;
    }
}

